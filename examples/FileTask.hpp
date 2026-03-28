#pragma once

#include "Task.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

class FileTask : public jobscheduler::Task {
public:
  explicit FileTask(int id, std::filesystem::path path, int priority = 0,
                    std::chrono::system_clock::time_point scheduledTime =
                        std::chrono::system_clock::now())
      : Task(std::format("FileTask#{}", id), scheduledTime, priority),
        path_(std::move(path)) {}

protected:
  jobscheduler::TaskResult execute() final {
    if (!std::filesystem::exists(path_)) {
      return {jobscheduler::ExecutionStatus::Failure, "File does not exist"};
    }

    std::ifstream file(path_);
    if (!file.is_open()) {
      return {jobscheduler::ExecutionStatus::Failure, "Failed to open file"};
    }

    std::size_t lines = 0;
    std::size_t words = 0;
    std::string line;
    while (std::getline(file, line)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      ++lines;
      std::istringstream iss(line);
      std::string word;
      while (iss >> word) {
        ++words;
      }
    }

    const auto size = std::filesystem::file_size(path_);
    return {jobscheduler::ExecutionStatus::Success,
            std::format("File: {}, Size: {} bytes, Lines: {}, Words: {}",
                        path_.filename().string(), size, lines, words)};
  }

private:
  std::filesystem::path path_;
};
