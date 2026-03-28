#pragma once

#include "Event.h"
#include "Macros.h"
#include "MpscChannel.hpp"
#include "TaskExecutionResult.h"
#include "TaskStatus.h"
#include <fstream>
#include <string>
#include <string_view>
#include <thread>

namespace jobscheduler {

class AsyncLogger {
public:
  explicit AsyncLogger(const std::string &logFilePath);
  ~AsyncLogger();
  DELETE_COPY_AND_MOVE(AsyncLogger);

  /// @brief Logs a status change for a task, including the old and new status.
  /// @param taskName The name of the task whose status is changing.
  /// @param oldStatus The previous status of the task.
  /// @param newStatus The new status of the task.
  /// @note Thread-safe and non-blocking.
  void logStatusChange(std::string_view taskName, TaskStatus oldStatus,
                       TaskStatus newStatus);

  /// @brief Logs the result of a task execution attempt.
  /// @param taskName The name of the task that was executed.
  /// @param result The result of the execution attempt.
  /// @param details Optional additional details about the execution result.
  /// @note Thread-safe and non-blocking.
  void logResult(std::string_view taskName, TaskExecutionResult result,
                 std::string_view details = {});

private:
  MpscChannel<LogEvent> channel_;
  std::ofstream logFile_;
  std::thread workerThread_;
};

} // namespace jobscheduler