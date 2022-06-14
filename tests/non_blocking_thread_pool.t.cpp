#include <gtest/gtest.h>

#include <thread_pool/thread_pool.hpp>

#include <thread>
#include <future>
#include <functional>
#include <memory>

namespace TestLinkage {
size_t getWorkerIdForCurrentThread()
{
#if defined(__freertos__) || defined(KPSR_FREERTOS_EMUL)
    return tp::detail::thread_id_get();
#else
    return *tp::detail::thread_id();
#endif
}

size_t getWorkerIdForCurrentThread2()
{
    return tp::Worker<std::function<void()>, tp::MPMCBoundedQueue>::getWorkerIdForCurrentThread();
}
}

TEST(ThreadPool, postJob)
{
    tp::NonBlockingThreadPool pool;

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
    tp::NonBlockingThreadPool pool;

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

TEST(ThreadPool, tryPostJobNoName)
{
    tp::NonBlockingThreadPool pool;

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

    tp::NonBlockingThreadPool pool(options);
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

    tp::NonBlockingThreadPool pool(options);
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
