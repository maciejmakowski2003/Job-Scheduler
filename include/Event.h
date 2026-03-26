#pragma once

#include "Task.h"
#include <variant>

namespace jobscheduler {

struct StopEvent {};

/// Event type that are used to communicate between the main thread and worker threads.
/// It can be either a Task or a StopEvent.
using Event = std::variant<std::shared_ptr<Task>, StopEvent>;

} // namespace jobscheduler