#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>
#include <optional>

namespace jobscheduler {

template <typename T>
class MpscChannel {
public:
  void send(const T &value) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(value);
    }

    cv_.notify_one();
  }

  void send(T &&value) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(value);
    }

    cv_.notify_one();
  }

  std::optional<T> receive() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !queue_.empty() || stopped_; });

    if (stopped_ && queue_.empty()) {
      return std::nullopt;
    }
    
    auto value = std::move(queue_.front());
    queue_.pop();
    return value;
  }

  std::optional<T> try_receive() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
      return std::nullopt;
    }

    auto value = std::move(queue_.front());
    queue_.pop();
    return value;
  }

  void stop() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stopped_ = true;
    }
    cv_.notify_all();
  }

private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
  bool stopped_ = false;
};

} // namespace jobscheduler
