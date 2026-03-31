#pragma once

#include "Macros.h"
#include <concepts>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <optional>

namespace jobscheduler {

/// @brief Concept for types that can be sent through MpscChannel.
/// T must be at least move-constructible (required by move-send and receive).
template <typename T>
concept ChannelMessage = std::move_constructible<T>;

/// @brief A thread-safe multi-producer single-consumer (MPSC) channel for
/// communication between threads.
/// @tparam T The type of messages that can be sent through the channel.
template <ChannelMessage T> class MpscChannel {
public:
  MpscChannel<T>() = default;
  ~MpscChannel() = default;

  DELETE_COPY_AND_MOVE(MpscChannel<T>);

  /// @brief Send a message through the channel.
  /// @param value The message to be sent.
  void send(const T &value) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(value);
    }

    cv_.notify_one();
  }

  /// @brief Send a message through the channel using move semantics.
  /// @param value The message to be sent.
  void send(T &&value) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(std::move(value));
    }

    cv_.notify_one();
  }

  /// @brief Receive a message from the channel. This method will block until a
  /// message is available or the channel is stopped.
  /// @return An optional containing the received message, or std::nullopt.
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

  /// @brief Try to receive a message from the channel without blocking.
  /// @return An optional containing the received message, or std::nullopt.
  std::optional<T> try_receive() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
      return std::nullopt;
    }

    auto value = std::move(queue_.front());
    queue_.pop();
    return value;
  }

  /// @brief Wait until a message is available or the channel is stopped, with a
  /// timeout.
  /// @param deadline The time point until which to wait for a message.
  /// @return true if a message is available, false if the channel is stopped or
  /// the timeout expired.
  bool wait_until(std::chrono::system_clock::time_point deadline) {
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_until(lock, deadline,
                          [this] { return !queue_.empty() || stopped_; });
  }

  /// @brief Wait until a message is available or the channel is stopped.
  void wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !queue_.empty() || stopped_; });
  }

  /// @brief Stop the channel. This method will unblock any waiting threads and cause
  /// them to receive std::nullopt if they try to receive from the channel.
  void stop() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stopped_ = true;
    }
    cv_.notify_all();
  }

  /// @brief Check if the channel has been stopped.
  bool is_stopped() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stopped_;
  }

private:
  std::queue<T> queue_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  bool stopped_{false};
};

} // namespace jobscheduler
