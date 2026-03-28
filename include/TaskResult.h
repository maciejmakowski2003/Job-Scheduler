#pragma once

#include <string>

namespace jobscheduler {

/// @brief Defines the possible outcomes of a task execution attempt.
enum class ExecutionStatus { Success, Failure, Reschedule };

struct TaskResult {
  ExecutionStatus status;
  std::string message = "";
};

} // namespace jobscheduler