#pragma once

#include <atomic>
#include <thread>

#if defined(__unix__) || defined(__rtems__)
#include <pthread.h>
#endif

#include <thread_pool/thread_params.hpp>
#include <thread_pool/free_workers_map.h>
#include <thread_pool/reader_writer_lock.h>

#include <spdlog/spdlog.h>

namespace tp
{
/**
 * @brief The Worker class owns task queue and executing thread.
 * In thread it tries to pop task from queue. If queue is empty then it tries
 * to steal task from the sibling worker. If steal was unsuccessful then spins
 * with one millisecond delay.
 */
template <typename Task, template<typename> class Queue>
class Worker
{
public:
    /**
     * @brief Worker Constructor.
     * @param queue_size Length of undelaying task queue.
     */
    explicit Worker(size_t queue_size, FreeWorkersMap & freeWorkers);

    /**
     * @brief Worker Constructor.
     * @param queue shared pointer to the queue.
     */
    explicit Worker(std::shared_ptr<Queue<std::pair<Task, ThreadParams>> > queue, FreeWorkersMap & freeWorkers);

    /**
     * @brief Move ctor implementation.
     */
    Worker(Worker&& rhs) noexcept;

    /**
     * @brief Move assignment implementaion.
     */
    Worker& operator=(Worker&& rhs) noexcept;

    /**
     * @brief start Create the executing thread and start tasks execution.
     * @param id Worker ID.
     */
    void start(size_t id);

    /**
     * @brief stop Stop all worker's thread and stealing activity.
     * Waits until the executing thread became finished.
     */
    void stop();

    /**
     * @brief post Post task to queue.
     * @param handler Handler to be executed in executing thread.
     * @return true on success.
     */
    template <typename Handler>
    bool post(Handler&& handler, ThreadParams&& params);

    /**
     * @brief getWorkerIdForCurrentThread Return worker ID associated with
     * current thread if exists.
     * @return Worker ID.
     */
    static size_t getWorkerIdForCurrentThread();

private:
    /**
     * @brief threadFunc Executing thread function.
     * @param id Worker ID to be associated with this thread.
     */
    void threadFunc(size_t id);

    std::shared_ptr<Queue<std::pair<Task, ThreadParams>> > m_queue;
    std::atomic<bool> m_running_flag;
    FreeWorkersMap & m_freeWorkers;
    std::thread m_thread;
};


/// Implementation

namespace detail
{
#if !(defined(__freertos__) || defined(KPSR_FREERTOS_EMUL))
    inline size_t* thread_id()
    {
        static thread_local size_t tss_id = -1u;
        return &tss_id;
    }
#else
    enum
    {
        thread_id_RD,
        thread_id_WR
    };

    inline void thread_id_op(size_t *id, int op)
    {
        static std::map<std::thread::id, size_t> thread_id_map;
        static ReaderWriterLock l;

        if (op == thread_id_RD)
        {
            l.lock_shared();
            auto it = thread_id_map.find(std::this_thread::get_id());
            if (it != thread_id_map.end())
                *id = it->second;
            else
                throw std::logic_error("thread_id_get(): id for this thread does not exist");
            l.unlock_shared();
        }
        else if (op == thread_id_WR)
        {
            l.lock();
            std::thread::id th_id = std::this_thread::get_id();
            if (thread_id_map.find(th_id) == thread_id_map.end())
                thread_id_map.insert(std::make_pair(th_id, *id));
            else
                thread_id_map[th_id] = *id;
            l.unlock();
        }
    }

    inline size_t thread_id_get()
    {
        size_t id;
        thread_id_op(&id, thread_id_RD);
        return id;
    }

    inline void thread_id_set(size_t& id)
    {
        thread_id_op(&id, thread_id_WR);
    }
#endif
}

template <typename Task, template<typename> class Queue>
inline Worker<Task, Queue>::Worker(size_t queue_size, FreeWorkersMap & freeWorkers)
    : m_queue(std::make_shared<Queue<std::pair<Task, ThreadParams>>>(queue_size))
    , m_running_flag(true)
    , m_freeWorkers(freeWorkers)
{
}

template <typename Task, template<typename> class Queue>
inline Worker<Task, Queue>::Worker(std::shared_ptr<Queue<std::pair<Task, ThreadParams>> > queue, FreeWorkersMap & freeWorkers)
    : m_queue(queue)
    , m_running_flag(true)
    , m_freeWorkers(freeWorkers)
{
}

template <typename Task, template<typename> class Queue>
inline Worker<Task, Queue>::Worker(Worker&& rhs) noexcept
{
    *this = rhs;
}

template <typename Task, template<typename> class Queue>
inline Worker<Task, Queue>& Worker<Task, Queue>::operator=(Worker&& rhs) noexcept
{
    if (this != &rhs)
    {
        m_queue = rhs.m_queue;
        m_running_flag = rhs.m_running_flag.load();
        m_thread = std::move(rhs.m_thread);
    }
    return *this;
}

template <typename Task, template<typename> class Queue>
inline void Worker<Task, Queue>::stop()
{
    m_running_flag.store(false, std::memory_order_relaxed);
    m_thread.join();
}

template <typename Task, template<typename> class Queue>
inline void Worker<Task, Queue>::start(size_t id)
{
    m_thread = std::thread(&Worker<Task, Queue>::threadFunc, this, id);
}

template <typename Task, template<typename> class Queue>
inline size_t Worker<Task, Queue>::getWorkerIdForCurrentThread()
{
#if !(defined(__freertos__) || defined(KPSR_FREERTOS_EMUL))
    return *detail::thread_id();
#else
    return detail::thread_id_get();
#endif
}

template <typename Task, template<typename> class Queue>
template <typename Handler>
inline bool Worker<Task, Queue>::post(Handler&& handler, ThreadParams&& params)
{
    return m_queue->push(std::make_pair<Task, ThreadParams>(std::forward<Handler>(handler), std::forward<ThreadParams>(params)));
}

template <typename Task, template<typename> class Queue>
inline void Worker<Task, Queue>::threadFunc(size_t id)
{
#if !(defined(__freertos__) || defined(KPSR_FREERTOS_EMUL))
    *detail::thread_id() = id;
#else
    detail::thread_id_set(id);
#endif

    // Task handler & name pair
    std::pair<Task, ThreadParams> handlerPair;

    while (m_running_flag.load(std::memory_order_relaxed))
    {
        if (m_queue->pop(handlerPair))
        {
            try
            {
                spdlog::debug("{}. Executing new job with name {}.", __PRETTY_FUNCTION__, handlerPair.second.getName());
                m_freeWorkers.setFree(id, false);
#if defined(__unix__) || defined(__rtems__)
                auto threadName = handlerPair.second.getName();
                if (!threadName.empty()) {
                    if (threadName.size() > 15) {
                        threadName.erase(16, std::string::npos);
                    }
                    pthread_setname_np(m_thread.native_handle(), threadName.c_str());
                }
                auto cpu_affinity_vector = handlerPair.second.getCpuAffinity();
                cpu_set_t cpuset;
                CPU_ZERO(&cpuset);
                if (!cpu_affinity_vector.empty()) {
                    for (auto& i: cpu_affinity_vector) {
                        CPU_SET(i, &cpuset);
                    }
                    pthread_setaffinity_np(m_thread.native_handle(),
                                                    sizeof(cpu_set_t), &cpuset);
                }
#endif
                handlerPair.first();
                m_freeWorkers.setFree(id, true);
                spdlog::debug("{}. Finished job with name {}.", __PRETTY_FUNCTION__, handlerPair.second.getName());

            }
            catch(...)
            {
                // suppress all exceptions
                spdlog::warn("{}. Exception during execution of {}.", __PRETTY_FUNCTION__, handlerPair.second.getName());
            }
        }
    }
}

}
