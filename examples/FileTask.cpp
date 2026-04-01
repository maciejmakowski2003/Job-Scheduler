#include "FileTask.h"
#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

FileTask::FileTask(int id, std::filesystem::path path, int priority,
                   std::chrono::system_clock::time_point scheduledTime)
    : Task(std::format("FileTask#{}", id), scheduledTime, priority),
      path_(std::move(path)) {}

jobscheduler::TaskResult FileTask::execute() {
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
