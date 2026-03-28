#pragma once

#include "Task.hpp"
#include <variant>

namespace jobscheduler {

struct StopEvent {};

struct LogEntryEvent {
  std::string taskName;
  std::string message;
  std::chrono::system_clock::time_point timestamp;
};

/// @brief Event type that are used to communicate between the main thread and worker threads.
using TaskEvent = std::variant<std::shared_ptr<Task>, StopEvent>;

/// @brief Event type that are used to communicate log entries between workers and the
/// logger thread.
using LogEvent = std::variant<LogEntryEvent, StopEvent>;

} // namespace jobscheduler