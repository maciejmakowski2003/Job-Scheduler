#include "CLI.h"
#include "ComputeTask.hpp"
#include "FileTask.hpp"
#include "PingTask.hpp"

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
      if (type.empty()) {
        std::cerr << "Usage: schedule <file|compute|ping> ...\n";
      } else {
        scheduleTask(type, iss);
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
            << std::string(150, '=') << '\n'
            << std::left
            << std::setw(cmdWidth) << "Command" << "Description" << '\n'
            << std::string(150, '-') << '\n'
            << std::setw(cmdWidth) << "schedule file <path> [priority] [--delay <ms>]"
            << "Schedule a file analysis task\n"
            << std::setw(cmdWidth)
            << "schedule compute [priority] [--delay <ms>]"
            << "Schedule a 5s compute task\n"
            << std::setw(cmdWidth)
            << "schedule ping <port> [interval_ms]"
            << "Periodically send a UDP ping to a local port (read with: nc -u -l <port>)\n"
            << std::setw(cmdWidth) << "status" << "List all tasks\n"
            << std::setw(cmdWidth) << "stop" << "Stop the scheduler and exit\n"
            << std::setw(cmdWidth) << "help" << "Show this help\n"
            << std::string(150, '-') << std::endl;
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

void CLI::scheduleTask(const std::string &type, std::istringstream &args) {
  if (type == "file") {
    scheduleFileTask(args);
  } else if (type == "compute") {
    scheduleComputeTask(args);
  } else if (type == "ping") {
    schedulePingTask(args);
  } else {
    std::cerr << "Unknown task type '" << type << "'. Use 'file', 'compute', or 'ping'.\n";
  }
}

void CLI::scheduleFileTask(std::istringstream &args) {
  std::string fileArg;
  args >> fileArg;
  if (fileArg.empty()) {
    std::cerr << "Usage: schedule file <path> [priority] [--delay <ms>]\n";
    return;
  }

  int priority = 0;
  auto scheduledTime = std::chrono::system_clock::now();
  std::string token;
  int positionalCount = 0;
  while (args >> token) {
    if (token == "--delay") {
      int delayMs = 0;
      if (args >> delayMs) {
        scheduledTime = std::chrono::system_clock::now() +
                        std::chrono::milliseconds{delayMs};
      }
    } else {
      if (positionalCount == 0) {
        try {
          priority = std::stoi(token);
        } catch (const std::exception &) {
          std::cerr << "Invalid priority '" << token << "': expected an integer.\n";
          return;
        }
      }
      ++positionalCount;
    }
  }

  const int id = nextId_++;
  auto task = std::make_shared<jobscheduler::FileTask>(id, fileArg, priority, scheduledTime);
  tasks_.push_back({id, "file", fileArg, task});
  pool_.schedule(task);
}

void CLI::scheduleComputeTask(std::istringstream &args) {
  int priority = 0;
  auto scheduledTime = std::chrono::system_clock::now();
  std::string token;
  int positionalCount = 0;
  while (args >> token) {
    if (token == "--delay") {
      int delayMs = 0;
      if (args >> delayMs) {
        scheduledTime = std::chrono::system_clock::now() + std::chrono::milliseconds{delayMs};
      }
    } else {
      if (positionalCount == 0) {
        try {
          priority = std::stoi(token);
        } catch (const std::exception &) {
          std::cerr << "Invalid priority '" << token << "': expected an integer.\n";
          return;
        }
      }
      ++positionalCount;
    }
  }

  const int id = nextId_++;
  auto task = std::make_shared<ComputeTask>(id, priority, std::chrono::milliseconds{5000}, scheduledTime);
  tasks_.push_back({id, "compute", "5000ms", task});
  pool_.schedule(task);
}

void CLI::schedulePingTask(std::istringstream &args) {
  std::string portArg;
  args >> portArg;
  if (portArg.empty()) {
    std::cerr << "Usage: schedule ping <port> [interval_ms]\n";
    return;
  }

  int port = 0, intervalMs = 1000;
  try { port = std::stoi(portArg); } catch (...) {
    std::cerr << "Invalid port '" << portArg << "': expected an integer.\n";
    return;
  }

  std::string token;
  int positionalCount = 0;
  while (args >> token) {
    try {
      int val = std::stoi(token);
      if (positionalCount == 0) {
        intervalMs = val;
      }
    } catch (const std::exception &) {
      std::cerr << "Invalid argument '" << token << "': expected an integer.\n";
      return;
    }
    ++positionalCount;
  }

  const int id = nextId_++;
  auto task = std::make_shared<PingTask>(
      id, static_cast<uint16_t>(port), jobscheduler::milliseconds(intervalMs));
  tasks_.push_back({id, "ping", "UDP:" + portArg, task});
  pool_.schedule(task);
}
