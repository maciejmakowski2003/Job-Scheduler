#pragma once

#include "Event.h"
#include "LoadBalancer.h"
#include "MpscChannel.hpp"
#include "Task.h"
#include <memory>
#include <thread>
#include <vector>

namespace jobscheduler {

class ThreadPool {
public:
  explicit ThreadPool(size_t numThreads);
  ~ThreadPool();

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;

  ThreadPool(ThreadPool &&) = delete;
  ThreadPool &operator=(ThreadPool &&) = delete;

  void schedule(const std::shared_ptr<Task> &task);

private:
  std::vector<std::thread> workers_;
  std::vector<std::unique_ptr<MpscChannel<Event>>> workerChannels_;

  std::unique_ptr<LoadBalancer> loadBalancer_;
  std::thread loadBalancerThread_;
  std::unique_ptr<MpscChannel<Event>> loadBalancerChannel_;

  void workerFunction(MpscChannel<Event> &channel);
};

} // namespace jobscheduler
