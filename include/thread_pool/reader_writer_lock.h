#pragma once

#include <condition_variable>

namespace tp
{

class ReaderWriterLock {
public:
    explicit ReaderWriterLock() = default;

    // Locks the mutex for shared ownership, blocks if the mutex is not available
    void lock_shared()
    {
        std::unique_lock<std::mutex> lk(rw_mutex);
        while (has_writers) {
            rw_cv.wait(lk);
        }
        readers++;
    }

    // Locks the mutex, blocks if the mutex is not available
    void lock()
    {
        std::unique_lock<std::mutex> lk(rw_mutex);
        while (has_writers || readers > 0) {
            rw_cv.wait(lk);
        }
        has_writers = true;
    }

    // Unlocks the mutex
    void unlock()
    {
        std::unique_lock<std::mutex> lk(rw_mutex);
        has_writers = false;
        rw_cv.notify_all();
    }

    // Unlocks the mutex (shared ownership)
    void unlock_shared()
    {
        std::unique_lock<std::mutex> lk(rw_mutex);
        if (readers == 1)
            rw_cv.notify_all();
        readers--;
    }

private:
    std::mutex rw_mutex;
    std::condition_variable rw_cv;
    uintmax_t readers{0};
    bool has_writers{false};
};

}
