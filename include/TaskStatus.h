#pragma once

namespace jobscheduler {

enum class TaskStatus {
  Pending,
  Running,
  Succeeded,
  Failed,
  Stopped
};

} // namespace jobscheduler