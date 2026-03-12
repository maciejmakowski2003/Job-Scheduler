#pragma once

#include "Task.h"
#include <variant>

namespace jobscheduler {

struct StopEvent {};
using Event = std::variant<std::shared_ptr<Task>, StopEvent>;

} // namespace jobscheduler