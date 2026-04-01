#pragma once

#include "Task.h"
#include <chrono>
#include <optional>

namespace jobscheduler {

/// @brief A task that automatically reschedules itself after every execution,
/// whether it succeeds or fails.
class PeriodicTask : public Task {
public:
  explicit PeriodicTask(
      std::string name,
      milliseconds interval,
      time_point scheduledTime = std::chrono::system_clock::now(),
      int priority = 0);

protected:
  std::optional<time_point> nextSchedule() override;

private:
  const milliseconds interval_;
};

} // namespace jobscheduler
