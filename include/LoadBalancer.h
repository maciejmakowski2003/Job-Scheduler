#pragma once

#include "MpscChannel.hpp"
#include "Macros.h"
#include "Event.h"
#include <queue>
#include <memory>

namespace jobscheduler {

class LoadBalancer {
public:
  LoadBalancer() = default;
  ~LoadBalancer() = default;
  DELETE_COPY_AND_MOVE(LoadBalancer);

  /// @brief Main loop of the load balancer.
  /// @details It continously execute 4 steps:
  /// 1. Receive incoming events from the main thread and add them to the
  /// scheduled queue.
  /// 2. Look up the schedule queue and move all tasks should be executed to
  /// ready qeueu.
  /// 3. Dispatch all tasks in the ready queue to worker threads in a
  /// round-robin manner.
  /// 4. Sleep until the next task is scheduled to be executed or a new event
  /// is received.
  /// @param inputChannel The channel to receive events from the main thread.
  /// @param workerChannels The channels to send tasks to worker threads.
  void run(MpscChannel<TaskEvent> &inputChannel,
           std::vector<std::unique_ptr<MpscChannel<TaskEvent>>> &workerChannels);

private:
  struct PriorityComparator {
    bool operator()(const std::shared_ptr<Task> &a, const std::shared_ptr<Task> &b) const noexcept {
      return a->getPriority() < b->getPriority();
    }
  };

  struct TimeComparator {
    bool operator()(const std::shared_ptr<Task> &a, const std::shared_ptr<Task> &b) const noexcept {
      return a->getScheduledTime() > b->getScheduledTime();
    }
  };

  std::priority_queue<std::shared_ptr<Task>, std::vector<std::shared_ptr<Task>>,
                      PriorityComparator> readyQueue_;

  std::priority_queue<std::shared_ptr<Task>,
                      std::vector<std::shared_ptr<Task>>, TimeComparator> scheduledQueue_;
};

} // namespace jobscheduler
