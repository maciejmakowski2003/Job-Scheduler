#pragma once

#include "MpscChannel.hpp"
#include "Event.h"
#include <queue>
#include <memory>

namespace jobscheduler {

class LoadBalancer {
public:
  LoadBalancer();

  LoadBalancer(const LoadBalancer &) = delete;
  LoadBalancer &operator=(const LoadBalancer &) = delete;
  LoadBalancer(LoadBalancer &&) = delete;
  LoadBalancer &operator=(LoadBalancer &&) = delete;
  ~LoadBalancer() = default;

  void run(MpscChannel<Event> &inputChannel,
           std::vector<std::unique_ptr<MpscChannel<Event>>> &workerChannels);

private:
  struct PriorityComparator {
    bool operator()(const std::shared_ptr<Task> &a, const std::shared_ptr<Task> &b) const {
      return a->getPriority() < b->getPriority();
    }
  };

  struct TimeComparator {
    bool operator()(const std::shared_ptr<Task> &a, const std::shared_ptr<Task> &b) const {
      return a->getScheduledTime() > b->getScheduledTime();
    }
  };

  std::priority_queue<std::shared_ptr<Task>, std::vector<std::shared_ptr<Task>>,
                      PriorityComparator> readyQueue_;

  std::priority_queue < std::shared_ptr<Task>,
      std::vector<std::shared_ptr<Task>>, TimeComparator> scheduledQueue_;
};

} // namespace jobscheduler
