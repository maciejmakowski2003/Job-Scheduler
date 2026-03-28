#include "ThreadPool.h"

namespace jobscheduler {

ThreadPool::ThreadPool(size_t numThreads)
    : loadBalancerChannel_(std::make_unique<MpscChannel<Event>>()),
      loadBalancer_(std::make_unique<LoadBalancer>()) {
  for (size_t i = 0; i < numThreads; ++i) {
    workerChannels_.emplace_back(std::make_unique<MpscChannel<Event>>());
    workers_.emplace_back(&ThreadPool::workerFunction, this, std::ref(*workerChannels_.back()), std::ref(*loadBalancerChannel_));
  }

  loadBalancerThread_ =
      std::thread(&LoadBalancer::run, loadBalancer_.get(),
                  std::ref(*loadBalancerChannel_), std::ref(workerChannels_));
}

ThreadPool::~ThreadPool() {
  loadBalancerChannel_->send(StopEvent{});
  if (loadBalancerThread_.joinable()) {
    loadBalancerThread_.join();
  }

  for (auto &worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

void ThreadPool::schedule(const std::shared_ptr<Task> &task) {
  loadBalancerChannel_->send(task);
}

void ThreadPool::workerFunction(MpscChannel<Event> &channel, MpscChannel<Event> &retryChannel) {
  while (true) {
    auto event = channel.receive();

    if (!event || std::holds_alternative<StopEvent>(*event)) [[unlikely]] {
      break;
    }

    auto task = std::get<std::shared_ptr<Task>>(*event);

    auto result = (*task)();
    if (result == Task::ExecutionResult::Retry ||
        result == Task::ExecutionResult::Reschedule) {
      retryChannel.send(task);
    }
  }
}

} // jobscheduler