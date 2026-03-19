#include "LoadBalancer.h"
#include <thread>

namespace jobscheduler {

void LoadBalancer::run(MpscChannel<Event> &inputChannel,
                        std::vector<std::unique_ptr<MpscChannel<Event>>> &workerChannels) {
  size_t workerIndex = 0;

  while (true) {
    if (auto event = inputChannel.try_receive()) {
      if (std::holds_alternative<StopEvent>(*event)) [[unlikely]] {
        for (size_t i = 0; i < workerChannels.size(); ++i) {
          workerChannels[i]->send(StopEvent{});
        }

        break;
      } else {
        auto task = std::get<std::shared_ptr<Task>>(*event);
        workerChannels[workerIndex]->send(task);
        workerIndex = (workerIndex + 1) % workerChannels.size();
      }
    }

    std::this_thread::sleep_for(std::chrono::microseconds(500));
  }
}

} // namespace jobscheduler