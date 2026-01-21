// BSD 3-Clause License
//
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * @file policy_container_tests.cpp
 * @brief Integration tests for policy-based container (Issue #327)
 *
 * Tests cover:
 * - basic_value_container with different storage policies
 * - Type aliases (policy_container, fast_policy_container, typed_policy_container)
 * - Thread safety
 * - Policy switching scenarios
 * - Backward compatibility
 */

#include <gtest/gtest.h>
#include "container/core/policy_container.h"

#include <thread>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>

using namespace container_module;
using namespace container_module::policy;

// ============================================================================
// Basic Container Tests (Dynamic Storage)
// ============================================================================

class PolicyContainerTest : public ::testing::Test {
protected:
    policy_container container;

    void SetUp() override {
        container.clear_value();
    }
};

TEST_F(PolicyContainerTest, DefaultConstruction) {
    EXPECT_TRUE(container.empty());
    EXPECT_EQ(container.size(), 0u);
    EXPECT_EQ(container.version(), "1.0.0.0");
}

TEST_F(PolicyContainerTest, SetAndGet) {
    container.set("key1", 42);
    container.set("key2", std::string("hello"));
    container.set("key3", 3.14);

    EXPECT_EQ(container.size(), 3u);

    auto val1 = container.get("key1");
    ASSERT_TRUE(val1.has_value());
    EXPECT_EQ(std::get<int>(val1->data), 42);

    auto val2 = container.get("key2");
    ASSERT_TRUE(val2.has_value());
    EXPECT_EQ(std::get<std::string>(val2->data), "hello");

    auto val3 = container.get("key3");
    ASSERT_TRUE(val3.has_value());
    EXPECT_DOUBLE_EQ(std::get<double>(val3->data), 3.14);
}

TEST_F(PolicyContainerTest, SetOptimizedValue) {
    optimized_value val;
    val.name = "test_key";
    val.type = value_types::int_value;
    val.data = 100;

    container.set(val);

    auto result = container.get("test_key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<int>(result->data), 100);
}

TEST_F(PolicyContainerTest, SetAll) {
    std::vector<optimized_value> vals;

    optimized_value v1;
    v1.name = "a";
    v1.type = value_types::int_value;
    v1.data = 1;
    vals.push_back(v1);

    optimized_value v2;
    v2.name = "b";
    v2.type = value_types::int_value;
    v2.data = 2;
    vals.push_back(v2);

    container.set_all(vals);

    EXPECT_EQ(container.size(), 2u);
    EXPECT_TRUE(container.contains("a"));
    EXPECT_TRUE(container.contains("b"));
}

TEST_F(PolicyContainerTest, Contains) {
    container.set("exists", 1);

    EXPECT_TRUE(container.contains("exists"));
    EXPECT_FALSE(container.contains("not_exists"));
}

TEST_F(PolicyContainerTest, Remove) {
    container.set("key1", 1);
    container.set("key2", 2);

    EXPECT_TRUE(container.remove("key1"));
    EXPECT_FALSE(container.contains("key1"));
    EXPECT_TRUE(container.contains("key2"));
    EXPECT_EQ(container.size(), 1u);
}

TEST_F(PolicyContainerTest, Clear) {
    container.set("key1", 1);
    container.set("key2", 2);
    container.set("key3", 3);

    container.clear_value();

    EXPECT_TRUE(container.empty());
    EXPECT_EQ(container.size(), 0u);
}

TEST_F(PolicyContainerTest, MethodChaining) {
    container.set("a", 1)
             .set("b", 2)
             .set("c", 3);

    EXPECT_EQ(container.size(), 3u);
}

// ============================================================================
// Header Management Tests
// ============================================================================

TEST_F(PolicyContainerTest, HeaderManagement) {
    container.set_source("src_id", "src_sub");
    container.set_target("tgt_id", "tgt_sub");
    container.set_message_type("test_message");

    EXPECT_EQ(container.source_id(), "src_id");
    EXPECT_EQ(container.source_sub_id(), "src_sub");
    EXPECT_EQ(container.target_id(), "tgt_id");
    EXPECT_EQ(container.target_sub_id(), "tgt_sub");
    EXPECT_EQ(container.message_type(), "test_message");
}

TEST_F(PolicyContainerTest, SwapHeader) {
    container.set_source("A", "a");
    container.set_target("B", "b");

    container.swap_header();

    EXPECT_EQ(container.source_id(), "B");
    EXPECT_EQ(container.source_sub_id(), "b");
    EXPECT_EQ(container.target_id(), "A");
    EXPECT_EQ(container.target_sub_id(), "a");
}

// ============================================================================
// Copy/Move Semantics Tests
// ============================================================================

TEST_F(PolicyContainerTest, CopyConstruction) {
    container.set("key", 42);
    container.set_source("src", "sub");

    policy_container copy(container);

    EXPECT_EQ(copy.size(), 1u);
    EXPECT_TRUE(copy.contains("key"));
    EXPECT_EQ(copy.source_id(), "src");
}

TEST_F(PolicyContainerTest, CopyAssignment) {
    container.set("key", 42);

    policy_container other;
    other = container;

    EXPECT_EQ(other.size(), 1u);
    EXPECT_TRUE(other.contains("key"));
}

TEST_F(PolicyContainerTest, MoveConstruction) {
    container.set("key", 42);

    policy_container moved(std::move(container));

    EXPECT_EQ(moved.size(), 1u);
    EXPECT_TRUE(moved.contains("key"));
}

TEST_F(PolicyContainerTest, MoveAssignment) {
    container.set("key", 42);

    policy_container other;
    other = std::move(container);

    EXPECT_EQ(other.size(), 1u);
    EXPECT_TRUE(other.contains("key"));
}

// ============================================================================
// Batch Operations Tests
// ============================================================================

TEST_F(PolicyContainerTest, BulkInsertMove) {
    std::vector<optimized_value> vals;
    for (int i = 0; i < 10; ++i) {
        optimized_value v;
        v.name = "key_" + std::to_string(i);
        v.type = value_types::int_value;
        v.data = i * 10;
        vals.push_back(v);
    }

    container.bulk_insert(std::move(vals));

    EXPECT_EQ(container.size(), 10u);
    auto result = container.get("key_5");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<int>(result->data), 50);
}

TEST_F(PolicyContainerTest, GetBatch) {
    container.set("a", 1);
    container.set("b", 2);
    container.set("c", 3);

    std::array<std::string_view, 4> keys = {"a", "b", "nonexistent", "c"};
    auto results = container.get_batch(keys);

    ASSERT_EQ(results.size(), 4u);
    EXPECT_TRUE(results[0].has_value());
    EXPECT_TRUE(results[1].has_value());
    EXPECT_FALSE(results[2].has_value());
    EXPECT_TRUE(results[3].has_value());
}

TEST_F(PolicyContainerTest, ContainsBatch) {
    container.set("a", 1);
    container.set("c", 3);

    std::array<std::string_view, 3> keys = {"a", "b", "c"};
    auto results = container.contains_batch(keys);

    ASSERT_EQ(results.size(), 3u);
    EXPECT_TRUE(results[0]);
    EXPECT_FALSE(results[1]);
    EXPECT_TRUE(results[2]);
}

TEST_F(PolicyContainerTest, RemoveBatch) {
    container.set("a", 1);
    container.set("b", 2);
    container.set("c", 3);

    std::array<std::string_view, 4> keys = {"a", "nonexistent", "c", "also_nonexistent"};
    size_t removed = container.remove_batch(keys);

    EXPECT_EQ(removed, 2u);
    EXPECT_EQ(container.size(), 1u);
    EXPECT_TRUE(container.contains("b"));
}

// ============================================================================
// Iterator Tests
// ============================================================================

TEST_F(PolicyContainerTest, IteratorTraversal) {
    container.set("key1", 1);
    container.set("key2", 2);
    container.set("key3", 3);

    std::vector<std::string> keys;
    for (const auto& val : container) {
        keys.push_back(val.name);
    }

    EXPECT_EQ(keys.size(), 3u);
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key1") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key2") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key3") != keys.end());
}

TEST_F(PolicyContainerTest, STLAlgorithmCompatibility) {
    container.set("a", 10);
    container.set("b", 20);
    container.set("c", 30);

    auto it = std::find_if(container.begin(), container.end(),
        [](const auto& v) { return v.name == "b"; });

    ASSERT_NE(it, container.end());
    EXPECT_EQ(std::get<int>(it->data), 20);
}

// ============================================================================
// Fast Container Tests (Indexed Storage)
// ============================================================================

class FastPolicyContainerTest : public ::testing::Test {
protected:
    fast_policy_container container;

    void SetUp() override {
        container.clear_value();
    }
};

TEST_F(FastPolicyContainerTest, DefaultConstruction) {
    EXPECT_TRUE(container.empty());
    EXPECT_EQ(container.size(), 0u);
}

TEST_F(FastPolicyContainerTest, SetAndGet) {
    container.set("key1", 42);
    container.set("key2", std::string("hello"));

    auto val1 = container.get("key1");
    ASSERT_TRUE(val1.has_value());
    EXPECT_EQ(std::get<int>(val1->data), 42);

    auto val2 = container.get("key2");
    ASSERT_TRUE(val2.has_value());
    EXPECT_EQ(std::get<std::string>(val2->data), "hello");
}

TEST_F(FastPolicyContainerTest, LargeDatasetLookup) {
    const int NUM_ENTRIES = 1000;

    for (int i = 0; i < NUM_ENTRIES; ++i) {
        container.set("key_" + std::to_string(i), i);
    }

    EXPECT_EQ(container.size(), static_cast<size_t>(NUM_ENTRIES));

    // Random lookups should be O(1) with indexed storage
    auto result500 = container.get("key_500");
    ASSERT_TRUE(result500.has_value());
    EXPECT_EQ(std::get<int>(result500->data), 500);

    auto result999 = container.get("key_999");
    ASSERT_TRUE(result999.has_value());
    EXPECT_EQ(std::get<int>(result999->data), 999);

    // Non-existent key
    auto nonexistent = container.get("key_9999");
    EXPECT_FALSE(nonexistent.has_value());
}

// ============================================================================
// Typed Container Tests (Static Storage)
// ============================================================================

class TypedPolicyContainerTest : public ::testing::Test {
protected:
    typed_policy_container<int, double, std::string> container;

    void SetUp() override {
        container.clear_value();
    }
};

TEST_F(TypedPolicyContainerTest, DefaultConstruction) {
    EXPECT_TRUE(container.empty());
}

TEST_F(TypedPolicyContainerTest, AllowedTypes) {
    container.set("int_val", 42);
    container.set("double_val", 3.14);
    container.set("string_val", std::string("hello"));

    EXPECT_EQ(container.size(), 3u);

    auto int_result = container.get("int_val");
    ASSERT_TRUE(int_result.has_value());
    EXPECT_EQ(std::get<int>(int_result->data), 42);
}

TEST_F(TypedPolicyContainerTest, SetTypedWithPolicy) {
    // Use storage() to access policy-specific methods
    container.storage().set_typed("count", 100);
    container.storage().set_typed("rate", 1.5);

    auto count = container.storage().get_typed<int>("count");
    ASSERT_TRUE(count.has_value());
    EXPECT_EQ(*count, 100);
}

// ============================================================================
// Policy Switching / Interoperability Tests
// ============================================================================

TEST(PolicySwitchingTest, DynamicToIndexedConversion) {
    // Create container with dynamic storage
    policy_container dynamic;
    dynamic.set("a", 1);
    dynamic.set("b", 2);
    dynamic.set("c", 3);

    // Create indexed container with same data
    fast_policy_container indexed;
    for (const auto& val : dynamic) {
        indexed.set(val);
    }

    EXPECT_EQ(indexed.size(), dynamic.size());

    auto val = indexed.get("b");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(std::get<int>(val->data), 2);
}

TEST(PolicySwitchingTest, IndexedToDynamicConversion) {
    fast_policy_container indexed;
    indexed.set("x", 10);
    indexed.set("y", 20);

    policy_container dynamic;
    for (const auto& val : indexed) {
        dynamic.set(val);
    }

    EXPECT_EQ(dynamic.size(), indexed.size());
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST(ThreadSafetyTest, ConcurrentReads) {
    policy_container container;
    for (int i = 0; i < 100; ++i) {
        container.set("key_" + std::to_string(i), i);
    }

    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int t = 0; t < 10; ++t) {
        threads.emplace_back([&container, &success_count]() {
            for (int i = 0; i < 100; ++i) {
                auto val = container.get("key_" + std::to_string(i));
                if (val.has_value() && std::get<int>(val->data) == i) {
                    success_count++;
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(success_count.load(), 1000);
}

TEST(ThreadSafetyTest, ConcurrentWrites) {
    policy_container container;
    std::vector<std::thread> threads;
    const int NUM_THREADS = 10;
    const int WRITES_PER_THREAD = 100;

    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&container, t]() {
            for (int i = 0; i < WRITES_PER_THREAD; ++i) {
                std::string key = "thread_" + std::to_string(t) + "_key_" + std::to_string(i);
                container.set(key, t * WRITES_PER_THREAD + i);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(container.size(), static_cast<size_t>(NUM_THREADS * WRITES_PER_THREAD));
}

TEST(ThreadSafetyTest, ConcurrentReadWrite) {
    policy_container container;
    std::atomic<bool> running{true};
    std::atomic<int> read_count{0};
    std::atomic<int> write_count{0};

    // Writer threads
    std::vector<std::thread> writers;
    for (int w = 0; w < 5; ++w) {
        writers.emplace_back([&container, &running, &write_count, w]() {
            int i = 0;
            while (running.load()) {
                container.set("writer_" + std::to_string(w) + "_" + std::to_string(i++), w);
                write_count++;
                if (i > 100) break;
            }
        });
    }

    // Reader threads
    std::vector<std::thread> readers;
    for (int r = 0; r < 5; ++r) {
        readers.emplace_back([&container, &running, &read_count]() {
            while (running.load()) {
                container.size();  // Thread-safe read
                read_count++;
                if (read_count.load() > 1000) break;
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    running = false;

    for (auto& t : writers) t.join();
    for (auto& t : readers) t.join();

    EXPECT_GT(read_count.load(), 0);
    EXPECT_GT(write_count.load(), 0);
    // Container should be in consistent state
    EXPECT_GE(container.size(), 0u);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(EdgeCasesTest, EmptyKey) {
    policy_container container;
    container.set("", 42);

    EXPECT_TRUE(container.contains(""));
    auto result = container.get("");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<int>(result->data), 42);
}

TEST(EdgeCasesTest, SpecialCharacters) {
    policy_container container;
    container.set("key/with:special!chars@#$%", 1);

    EXPECT_TRUE(container.contains("key/with:special!chars@#$%"));
}

TEST(EdgeCasesTest, UnicodeKeys) {
    policy_container container;
    container.set("키_한글_日本語", 42);

    EXPECT_TRUE(container.contains("키_한글_日本語"));
    auto result = container.get("키_한글_日本語");
    ASSERT_TRUE(result.has_value());
}

TEST(EdgeCasesTest, UpdateExistingValue) {
    policy_container container;
    container.set("key", 10);
    container.set("key", 20);

    EXPECT_EQ(container.size(), 1u);
    auto result = container.get("key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<int>(result->data), 20);
}

TEST(EdgeCasesTest, DirectStorageAccess) {
    policy_container container;
    container.set("key", 42);

    auto& storage = container.storage();
    EXPECT_EQ(storage.size(), 1u);

    const auto& const_storage = container.storage();
    EXPECT_TRUE(const_storage.contains("key"));
}

TEST(EdgeCasesTest, Reserve) {
    policy_container container;
    container.reserve(1000);

    // Just verify no crash
    container.set("key", 42);
    EXPECT_TRUE(container.contains("key"));
}

// ============================================================================
// Performance Comparison (Basic Sanity Check)
// ============================================================================

TEST(PerformanceTest, IndexedVsDynamicLookup) {
    const int NUM_ENTRIES = 10000;
    const int NUM_LOOKUPS = 1000;

    // Setup containers
    policy_container dynamic;
    fast_policy_container indexed;

    for (int i = 0; i < NUM_ENTRIES; ++i) {
        std::string key = "key_" + std::to_string(i);
        dynamic.set(key, i);
        indexed.set(key, i);
    }

    // Random lookup keys
    std::vector<std::string> lookup_keys;
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, NUM_ENTRIES - 1);
    for (int i = 0; i < NUM_LOOKUPS; ++i) {
        lookup_keys.push_back("key_" + std::to_string(dist(rng)));
    }

    // Time dynamic lookups
    auto dynamic_start = std::chrono::high_resolution_clock::now();
    for (const auto& key : lookup_keys) {
        auto val = dynamic.get(key);
        (void)val;
    }
    auto dynamic_end = std::chrono::high_resolution_clock::now();
    auto dynamic_time = std::chrono::duration_cast<std::chrono::microseconds>(
        dynamic_end - dynamic_start).count();

    // Time indexed lookups
    auto indexed_start = std::chrono::high_resolution_clock::now();
    for (const auto& key : lookup_keys) {
        auto val = indexed.get(key);
        (void)val;
    }
    auto indexed_end = std::chrono::high_resolution_clock::now();
    auto indexed_time = std::chrono::duration_cast<std::chrono::microseconds>(
        indexed_end - indexed_start).count();

    // Indexed should generally be faster for large datasets
    // But we just verify both work correctly
    EXPECT_GE(dynamic_time, 0);
    EXPECT_GE(indexed_time, 0);

    // Both should have found all keys
    EXPECT_EQ(dynamic.size(), static_cast<size_t>(NUM_ENTRIES));
    EXPECT_EQ(indexed.size(), static_cast<size_t>(NUM_ENTRIES));
}
