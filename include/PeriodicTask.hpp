#pragma once

#include "Task.hpp"
#include <optional>
#include <chrono>
#include <limits>

namespace jobscheduler {

/// @brief A task that automatically reschedules itself after every execution,
/// whether it succeeds or fails.
class PeriodicTask : public Task {
public:
  explicit PeriodicTask(
      std::string name,
      milliseconds interval,
      time_point scheduledTime = std::chrono::system_clock::now(),
      int priority = 0)
      : Task(std::move(name), scheduledTime, priority,
             std::numeric_limits<int>::max(), interval),
        interval_(interval) {}

protected:
  std::optional<time_point> nextSchedule() override {
    return std::chrono::system_clock::now() + interval_;
  }

private:
  const milliseconds interval_;
};

} // namespace jobscheduler
