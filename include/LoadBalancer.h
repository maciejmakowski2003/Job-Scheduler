#pragma once

#include "MpscChannel.hpp"
#include "Event.h"

namespace jobscheduler {

class LoadBalancer {
public:
  void run(MpscChannel<Event> &inputChannel,
           std::vector<std::unique_ptr<MpscChannel<Event>>> &workerChannels);
};

} // namespace jobscheduler
