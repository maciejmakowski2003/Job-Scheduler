#pragma once

#include "Task.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

namespace jobscheduler {

class FileTask : public Task {
public:
  explicit FileTask(int id, std::filesystem::path path, int priority = 0,
                    std::chrono::system_clock::time_point scheduledTime = std::chrono::system_clock::now())
      : Task(std::format("FileTask#{}", id), scheduledTime, priority),
        path_(std::move(path)) {}

protected:
  bool execute() final {
    if (!std::filesystem::exists(path_)) {
      return false;
    }

    std::ifstream file(path_);
    if (!file.is_open()) {
      return false;
    }

    std::size_t lines = 0;
    std::size_t words = 0;
    std::string line;
    while (std::getline(file, line)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
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
