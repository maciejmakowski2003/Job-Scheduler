#pragma once

#include "Task.h"
#include <chrono>
#include <format>
#include <thread>

class ComputeTask : public jobscheduler::Task {
public:
  explicit ComputeTask(
      int id, int priority,
      std::chrono::milliseconds duration = std::chrono::milliseconds{5000},
      std::chrono::system_clock::time_point scheduledTime =
          std::chrono::system_clock::now())
      : Task(std::format("ComputeTask#{}", id), scheduledTime, priority, 2),
        duration_(duration) {}

protected:
  bool execute() final {
    std::this_thread::sleep_for(duration_);
    return true;
  }

private:
  std::chrono::milliseconds duration_;
};
