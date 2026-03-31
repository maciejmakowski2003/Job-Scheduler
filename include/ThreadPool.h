#pragma once

#include "AsyncLogger.h"
#include "Event.h"
#include "LoadBalancer.h"
#include "MpscChannel.hpp"
#include "Task.h"
#include "Macros.h"
#include <atomic>
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

  /// @brief Stops the thread pool and all worker threads gracefully.
  /// Blocking call that waits for all threads to finish their current tasks before exiting.
  void stop();

private:
  std::shared_ptr<AsyncLogger> logger_;
  std::unique_ptr<MpscChannel<TaskEvent>> loadBalancerChannel_;
  std::unique_ptr<LoadBalancer> loadBalancer_;

  std::vector<std::unique_ptr<MpscChannel<TaskEvent>>> workerChannels_;
  std::vector<std::thread> workers_;
  std::thread loadBalancerThread_;

  std::atomic<bool> stopped_{false};

  /// @brief The worker function that each worker thread runs. It continuously
  /// listens for events on its channel and executes tasks accordingly.
  /// @param channel The channel through which the worker receives events.
  /// @param retryChannel The channel to re-queue tasks that need to be retried.
  void workerFunction(MpscChannel<TaskEvent> &channel,
                      MpscChannel<TaskEvent> &retryChannel);
};

} // namespace jobscheduler
