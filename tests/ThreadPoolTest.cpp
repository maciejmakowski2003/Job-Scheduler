#include <gtest/gtest.h>
#include "ThreadPool.h"
#include "TaskStatus.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

using namespace jobscheduler;

class TrackingTask : public Task {
public:
    explicit TrackingTask(
        int priority, std::function<bool()> cb = [] { return true; },
        std::chrono::system_clock::time_point t =
            std::chrono::system_clock::now(),
        int retryCount = 0,
        milliseconds retryTimeout = milliseconds(500))
        : Task(t, priority, retryCount, retryTimeout), cb_(std::move(cb)) {}

    bool execute() override { return cb_(); }

private:
  std::function<bool()> cb_;
};

static void waitForStatus(const std::shared_ptr<Task> &task, TaskStatus expected,
                           std::chrono::milliseconds timeout = std::chrono::milliseconds{2000}) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (task->getStatus() != expected) {
        ASSERT_LT(std::chrono::steady_clock::now(), deadline) << "Timed out waiting for task status";
        std::this_thread::sleep_for(std::chrono::milliseconds{5});
    }
}

static std::shared_ptr<TrackingTask>
createObservableTrackingTask(int priority,
                             std::chrono::system_clock::time_point time,
                             std::vector<int> &order, std::mutex &mutex) {
  return std::make_shared<TrackingTask>(
      priority,
      [priority, &order, &mutex] {
        std::lock_guard lock(mutex);
        order.push_back(priority);
        return true;
      },
      time);
}

TEST(ThreadPool, SuccessfulTaskReachesSucceededStatus) {
    ThreadPool pool(1);
    auto task = std::make_shared<TrackingTask>(0);
    pool.schedule(task);
    waitForStatus(task, TaskStatus::Succeeded);
}

TEST(ThreadPool, TaskReturningFalseReachesFailedStatus) {
    ThreadPool pool(1);
    auto task = std::make_shared<TrackingTask>(0, [] { return false; });
    pool.schedule(task);
    waitForStatus(task, TaskStatus::Failed);
}

TEST(ThreadPool, AllTasksCompleteWithMultipleWorkers) {
    ThreadPool pool(4);
    const int N = 20;
    std::vector<std::shared_ptr<TrackingTask>> tasks;
    tasks.reserve(N);

    for (int i = 0; i < N; ++i) {
        tasks.push_back(std::make_shared<TrackingTask>(0));
        pool.schedule(tasks.back());
    }

    for (const auto &t : tasks) {
      waitForStatus(t, TaskStatus::Succeeded);
    }
}

TEST(ThreadPool, DelayedTaskDoesNotExecuteBeforeItsScheduledTime) {
    ThreadPool pool(1);
    auto delay = std::chrono::milliseconds{300};
    auto scheduledTime = std::chrono::system_clock::now() + delay;
    auto task = std::make_shared<TrackingTask>(0, [] { return true; }, scheduledTime);

    auto submitTime = std::chrono::steady_clock::now();
    pool.schedule(task);

    waitForStatus(task, TaskStatus::Succeeded, std::chrono::milliseconds{3000});

    auto elapsed = std::chrono::steady_clock::now() - submitTime;
    EXPECT_GE(elapsed, delay);
}

TEST(ThreadPool, TasksAreDispatchedInPriorityOrderWithSingleWorker) {
    ThreadPool pool(1);
    std::vector<int> order;
    std::mutex mu;
    auto futureTime = std::chrono::system_clock::now() + std::chrono::milliseconds{200};

    auto t1 = createObservableTrackingTask(1, futureTime, order, mu);
    auto t5 = createObservableTrackingTask(5, futureTime, order, mu);
    auto t10 = createObservableTrackingTask(10, futureTime, order, mu);
    pool.schedule(t1);
    pool.schedule(t5);
    pool.schedule(t10);

    waitForStatus(t1, TaskStatus::Succeeded, std::chrono::milliseconds{3000});

    std::lock_guard lock(mu);
    EXPECT_EQ(order, (std::vector<int>{10, 5, 1}));
}

TEST(ThreadPool, TasksAreDispatchedInScheduledTimeOrderWithSingleWorker) {
    ThreadPool pool(1);
    std::vector<int> order;
    std::mutex mu;
    auto now = std::chrono::system_clock::now();

    auto t1 = createObservableTrackingTask(1, now + std::chrono::milliseconds{500}, order, mu);
    auto t2 = createObservableTrackingTask(2, now + std::chrono::milliseconds{400}, order, mu);
    auto t3 = createObservableTrackingTask(3, now + std::chrono::milliseconds{200}, order, mu);
    auto t4 = createObservableTrackingTask(4, now + std::chrono::milliseconds{100}, order, mu);
    auto t5 = createObservableTrackingTask(5, now + std::chrono::milliseconds{300}, order, mu);
    pool.schedule(t1);
    pool.schedule(t2);
    pool.schedule(t3);
    pool.schedule(t4);
    pool.schedule(t5);

    waitForStatus(t1, TaskStatus::Succeeded, std::chrono::milliseconds{3000});

    std::lock_guard lock(mu);
    EXPECT_EQ(order, (std::vector<int>{4, 3, 5, 2, 1}));
}

TEST(ThreadPool, TaskWithNoRetriesFailsImmediately) {
    ThreadPool pool(1);
    std::atomic<int> attempts{0};
    auto task = std::make_shared<TrackingTask>(0, [&] {
        ++attempts;
        return false;
    }, std::chrono::system_clock::now(), 0);

    pool.schedule(task);
    waitForStatus(task, TaskStatus::Failed);

    EXPECT_EQ(attempts.load(), 1);
}

TEST(ThreadPool, TaskIsRetriedExactlyRetryCountTimes) {
    ThreadPool pool(1);
    std::atomic<int> attempts{0};
    constexpr int retries = 3;
    auto task = std::make_shared<TrackingTask>(0, [&] {
        ++attempts;
        return false;
    }, std::chrono::system_clock::now(), retries, milliseconds(0));

    pool.schedule(task);
    waitForStatus(task, TaskStatus::Failed, std::chrono::milliseconds{5000});

    EXPECT_EQ(attempts.load(), retries + 1);
}

TEST(ThreadPool, TaskSucceedsOnRetry) {
    ThreadPool pool(1);
    std::atomic<int> attempts{0};
    auto task = std::make_shared<TrackingTask>(0, [&] {
        return ++attempts >= 3;
    }, std::chrono::system_clock::now(), 2, milliseconds(0));

    pool.schedule(task);
    waitForStatus(task, TaskStatus::Succeeded, std::chrono::milliseconds{5000});

    EXPECT_EQ(attempts.load(), 3);
}

TEST(ThreadPool, RetryIsDelayedByRetryTimeout) {
    ThreadPool pool(1);
    constexpr auto retryTimeout = milliseconds(300);
    std::atomic<int> attempts{0};
    std::chrono::steady_clock::time_point firstFailTime;

    auto task = std::make_shared<TrackingTask>(0, [&] {
        if (++attempts == 1) {
            firstFailTime = std::chrono::steady_clock::now();
            return false;
        }
        return true;
    }, std::chrono::system_clock::now(),1, retryTimeout);

    pool.schedule(task);
    waitForStatus(task, TaskStatus::Succeeded, std::chrono::milliseconds{5000});

    auto elapsed = std::chrono::steady_clock::now() - firstFailTime;
    EXPECT_GE(elapsed, retryTimeout);
}
