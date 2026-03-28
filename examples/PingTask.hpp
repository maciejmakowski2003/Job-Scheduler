#pragma once

#include "PeriodicTask.hpp"
#include <arpa/inet.h>
#include <format>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class PingTask : public jobscheduler::PeriodicTask {
public:
  explicit PingTask(int id, uint16_t port, jobscheduler::milliseconds interval)
      : PeriodicTask(std::format("PingTask#{}", id), interval), port_(port) {}

  ~PingTask() override {
    if (sockfd_ >= 0)
      close(sockfd_);
  }

protected:
  jobscheduler::TaskResult execute() override {
    if (sockfd_ < 0) {
      sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
      if (sockfd_ < 0) {
        return {jobscheduler::ExecutionStatus::Failure,
                "Failed to create socket"};
      }
    }

    const std::string msg =
        std::format("PING [{:%H:%M:%S}]\n", std::chrono::system_clock::now());

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ssize_t sent = sendto(sockfd_, msg.c_str(), msg.size(), 0,
                          reinterpret_cast<sockaddr *>(&addr), sizeof(addr));

    if (sent < 0) {
      return {jobscheduler::ExecutionStatus::Failure, "Failed to send ping"};
    }

    return {jobscheduler::ExecutionStatus::Success,
            std::format("Sent: {}", msg)};
  }

private:
  uint16_t port_;
  int sockfd_ = -1;
};
