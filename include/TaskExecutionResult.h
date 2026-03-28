#pragma once

namespace jobscheduler {

/// @brief Defines the possible outcomes of a task execution attempt.
enum class TaskExecutionResult { Success, Failure, Retry, Reschedule };

} // namespace jobscheduler