#include "AsyncLogger.h"
#include <format>
#include <string_view>

namespace jobscheduler {

std::string_view taskStatusToString(TaskStatus status) {
  switch (status) {
  case TaskStatus::Pending:   return "Pending";
  case TaskStatus::Running:   return "Running";
  case TaskStatus::Succeeded: return "Succeeded";
  case TaskStatus::Failed:    return "Failed";
  case TaskStatus::Stopped:   return "Stopped";
  }
  return "Unknown";
}

std::string_view taskResultToString(TaskExecutionResult result) {
  switch (result) {
  case TaskExecutionResult::Success:    return "Success";
  case TaskExecutionResult::Failure:    return "Failure";
  case TaskExecutionResult::Retry:      return "Retry";
  case TaskExecutionResult::Reschedule: return "Reschedule";
  }
  return "Unknown";
}

AsyncLogger::AsyncLogger(const std::string &logFilePath)
    : logFile_(logFilePath, std::ios::app) {
  workerThread_ = std::thread([this] {
    while (true) {
      auto event = channel_.receive();
      if (!event.has_value()) {
        break;
      }

      if (std::holds_alternative<StopEvent>(*event)) [[unlikely]] {
        break;
      }
      
      auto &entry = std::get<LogEntryEvent>(*event);
      logFile_ << std::format("[{:%Y-%m-%dT%H:%M:%SZ}] {}: {}\n",
                              entry.timestamp, entry.taskName, entry.message);
      logFile_.flush();
    }
  });
}

AsyncLogger::~AsyncLogger() {
  channel_.stop();
  if (workerThread_.joinable()) {
    workerThread_.join();
  }
}

void AsyncLogger::logStatusChange(std::string_view taskName,
                                  TaskStatus oldStatus, TaskStatus newStatus) {
  channel_.send(LogEntryEvent{
      std::string(taskName),
      std::format("status changed from {} to {}", taskStatusToString(oldStatus),
                  taskStatusToString(newStatus)),
      std::chrono::system_clock::now()});
}

void AsyncLogger::logResult(std::string_view taskName,
                            TaskExecutionResult result,
                            std::string_view details) {
  std::string message =
      details.empty()
          ? std::format("result={}", taskResultToString(result))
          : std::format("result={} ({})", taskResultToString(result), details);
  
  channel_.send(LogEntryEvent{std::string(taskName), std::move(message),
                              std::chrono::system_clock::now()});
}

} // namespace jobscheduler
