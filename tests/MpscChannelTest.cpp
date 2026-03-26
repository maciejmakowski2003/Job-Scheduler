#include <gtest/gtest.h>
#include "MpscChannel.hpp"
#include <chrono>
#include <thread>
#include <vector>

using namespace jobscheduler;

TEST(MpscChannel, SendAndReceiveReturnsTheSentValue) {
    MpscChannel<int> ch;
    ch.send(42);
    auto val = ch.receive();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 42);
}

TEST(MpscChannel, TryReceiveReturnsNulloptOnEmptyChannel) {
    MpscChannel<int> ch;
    EXPECT_FALSE(ch.try_receive().has_value());
}

TEST(MpscChannel, TryReceiveReturnsValueWhenPresent) {
    MpscChannel<int> ch;
    ch.send(7);
    auto val = ch.try_receive();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 7);
}

TEST(MpscChannel, ReceivePreservesFifoOrder) {
    MpscChannel<int> ch;
    ch.send(1);
    ch.send(2);
    ch.send(3);
    EXPECT_EQ(*ch.receive(), 1);
    EXPECT_EQ(*ch.receive(), 2);
    EXPECT_EQ(*ch.receive(), 3);
}

TEST(MpscChannel, StopUnblocksAWaitingReceive) {
    MpscChannel<int> ch;
    std::thread t([&] { ch.stop(); });
    auto val = ch.receive();
    EXPECT_FALSE(val.has_value());
    t.join();
}

TEST(MpscChannel, WaitUntilReturnsFalseOnTimeout) {
    MpscChannel<int> ch;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds{50};
    EXPECT_FALSE(ch.wait_until(deadline));
}

TEST(MpscChannel, WaitUntilReturnsTrueWhenValueArrivesBeforeDeadline) {
    MpscChannel<int> ch;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds{1000};
    std::thread t([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds{30});
        ch.send(1);
    });
    EXPECT_TRUE(ch.wait_until(deadline));
    t.join();
}

TEST(MpscChannel, AllMessagesFromMultipleSendersArrive) {
    MpscChannel<int> ch;
    const int N = 10;
    std::vector<std::thread> senders;

    for (int i = 0; i < N; ++i) {
      senders.emplace_back([&ch, i] { ch.send(i); });
    }

    for (auto &t : senders) {
      t.join();
    }

    int count = 0;
    while (ch.try_receive().has_value()) {
      count++;
    }

    EXPECT_EQ(count, N);
}
