#pragma once

#include "PeriodicTask.hpp"
#include <string>
#include <format>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class PingTask : public jobscheduler::PeriodicTask {
public:
  explicit PingTask(uint16_t port,
                    jobscheduler::milliseconds interval)
      : PeriodicTask(interval), port_(port) {}

  ~PingTask() override {
    if (sockfd_ >= 0)
      close(sockfd_);
  }

protected:
  bool execute() override {
    if (sockfd_ < 0) {
      sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
      if (sockfd_ < 0) return false;
    }

    const std::string msg =
        std::format("PING [{:%H:%M:%S}]\n", std::chrono::system_clock::now());

    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port_);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ssize_t sent = sendto(sockfd_, msg.c_str(), msg.size(), 0,
                          reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    return sent > 0;
  }

private:
  uint16_t port_;
  int sockfd_ = -1;
};
