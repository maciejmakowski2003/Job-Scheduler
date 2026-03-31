#include "AsyncLogger.h"
#include <format>
#include <stdexcept>
#include <string_view>

namespace jobscheduler {

std::string_view taskStatusToString(TaskStatus status) {
  switch (status) {
  case TaskStatus::Pending:
    return "Pending";
  case TaskStatus::Running:
    return "Running";
  case TaskStatus::Succeeded:
    return "Succeeded";
  case TaskStatus::Failed:
    return "Failed";
  case TaskStatus::Stopped:
    return "Stopped";
  }
  return "Unknown";
}

std::string_view taskResultToString(ExecutionStatus result) {
  switch (result) {
  case ExecutionStatus::Success:
    return "Success";
  case ExecutionStatus::Failure:
    return "Failure";
  case ExecutionStatus::Reschedule:
    return "Reschedule";
  }
  return "Unknown";
}

AsyncLogger::AsyncLogger(const std::string &logFilePath)
    : logFile_(logFilePath, std::ios::app) {
  if (!logFile_.is_open()) {
    throw std::runtime_error("AsyncLogger: failed to open log file: " +
                             logFilePath);
  }

  workerThread_ = std::thread([this] {
    while (true) {
      auto event = channel_.receive();
      if (!event) [[unlikely]] {
        break;
      }

      logFile_ << std::format("[{:%Y-%m-%dT%H:%M:%SZ}] {}: {}\n",
                              event->timestamp, event->taskName, event->message);
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
  channel_.send(LogEvent{std::string(taskName),
                              std::format("status changed from {} to {}",
                                          taskStatusToString(oldStatus),
                                          taskStatusToString(newStatus)),
                              std::chrono::system_clock::now()});
}

void AsyncLogger::logResult(std::string_view taskName, TaskResult result) {
  std::string message =
      result.message.empty()
          ? std::format("result={}", taskResultToString(result.status))
          : std::format("result={} ({})", taskResultToString(result.status),
                        result.message);

  channel_.send(LogEvent{std::string(taskName), std::move(message),
                              std::chrono::system_clock::now()});
}

} // namespace jobscheduler
