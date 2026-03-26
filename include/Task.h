#pragma once

#include "TaskStatus.h"
#include "Macros.h"
#include <atomic>
#include <chrono>

namespace jobscheduler {

/// @brief The Task class represent a unit of work that can be scheduled and
/// executed by job scheduler. It contains information about the task's
/// priority, scheduled time, and status.
class Task {
public:
  explicit Task(int priority,
                std::chrono::system_clock::time_point scheduledTime =
                    std::chrono::system_clock::now())
      : priority_(priority), scheduledTime_(scheduledTime),
        status_(TaskStatus::Pending) {}

  DELETE_COPY_AND_MOVE(Task);
  virtual ~Task() = default;

  /// @brief Invoke the task's execution. This operator allows the task to be
  /// called like a function, which will invoke the execute() method.
  /// @return true if the task executed successfully, false otherwise.
  bool operator()() {
    return execute();
  }

  /// @brief Get the priority of the task.
  /// @return The priority of the task.
  /// @note This method is thread-safe.
  int getPriority() const noexcept { return priority_; }

  /// @brief Get the current status of the task.
  /// @return The current status of the task.
  /// @note This method is thread-safe.
  TaskStatus getStatus() const noexcept {
    return status_.load(std::memory_order_acquire);
  }

  /// @brief Get the scheduled time of the task.
  /// @return The scheduled time of the task.
  /// @note This method is thread-safe.
  auto getScheduledTime() const noexcept { return scheduledTime_;}

  /// @brief Set the status of the task.
  /// @param newStatus The new status to set for the task.
  /// @note This method is thread-safe.
  void setStatus(TaskStatus newStatus) noexcept {
    status_.store(newStatus, std::memory_order_release);
  }

  /// @brief Execute the task. This method should be overridden by derived
  /// classes to define the actual work to be done by the task.
  /// @return true if the task executed successfully, false otherwise.
  virtual bool execute() = 0;

protected:
  const int priority_;
  const std::chrono::system_clock::time_point scheduledTime_;
  std::atomic<TaskStatus> status_;
};

} // namespace jobscheduler
