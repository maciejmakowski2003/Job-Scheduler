#include "ComputeTask.h"
#include <format>
#include <thread>

ComputeTask::ComputeTask(
    int id, int priority,
    std::chrono::milliseconds duration,
    std::chrono::system_clock::time_point scheduledTime)
    : Task(std::format("ComputeTask#{}", id), scheduledTime, priority, 2),
      duration_(duration) {}

jobscheduler::TaskResult ComputeTask::execute() {
  std::this_thread::sleep_for(duration_);
  return {jobscheduler::ExecutionStatus::Success};
}
