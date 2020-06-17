#pragma once

#include <algorithm>
#include <thread>

namespace tp
{

/**
 * @brief The ThreadPoolOptions class provides creation options for
 * ThreadPool.
 */
class ThreadPoolOptions
{
public:
    /**
     * @brief ThreadPoolOptions Construct default options for thread pool.
     */
    ThreadPoolOptions();

    /**
     * @brief setThreadCount Set thread count.
     * @param count Number of threads to be created.
     */
    void setThreadCount(size_t count);

    /**
     * @brief setQueueSize Set single worker queue size.
     * @param count Maximum length of queue of single worker.
     */
    void setQueueSize(size_t size);

    /**
     * @brief setCritical Set if threadpool deals with critical threads
     * @param critical True if critical threads, else false.
     */
    void setCritical(bool critical);

    /**
     * @brief threadCount Return thread count.
     */
    size_t threadCount() const;

    /**
     * @brief queueSize Return single worker queue size.
     */
    size_t queueSize() const;

    bool critical() const;
private:
    size_t m_thread_count;
    size_t m_queue_size;
    bool m_is_critical;
};

/// Implementation

inline ThreadPoolOptions::ThreadPoolOptions()
    : m_thread_count(std::max<size_t>(1u, std::thread::hardware_concurrency()))
    , m_queue_size(1024u)
    , m_is_critical(false)
{
}

inline void ThreadPoolOptions::setThreadCount(size_t count)
{
    m_thread_count = std::max<size_t>(1u, count);
}

inline void ThreadPoolOptions::setQueueSize(size_t size)
{
    m_queue_size = std::max<size_t>(1u, size);
}

inline size_t ThreadPoolOptions::threadCount() const
{
    return m_thread_count;
}

inline size_t ThreadPoolOptions::queueSize() const
{
    return m_queue_size;
}

inline void ThreadPoolOptions::setCritical(bool critical)
{
    m_is_critical = critical;
}

inline bool ThreadPoolOptions::critical() const
{
    return m_is_critical;
}

}
