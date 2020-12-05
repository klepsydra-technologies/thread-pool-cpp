#include <gtest/gtest.h>

#include <thread_pool/thread_pool_blocking_queue.h>
#include <thread_pool/thread_params.hpp>
#include <thread_pool/fixed_function.hpp>
#include <thread_pool/safe_queue.h>

#include <thread>
#include <future>
#include <functional>
#include <memory>

TEST(BlockingQueue, pushItemNominal) {
    size_t queueSize = 2;
    tp::BlockingQueue<int> intQueue(queueSize);

    ASSERT_TRUE(intQueue.push(std::rand()));
    ASSERT_TRUE(intQueue.push(std::rand()));
    ASSERT_FALSE(intQueue.push(std::rand()));

}

TEST(BlockingQueue, pushItemLvalue) {
    size_t queueSize = 2;
    tp::BlockingQueue<int> intQueue(queueSize);

    int valueToAdd = std::rand();
    ASSERT_TRUE(intQueue.push(std::move(valueToAdd)));
    int valuePop(0);
    ASSERT_TRUE(intQueue.pop(valuePop));
    ASSERT_EQ(valueToAdd, valuePop);
}

TEST(BlockingQueue, popItemNominal) {
    size_t queueSize = 2;
    tp::BlockingQueue<int> intQueue(queueSize);

    ASSERT_TRUE(intQueue.push(std::rand()));
    ASSERT_TRUE(intQueue.push(std::rand()));
    int valueToPop;
    ASSERT_TRUE(intQueue.pop(valueToPop));
    ASSERT_TRUE(intQueue.pop(valueToPop));
    ASSERT_FALSE(intQueue.pop(valueToPop));
}

int main(int argc, char **argv) {
    std::srand(std::time(nullptr));
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
