#include "PeriodicTask.h"
#include <limits>

namespace jobscheduler {

PeriodicTask::PeriodicTask(
    std::string name,
    milliseconds interval,
    time_point scheduledTime,
    int priority)
    : Task(std::move(name), scheduledTime, priority,
           std::numeric_limits<int>::max(), interval),
      interval_(interval) {}

std::optional<time_point> PeriodicTask::nextSchedule() {
  return std::chrono::system_clock::now() + interval_;
}

} // namespace jobscheduler
