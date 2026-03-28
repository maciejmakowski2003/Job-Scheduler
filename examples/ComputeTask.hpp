#pragma once

#include "Task.hpp"
#include <chrono>
#include <thread>

class ComputeTask : public jobscheduler::Task {
public:
  explicit ComputeTask(int priority,
                       std::chrono::milliseconds duration = std::chrono::milliseconds{5000},
                       std::chrono::system_clock::time_point scheduledTime = std::chrono::system_clock::now())
      : Task(scheduledTime, priority, 2), duration_(duration) {}

  bool execute() override {
    std::this_thread::sleep_for(duration_);
    return true;
  }

private:
  std::chrono::milliseconds duration_;
};
