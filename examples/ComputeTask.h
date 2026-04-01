#pragma once

#include "Task.h"
#include <chrono>

class ComputeTask : public jobscheduler::Task {
public:
  explicit ComputeTask(
      int id, int priority,
      std::chrono::milliseconds duration = std::chrono::milliseconds{5000},
      std::chrono::system_clock::time_point scheduledTime =
          std::chrono::system_clock::now());

protected:
  jobscheduler::TaskResult execute() final;

private:
  std::chrono::milliseconds duration_;
};
