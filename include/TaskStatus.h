#pragma once

namespace jobscheduler {

/// @brief Enum representing the status of a task.
/// @details Status of a task can be one of the following:
/// - Pending: The task is waiting to be executed.
/// - Running: The task is currently being executed.
/// - Succeeded: The task has completed successfully.
/// - Failed: The task has completed with a failure.
/// - Stopped: The task has been stopped before completion.
enum class TaskStatus {
  Pending,
  Running,
  Succeeded,
  Failed,
  Stopped
};

} // namespace jobscheduler