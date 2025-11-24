/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>
#include "test_compat.h"
#include "container/internal/memory_pool.h"

#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <barrier>
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
    std::atomic<bool> running{true};

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

            for (int j = 0; j < operations_per_thread && running.load(); ++j) {
                try {
                    int key_idx = dist(rng);
                    std::string key = "key_" + std::to_string(key_idx);
                    container->set(key, value(key, thread_id * 1000 + j));
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
                    auto value = container->get(key);
                    // Just access the value
                    if (!value.has_value()) {
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
    std::atomic<bool> running{true};

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

            for (int j = 0; j < operations_per_thread && running.load(); ++j) {
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

// Test 5: Container clear during operations
TEST_F(ContainerThreadSafetyTest, ClearDuringOperations) {
    auto container = std::make_shared<thread_safe_container>();

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
                    std::string key = "worker_" + std::to_string(thread_id) + "_" + std::to_string(j);
                    container->set(key, value(key, j));
                    container->get(key);
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
                    container->clear();
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
    std::atomic<bool> running{true};

    // Iterator threads
    for (int i = 0; i < num_iterator_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iterations && running.load(); ++j) {
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
                std::this_thread::sleep_for(5ms);
            }
        });
    }

    // Modifier threads
    for (int i = 0; i < num_modifier_threads; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            for (int j = 0; j < iterations && running.load(); ++j) {
                try {
                    std::string key = "new_" + std::to_string(thread_id) + "_" + std::to_string(j);
                    container->set(key, value(key, j));
                    container->remove("item_" + std::to_string(50 + (j % 50)));
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
                    std::this_thread::sleep_for(1ms);
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
    std::atomic<bool> running{true};

    // Reader threads calling keys()
    for (int i = 0; i < num_reader_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < operations_per_thread && running.load(); ++j) {
                try {
                    auto all_keys = container->keys();
                    // Process keys
                    for (const auto& key : all_keys) {
                        (void)key;
                    }
                } catch (...) {
                    ++errors;
                }
                std::this_thread::sleep_for(5ms);
            }
        });
    }

    // Writer threads modifying container
    for (int i = 0; i < num_writer_threads; ++i) {
        threads.emplace_back([&, thread_id = i]() {
            for (int j = 0; j < operations_per_thread && running.load(); ++j) {
                try {
                    std::string key = "dynamic_" + std::to_string(thread_id) + "_" + std::to_string(j);
                    container->set(key, value(key, j));

                    if (j % 10 == 0) {
                        container->remove("initial_" + std::to_string(j % 50));
                    }
                } catch (...) {
                    ++errors;
                }
                std::this_thread::sleep_for(3ms);
            }
        });
    }

    std::this_thread::sleep_for(1000ms);
    running.store(false);

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
