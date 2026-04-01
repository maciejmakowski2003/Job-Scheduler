#include "ThreadPool.h"

namespace jobscheduler {

ThreadPool::ThreadPool(size_t numThreads)
    : logger_(std::make_shared<AsyncLogger>()),
      loadBalancerChannel_(std::make_unique<MpscChannel<TaskEvent>>()),
      loadBalancer_(std::make_unique<LoadBalancer>()) {
  for (size_t i = 0; i < numThreads; ++i) {
    workerChannels_.emplace_back(std::make_unique<MpscChannel<TaskEvent>>());
    workers_.emplace_back(&ThreadPool::workerFunction, this,
                          std::ref(*workerChannels_.back()),
                          std::ref(*loadBalancerChannel_));
  }

  loadBalancerThread_ =
      std::thread(&LoadBalancer::run, loadBalancer_.get(),
                  std::ref(*loadBalancerChannel_), std::ref(workerChannels_));
}

ThreadPool::~ThreadPool() {
  stop();
}

void ThreadPool::schedule(const std::shared_ptr<Task> &task) {
  if (stopped_.load(std::memory_order_acquire)) {
    throw std::runtime_error("ThreadPool is stopped. Cannot schedule new tasks.");
  }
  
  task->onStatusChange([logger = logger_, name = std::string(task->getName())](
                           TaskStatus from, TaskStatus to) {
    logger->logStatusChange(name, from, to);
  });

  loadBalancerChannel_->send(task);
}

void ThreadPool::stop() {
  if (stopped_.exchange(true, std::memory_order_acq_rel)) {
    return;
  }

  loadBalancerChannel_->stop();
  if (loadBalancerThread_.joinable()) {
    loadBalancerThread_.join();
  }

  for (auto &worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }

  // drain tasks re-queued by worker after load balancer stops
  while (auto task = loadBalancerChannel_->try_receive()) {
    (*task)->setStatus(TaskStatus::Stopped);
  }
}

void ThreadPool::workerFunction(MpscChannel<TaskEvent> &channel,
                                MpscChannel<TaskEvent> &retryChannel) {
  while (true) {
    auto event = channel.receive();

    if (!event) [[unlikely]] {
      while (auto remainingEvent = channel.try_receive()) {
        (*remainingEvent)->setStatus(TaskStatus::Stopped);
      }
      break;
    }

    auto task = *event;

    auto result = (*task)();
    logger_->logResult(task->getName(), result);

    if (result.status == ExecutionStatus::Reschedule) {
      retryChannel.send(task);
    }
  }
}

} // namespace jobscheduler