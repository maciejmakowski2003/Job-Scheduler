#pragma once

#include "Task.h"
#include <memory>

namespace jobscheduler {

/// @brief Event type that are used to communicate between the main thread and worker threads.
using TaskEvent = std::shared_ptr<Task>;

/// @brief Event type that are used to communicate log entries between workers and the
/// logger thread.
struct LogEvent {
  std::string taskName;
  std::string message;
  std::chrono::system_clock::time_point timestamp;
};

} // namespace jobscheduler