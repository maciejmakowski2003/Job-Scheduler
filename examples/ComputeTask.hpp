#pragma once

#include "Task.h"
#include <chrono>
#include <iostream>
#include <thread>

class ComputeTask : public jobscheduler::Task {
public:
  explicit ComputeTask(int priority,
                       std::chrono::milliseconds duration = std::chrono::milliseconds{100})
      : Task(priority), duration_(duration) {}

  bool execute() override {
    std::this_thread::sleep_for(duration_);
    return true;
  }

private:
  std::chrono::milliseconds duration_;
};
