/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>
#include "test_compat.h"
#include "container/internal/memory_pool.h"
#include "container/internal/rcu_value.h"
#include "container/internal/epoch_manager.h"

#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <barrier>
#include <latch>
#include <algorithm>
#include <random>

using namespace container_module;
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
    auto container = std::make_shared<thread_safe_container>();

    const int num_readers = 10;
    const int num_writers = 5;
    const int operations_per_thread = 1000;
    const int num_keys = 100;

    std::vector<std::thread> threads;
    std::atomic<int> read_errors{0};
    std::atomic<int> write_errors{0};
    std::latch completion_latch(num_readers + num_writers);

    // Pre-populate container
    for (int i = 0; i < num_keys; ++i) {
        std::string key = "key_" + std::to_string(i);
        container->set(key, value(key, i));
    }

    // Writer threads
    for (int i = 0; i < num_writers; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            std::mt19937 rng(thread_id);
            std::uniform_int_distribution<int> dist(0, num_keys - 1);

            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    int key_idx = dist(rng);
                    std::string key = "key_" + std::to_string(key_idx);
                    container->set(key, value(key, thread_id * 1000 + j));
                } catch (...) {
                    ++write_errors;
                }

                if (j % 100 == 0) {
                    std::this_thread::yield();  // Yield instead of sleep for throttling
                }
            }
            completion_latch.count_down();
        });
    }

    // Reader threads
    for (int i = 0; i < num_readers; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            std::mt19937 rng(thread_id + 1000);
            std::uniform_int_distribution<int> dist(0, num_keys - 1);

            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    int key_idx = dist(rng);
                    std::string key = "key_" + std::to_string(key_idx);
                    auto value = container->get(key);
                    // Just access the value
                    if (!value.has_value()) {
                        ++read_errors;
                    }
                } catch (...) {
                    ++read_errors;
                }
            }
            completion_latch.count_down();
        });
    }

    // Wait for all threads to complete their operations
    completion_latch.wait();

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(read_errors.load(), 0) << "No read errors should occur";
    EXPECT_EQ(write_errors.load(), 0) << "No write errors should occur";
}

// Test 2: Memory pool stress test
TEST_F(ContainerThreadSafetyTest, MemoryPoolStress) {
    const int num_threads = 12;
    const int allocations_per_thread = 1000;
    const size_t allocation_size = 64;

    internal::fixed_block_pool pool(allocation_size, 1000);
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
                    std::this_thread::yield();  // Yield for throttling
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

// Test 3: Container bulk operations
TEST_F(ContainerThreadSafetyTest, BulkOperations) {
    auto container = std::make_shared<thread_safe_container>();

    const int num_threads = 10;
    const int operations_per_thread = 500;

    std::vector<std::thread> threads;
    std::atomic<int> errors{0};

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    // Bulk update
                    container->bulk_update([thread_id, j](auto& values) {
                        std::string key = "bulk_" + std::to_string(thread_id);
                        values[key] = value(key, j);
                    });
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
    EXPECT_EQ(container->size(), num_threads);
}

// Test 4: Mixed operations stress test
TEST_F(ContainerThreadSafetyTest, MixedOperationsStress) {
    auto container = std::make_shared<thread_safe_container>();

    const int num_threads = 15;
    const int operations_per_thread = 400;

    std::vector<std::thread> threads;
    std::atomic<int> errors{0};
    std::latch completion_latch(num_threads);

    // Pre-populate
    for (int i = 0; i < 50; ++i) {
        std::string key = "init_" + std::to_string(i);
        container->set(key, value(key, i));
    }

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            std::mt19937 rng(thread_id);
            std::uniform_int_distribution<int> op_dist(0, 4);
            std::uniform_int_distribution<int> key_dist(0, 99);

            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    std::string key = "key_" + std::to_string(key_dist(rng));
                    int op = op_dist(rng);

                    switch (op) {
                        case 0: // Set
                            container->set(key, value(key, thread_id * 1000 + j));
                            break;
                        case 1: // Get
                            container->get(key);
                            break;
                        case 2: // Remove
                            container->remove(key);
                            break;
                        case 3: // Contains
                            container->contains(key);
                            break;
                        case 4: // Size check
                            container->size();
                            break;
                    }
                } catch (...) {
                    ++errors;
                }

                if (j % 50 == 0) {
                    std::this_thread::yield();  // Yield instead of sleep
                }
            }
            completion_latch.count_down();
        });
    }

    completion_latch.wait();

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(errors.load(), 0);
}

// Test 5: Container clear during operations
TEST_F(ContainerThreadSafetyTest, ClearDuringOperations) {
    auto container = std::make_shared<thread_safe_container>();

    const int num_worker_threads = 8;
    const int num_clear_threads = 2;
    const int operations_per_thread = 500;

    std::vector<std::thread> threads;
    std::atomic<int> errors{0};
    std::atomic<bool> workers_done{false};
    std::latch worker_latch(num_worker_threads);

    // Worker threads
    for (int i = 0; i < num_worker_threads; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    std::string key = "worker_" + std::to_string(thread_id) + "_" + std::to_string(j);
                    container->set(key, value(key, j));
                    container->get(key);
                } catch (...) {
                    ++errors;
                }
            }
            worker_latch.count_down();
        });
    }

    // Clear threads - run until workers are done
    for (int i = 0; i < num_clear_threads; ++i) {
        threads.emplace_back([&]() {
            // Wait briefly for workers to start
            std::this_thread::yield();
            while (!workers_done.load()) {
                try {
                    container->clear();
                    std::this_thread::yield();  // Yield between clears
                } catch (...) {
                    ++errors;
                }
            }
        });
    }

    // Wait for workers to complete
    worker_latch.wait();
    workers_done.store(true);

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(errors.load(), 0);
}

// Test 6: Iterator safety with for_each
TEST_F(ContainerThreadSafetyTest, ForEachSafety) {
    auto container = std::make_shared<thread_safe_container>();

    // Pre-populate
    for (int i = 0; i < 100; ++i) {
        std::string key = "item_" + std::to_string(i);
        container->set(key, value(key, i * 2));
    }

    const int num_iterator_threads = 5;
    const int num_modifier_threads = 3;
    const int iterations = 100;

    std::vector<std::thread> threads;
    std::atomic<int> errors{0};
    std::latch completion_latch(num_iterator_threads + num_modifier_threads);

    // Iterator threads
    for (int i = 0; i < num_iterator_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iterations; ++j) {
                try {
                    size_t count = 0;
                    container->for_each([&count](const auto& key, const auto& value) {
                        ++count;
                        (void)key;
                        (void)value;
                    });
                } catch (...) {
                    ++errors;
                }
                std::this_thread::yield();  // Yield for iteration pacing
            }
            completion_latch.count_down();
        });
    }

    // Modifier threads
    for (int i = 0; i < num_modifier_threads; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            for (int j = 0; j < iterations; ++j) {
                try {
                    std::string key = "new_" + std::to_string(thread_id) + "_" + std::to_string(j);
                    container->set(key, value(key, j));
                    container->remove("item_" + std::to_string(50 + (j % 50)));
                } catch (...) {
                    ++errors;
                }
                std::this_thread::yield();  // Yield for modifier pacing
            }
            completion_latch.count_down();
        });
    }

    completion_latch.wait();

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(errors.load(), 0);
}

// Test 7: High contention on single key
TEST_F(ContainerThreadSafetyTest, SingleKeyContention) {
    auto container = std::make_shared<thread_safe_container>();
    const std::string hot_key = "hot_key";
    container->set(hot_key, value(hot_key, 0));

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
                    auto val = container->get(hot_key);

                    // Update
                    container->set(hot_key, value(hot_key, thread_id * 1000 + j));
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

// Test 8: Typed get/set operations
TEST_F(ContainerThreadSafetyTest, TypedOperations) {
    auto container = std::make_shared<thread_safe_container>();

    const int num_threads = 8;
    const int operations_per_thread = 500;

    std::vector<std::thread> threads;
    std::atomic<int> errors{0};

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    std::string key = "typed_" + std::to_string(thread_id);

                    // Set different types
                    if (j % 3 == 0) {
                        container->set_typed(key, j);
                    } else if (j % 3 == 1) {
                        container->set_typed(key, static_cast<double>(j) * 1.5);
                    } else {
                        container->set_typed(key, std::string("value_" + std::to_string(j)));
                    }

                    // Get with type check
                    auto val = container->get(key);
                    if (!val.has_value()) {
                        ++errors;
                    }
                } catch (...) {
                    ++errors;
                }

                if (j % 50 == 0) {
                    std::this_thread::yield();  // Yield for throttling
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(errors.load(), 0);
}

// Test 9: Keys() operation during modifications
TEST_F(ContainerThreadSafetyTest, KeysOperationConcurrent) {
    auto container = std::make_shared<thread_safe_container>();

    // Pre-populate
    for (int i = 0; i < 50; ++i) {
        std::string key = "initial_" + std::to_string(i);
        container->set(key, value(key, i));
    }

    const int num_reader_threads = 5;
    const int num_writer_threads = 5;
    const int operations_per_thread = 200;

    std::vector<std::thread> threads;
    std::atomic<int> errors{0};
    std::latch completion_latch(num_reader_threads + num_writer_threads);

    // Reader threads calling keys()
    for (int i = 0; i < num_reader_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    auto all_keys = container->keys();
                    // Process keys
                    for (const auto& key : all_keys) {
                        (void)key;
                    }
                } catch (...) {
                    ++errors;
                }
                std::this_thread::yield();  // Yield for pacing
            }
            completion_latch.count_down();
        });
    }

    // Writer threads modifying container
    for (int i = 0; i < num_writer_threads; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    std::string key = "dynamic_" + std::to_string(thread_id) + "_" + std::to_string(j);
                    container->set(key, value(key, j));

                    if (j % 10 == 0) {
                        container->remove("initial_" + std::to_string(j % 50));
                    }
                } catch (...) {
                    ++errors;
                }
                std::this_thread::yield();  // Yield for pacing
            }
            completion_latch.count_down();
        });
    }

    completion_latch.wait();

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
        auto container = std::make_shared<thread_safe_container>();
        std::vector<std::thread> threads;

        for (int i = 0; i < threads_per_iteration; ++i) {
            threads.emplace_back([&, thread_id = i]() {
                for (int j = 0; j < operations_per_thread; ++j) {
                    try {
                        std::string key = "iter_" + std::to_string(iteration) +
                                        "_thread_" + std::to_string(thread_id) +
                                        "_op_" + std::to_string(j);
                        container->set(key, value(key, j * 2));
                        container->get(key);
                        if (j % 10 == 0) {
                            container->remove(key);
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

// Test 11: Nested container storage and retrieval
TEST_F(ContainerThreadSafetyTest, NestedContainerStorage) {
    auto inner = std::make_shared<thread_safe_container>();
    inner->set_typed("inner_key", 42);
    inner->set_typed("inner_string", std::string("nested value"));

    thread_safe_container outer;
    outer.set_container("nested", inner);

    auto retrieved = outer.get_container("nested");
    ASSERT_NE(retrieved, nullptr);

    auto val = retrieved->get_variant("inner_key");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val->get<int32_t>().value(), 42);

    auto str_val = retrieved->get_variant("inner_string");
    ASSERT_TRUE(str_val.has_value());
    EXPECT_EQ(str_val->get<std::string>().value(), "nested value");
}

// Test 12: Recursive serialization (3-level nested container)
TEST_F(ContainerThreadSafetyTest, RecursiveSerialization) {
    // Create 3-level nested container
    auto level3 = std::make_shared<thread_safe_container>();
    level3->set_typed("data", std::string("deepest"));
    level3->set_typed("depth", 3);

    auto level2 = std::make_shared<thread_safe_container>();
    level2->set_container("child", level3);
    level2->set_typed("depth", 2);

    auto level1 = std::make_shared<thread_safe_container>();
    level1->set_container("child", level2);
    level1->set_typed("depth", 1);

    // Serialize
    auto bytes = level1->serialize();
    EXPECT_GT(bytes.size(), 0);

    // Deserialize
    auto restored = thread_safe_container::deserialize(bytes);
    ASSERT_NE(restored, nullptr);

    // Verify depth 1
    auto depth1_val = restored->get_variant("depth");
    ASSERT_TRUE(depth1_val.has_value());
    EXPECT_EQ(depth1_val->get<int32_t>().value(), 1);

    // Verify depth 2
    auto l2 = restored->get_container("child");
    ASSERT_NE(l2, nullptr);
    auto depth2_val = l2->get_variant("depth");
    ASSERT_TRUE(depth2_val.has_value());
    EXPECT_EQ(depth2_val->get<int32_t>().value(), 2);

    // Verify depth 3
    auto l3 = l2->get_container("child");
    ASSERT_NE(l3, nullptr);
    auto depth3_val = l3->get_variant("depth");
    ASSERT_TRUE(depth3_val.has_value());
    EXPECT_EQ(depth3_val->get<int32_t>().value(), 3);

    auto data = l3->get_variant("data");
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(data->get<std::string>().value(), "deepest");
}

// Test 13: Concurrent nested container access
TEST_F(ContainerThreadSafetyTest, ConcurrentNestedAccess) {
    auto container = std::make_shared<thread_safe_container>();

    const int num_threads = 10;
    std::vector<std::thread> threads;
    std::atomic<int> errors{0};

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&container, &errors, i]() {
            try {
                auto nested = std::make_shared<thread_safe_container>();
                nested->set_typed("id", i);
                nested->set_typed("name", std::string("thread_" + std::to_string(i)));
                container->set_container("nested_" + std::to_string(i), nested);
            } catch (...) {
                ++errors;
            }
        });
    }

    for (auto& t : threads) t.join();

    EXPECT_EQ(errors.load(), 0);

    // Verify all nested containers
    for (int i = 0; i < num_threads; ++i) {
        auto nested = container->get_container("nested_" + std::to_string(i));
        ASSERT_NE(nested, nullptr) << "Nested container " << i << " not found";

        auto id_val = nested->get_variant("id");
        ASSERT_TRUE(id_val.has_value());
        EXPECT_EQ(id_val->get<int32_t>().value(), i);
    }
}

// Test 14: Container value round-trip serialization
TEST_F(ContainerThreadSafetyTest, ContainerValueRoundTrip) {
    auto container = std::make_shared<thread_safe_container>();
    container->set_typed("key1", 123);
    container->set_typed("key2", std::string("value"));
    container->set_typed("key3", 3.14159);

    value val("nested", container);

    auto bytes = val.serialize();
    EXPECT_GT(bytes.size(), 0);

    auto restored = value::deserialize(bytes);
    ASSERT_TRUE(restored.has_value());
    EXPECT_EQ(restored->type(), value_types::container_value);

    auto restored_container = restored->get<std::shared_ptr<thread_safe_container>>();
    ASSERT_TRUE(restored_container.has_value());
    ASSERT_NE(restored_container.value(), nullptr);

    auto key1_val = restored_container.value()->get_variant("key1");
    ASSERT_TRUE(key1_val.has_value());
    EXPECT_EQ(key1_val->get<int32_t>().value(), 123);

    auto key2_val = restored_container.value()->get_variant("key2");
    ASSERT_TRUE(key2_val.has_value());
    EXPECT_EQ(key2_val->get<std::string>().value(), "value");
}

// Test 15: Circular reference prevention
TEST_F(ContainerThreadSafetyTest, CircularReferencePrevention) {
    auto container1 = std::make_shared<thread_safe_container>();
    auto container2 = std::make_shared<thread_safe_container>();

    container1->set_typed("name", std::string("container1"));
    container2->set_typed("name", std::string("container2"));

    // Create circular reference: container1 -> container2 -> container1
    container1->set_container("ref", container2);
    container2->set_container("ref", container1);

    // Serialization should not hang (circular reference is handled)
    auto bytes = container1->serialize();

    // Should complete without hanging
    EXPECT_GT(bytes.size(), 0);

    // Deserialize and verify structure is preserved (circular part becomes null)
    auto restored = thread_safe_container::deserialize(bytes);
    ASSERT_NE(restored, nullptr);

    auto name_val = restored->get_variant("name");
    ASSERT_TRUE(name_val.has_value());
    EXPECT_EQ(name_val->get<std::string>().value(), "container1");
}

// Test 16: set_variant and get_variant API
TEST_F(ContainerThreadSafetyTest, SetGetVariantAPI) {
    thread_safe_container container;

    // Create value and set using set_variant
    value v1("test_key", 42);
    container.set_variant(v1);

    // Get using get_variant
    auto retrieved = container.get_variant("test_key");
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->get<int32_t>().value(), 42);

    // Test with string value
    value v2("string_key", std::string("hello world"));
    container.set_variant(v2);

    auto str_retrieved = container.get_variant("string_key");
    ASSERT_TRUE(str_retrieved.has_value());
    EXPECT_EQ(str_retrieved->get<std::string>().value(), "hello world");

    // Test get_variant with non-existent key
    auto missing = container.get_variant("non_existent");
    EXPECT_FALSE(missing.has_value());
}

// Test 17: Lock-free container reader basic operations
TEST_F(ContainerThreadSafetyTest, LockFreeReaderBasicOperations) {
    auto container = std::make_shared<thread_safe_container>();

    // Populate container
    container->set_typed("int_key", 42);
    container->set_typed("string_key", std::string("hello"));
    container->set_typed("double_key", 3.14159);

    // Create lock-free reader
    lockfree_container_reader reader(container);

    // Verify lock-free reads
    auto int_val = reader.get<int32_t>("int_key");
    ASSERT_TRUE(int_val.has_value());
    EXPECT_EQ(int_val.value(), 42);

    auto str_val = reader.get<std::string>("string_key");
    ASSERT_TRUE(str_val.has_value());
    EXPECT_EQ(str_val.value(), "hello");

    auto dbl_val = reader.get<double>("double_key");
    ASSERT_TRUE(dbl_val.has_value());
    EXPECT_NEAR(dbl_val.value(), 3.14159, 0.0001);

    // Test contains
    EXPECT_TRUE(reader.contains("int_key"));
    EXPECT_FALSE(reader.contains("non_existent"));

    // Test size
    EXPECT_EQ(reader.size(), 3);

    // Test empty
    EXPECT_FALSE(reader.empty());

    // Test keys
    auto keys = reader.keys();
    EXPECT_EQ(keys.size(), 3);
}

// Test 18: Lock-free reader refresh functionality
TEST_F(ContainerThreadSafetyTest, LockFreeReaderRefresh) {
    auto container = std::make_shared<thread_safe_container>();
    container->set_typed("key1", 100);

    lockfree_container_reader reader(container);

    // Initial read
    auto val1 = reader.get<int32_t>("key1");
    ASSERT_TRUE(val1.has_value());
    EXPECT_EQ(val1.value(), 100);

    // Modify container
    container->set_typed("key1", 200);
    container->set_typed("key2", 300);

    // Reader still sees old snapshot
    auto val1_stale = reader.get<int32_t>("key1");
    ASSERT_TRUE(val1_stale.has_value());
    EXPECT_EQ(val1_stale.value(), 100);

    auto val2_missing = reader.get<int32_t>("key2");
    EXPECT_FALSE(val2_missing.has_value());

    // Refresh and verify new values
    reader.refresh();
    EXPECT_GE(reader.refresh_count(), 2);

    auto val1_new = reader.get<int32_t>("key1");
    ASSERT_TRUE(val1_new.has_value());
    EXPECT_EQ(val1_new.value(), 200);

    auto val2_new = reader.get<int32_t>("key2");
    ASSERT_TRUE(val2_new.has_value());
    EXPECT_EQ(val2_new.value(), 300);
}

// Test 19: Lock-free reader concurrent reads (no locking)
TEST_F(ContainerThreadSafetyTest, LockFreeReaderConcurrentReads) {
    auto container = std::make_shared<thread_safe_container>();

    // Populate container
    for (int i = 0; i < 100; ++i) {
        std::string key = "key_" + std::to_string(i);
        container->set(key, value(key, i * 2));
    }

    lockfree_container_reader reader(container);

    const int num_readers = 50;
    const int reads_per_thread = 10000;

    std::vector<std::thread> threads;
    std::atomic<int> errors{0};
    std::atomic<size_t> total_reads{0};
    std::barrier sync_point(num_readers);

    for (int i = 0; i < num_readers; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            sync_point.arrive_and_wait();  // All threads start together

            std::mt19937 rng(thread_id);
            std::uniform_int_distribution<int> dist(0, 99);

            for (int j = 0; j < reads_per_thread; ++j) {
                try {
                    int key_idx = dist(rng);
                    std::string key = "key_" + std::to_string(key_idx);

                    // Lock-free read!
                    auto val = reader.get<int32_t>(key);
                    if (val.has_value() && val.value() != key_idx * 2) {
                        ++errors;
                    }
                    total_reads.fetch_add(1, std::memory_order_relaxed);
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
    EXPECT_EQ(total_reads.load(), num_readers * reads_per_thread);
}

// Test 20: Lock-free reader with concurrent writes to source
TEST_F(ContainerThreadSafetyTest, LockFreeReaderConcurrentWithWrites) {
    auto container = std::make_shared<thread_safe_container>();

    // Pre-populate
    for (int i = 0; i < 50; ++i) {
        std::string key = "key_" + std::to_string(i);
        container->set(key, value(key, i));
    }

    lockfree_container_reader reader(container);

    const int num_readers = 20;
    const int num_writers = 5;
    const int num_refreshers = 2;
    const int operations_per_thread = 2000;

    std::vector<std::thread> threads;
    std::atomic<int> errors{0};
    std::atomic<bool> done{false};
    std::latch completion_latch(num_readers + num_writers);

    // Reader threads (lock-free reads)
    for (int i = 0; i < num_readers; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            std::mt19937 rng(thread_id);
            std::uniform_int_distribution<int> dist(0, 49);

            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    int key_idx = dist(rng);
                    std::string key = "key_" + std::to_string(key_idx);

                    // Lock-free read - should never throw or crash
                    auto val = reader.get<int32_t>(key);
                    // Value may or may not exist depending on refresh state
                    (void)val;
                } catch (...) {
                    ++errors;
                }
            }
            completion_latch.count_down();
        });
    }

    // Writer threads (modify source container)
    for (int i = 0; i < num_writers; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            std::mt19937 rng(thread_id + 100);
            std::uniform_int_distribution<int> dist(0, 49);

            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    int key_idx = dist(rng);
                    std::string key = "key_" + std::to_string(key_idx);
                    container->set(key, value(key, thread_id * 10000 + j));
                } catch (...) {
                    ++errors;
                }

                if (j % 100 == 0) {
                    std::this_thread::yield();
                }
            }
            completion_latch.count_down();
        });
    }

    // Refresher threads
    for (int i = 0; i < num_refreshers; ++i) {
        threads.emplace_back([&reader, &done, &errors]() {
            while (!done.load()) {
                try {
                    reader.refresh();
                } catch (...) {
                    ++errors;
                }
                std::this_thread::yield();
            }
        });
    }

    // Wait for readers and writers to complete
    completion_latch.wait();
    done.store(true);

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(errors.load(), 0);
    EXPECT_GT(reader.refresh_count(), 1);
}

// Test 21: Lock-free reader for_each iteration
TEST_F(ContainerThreadSafetyTest, LockFreeReaderForEach) {
    auto container = std::make_shared<thread_safe_container>();

    // Populate
    for (int i = 0; i < 100; ++i) {
        std::string key = "item_" + std::to_string(i);
        container->set(key, value(key, i * 3));
    }

    lockfree_container_reader reader(container);

    // Iterate using for_each
    size_t count = 0;
    int64_t sum = 0;
    reader.for_each([&count, &sum](const std::string& key, const value& val) {
        ++count;
        auto int_val = val.get<int32_t>();
        if (int_val.has_value()) {
            sum += int_val.value();
        }
        (void)key;
    });

    EXPECT_EQ(count, 100);
    // Sum of 0*3 + 1*3 + 2*3 + ... + 99*3 = 3 * (99 * 100 / 2) = 14850
    EXPECT_EQ(sum, 14850);
}

// Test 22: Lock-free reader with factory method
TEST_F(ContainerThreadSafetyTest, LockFreeReaderFactoryMethod) {
    auto container = std::make_shared<thread_safe_container>();
    container->set_typed("test", 42);

    // Create reader using factory method
    auto reader = container->create_lockfree_reader();
    ASSERT_NE(reader, nullptr);

    auto val = reader->get<int32_t>("test");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), 42);

    // Verify source is correct
    EXPECT_EQ(reader->source(), container);
}

// Test 23: RCU value basic operations
TEST_F(ContainerThreadSafetyTest, RcuValueBasicOperations) {
    rcu_value<int> counter{0};

    // Read initial value
    auto snapshot = counter.read();
    ASSERT_NE(snapshot, nullptr);
    EXPECT_EQ(*snapshot, 0);

    // Update value
    counter.update(42);
    EXPECT_EQ(counter.update_count(), 1);

    // Read new value
    auto new_snapshot = counter.read();
    ASSERT_NE(new_snapshot, nullptr);
    EXPECT_EQ(*new_snapshot, 42);

    // Old snapshot still valid
    EXPECT_EQ(*snapshot, 0);
}

// Test 24: RCU value concurrent operations
TEST_F(ContainerThreadSafetyTest, RcuValueConcurrentOperations) {
    rcu_value<std::string> data{"initial"};

    const int num_readers = 30;
    const int num_writers = 5;
    const int operations_per_thread = 5000;

    std::vector<std::thread> threads;
    std::atomic<int> errors{0};
    std::barrier sync_point(num_readers + num_writers);

    // Reader threads
    for (int i = 0; i < num_readers; ++i) {
        threads.emplace_back([&]() {
            sync_point.arrive_and_wait();

            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    auto snapshot = data.read();
                    // Just access the data
                    if (snapshot && snapshot->empty()) {
                        // Valid but empty string is unexpected in this test
                    }
                } catch (...) {
                    ++errors;
                }
            }
        });
    }

    // Writer threads
    for (int i = 0; i < num_writers; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            sync_point.arrive_and_wait();

            for (int j = 0; j < operations_per_thread; ++j) {
                try {
                    data.update("thread_" + std::to_string(thread_id) + "_" + std::to_string(j));
                } catch (...) {
                    ++errors;
                }

                if (j % 100 == 0) {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(errors.load(), 0);
    EXPECT_GT(data.update_count(), 0);
}

// Test 25: RCU value compare and update
TEST_F(ContainerThreadSafetyTest, RcuValueCompareAndUpdate) {
    rcu_value<int> counter{0};

    // Successful CAS
    auto expected = counter.read();
    EXPECT_TRUE(counter.compare_and_update(expected, 1));
    EXPECT_EQ(*counter.read(), 1);

    // Failed CAS (expected doesn't match)
    EXPECT_FALSE(counter.compare_and_update(expected, 2));
    EXPECT_EQ(*counter.read(), 1);

    // Successful CAS with new expected
    expected = counter.read();
    EXPECT_TRUE(counter.compare_and_update(expected, 100));
    EXPECT_EQ(*counter.read(), 100);
}

// Test 26: Epoch manager basic operations
TEST_F(ContainerThreadSafetyTest, EpochManagerBasicOperations) {
    epoch_manager& em = epoch_manager::instance();

    // Enter and exit critical section
    EXPECT_FALSE(em.in_critical_section());
    em.enter_critical();
    EXPECT_TRUE(em.in_critical_section());
    em.exit_critical();
    EXPECT_FALSE(em.in_critical_section());
}

// Test 27: Epoch guard RAII
TEST_F(ContainerThreadSafetyTest, EpochGuardRAII) {
    epoch_manager& em = epoch_manager::instance();

    EXPECT_FALSE(em.in_critical_section());

    {
        epoch_guard guard;
        EXPECT_TRUE(em.in_critical_section());
    }

    EXPECT_FALSE(em.in_critical_section());
}

// Test 28: Epoch manager deferred deletion
TEST_F(ContainerThreadSafetyTest, EpochManagerDeferredDeletion) {
    epoch_manager& em = epoch_manager::instance();

    std::atomic<int> delete_count{0};

    // Defer deletion of some objects
    for (int i = 0; i < 10; ++i) {
        int* ptr = new int(i);
        em.defer_delete(ptr, [&delete_count](void* p) {
            delete static_cast<int*>(p);
            ++delete_count;
        });
    }

    EXPECT_GT(em.pending_count(), 0);

    // Advance epochs and trigger GC
    for (int i = 0; i < 5; ++i) {
        em.try_gc();
    }

    // Force GC to clean up any remaining
    em.force_gc();

    // All should be deleted
    EXPECT_EQ(delete_count.load(), 10);
}

// =============================================================================
// Auto-Refresh Reader Tests (Issue #262)
// =============================================================================

TEST(AutoRefreshReaderTest, BasicConstruction) {
    auto container = std::make_shared<thread_safe_container>();
    container->set("key1", value("key1", 42));

    auto reader = container->create_auto_refresh_reader(std::chrono::milliseconds(50));

    EXPECT_TRUE(reader->is_running());
    EXPECT_EQ(reader->refresh_interval(), std::chrono::milliseconds(50));
    EXPECT_EQ(reader->size(), 1U);

    auto val = reader->get<int32_t>("key1");
    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(*val, 42);
}

TEST(AutoRefreshReaderTest, AutoRefreshUpdatesValues) {
    auto container = std::make_shared<thread_safe_container>();
    container->set("counter", value("counter", 0));

    auto reader = container->create_auto_refresh_reader(std::chrono::milliseconds(20));

    // Initial value
    auto initial = reader->get<int32_t>("counter");
    EXPECT_TRUE(initial.has_value());
    EXPECT_EQ(*initial, 0);

    // Update container
    container->set("counter", value("counter", 100));

    // Wait for auto-refresh to catch the update
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    auto updated = reader->get<int32_t>("counter");
    EXPECT_TRUE(updated.has_value());
    EXPECT_EQ(*updated, 100);
}

TEST(AutoRefreshReaderTest, StopAndRestart) {
    auto container = std::make_shared<thread_safe_container>();
    container->set("key", value("key", std::string("test")));

    auto reader = container->create_auto_refresh_reader(std::chrono::milliseconds(10));

    EXPECT_TRUE(reader->is_running());
    size_t count_before = reader->refresh_count();

    // Wait for some refreshes
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_GT(reader->refresh_count(), count_before);

    reader->stop();
    EXPECT_FALSE(reader->is_running());

    size_t count_after_stop = reader->refresh_count();

    // No more refreshes after stop
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    EXPECT_EQ(reader->refresh_count(), count_after_stop);

    // Values still accessible after stop
    auto val = reader->get<std::string>("key");
    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(*val, "test");
}

TEST(AutoRefreshReaderTest, ManualRefreshWhileAutoRunning) {
    auto container = std::make_shared<thread_safe_container>();
    container->set("value", value("value", 1));

    auto reader = container->create_auto_refresh_reader(std::chrono::milliseconds(100));

    size_t initial_count = reader->refresh_count();

    // Manual refresh should work
    reader->refresh();
    EXPECT_EQ(reader->refresh_count(), initial_count + 1);

    // Update container
    container->set("value", value("value", 999));

    // Manual refresh picks up change immediately
    reader->refresh();
    auto val = reader->get<int32_t>("value");
    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(*val, 999);
}

TEST(AutoRefreshReaderTest, ConcurrentReads) {
    auto container = std::make_shared<thread_safe_container>();
    for (int i = 0; i < 100; ++i) {
        std::string key = "key" + std::to_string(i);
        container->set(key, value(key, i));
    }

    auto reader = container->create_auto_refresh_reader(std::chrono::milliseconds(10));

    std::atomic<int> successful_reads{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < 10; ++t) {
        threads.emplace_back([&reader, &successful_reads]() {
            for (int i = 0; i < 1000; ++i) {
                int key_idx = i % 100;
                auto val = reader->get<int32_t>("key" + std::to_string(key_idx));
                if (val.has_value()) {
                    successful_reads.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(successful_reads.load(), 10000);
}

TEST(AutoRefreshReaderTest, ContainerMethodDelegation) {
    auto container = std::make_shared<thread_safe_container>();
    container->set("a", value("a", 1));
    container->set("b", value("b", 2));
    container->set("c", value("c", 3));

    auto reader = container->create_auto_refresh_reader(std::chrono::milliseconds(100));

    // Test all delegated methods
    EXPECT_EQ(reader->size(), 3U);
    EXPECT_FALSE(reader->empty());
    EXPECT_TRUE(reader->contains("a"));
    EXPECT_TRUE(reader->contains("b"));
    EXPECT_TRUE(reader->contains("c"));
    EXPECT_FALSE(reader->contains("nonexistent"));

    auto keys = reader->keys();
    EXPECT_EQ(keys.size(), 3U);

    int sum = 0;
    reader->for_each([&sum](const std::string& key, const value& val) {
        auto v = val.get<int32_t>();
        if (v.has_value()) {
            sum += static_cast<int>(*v);
        }
    });
    EXPECT_EQ(sum, 6);  // 1 + 2 + 3

    // Access underlying reader
    auto underlying = reader->reader();
    EXPECT_NE(underlying, nullptr);

    // Access source container
    auto source = reader->source();
    EXPECT_EQ(source, container);
}
