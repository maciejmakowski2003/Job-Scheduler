#pragma once

#include "Event.h"
#include "LoadBalancer.h"
#include "MpscChannel.hpp"
#include "Task.h"
#include "Macros.h"
#include <memory>
#include <thread>
#include <vector>

namespace jobscheduler {

/// @brief A thread pool that manages a fixed number of worker threads to
/// execute tasks concurrently. It uses load balancer to distribute tasks among
/// workers and ensures efficient task scheduling.
class ThreadPool {
public:
  explicit ThreadPool(size_t numThreads);
  DELETE_COPY_AND_MOVE(ThreadPool);
  ~ThreadPool();

  /// @brief Schedules a task for execution by the thread pool.
  /// @param task A shared pointer to the task to be executed.
  void schedule(const std::shared_ptr<Task> &task);

private:
  std::vector<std::thread> workers_;
  std::vector<std::unique_ptr<MpscChannel<Event>>> workerChannels_;

  std::unique_ptr<LoadBalancer> loadBalancer_;
  std::thread loadBalancerThread_;
  std::unique_ptr<MpscChannel<Event>> loadBalancerChannel_;

  /// @brief The worker function that each worker thread runs. It continuously
  /// listens for events on its channel and executes tasks accordingly.
  /// @param channel The channel through which the worker receives events.
  void workerFunction(MpscChannel<Event> &channel);
};

} // namespace jobscheduler
