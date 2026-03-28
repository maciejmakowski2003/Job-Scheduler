#include "Task.h"

namespace jobscheduler {

Task::Task(std::string name, time_point scheduledTime, int priority,
           int retryCount, milliseconds retryTimeout)
    : name_(std::move(name)), scheduledTime_(scheduledTime),
      retryTimeout_(retryTimeout), priority_(priority),
      retryCount_(retryCount), status_(TaskStatus::Pending) {}

TaskExecutionResult Task::operator()() {
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

std::string_view Task::getName() const noexcept { return name_; }

int Task::getPriority() const noexcept { return priority_; }

TaskStatus Task::getStatus() const noexcept {
  return status_.load(std::memory_order_acquire);
}

time_point Task::getScheduledTime() const noexcept { return scheduledTime_; }

void Task::setStatus(TaskStatus newStatus) noexcept {
  const auto oldStatus = status_.exchange(newStatus, std::memory_order_acq_rel);
  if (onStatusChange_) {
    onStatusChange_(oldStatus, newStatus);
  }
}

void Task::onStatusChange(std::function<void(TaskStatus, TaskStatus)> callback) {
  onStatusChange_ = std::move(callback);
}

std::optional<time_point> Task::nextSchedule() { return std::nullopt; }

} // namespace jobscheduler
