#pragma once

#include "TaskStatus.h"
#include "TaskExecutionResult.h"
#include "Macros.h"
#include <atomic>
#include <chrono>
#include <optional>
#include <string>
#include <string_view>

namespace jobscheduler {

using time_point = std::chrono::system_clock::time_point;
using milliseconds = std::chrono::milliseconds;

/// @brief The Task class represent a unit of work that can be scheduled and
/// executed by job scheduler. It contains information about the task's
/// priority, scheduled time, and status.
class Task {
public:
  explicit Task(std::string name,
                time_point scheduledTime = std::chrono::system_clock::now(),
                int priority = 0, int retryCount = 0,
                milliseconds retryTimeout = milliseconds(500))
      : name_(std::move(name)), scheduledTime_(scheduledTime),
        retryTimeout_(retryTimeout), priority_(priority),
        retryCount_(retryCount), status_(TaskStatus::Pending) {}

  virtual ~Task() = default;
  DELETE_COPY_AND_MOVE(Task);

  /// @brief Executes the task and manages its lifecycle state.
  /// Calls execute(), updates status, and on failure decrements the retry
  /// counter and reschedules the next attempt by retryTimeout_.
  /// @return The result of the execution attempt, indicating success, failure, or retry.
  /// @note Must not be called concurrently with getScheduledTime().
  TaskExecutionResult operator()() {
    setStatus(TaskStatus::Running);

    if (execute()) {
      if (auto next = nextSchedule()) {
        scheduledTime_ = *next;
        setStatus(TaskStatus::Pending);
        return TaskExecutionResult::Reschedule;
      }
      setStatus(TaskStatus::Succeeded);
      return TaskExecutionResult::Success;
    }

    --retryCount_;

    if (retryCount_ >= 0) {
      setStatus(TaskStatus::Pending);
      scheduledTime_ = std::chrono::system_clock::now() + retryTimeout_;
      return TaskExecutionResult::Retry;
    }

    setStatus(TaskStatus::Failed);
    return TaskExecutionResult::Failure;
  }

  /// @brief Get the name of the task.
  /// @return The name of the task.
  std::string_view getName() const noexcept { return name_; }

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
  /// @return true if the work completed successfully, false if it failed.
  /// @note Not thread-safe. Called exclusively from operator().
  virtual bool execute() = 0;

  /// @brief Override to reschedule the task after a successful execution.
  /// @return The next time point to run, or std::nullopt to mark as Succeeded.
  /// @note Not thread-safe. Called exclusively from operator() after execute() returns true.
  virtual std::optional<time_point> nextSchedule() { return std::nullopt; }

private:
  const std::string name_;
  time_point scheduledTime_;
  const milliseconds retryTimeout_;
  const int priority_;
  int retryCount_;
  std::atomic<TaskStatus> status_;
};

} // namespace jobscheduler
