#include <gtest/gtest.h>

#include <thread_pool/thread_pool_blocking_queue.h>
#include <thread_pool/thread_params.hpp>
#include <thread_pool/fixed_function.hpp>
#include <thread_pool/safe_queue.h>

#include <thread>
#include <future>
#include <functional>
#include <memory>

#include <spdlog/spdlog.h>

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

TEST(BlockingQueue, mtNominal) {
    size_t queueSize = 2;
    tp::BlockingQueue<int> intQueue(queueSize);

    int producerCounter = 0;
    std::thread producer([&intQueue, &producerCounter]() {
        for (int i = 0; i < 100; i++) {
            intQueue.push(std::rand());
            producerCounter++;
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
    });

    int consumerCounter = 0;
    std::thread consumer([&intQueue, &consumerCounter]() {
        int valueToPop;
        for (int i = 0; i < 100; i++) {
            intQueue.pop(valueToPop);
            consumerCounter++;
            std::cout << "BlockingQueue::mtNominal. New value received: " << valueToPop << std::endl;
        }
    });

    producer.join();
    consumer.join();

    ASSERT_EQ(producerCounter, consumerCounter);
}

TEST(BlockingQueue, mtMultipleConsumer) {
    size_t queueSize = 2;
    tp::BlockingQueue<int> intQueue(queueSize);

    int producerCounter = 0;
    std::thread producer([&intQueue, &producerCounter]() {
        for (int i = 0; i < 90; i++) {
            intQueue.push(std::rand());
            producerCounter++;
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
    });

    int consumerCounter1 = 0;
    std::thread consumer1([&intQueue, &consumerCounter1]() {
        int valueToPop;
        for (int i = 0; i < 100; i++) {
            bool ok = intQueue.pop(valueToPop);
            if (ok) {
                consumerCounter1++;
                spdlog::debug("BlockingQueue::mtMultipleConsumer1. New value received: {}", valueToPop);
            }
        }
    });

    int consumerCounter2 = 0;
    std::thread consumer2([&intQueue, &consumerCounter2]() {
        int valueToPop;
        for (int i = 0; i < 100; i++) {
            bool ok = intQueue.pop(valueToPop);
            if (ok) {
                consumerCounter2++;
                spdlog::debug("BlockingQueue::mtMultipleConsumer2. New value received: {}", valueToPop);
            }
        }
    });

    int consumerCounter3 = 0;
    std::thread consumer3([&intQueue, &consumerCounter3]() {
        int valueToPop;
        for (int i = 0; i < 100; i++) {
            bool ok = intQueue.pop(valueToPop);
            if (ok) {
                consumerCounter3++;
                spdlog::debug("BlockingQueue::mtMultipleConsumer3. New value received: {}", valueToPop);
            }
        }
    });

    int consumerCounter4 = 0;
    std::thread consumer4([&intQueue, &consumerCounter4]() {
        int valueToPop;
        for (int i = 0; i < 100; i++) {
            bool ok = intQueue.pop(valueToPop);
            if (ok) {
                consumerCounter4++;
                spdlog::debug("BlockingQueue::mtMultipleConsumer4. New value received: {}", valueToPop);
            }
        }
    });

    int consumerCounter5 = 0;
    std::thread consumer5([&intQueue, &consumerCounter5]() {
        int valueToPop;
        for (int i = 0; i < 100; i++) {
            bool ok = intQueue.pop(valueToPop);
            if (ok) {
                consumerCounter5++;
                spdlog::debug("BlockingQueue::mtMultipleConsumer5. New value received: {}", valueToPop);
            }
        }
    });

    producer.join();
    consumer1.join();
    consumer2.join();
    consumer3.join();
    consumer4.join();
    consumer5.join();

    ASSERT_EQ(producerCounter, consumerCounter1 + consumerCounter2 + consumerCounter3 + consumerCounter4 + consumerCounter5);
}


int main(int argc, char **argv) {
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    spdlog::set_level(spdlog::level::debug);

    std::srand(std::time(nullptr));
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
