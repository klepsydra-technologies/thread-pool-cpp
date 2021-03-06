#include <gtest/gtest.h>

#include <thread_pool/thread_pool.hpp>
#include <thread_pool/thread_params.hpp>
#include <thread_pool/fixed_function.hpp>
#include <thread_pool/safe_queue.h>

#include <thread>
#include <future>
#include <functional>
#include <memory>

TEST(ThreadPool, postJob)
{
    tp::BlockingThreadPool pool;

    std::packaged_task<int()> t([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return 42;
        });

    std::future<int> r = t.get_future();

    pool.post(t);

    ASSERT_EQ(42, r.get());
}

TEST(ThreadPool, tryPostJob)
{
    tp::BlockingThreadPool pool;

    std::packaged_task<int()> t([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return 42;
        });

    std::future<int> r = t.get_future();
    std::string name("testThread");
    ASSERT_TRUE(pool.tryPost(t, name));

    ASSERT_EQ(42, r.get());
}

TEST(ThreadPool, tryPostJobame)
{
    tp::BlockingThreadPool pool;

    std::packaged_task<int()> t([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return 42;
        });

    std::future<int> r = t.get_future();
    ASSERT_TRUE(pool.tryPost(t));

    ASSERT_EQ(42, r.get());
}

TEST(ThreadPool, postJobCritical)
{
    tp::ThreadPoolOptions options;
    options.setThreadCount(1);
    options.setCritical(true);

    tp::BlockingThreadPool pool(options);
    std::packaged_task<int()> t([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return 42;
        });

    std::future<int> r = t.get_future();

    pool.post(t);
    ASSERT_THROW(pool.post(t), std::runtime_error);

    ASSERT_EQ(42, r.get());
}

TEST(ThreadPool, postJobCriticalAfterFree)
{
    tp::ThreadPoolOptions options;
    options.setThreadCount(1);
    options.setCritical(true);

    tp::BlockingThreadPool pool(options);
    std::packaged_task<int()> t([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return 42;
        });

    std::future<int> r = t.get_future();

    pool.post(t);

    std::packaged_task<int()> second_task(
        []() {
            return 40;
        });

    // Wait for first task to finish before posting second task.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // Critical threadpool can be reused if first task is over.
    ASSERT_NO_THROW(pool.post(second_task));

    ASSERT_EQ(42, r.get());
}
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
