#include "ThreadPool.h"
#include "TaskStatus.h"

namespace jobscheduler {

ThreadPool::ThreadPool(size_t numThreads) {
  for (size_t i = 0; i < numThreads; ++i) {
    workerChannels_.emplace_back(std::make_unique<MpscChannel<Event>>());
    workers_.emplace_back(&ThreadPool::workerFunction, this, std::ref(*workerChannels_.back()));
  }

  loadBalancer_ = std::make_unique<LoadBalancer>();
  loadBalancerChannel_ = std::make_unique<MpscChannel<Event>>();
  loadBalancerThread_ = std::thread(&LoadBalancer::run, loadBalancer_.get(), std::ref(*loadBalancerChannel_), std::ref(workerChannels_));
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

void ThreadPool::workerFunction(MpscChannel<Event> &channel) {
  while (true) {
    auto event = channel.receive();

    if (!event || std::holds_alternative<StopEvent>(*event)) [[unlikely]] {
      break;
    }

    auto task = std::get<std::shared_ptr<Task>>(*event);
    task->setStatus(TaskStatus::Running);

    if (task->execute()) {
      task->setStatus(TaskStatus::Succeeded);
    } else {
      task->setStatus(TaskStatus::Failed);
    }
  }
}

} // jobscheduler