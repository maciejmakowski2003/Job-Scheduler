#include "LoadBalancer.h"

namespace jobscheduler {


void LoadBalancer::run(
    MpscChannel<TaskEvent> &inputChannel,
    std::vector<std::unique_ptr<MpscChannel<TaskEvent>>> &workerChannels) {
  size_t workerIndex = 0;


  try {
    while (true) {
      while (auto event = inputChannel.try_receive()) {
        if (std::holds_alternative<StopEvent>(*event)) [[unlikely]] {
          for (size_t i = 0; i < workerChannels.size(); ++i) {
            workerChannels[i]->send(StopEvent{});
          }

          return;
        } else {
          auto task = std::get<std::shared_ptr<Task>>(*event);
          scheduledQueue_.push(std::move(task));
        }
      }

      auto now = std::chrono::system_clock::now();
      while (!scheduledQueue_.empty() &&
            scheduledQueue_.top()->getScheduledTime() <= now) {
        readyQueue_.push(scheduledQueue_.top());
        scheduledQueue_.pop();
      }

      while (!readyQueue_.empty()) {
        auto task = readyQueue_.top();
        readyQueue_.pop();

        workerChannels[workerIndex]->send(task);
        workerIndex = (workerIndex + 1) % workerChannels.size();
      }

      if (!scheduledQueue_.empty()) {
        auto nextWakeUpTime = scheduledQueue_.top()->getScheduledTime();
        inputChannel.wait_until(nextWakeUpTime);
      } else {
        inputChannel.wait();
      }
    }
  } catch (...) {
    for (auto &ch : workerChannels) {
      ch->send(StopEvent{});
    }
    throw;
  }
}

} // namespace jobscheduler