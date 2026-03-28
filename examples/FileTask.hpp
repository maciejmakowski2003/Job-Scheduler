#pragma once

#include "Task.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace jobscheduler {

class FileTask : public Task {
public:
  explicit FileTask(std::filesystem::path path, int priority = 0,
                    std::chrono::system_clock::time_point scheduledTime = std::chrono::system_clock::now())
      : Task(scheduledTime, priority), path_(std::move(path)) {}

protected:
  bool execute() final {
    if (!std::filesystem::exists(path_)) {
      // file does not exist
      return false;
    }

    std::ifstream file(path_);
    if (!file.is_open()) {
      // connot open file for reading (permissions, etc.)
      return false;
    }

    std::size_t lines = 0;
    std::size_t words = 0;
    std::string line;
    while (std::getline(file, line)) {
      ++lines;
      std::istringstream iss(line);
      std::string word;
      while (iss >> word) {
        ++words;
      }
    }

    const auto size = std::filesystem::file_size(path_);
    // TODO add task result reporting mechanism
    return true;
  }

private:
  std::filesystem::path path_;
};

} // namespace jobscheduler
