/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>
#include "container_system/core/container.h"
#include "container_system/core/value.h"
#include "container_system/internal/thread_safe_container.h"
#include "container_system/internal/simd_processor.h"
#include "container_system/internal/memory_pool.h"

#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <barrier>
#include <algorithm>
#include <random>

using namespace kcenon::container;
using namespace std::chrono_literals;

class ContainerThreadSafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test data
    }

    void TearDown() override {
        // Cleanup
    }
};

// Test 1: Concurrent read/write on thread_safe_container
TEST_F(ContainerThreadSafetyTest, ConcurrentReadWrite) {
    thread_safe_container<std::string, Value> container;

    const int num_readers = 10;
    const int num_writers = 5;
    const int operations_per_thread = 1000;
    const int num_keys = 100;

    std::vector<std::thread> threads;
    std::atomic<int> read_errors{0};
    std::atomic<int> write_errors{0};
    std::atomic<bool> running{true};

    // Pre-populate container
    for (int i = 0; i < num_keys; ++i) {
        container.insert("key_" + std::to_string(i), Value(i));
    }

    // Writer threads
    for (int i = 0; i < num_writers; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            std::mt19937 rng(thread_id);
            std::uniform_int_distribution<int> dist(0, num_keys - 1);

            for (int j = 0; j < operations_per_thread && running.load(); ++j) {
                try {
                    int key_idx = dist(rng);
                    std::string key = "key_" + std::to_string(key_idx);
                    container.insert_or_assign(key, Value(thread_id * 1000 + j));
                } catch (...) {
                    ++write_errors;
                }

                if (j % 100 == 0) {
                    std::this_thread::sleep_for(1ms);
                }
            }
        });
    }

    // Reader threads
    for (int i = 0; i < num_readers; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            std::mt19937 rng(thread_id + 1000);
            std::uniform_int_distribution<int> dist(0, num_keys - 1);

            for (int j = 0; j < operations_per_thread && running.load(); ++j) {
                try {
                    int key_idx = dist(rng);
                    std::string key = "key_" + std::to_string(key_idx);
                    auto value = container.find(key);
                    // Just access the value
                    if (!value) {
                        ++read_errors;
                    }
                } catch (...) {
                    ++read_errors;
                }
            }
        });
    }

    // Let it run for a bit
    std::this_thread::sleep_for(500ms);
    running.store(false);

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(read_errors.load(), 0) << "No read errors should occur";
    EXPECT_EQ(write_errors.load(), 0) << "No write errors should occur";
}

// Test 2: SIMD concurrent access
TEST_F(ContainerThreadSafetyTest, SIMDConcurrentAccess) {
#ifdef __AVX2__
    const int num_threads = 8;
    const int operations_per_thread = 500;
    const size_t data_size = 1024; // Must be multiple of 32 for AVX2

    std::vector<float> shared_data(data_size, 0.0f);
    std::atomic<int> errors{0};
    std::vector<std::thread> threads;

    simd_processor processor;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            std::vector<float> local_data(data_size);

            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    // Initialize local data
                    std::fill(local_data.begin(), local_data.end(),
                             static_cast<float>(thread_id * 100 + j));

                    // Process with SIMD
                    processor.process_batch(local_data.data(), data_size);

                    // Verify alignment
                    if (reinterpret_cast<uintptr_t>(local_data.data()) % 32 != 0) {
                        ++errors;
                    }
                } catch (...) {
                    ++errors;
                }

                if (j % 50 == 0) {
                    std::this_thread::sleep_for(1ms);
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(errors.load(), 0) << "No SIMD processing errors should occur";
#else
    GTEST_SKIP() << "AVX2 not available, skipping SIMD test";
#endif
}

// Test 3: Memory pool stress test
TEST_F(ContainerThreadSafetyTest, MemoryPoolStress) {
    const int num_threads = 12;
    const int allocations_per_thread = 1000;
    const size_t allocation_size = 64;

    memory_pool pool(allocation_size, 1000);
    std::vector<std::thread> threads;
    std::atomic<int> allocation_errors{0};
    std::atomic<int> deallocation_errors{0};

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            std::vector<void*> allocations;
            allocations.reserve(allocations_per_thread);

            // Allocate
            for (int j = 0; j < allocations_per_thread; ++j) {
                try {
                    void* ptr = pool.allocate();
                    if (ptr) {
                        allocations.push_back(ptr);
                    } else {
                        ++allocation_errors;
                    }
                } catch (...) {
                    ++allocation_errors;
                }

                if (j % 100 == 0) {
                    std::this_thread::sleep_for(1ms);
                }
            }

            // Deallocate
            for (void* ptr : allocations) {
                try {
                    pool.deallocate(ptr);
                } catch (...) {
                    ++deallocation_errors;
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(allocation_errors.load(), 0) << "No allocation errors should occur";
    EXPECT_EQ(deallocation_errors.load(), 0) << "No deallocation errors should occur";
}

// Test 4: Container resize during concurrent modifications
TEST_F(ContainerThreadSafetyTest, ContainerResize) {
    thread_safe_container<int, Value> container;

    const int num_threads = 10;
    const int operations_per_thread = 500;

    std::vector<std::thread> threads;
    std::atomic<int> errors{0};
    std::atomic<int> next_key{0};

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    int key = next_key.fetch_add(1);
                    container.insert(key, Value(key * 2));

                    // Occasional reads
                    if (key % 10 == 0 && key > 0) {
                        auto val = container.find(key - 5);
                    }
                } catch (...) {
                    ++errors;
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(errors.load(), 0);
    EXPECT_EQ(container.size(), num_threads * operations_per_thread);
}

// Test 5: Value serialization/deserialization concurrent access
TEST_F(ContainerThreadSafetyTest, ConcurrentSerialization) {
    const int num_threads = 8;
    const int operations_per_thread = 300;

    std::vector<std::thread> threads;
    std::atomic<int> serialize_errors{0};
    std::atomic<int> deserialize_errors{0};

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    // Create a value
                    Value original(thread_id * 1000 + j);

                    // Serialize
                    auto serialized = original.serialize();

                    // Deserialize
                    Value deserialized;
                    if (!deserialized.deserialize(serialized)) {
                        ++deserialize_errors;
                    }

                    // Verify
                    if (deserialized.as_int() != original.as_int()) {
                        ++serialize_errors;
                    }
                } catch (...) {
                    ++serialize_errors;
                }

                if (j % 50 == 0) {
                    std::this_thread::sleep_for(1ms);
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(serialize_errors.load(), 0);
    EXPECT_EQ(deserialize_errors.load(), 0);
}

// Test 6: Mixed operations stress test
TEST_F(ContainerThreadSafetyTest, MixedOperationsStress) {
    thread_safe_container<std::string, Value> container;

    const int num_threads = 15;
    const int operations_per_thread = 400;

    std::vector<std::thread> threads;
    std::atomic<int> errors{0};
    std::atomic<bool> running{true};

    // Pre-populate
    for (int i = 0; i < 50; ++i) {
        container.insert("init_" + std::to_string(i), Value(i));
    }

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            std::mt19937 rng(thread_id);
            std::uniform_int_distribution<int> op_dist(0, 4);
            std::uniform_int_distribution<int> key_dist(0, 99);

            for (int j = 0; j < operations_per_thread && running.load(); ++j) {
                try {
                    std::string key = "key_" + std::to_string(key_dist(rng));
                    int op = op_dist(rng);

                    switch (op) {
                        case 0: // Insert
                            container.insert(key, Value(thread_id * 1000 + j));
                            break;
                        case 1: // Find
                            container.find(key);
                            break;
                        case 2: // Erase
                            container.erase(key);
                            break;
                        case 3: // Update
                            container.insert_or_assign(key, Value(j));
                            break;
                        case 4: // Size check
                            container.size();
                            break;
                    }
                } catch (...) {
                    ++errors;
                }

                if (j % 50 == 0) {
                    std::this_thread::sleep_for(1ms);
                }
            }
        });
    }

    std::this_thread::sleep_for(300ms);
    running.store(false);

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(errors.load(), 0);
}

// Test 7: Container clear during operations
TEST_F(ContainerThreadSafetyTest, ClearDuringOperations) {
    thread_safe_container<int, Value> container;

    const int num_worker_threads = 8;
    const int num_clear_threads = 2;
    const int operations_per_thread = 500;

    std::vector<std::thread> threads;
    std::atomic<int> errors{0};
    std::atomic<bool> running{true};

    // Worker threads
    for (int i = 0; i < num_worker_threads; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            for (int j = 0; j < operations_per_thread && running.load(); ++j) {
                try {
                    container.insert(thread_id * 1000 + j, Value(j));
                    container.find(thread_id * 500 + j / 2);
                } catch (...) {
                    ++errors;
                }
            }
        });
    }

    // Clear threads
    for (int i = 0; i < num_clear_threads; ++i) {
        threads.emplace_back([&]() {
            std::this_thread::sleep_for(50ms);
            while (running.load()) {
                try {
                    container.clear();
                    std::this_thread::sleep_for(100ms);
                } catch (...) {
                    ++errors;
                }
            }
        });
    }

    std::this_thread::sleep_for(400ms);
    running.store(false);

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(errors.load(), 0);
}

// Test 8: Iterator safety during modifications
TEST_F(ContainerThreadSafetyTest, IteratorSafety) {
    thread_safe_container<int, Value> container;

    // Pre-populate
    for (int i = 0; i < 100; ++i) {
        container.insert(i, Value(i * 2));
    }

    const int num_iterator_threads = 5;
    const int num_modifier_threads = 3;
    const int iterations = 100;

    std::vector<std::thread> threads;
    std::atomic<int> errors{0};
    std::atomic<bool> running{true};

    // Iterator threads
    for (int i = 0; i < num_iterator_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iterations && running.load(); ++j) {
                try {
                    size_t count = 0;
                    container.for_each([&count](const auto& key, const auto& value) {
                        ++count;
                    });
                } catch (...) {
                    ++errors;
                }
                std::this_thread::sleep_for(5ms);
            }
        });
    }

    // Modifier threads
    for (int i = 0; i < num_modifier_threads; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            for (int j = 0; j < iterations && running.load(); ++j) {
                try {
                    container.insert(1000 + thread_id * 100 + j, Value(j));
                    container.erase(50 + (j % 50));
                } catch (...) {
                    ++errors;
                }
                std::this_thread::sleep_for(10ms);
            }
        });
    }

    std::this_thread::sleep_for(600ms);
    running.store(false);

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(errors.load(), 0);
}

// Test 9: High contention on single key
TEST_F(ContainerThreadSafetyTest, SingleKeyContention) {
    thread_safe_container<std::string, Value> container;
    const std::string hot_key = "hot_key";
    container.insert(hot_key, Value(0));

    const int num_threads = 20;
    const int operations_per_thread = 1000;

    std::vector<std::thread> threads;
    std::atomic<int> errors{0};
    std::barrier sync_point(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            sync_point.arrive_and_wait(); // All start together

            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    // Read
                    auto val = container.find(hot_key);

                    // Update
                    container.insert_or_assign(hot_key, Value(thread_id * 1000 + j));
                } catch (...) {
                    ++errors;
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(errors.load(), 0);
}

// Test 10: Memory safety - no leaks during concurrent operations
TEST_F(ContainerThreadSafetyTest, MemorySafetyTest) {
    const int num_iterations = 50;
    const int threads_per_iteration = 10;
    const int operations_per_thread = 100;

    std::atomic<int> total_errors{0};

    for (int iteration = 0; iteration < num_iterations; ++iteration) {
        thread_safe_container<int, Value> container;
        std::vector<std::thread> threads;

        for (int i = 0; i < threads_per_iteration; ++i) {
            threads.emplace_back([&, thread_id = i]() {
                for (int j = 0; j < operations_per_thread; ++j) {
                    try {
                        int key = thread_id * 1000 + j;
                        container.insert(key, Value(key * 2));
                        container.find(key);
                        if (j % 10 == 0) {
                            container.erase(key);
                        }
                    } catch (...) {
                        ++total_errors;
                    }
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        // Container destructor called here
    }

    EXPECT_EQ(total_errors.load(), 0);
}
