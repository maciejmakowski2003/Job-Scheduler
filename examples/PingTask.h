#pragma once

#include "PeriodicTask.h"
#include <cstdint>

class PingTask : public jobscheduler::PeriodicTask {
public:
  explicit PingTask(int id, uint16_t port, jobscheduler::milliseconds interval);
  ~PingTask() override;

protected:
  jobscheduler::TaskResult execute() override;

private:
  uint16_t port_;
  int sockfd_ = -1;
};
