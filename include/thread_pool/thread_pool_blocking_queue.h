#ifndef THREADPOOL_BLOCKING_QUEUE_H
#define THREADPOOL_BLOCKING_QUEUE_H

#include <thread_pool/safe_queue.h>

namespace tp {
template<class T>
class BlockingQueue {

public:
    //TODO: enable timeout via configuration
    BlockingQueue(size_t size, std::uint64_t timeout = 100000)
        : _decorableQueue(size)
        , _timeout(timeout)
    {}

    bool pop(T& item) {
        _decorableQueue.timeout_move_pop(item, _timeout);
        return true;
    }

    bool push(T&& item) {
        return _decorableQueue.try_move_push(item);
    }

private:
    SafeQueue<T> _decorableQueue;
    std::uint64_t _timeout;
};
}

#endif // THREADPOOL_BLOCKING_QUEUE_H
