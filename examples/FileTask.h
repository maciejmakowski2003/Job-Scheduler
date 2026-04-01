#pragma once

#include "Task.h"
#include <chrono>
#include <filesystem>

class FileTask : public jobscheduler::Task {
public:
  explicit FileTask(int id, std::filesystem::path path, int priority = 0,
                    std::chrono::system_clock::time_point scheduledTime =
                        std::chrono::system_clock::now());

protected:
  jobscheduler::TaskResult execute() final;

private:
  std::filesystem::path path_;
};
