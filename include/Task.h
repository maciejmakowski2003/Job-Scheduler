#pragma once

#include "TaskStatus.h"
#include <atomic>
#include <chrono>

namespace jobscheduler {

class Task {
public:
  explicit Task(int priority, std::chrono::system_clock::time_point scheduledTime = std::chrono::system_clock::now())
      : priority_(priority), scheduledTime_(scheduledTime), status_(TaskStatus::Pending) {}

  int getPriority() const noexcept { return priority_; }
  TaskStatus getStatus() const noexcept { return status_.load(std::memory_order_acquire); }
  auto getScheduledTime() const noexcept { return scheduledTime_;}

  void setStatus(TaskStatus newStatus) noexcept {
    status_.store(newStatus, std::memory_order_release);
  }

  virtual ~Task() = default;
  virtual bool execute() = 0;

protected:
  const int priority_;
  const std::chrono::system_clock::time_point scheduledTime_;
  std::atomic<TaskStatus> status_;
};

} // namespace jobscheduler
