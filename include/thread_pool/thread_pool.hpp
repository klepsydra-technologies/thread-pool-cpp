#pragma once

#include <thread_pool/fixed_function.hpp>
#include <thread_pool/thread_pool_options.hpp>
#include <thread_pool/thread_params.hpp>
#include <thread_pool/worker.hpp>

#include <concurrentqueue.h>

#include <atomic>
#include <memory>
#include <stdexcept>
#include <vector>

namespace tp
{

template <typename Task>
using ConcurrentQueue = moodycamel::ConcurrentQueue<Task, moodycamel::ConcurrentQueueDefaultTraits>;

template <typename Task, template<typename> class Queue>
class ThreadPoolImpl;
using ThreadPool = ThreadPoolImpl<FixedFunction<void(), 128>,
                                  ConcurrentQueue>;

/**
 * @brief The ThreadPool class implements thread pool pattern.
 * It is highly scalable and fast.
 * It is header only.
 * It implements both work-stealing and work-distribution balancing
 * startegies.
 * It implements cooperative scheduling strategy for tasks.
 */
template <typename Task, template<typename> class Queue>
class ThreadPoolImpl {
public:
    /**
     * @brief ThreadPool Construct and start new thread pool.
     * @param options Creation options.
     */
    explicit ThreadPoolImpl(
        const ThreadPoolOptions& options = ThreadPoolOptions());

    /**
     * @brief Move ctor implementation.
     */
    ThreadPoolImpl(ThreadPoolImpl&& rhs) noexcept;

    /**
     * @brief ~ThreadPool Stop all workers and destroy thread pool.
     */
    ~ThreadPoolImpl();

    /**
     * @brief Move assignment implementaion.
     */
    ThreadPoolImpl& operator=(ThreadPoolImpl&& rhs) noexcept;

    /**
     * @brief post Try post job to thread pool.
     * @param handler Handler to be called from thread pool worker. It has
     * to be callable as 'handler()'.
     * @param name Name to be set as thread name. Must have max length 15 For linux.
     * @return 'true' on success, false otherwise.
     * @note All exceptions thrown by handler will be suppressed.
     */
    template <typename Handler>
    bool tryPost(Handler&& handler, const std::string& name = "", const std::vector<int>& cpuset = std::vector<int>());

    /**
     * @brief post Post job to thread pool.
     * @param handler Handler to be called from thread pool worker. It has
     * to be callable as 'handler()'.
     * @throw std::overflow_error if worker's queue is full.
     * @note All exceptions thrown by handler will be suppressed.
     */
    template <typename Handler>
    void post(Handler&& handler);

private:
    size_t getWorkerId();

    std::vector<std::unique_ptr<Worker<Task, Queue>>> m_workers;
    std::atomic<size_t> m_next_worker;
    const bool m_critical;
};


/// Implementation

template <typename Task, template<typename> class Queue>
inline ThreadPoolImpl<Task, Queue>::ThreadPoolImpl(
                                            const ThreadPoolOptions& options)
    : m_workers(options.threadCount())
    , m_next_worker(0)
    , m_critical(options.critical())
{
    for(auto& worker_ptr : m_workers)
    {
        worker_ptr.reset(new Worker<Task, Queue>(options.queueSize()));
    }

    for(size_t i = 0; i < m_workers.size(); ++i)
    {
        Worker<Task, Queue>* steal_donor =
                                m_workers[(i + 1) % m_workers.size()].get();
        m_workers[i]->start(i, steal_donor);
    }
}

template <typename Task, template<typename> class Queue>
inline ThreadPoolImpl<Task, Queue>::ThreadPoolImpl(ThreadPoolImpl<Task, Queue>&& rhs) noexcept
{
    *this = rhs;
}

template <typename Task, template<typename> class Queue>
inline ThreadPoolImpl<Task, Queue>::~ThreadPoolImpl()
{
    for (auto& worker_ptr : m_workers)
    {
        worker_ptr->stop();
    }
}

template <typename Task, template<typename> class Queue>
inline ThreadPoolImpl<Task, Queue>&
ThreadPoolImpl<Task, Queue>::operator=(ThreadPoolImpl<Task, Queue>&& rhs) noexcept
{
    if (this != &rhs)
    {
        m_workers = std::move(rhs.m_workers);
        m_next_worker = rhs.m_next_worker.load();
    }
    return *this;
}

template <typename Task, template<typename> class Queue>
template <typename Handler>
inline bool ThreadPoolImpl<Task, Queue>::tryPost(Handler&& handler, const std::string& name, const std::vector<int>& cpuset)
{
    auto id = getWorkerId();
    if ((id >= m_workers.size()) && m_critical) {
        return false;
    } else {
        ThreadParams params{name, cpuset};
        return m_workers[id % m_workers.size()]->post(std::forward<Handler>(handler), std::move(params));
    }
}

template <typename Task, template<typename> class Queue>
template <typename Handler>
inline void ThreadPoolImpl<Task, Queue>::post(Handler&& handler)
{
    const auto ok = tryPost(std::forward<Handler>(handler));
    if (!ok)
    {
        throw std::runtime_error("thread pool queue is full");
    }
}

template <typename Task, template<typename> class Queue>
inline size_t ThreadPoolImpl<Task, Queue>::getWorkerId()
{
    auto id = Worker<Task, Queue>::getWorkerIdForCurrentThread();

    if (id > m_workers.size())
    {
        id = m_next_worker.fetch_add(1, std::memory_order_relaxed);
    }

    return id;
}
}
