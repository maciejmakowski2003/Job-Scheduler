#pragma once

#include "ThreadPool.h"
#include "Task.h"
#include "TaskStatus.h"

#include <memory>
#include <sstream>
#include <string>
#include <vector>

struct TaskEntry {
  int id;
  std::string type;
  std::string description;
  std::shared_ptr<jobscheduler::Task> task;
};

class CLI {
public:
  explicit CLI(std::size_t numThreads = 4);

  void run();

private:
  jobscheduler::ThreadPool pool_;
  std::size_t numThreads_;
  std::vector<TaskEntry> tasks_;
  int nextId_ = 1;

  static std::string_view statusToString(jobscheduler::TaskStatus status);
  static void printHelp();
  void printStatus();
  void scheduleTask(const std::string &type, std::istringstream &args);
};
