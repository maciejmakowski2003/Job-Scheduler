#include "CLI.h"
#include "ComputeTask.hpp"
#include "FileTask.h"

#include <chrono>
#include <iomanip>
#include <iostream>

CLI::CLI(std::size_t numThreads) : pool_(numThreads), numThreads_(numThreads) {}

void CLI::run() {
  std::cout << "Job Scheduler CLI  [threads: " << numThreads_ << "]\n"
            << "Type 'help' for available commands.\n\n";

  std::string line;
  while (true) {
    std::cout << "> " << std::flush;
    if (!std::getline(std::cin, line)) {
      break;
    }

    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd.empty()) {
      continue;
    } else if (cmd == "stop") {
      std::cout << "Stopping scheduler...\n";
      break;
    } else if (cmd == "help") {
      printHelp();
    } else if (cmd == "status") {
      printStatus();
    } else if (cmd == "schedule") {
      std::string type;
      iss >> type;
      if (type == "file") {
        scheduleFile(iss);
      } else if (type == "compute") {
        scheduleCompute(iss);
      } else {
        std::cerr << "Unknown task type '" << type
                  << "'. Use 'file' or 'compute'.\n";
      }
    } else {
      std::cerr << "Unknown command '" << cmd << "'. Type 'help' for usage.\n";
    }
  }
}

std::string_view CLI::statusToString(jobscheduler::TaskStatus status) {
  switch (status) {
  case jobscheduler::TaskStatus::Pending:   return "Pending";
  case jobscheduler::TaskStatus::Running:   return "Running";
  case jobscheduler::TaskStatus::Succeeded: return "Succeeded";
  case jobscheduler::TaskStatus::Failed:    return "Failed";
  case jobscheduler::TaskStatus::Stopped:   return "Stopped";
  }
  return "Unknown";
}

void CLI::printHelp() {
  const int cmdWidth = 45;

  std::cout << "\nAvailable Commands:\n"
            << std::string(70, '=') << '\n'
            << std::left
            << std::setw(cmdWidth) << "Command" << "Description" << '\n'
            << std::string(70, '-') << '\n'
            << std::setw(cmdWidth) << "schedule file <path> [priority]"
            << "Schedule a file task\n"
            << std::setw(cmdWidth)
            << "schedule compute [duration_ms] [priority]"
            << "Schedule a compute task\n"
            << std::setw(cmdWidth) << "status" << "List all tasks\n"
            << std::setw(cmdWidth) << "stop" << "Stop the scheduler and exit\n"
            << std::setw(cmdWidth) << "help" << "Show this help\n"
            << std::string(70, '-') << std::endl;
}

void CLI::printStatus() {
  if (tasks_.empty()) {
    std::cout << "No tasks scheduled.\n";
    return;
  }
  std::cout << std::left
            << std::setw(6)  << "ID"
            << std::setw(12) << "Type"
            << std::setw(32) << "Description"
            << std::setw(10) << "Priority"
            << "Status\n"
            << std::string(70, '-') << '\n';
  for (const auto &entry : tasks_) {
    std::cout << std::setw(6)  << entry.id
              << std::setw(12) << entry.type
              << std::setw(32) << entry.description
              << std::setw(10) << entry.task->getPriority()
              << statusToString(entry.task->getStatus()) << '\n';
  }
}

void CLI::scheduleFile(std::istringstream &args) {
  std::string path;
  int priority = 0;
  args >> path;
  if (path.empty()) {
    std::cerr << "Usage: schedule file <path> [priority]\n";
    return;
  }
  args >> priority;

  auto task = std::make_shared<jobscheduler::FileTask>(path, priority);
  const int id = nextId_++;
  tasks_.push_back({id, "file", path, task});
  pool_.schedule(task);
  std::cout << "Scheduled file task #" << id << ": " << path << '\n';
}

void CLI::scheduleCompute(std::istringstream &args) {
  int durationMs = 100;
  int priority = 0;
  args >> durationMs >> priority;

  auto task = std::make_shared<ComputeTask>(priority,
                                            std::chrono::milliseconds{durationMs});
  const int id = nextId_++;
  tasks_.push_back({id, "compute", std::to_string(durationMs) + "ms", task});
  pool_.schedule(task);
  std::cout << "Scheduled compute task #" << id
            << " (duration: " << durationMs << "ms, priority: " << priority << ")\n";
}
