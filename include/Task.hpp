#pragma once

#include "TaskStatus.h"
#include "Macros.h"
#include <atomic>
#include <chrono>

namespace jobscheduler {

using time_point = std::chrono::system_clock::time_point;
using milliseconds = std::chrono::milliseconds;

/// @brief The Task class represent a unit of work that can be scheduled and
/// executed by job scheduler. It contains information about the task's
/// priority, scheduled time, and status.
class Task {
public:
  /// @brief Defines the possible outcomes of a task execution attempt.
  enum class ExecutionResult { Success, Failure, Retry };

  /// @brief Constructs a Task.
  /// @param scheduledTime When the task becomes eligible for execution.
  /// @param priority Higher value means higher scheduling priority.
  /// @param retryCount Number of additional attempts allowed after the first failure. 0 means run once with no retries.
  /// @param retryTimeout How long to wait before re-queuing after a failure.
  explicit Task(time_point scheduledTime = std::chrono::system_clock::now(),
                int priority = 0, int retryCount = 0,
                milliseconds retryTimeout = milliseconds(500))
      : scheduledTime_(scheduledTime), retryTimeout_(retryTimeout),
        priority_(priority), retryCount_(retryCount),
        status_(TaskStatus::Pending) {}

  virtual ~Task() = default;
  DELETE_COPY_AND_MOVE(Task);

  /// @brief Executes the task and manages its lifecycle state.
  /// Calls execute(), updates status, and on failure decrements the retry
  /// counter and reschedules the next attempt by retryTimeout_.
  /// @return The result of the execution attempt, indicating success, failure, or retry.
  /// @note Must not be called concurrently with getScheduledTime().
  ExecutionResult operator()() {
    setStatus(TaskStatus::Running);

    if (execute()) {
      setStatus(TaskStatus::Succeeded);
      return ExecutionResult::Success;
    }

    --retryCount_;

    if (retryCount_ >= 0) {
      setStatus(TaskStatus::Pending);
      scheduledTime_ = std::chrono::system_clock::now() + retryTimeout_;
      return ExecutionResult::Retry;
    }

    setStatus(TaskStatus::Failed);
    return ExecutionResult::Failure;
  }

  /// @brief Get the priority of the task.
  /// @return The priority of the task.
  /// @note Thread-safe.
  int getPriority() const noexcept { return priority_; }

  /// @brief Get the current status of the task.
  /// @return The current status of the task.
  /// @note Thread-safe.
  TaskStatus getStatus() const noexcept {
    return status_.load(std::memory_order_acquire);
  }

  /// @brief Get the next scheduled execution time.
  /// @return The time point at or after which the task should next run.
  /// @note Not thread-safe. Do not call while the task may be executing.
  auto getScheduledTime() const noexcept { return scheduledTime_; }

  /// @brief Set the status of the task.
  /// @param newStatus The new status to set.
  /// @note Thread-safe.
  void setStatus(TaskStatus newStatus) noexcept {
    status_.store(newStatus, std::memory_order_release);
  }

protected:
  /// @brief Perform the actual work of this task.
  /// @return true if the work completed successfully, false is it failed.
  /// @note Not thread-safe. It is called exclusively from operator().
  virtual bool execute() = 0;

private:
  time_point scheduledTime_;
  const milliseconds retryTimeout_;
  const int priority_;
  int retryCount_;
  std::atomic<TaskStatus> status_;
};

} // namespace jobscheduler
