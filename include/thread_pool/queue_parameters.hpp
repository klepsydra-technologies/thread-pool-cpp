#pragma once

#include <concurrentqueue.h>

namespace tp {

struct QueueTraits : public moodycamel::ConcurrentQueueDefaultTraits {
#if defined(__rtems__)
    static const size_t BLOCK_SIZE = 8;
    static const size_t EXPLICIT_INITIAL_INDEX_SIZE = 8;
    static const size_t IMPLICIT_INITIAL_INDEX_SIZE = 8;
#endif
};

template <typename Task>
using ConcurrentQueue = moodycamel::ConcurrentQueue<Task, QueueTraits>;

}
