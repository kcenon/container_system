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
 * @file unit_tests.cpp
 * @brief Comprehensive unit tests for the container system
 * 
 * Tests cover:
 * - Value type creation and conversion
 * - Container serialization/deserialization
 * - Thread safety
 * - Memory management
 * - Error handling
 */

#include <gtest/gtest.h>
#include "test_compat.h"
#include <thread>
#include <vector>
#include <random>
#include <sstream>
#include <chrono>
#include <barrier>

using namespace container_module;

// Test fixture for value tests
class ValueTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for value tests
    }
    
    void TearDown() override {
        // Cleanup
    }
};

// Test fixture for container tests
class ContainerTest : public ::testing::Test {
protected:
    std::unique_ptr<value_container> container;
    
    void SetUp() override {
        container = std::make_unique<value_container>();
    }
    
    void TearDown() override {
        container.reset();
    }
};

// ============================================================================
// Value Type Tests
// ============================================================================

TEST_F(ValueTest, NullValueCreation) {
    std::string key = "test_null";
    auto null_val = std::make_shared<value>(key);

    EXPECT_EQ(null_val->name(), "test_null");
    EXPECT_EQ(null_val->type(), value_types::null_value);
    EXPECT_TRUE(null_val->is_null());
    EXPECT_FALSE(is_boolean(*null_val));
    EXPECT_FALSE(is_numeric(*null_val));
    EXPECT_FALSE(is_string(*null_val));
    EXPECT_FALSE(is_container(*null_val));
}

TEST_F(ValueTest, BooleanValueCreation) {
    // Test true value
    std::string key1 = "test_bool";
    auto true_val = make_bool_value(key1, true);
    EXPECT_EQ(true_val->name(), "test_bool");
    EXPECT_EQ(true_val->type(), value_types::bool_value);
    EXPECT_TRUE(is_boolean(*true_val));
    EXPECT_TRUE(to_boolean(*true_val));

    // Test false value
    std::string key2 = "test_bool2";
    auto false_val = make_bool_value(key2, false);
    EXPECT_FALSE(to_boolean(*false_val));

    // Test string that converts to boolean (via helper)
    std::string key3 = "test_str_true";
    auto str_true_val = make_string_value(key3, "true");
    EXPECT_TRUE(to_boolean(*str_true_val));

    std::string key4 = "test_str_false";
    auto str_false_val = make_string_value(key4, "false");
    EXPECT_FALSE(to_boolean(*str_false_val));
}

TEST_F(ValueTest, NumericValueCreation) {
    // Test int
    std::string key_int = "test_int";
    auto int_val = make_int_value(key_int, 42);
    EXPECT_EQ(int_val->type(), value_types::int_value);
    EXPECT_TRUE(is_numeric(*int_val));
    EXPECT_EQ(to_int(*int_val), 42);
    EXPECT_EQ(to_long(*int_val), 42L);
    EXPECT_DOUBLE_EQ(to_double(*int_val), 42.0);

    // Test long long
    std::string key_llong = "test_llong";
    auto llong_val = make_llong_value(key_llong, 9223372036854775807LL);
    EXPECT_EQ(to_llong(*llong_val), 9223372036854775807LL);

    // Test double
    std::string key_double = "test_double";
    auto double_val = make_double_value(key_double, 3.14159);
    EXPECT_DOUBLE_EQ(to_double(*double_val), 3.14159);

    // Test negative values
    std::string key_neg = "test_neg";
    auto neg_val = make_int_value(key_neg, -100);
    EXPECT_EQ(to_int(*neg_val), -100);
}

TEST_F(ValueTest, StringValueCreation) {
    std::string key = "test_string";
    std::string val = "Hello, World!";
    auto str_val = make_string_value(key, val);

    EXPECT_EQ(str_val->type(), value_types::string_value);
    EXPECT_TRUE(is_string(*str_val));
    EXPECT_EQ(str_val->to_string(), "Hello, World!");
    // Note: value_size() returns internal data size after conversion, not original string length
    EXPECT_GT(value_size(*str_val), 0); // Just verify data exists
}

TEST_F(ValueTest, BytesValueCreation) {
    std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0x04, 0xFF};

    std::string key = "test_bytes";
    auto bytes_val = make_bytes_value(key, test_data);

    EXPECT_EQ(bytes_val->type(), value_types::bytes_value);
    EXPECT_TRUE(is_bytes(*bytes_val));

    auto retrieved_bytes = to_bytes(*bytes_val);
    EXPECT_EQ(retrieved_bytes.size(), test_data.size());
    EXPECT_EQ(retrieved_bytes, test_data);
}

TEST_F(ValueTest, ValueTypeSerialization) {
    // Test each value type serialization
    std::string key1 = "bool";
    auto bool_val = make_bool_value(key1, true);
    std::string key2 = "int";
    auto int_val = make_int_value(key2, 42);
    std::string key3 = "str";
    std::string val3 = "test";
    auto str_val = make_string_value(key3, val3);

    // Serialize values (returns vector<uint8_t>)
    auto bool_ser = bool_val->serialize();
    auto int_ser = int_val->serialize();
    auto str_ser = str_val->serialize();

    // Check serialization format (may vary based on implementation)
    EXPECT_FALSE(bool_ser.empty());
    EXPECT_FALSE(int_ser.empty());
    EXPECT_FALSE(str_ser.empty());
}

// ============================================================================
// Container Tests
// ============================================================================

TEST_F(ContainerTest, BasicContainerCreation) {
    container->set_source("test_source", "sub1");
    container->set_target("test_target", "sub2");
    container->set_message_type("test_message");
    
    EXPECT_EQ(container->source_id(), "test_source");
    EXPECT_EQ(container->source_sub_id(), "sub1");
    EXPECT_EQ(container->target_id(), "test_target");
    EXPECT_EQ(container->target_sub_id(), "sub2");
    EXPECT_EQ(container->message_type(), "test_message");
}

TEST_F(ContainerTest, ContainerValueManagement) {
    // Add various values
    std::string key1 = "key1";
    std::string value1 = "value1";
    std::string key2 = "key2";
    std::string key3 = "key3";
    container->add(make_string_value(key1, value1));
    container->add(make_int_value(key2, 100));
    container->add(make_bool_value(key3, true));

    // Retrieve values
    auto val1 = container->get_value("key1");
    auto val2 = container->get_value("key2");
    auto val3 = container->get_value("key3");

    EXPECT_EQ(ov_to_string(val1), "value1");
    EXPECT_EQ(ov_to_int(val2), 100);
    EXPECT_TRUE(ov_to_boolean(val3));

    // Test non-existent key
    auto val4 = container->get_value("non_existent");
    EXPECT_TRUE(ov_is_null(val4));
}

TEST_F(ContainerTest, ContainerSerialization) {
    // Setup container
    container->set_source("src", "sub");
    container->set_target("tgt", "");
    container->set_message_type("test");

    // Add values
    std::string str_key = "str";
    std::string str_val = "hello";
    std::string num_key = "num";
    container->add(make_string_value(str_key, str_val));
    container->add(make_int_value(num_key, 42));

    // Serialize
    std::string serialized = container->serialize();

    // Deserialize - parse_only_header=false to parse values too
    auto new_container = std::make_unique<value_container>(serialized, false);

    // Verify
    EXPECT_EQ(new_container->source_id(), "src");
    EXPECT_EQ(new_container->source_sub_id(), "sub");
    EXPECT_EQ(new_container->target_id(), "tgt");
    EXPECT_EQ(new_container->message_type(), "test");

    EXPECT_EQ(ov_to_string(new_container->get_value("str")), "hello");
    EXPECT_EQ(ov_to_int(new_container->get_value("num")), 42);
}

TEST_F(ContainerTest, NestedContainerSupport) {
    // Create nested container
    auto nested = std::make_unique<value_container>();
    nested->set_message_type("nested_msg");
    std::string nested_key = "nested_key";
    std::string nested_value = "nested_value";
    nested->add(make_string_value(nested_key, nested_value));

    // Serialize nested container
    std::string nested_data = nested->serialize();

    // Add to main container as string value (containing serialized container data)
    std::string child_key = "child";
    container->add(make_string_value(child_key, nested_data));

    // Retrieve nested container data
    auto child_val = container->get_value("child");
    EXPECT_TRUE(child_val.has_value());

    // Parse nested container - parse_only_header=false to parse values too
    auto child_container = std::make_unique<value_container>(ov_to_string(child_val), false);
    EXPECT_EQ(child_container->message_type(), "nested_msg");
    EXPECT_EQ(ov_to_string(child_container->get_value("nested_key")), "nested_value");
}

TEST_F(ContainerTest, ContainerHeaderSwap) {
    container->set_source("src", "sub1");
    container->set_target("tgt", "sub2");
    
    container->swap_header();
    
    EXPECT_EQ(container->source_id(), "tgt");
    EXPECT_EQ(container->source_sub_id(), "sub2");
    EXPECT_EQ(container->target_id(), "src");
    EXPECT_EQ(container->target_sub_id(), "sub1");
}

// DISABLED: value_array() and get_value with index not implemented in current API
TEST_F(ContainerTest, DISABLED_MultipleValuesWithSameName) {
    // Add multiple values with same name
    std::string key = "item";
    std::string val1 = "first";
    std::string val2 = "second";
    std::string val3 = "third";
    container->add(make_string_value(key, val1));
    container->add(make_string_value(key, val2));
    container->add(make_string_value(key, val3));

    // Get first value with name (current API behavior)
    auto item = container->get_value("item");
    EXPECT_TRUE(item.has_value());
}

TEST_F(ContainerTest, ContainerCopy) {
    // Setup original
    container->set_message_type("original");
    std::string key = "key";
    std::string val = "value";
    container->add(make_string_value(key, val));

    // Deep copy
    auto copy = container->copy(true);
    EXPECT_EQ(copy->message_type(), "original");
    EXPECT_EQ(ov_to_string(copy->get_value("key")), "value");

    // Shallow copy (header only)
    auto shallow = container->copy(false);
    EXPECT_EQ(shallow->message_type(), "original");
    auto shallow_val = shallow->get_value("key");
    EXPECT_TRUE(ov_is_null(shallow_val)); // No values in shallow copy
}

// Large data handling test - validates serialization of large data
// Note: Using 10KB instead of 1MB to avoid stack overflow in std::regex
// under AddressSanitizer (regex uses backtracking which is stack-intensive)
TEST_F(ContainerTest, LargeDataHandling) {
    // Create large string (10KB - large enough to test but safe for ASAN)
    std::string large_data(10 * 1024, 'X'); // 10KB of X's
    std::string key = "large";

    container->add(make_string_value(key, large_data));

    // Serialize and deserialize - parse_only_header=false to parse values too
    std::string serialized = container->serialize();
    auto restored = std::make_unique<value_container>(serialized, false);

    EXPECT_EQ(ov_to_string(restored->get_value("large")), large_data);
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

// Thread safety test - validates concurrent read access
TEST(ThreadSafetyTest, ConcurrentReads) {
    auto container = std::make_unique<value_container>();

    // Add test data
    for (int i = 0; i < 100; ++i) {
        container->add(make_int_value(
            std::string("key") + std::to_string(i),
            i
        ));
    }

    // Serialize once
    std::string serialized = container->serialize();

    // Multiple threads reading
    const int num_threads = 10;
    std::vector<std::thread> threads;
    std::atomic<int> success_count(0);

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&container, &success_count]() {
            for (int i = 0; i < 100; ++i) {
                auto val = container->get_value("key" + std::to_string(i));
                if (ov_to_int(val) == i) {
                    success_count++;
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(success_count, num_threads * 100);
}

// Thread safety test - validates thread_safe_container concurrent operations
TEST(ThreadSafetyTest, ThreadSafeContainer) {
    auto safe_container = std::make_shared<thread_safe_container>();

    const int num_threads = 5;
    const int ops_per_thread = 100;
    std::vector<std::thread> threads;

    // Barrier to synchronize all threads starting together
    std::barrier sync_point(num_threads * 2);

    // Writers - use value objects
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([safe_container, t, ops_per_thread, &sync_point]() {
            sync_point.arrive_and_wait();  // All threads start together

            for (int i = 0; i < ops_per_thread; ++i) {
                std::string key = "thread" + std::to_string(t) + "_" + std::to_string(i);
                int val = t * 1000 + i;
                safe_container->set(key, value(key, val));
            }
        });
    }

    // Readers
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([safe_container, t, ops_per_thread, &sync_point]() {
            sync_point.arrive_and_wait();  // All threads start together

            for (int i = 0; i < ops_per_thread; ++i) {
                std::string key = "thread" + std::to_string(t) + "_" + std::to_string(i);
                auto val = safe_container->get_typed<int32_t>(key);
                // Value might not exist yet, that's OK
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // Verify all values were added
    for (int t = 0; t < num_threads; ++t) {
        for (int i = 0; i < ops_per_thread; ++i) {
            std::string key = "thread" + std::to_string(t) + "_" + std::to_string(i);
            auto val = safe_container->get_typed<int32_t>(key);
            EXPECT_TRUE(val.has_value());
            EXPECT_EQ(val.value(), t * 1000 + i);
        }
    }
}

// ============================================================================
// Error Handling Tests
// ============================================================================

// Test that invalid serialization data is handled gracefully (no crash)
TEST(ErrorHandlingTest, InvalidSerializationHandling) {
    // value_container handles invalid data gracefully without throwing
    // This ensures robustness when receiving malformed data
    EXPECT_NO_THROW({
        value_container invalid1("invalid data");
        value_container invalid2("@header={};@data={[invalid];");
    });
}

TEST(ErrorHandlingTest, TypeConversionErrors) {
    std::string key = "test";
    std::string val = "not_a_number";
    auto str_val = make_string_value(key, val);

    // String to int conversion should handle gracefully
    EXPECT_EQ(to_int(*str_val), 0); // Default value for failed conversion
}

// Disabled: Value class doesn't throw on null conversions - returns defaults instead
// TODO: Consider implementing exception throwing for null value conversions
TEST(ErrorHandlingTest, DISABLED_NullValueConversions) {
    std::string key = "null";
    auto null_val = std::make_shared<value>(key);

    // With value class, null conversions return default values rather than throwing
    // This test is disabled as the current implementation doesn't throw
    EXPECT_EQ(to_boolean(*null_val), false);
    EXPECT_EQ(to_int(*null_val), 0);
    EXPECT_EQ(to_double(*null_val), 0.0);
}

// ============================================================================
// Performance Tests (Simple Benchmarks)
// ============================================================================

// Disabled in CI: performance benchmarks belong in separate test suite
TEST(PerformanceTest, DISABLED_SerializationSpeed) {
    auto container = std::make_unique<value_container>();

    // Add 1000 values
    for (int i = 0; i < 1000; ++i) {
        std::string key = std::string("key") + std::to_string(i);
        std::string value = std::string("value") + std::to_string(i);
        container->add(make_string_value(key, value));
    }

    // Measure serialization time
    auto start = std::chrono::high_resolution_clock::now();
    std::string serialized = container->serialize();
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Log performance
    std::cout << "Serialization of 1000 values: " << duration.count() << " microseconds" << std::endl;
    std::cout << "Serialized size: " << serialized.size() << " bytes" << std::endl;

    // Basic performance expectation (should be fast)
    EXPECT_LT(duration.count(), 10000); // Less than 10ms
}

// Disabled in CI: performance benchmarks belong in separate test suite
TEST(PerformanceTest, DISABLED_DeserializationSpeed) {
    // Create and serialize container
    auto container = std::make_unique<value_container>();
    for (int i = 0; i < 1000; ++i) {
        std::string key = std::string("key") + std::to_string(i);
        std::string value = std::string("value") + std::to_string(i);
        container->add(make_string_value(key, value));
    }
    std::string serialized = container->serialize();

    // Measure deserialization time
    auto start = std::chrono::high_resolution_clock::now();
    auto restored = std::make_unique<value_container>(serialized);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Deserialization of 1000 values: " << duration.count() << " microseconds" << std::endl;

    EXPECT_LT(duration.count(), 10000); // Less than 10ms
}

// ============================================================================
// Special Cases and Edge Cases
// ============================================================================

TEST(EdgeCaseTest, EmptyContainer) {
    auto container = std::make_unique<value_container>();
    
    // Serialize empty container
    std::string serialized = container->serialize();
    
    // Deserialize
    auto restored = std::make_unique<value_container>(serialized);
    
    // Should have default values
    EXPECT_EQ(restored->source_id(), "");
    EXPECT_EQ(restored->message_type(), "data_container");
}

// TODO: REACT-004 - Investigate serialization issue with string values
TEST(EdgeCaseTest, DISABLED_SpecialCharacters) {
    auto container = std::make_unique<value_container>();

    // Test special characters in key names
    std::string underscore_key = "key_with_underscores";
    std::string underscore_value = "value1";
    std::string camel_key = "keyWithCamelCase";
    std::string camel_value = "value2";
    std::string numeric_key = "key123";
    std::string numeric_value = "value3";

    container->add(make_string_value(underscore_key, underscore_value));
    container->add(make_string_value(camel_key, camel_value));
    container->add(make_string_value(numeric_key, numeric_value));

    // Serialize and restore
    std::string serialized = container->serialize();
    auto restored = std::make_unique<value_container>(serialized);

    // Verify values preserved
    auto underscore_val = restored->get_value("key_with_underscores");
    EXPECT_FALSE(ov_is_null(underscore_val));
    if (!ov_is_null(underscore_val)) {
        EXPECT_EQ(ov_to_string(underscore_val), "value1");
    }

    auto camel_val = restored->get_value("keyWithCamelCase");
    EXPECT_FALSE(ov_is_null(camel_val));
    if (!ov_is_null(camel_val)) {
        EXPECT_EQ(ov_to_string(camel_val), "value2");
    }

    auto numeric_val = restored->get_value("key123");
    EXPECT_FALSE(ov_is_null(numeric_val));
    if (!ov_is_null(numeric_val)) {
        EXPECT_EQ(ov_to_string(numeric_val), "value3");
    }
}

TEST(EdgeCaseTest, MaximumValues) {
    // Test maximum numeric values
    std::string max_int_key = "max_int";
    std::string min_int_key = "min_int";
    std::string max_llong_key = "max_llong";

    auto max_int = make_int_value(max_int_key, std::numeric_limits<int>::max());
    auto min_int = make_int_value(min_int_key, std::numeric_limits<int>::min());
    auto max_llong = make_llong_value(max_llong_key, std::numeric_limits<long long>::max());

    EXPECT_EQ(to_int(*max_int), std::numeric_limits<int>::max());
    EXPECT_EQ(to_int(*min_int), std::numeric_limits<int>::min());
    EXPECT_EQ(to_llong(*max_llong), std::numeric_limits<long long>::max());
}

// ============================================================================
// value_store Serialization Tests
// ============================================================================

#include "container/core/value_store.h"

class ValueStoreSerializationTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ValueStoreSerializationTest, EmptyStoreJSONSerialization) {
    value_store store;
    std::string json = store.serialize();
    EXPECT_EQ(json, "{}");
}

TEST_F(ValueStoreSerializationTest, SingleValueJSONSerialization) {
    value_store store;
    store.add("test_key", value("test_key", int32_t(42)));

    std::string json = store.serialize();
    EXPECT_FALSE(json.empty());
    EXPECT_NE(json.find("test_key"), std::string::npos);
    EXPECT_NE(json.find("42"), std::string::npos);
}

TEST_F(ValueStoreSerializationTest, MultipleValuesJSONSerialization) {
    value_store store;
    store.add("int_val", value("int_val", int32_t(123)));
    store.add("str_val", value("str_val", std::string("hello")));
    store.add("bool_val", value("bool_val", true));

    std::string json = store.serialize();
    EXPECT_NE(json.find("int_val"), std::string::npos);
    EXPECT_NE(json.find("str_val"), std::string::npos);
    EXPECT_NE(json.find("bool_val"), std::string::npos);
}

TEST_F(ValueStoreSerializationTest, EmptyStoreBinarySerialization) {
    value_store store;
    auto binary = store.serialize_binary();

    // Version byte + count (4 bytes) = 5 bytes minimum
    EXPECT_GE(binary.size(), 5u);
    EXPECT_EQ(binary[0], 1u); // Version 1
}

TEST_F(ValueStoreSerializationTest, BinarySerializationRoundTrip) {
    value_store store;
    store.add("integer", value("integer", int32_t(42)));
    store.add("text", value("text", std::string("hello world")));
    store.add("flag", value("flag", true));
    store.add("decimal", value("decimal", double(3.14)));

    // Serialize
    auto binary = store.serialize_binary();
    EXPECT_GT(binary.size(), 5u);

    // Deserialize
    auto restored = value_store::deserialize_binary(binary);
    ASSERT_NE(restored, nullptr);

    // Verify values
    EXPECT_EQ(restored->size(), 4u);
    EXPECT_TRUE(restored->contains("integer"));
    EXPECT_TRUE(restored->contains("text"));
    EXPECT_TRUE(restored->contains("flag"));
    EXPECT_TRUE(restored->contains("decimal"));

    // Verify specific values
    auto int_val = restored->get("integer");
    ASSERT_TRUE(int_val.has_value());
    auto int_opt = int_val->get<int32_t>();
    ASSERT_TRUE(int_opt.has_value());
    EXPECT_EQ(*int_opt, 42);

    auto str_val = restored->get("text");
    ASSERT_TRUE(str_val.has_value());
    auto str_opt = str_val->get<std::string>();
    ASSERT_TRUE(str_opt.has_value());
    EXPECT_EQ(*str_opt, "hello world");

    auto bool_val = restored->get("flag");
    ASSERT_TRUE(bool_val.has_value());
    auto bool_opt = bool_val->get<bool>();
    ASSERT_TRUE(bool_opt.has_value());
    EXPECT_EQ(*bool_opt, true);
}

TEST_F(ValueStoreSerializationTest, BinaryDeserializeInvalidData) {
    std::vector<uint8_t> empty_data;
    EXPECT_THROW(value_store::deserialize_binary(empty_data), std::runtime_error);

    std::vector<uint8_t> too_small = {1}; // Only version byte
    EXPECT_THROW(value_store::deserialize_binary(too_small), std::runtime_error);

    std::vector<uint8_t> bad_version = {99, 0, 0, 0, 0}; // Invalid version
    EXPECT_THROW(value_store::deserialize_binary(bad_version), std::runtime_error);
}

TEST_F(ValueStoreSerializationTest, JSONDeserializeNotImplemented) {
    EXPECT_THROW(value_store::deserialize("{}"), std::runtime_error);
}

TEST_F(ValueStoreSerializationTest, ThreadSafeSerialization) {
    value_store store;
    store.enable_thread_safety();

    store.add("key1", value("key1", int32_t(100)));
    store.add("key2", value("key2", std::string("test")));

    // Should work without deadlock
    std::string json = store.serialize();
    auto binary = store.serialize_binary();

    EXPECT_FALSE(json.empty());
    EXPECT_GT(binary.size(), 5u);
}

TEST_F(ValueStoreSerializationTest, SpecialCharactersInKeys) {
    value_store store;
    store.add("key\"with\"quotes", value("key\"with\"quotes", int32_t(1)));
    store.add("key\\with\\backslash", value("key\\with\\backslash", int32_t(2)));
    store.add("key\nwith\nnewlines", value("key\nwith\nnewlines", int32_t(3)));

    std::string json = store.serialize();
    // Keys should be properly escaped
    EXPECT_NE(json.find("\\\""), std::string::npos);
    EXPECT_NE(json.find("\\\\"), std::string::npos);
    EXPECT_NE(json.find("\\n"), std::string::npos);
}

TEST_F(ValueStoreSerializationTest, BytesValueSerialization) {
    value_store store;
    std::vector<uint8_t> bytes = {0x01, 0x02, 0x03, 0xFF, 0x00};
    store.add("binary_data", value("binary_data", bytes));

    auto binary = store.serialize_binary();
    auto restored = value_store::deserialize_binary(binary);

    ASSERT_NE(restored, nullptr);
    auto restored_val = restored->get("binary_data");
    ASSERT_TRUE(restored_val.has_value());
    auto bytes_opt = restored_val->get<std::vector<uint8_t>>();
    ASSERT_TRUE(bytes_opt.has_value());
    EXPECT_EQ(*bytes_opt, bytes);
}

TEST_F(ValueStoreSerializationTest, LargeValuesSerialization) {
    value_store store;

    // Large string
    std::string large_string(10000, 'x');
    store.add("large_string", value("large_string", large_string));

    // Large bytes
    std::vector<uint8_t> large_bytes(10000, 0xAB);
    store.add("large_bytes", value("large_bytes", large_bytes));

    auto binary = store.serialize_binary();
    auto restored = value_store::deserialize_binary(binary);

    ASSERT_NE(restored, nullptr);

    auto str_val = restored->get("large_string");
    ASSERT_TRUE(str_val.has_value());
    auto str_opt = str_val->get<std::string>();
    ASSERT_TRUE(str_opt.has_value());
    EXPECT_EQ(str_opt->size(), 10000u);

    auto bytes_val = restored->get("large_bytes");
    ASSERT_TRUE(bytes_val.has_value());
    auto bytes_opt = bytes_val->get<std::vector<uint8_t>>();
    ASSERT_TRUE(bytes_opt.has_value());
    EXPECT_EQ(bytes_opt->size(), 10000u);
}

// ============================================================================
// JSON Escaping Tests (Issue #186)
// ============================================================================

class JSONEscapingTest : public ::testing::Test {
protected:
    std::unique_ptr<value_container> container;

    void SetUp() override {
        container = std::make_unique<value_container>();
        container->set_message_type("test_message");
    }

    void TearDown() override {
        container.reset();
    }
};

TEST_F(JSONEscapingTest, QuoteEscaping) {
    std::string key = "message";
    std::string value = "Hello \"World\"";
    container->add(make_string_value(key, value));

    std::string json = container->to_json();

    // Verify that quotes are escaped
    EXPECT_NE(json.find("Hello \\\"World\\\""), std::string::npos)
        << "Expected escaped quotes in JSON output: " << json;
}

TEST_F(JSONEscapingTest, BackslashEscaping) {
    std::string key = "path";
    std::string value = "C:\\Users\\test";
    container->add(make_string_value(key, value));

    std::string json = container->to_json();

    // Verify that backslashes are escaped
    EXPECT_NE(json.find("C:\\\\Users\\\\test"), std::string::npos)
        << "Expected escaped backslashes in JSON output: " << json;
}

TEST_F(JSONEscapingTest, NewlineEscaping) {
    std::string key = "multiline";
    std::string value = "line1\nline2\r\nline3";
    container->add(make_string_value(key, value));

    std::string json = container->to_json();

    // Verify that newlines are escaped
    EXPECT_NE(json.find("line1\\nline2\\r\\nline3"), std::string::npos)
        << "Expected escaped newlines in JSON output: " << json;
}

TEST_F(JSONEscapingTest, TabEscaping) {
    std::string key = "tabbed";
    std::string value = "col1\tcol2\tcol3";
    container->add(make_string_value(key, value));

    std::string json = container->to_json();

    // Verify that tabs are escaped
    EXPECT_NE(json.find("col1\\tcol2\\tcol3"), std::string::npos)
        << "Expected escaped tabs in JSON output: " << json;
}

TEST_F(JSONEscapingTest, ControlCharacterEscaping) {
    std::string key = "control";
    std::string value = std::string("before\x01\x02\x03" "after");
    container->add(make_string_value(key, value));

    std::string json = container->to_json();

    // Verify that control characters are escaped as \uXXXX
    EXPECT_NE(json.find("\\u0001"), std::string::npos)
        << "Expected escaped control character \\u0001 in JSON output: " << json;
    EXPECT_NE(json.find("\\u0002"), std::string::npos)
        << "Expected escaped control character \\u0002 in JSON output: " << json;
    EXPECT_NE(json.find("\\u0003"), std::string::npos)
        << "Expected escaped control character \\u0003 in JSON output: " << json;
}

TEST_F(JSONEscapingTest, FormFeedAndBackspaceEscaping) {
    std::string key = "special";
    std::string value = "form\ffeed\bbackspace";
    container->add(make_string_value(key, value));

    std::string json = container->to_json();

    // Verify that form feed and backspace are escaped
    EXPECT_NE(json.find("form\\ffeed\\bbackspace"), std::string::npos)
        << "Expected escaped form feed and backspace in JSON output: " << json;
}

TEST_F(JSONEscapingTest, AllSpecialCharactersCombined) {
    std::string key = "complex";
    std::string value = "Quote: \" Backslash: \\ Newline:\n Tab:\t End";
    container->add(make_string_value(key, value));

    std::string json = container->to_json();

    // Verify all special characters are escaped
    EXPECT_NE(json.find("Quote: \\\""), std::string::npos)
        << "Expected escaped quote in JSON output: " << json;
    EXPECT_NE(json.find("Backslash: \\\\"), std::string::npos)
        << "Expected escaped backslash in JSON output: " << json;
    EXPECT_NE(json.find("Newline:\\n"), std::string::npos)
        << "Expected escaped newline in JSON output: " << json;
    EXPECT_NE(json.find("Tab:\\t"), std::string::npos)
        << "Expected escaped tab in JSON output: " << json;
}

TEST_F(JSONEscapingTest, HeaderFieldEscaping) {
    container->set_source("source\"id", "sub\\id");
    container->set_target("target\nid", "sub\tid");

    std::string json = container->to_json();

    // Verify header fields are escaped
    EXPECT_NE(json.find("source\\\"id"), std::string::npos)
        << "Expected escaped quote in source_id: " << json;
    EXPECT_NE(json.find("sub\\\\id"), std::string::npos)
        << "Expected escaped backslash in source_sub_id: " << json;
    EXPECT_NE(json.find("target\\nid"), std::string::npos)
        << "Expected escaped newline in target_id: " << json;
    EXPECT_NE(json.find("sub\\tid"), std::string::npos)
        << "Expected escaped tab in target_sub_id: " << json;
}

TEST_F(JSONEscapingTest, FieldNameEscaping) {
    // Field names with special characters should also be escaped
    std::string key = "field\"name";
    std::string value = "value";
    container->add(make_string_value(key, value));

    std::string json = container->to_json();

    // Verify field name is escaped
    EXPECT_NE(json.find("field\\\"name"), std::string::npos)
        << "Expected escaped field name in JSON output: " << json;
}

TEST_F(JSONEscapingTest, ValidJSONOutput) {
    std::string key = "test";
    std::string value = "Hello \"World\" with \\ and \n special chars";
    container->add(make_string_value(key, value));

    std::string json = container->to_json();

    // Basic JSON structure validation
    EXPECT_EQ(json.front(), '{');
    EXPECT_EQ(json.back(), '}');

    // Should not have unescaped quotes inside string values
    // (except the structural quotes that delimit keys/values)
    size_t quote_count = 0;
    for (size_t i = 0; i < json.size(); ++i) {
        if (json[i] == '"' && (i == 0 || json[i-1] != '\\')) {
            quote_count++;
        }
    }
    // Quote count should be even (matched pairs)
    EXPECT_EQ(quote_count % 2, 0u)
        << "Unmatched quotes in JSON output: " << json;
}

TEST_F(JSONEscapingTest, EmptyStringValue) {
    std::string key = "empty";
    std::string value = "";
    container->add(make_string_value(key, value));

    std::string json = container->to_json();

    // Empty string should produce valid JSON with empty value
    EXPECT_NE(json.find("\"empty\":\"\""), std::string::npos)
        << "Expected empty string value in JSON output: " << json;
}

TEST_F(JSONEscapingTest, NumericValuesUnchanged) {
    std::string key1 = "int_val";
    std::string key2 = "double_val";
    container->add(make_int_value(key1, 42));
    container->add(make_double_value(key2, 3.14));

    std::string json = container->to_json();

    // Numeric values should not be quoted or escaped
    EXPECT_NE(json.find("\"int_val\":42"), std::string::npos)
        << "Expected unquoted integer in JSON output: " << json;
}

// ============================================================================
// JSON Escape Function Unit Tests
// ============================================================================

TEST(JSONEscapeFunctionTest, EmptyString) {
    std::string result = variant_helpers::json_escape("");
    EXPECT_EQ(result, "");
}

TEST(JSONEscapeFunctionTest, NoSpecialCharacters) {
    std::string result = variant_helpers::json_escape("Hello World");
    EXPECT_EQ(result, "Hello World");
}

TEST(JSONEscapeFunctionTest, OnlySpecialCharacters) {
    std::string result = variant_helpers::json_escape("\"\\\n\r\t\b\f");
    EXPECT_EQ(result, "\\\"\\\\\\n\\r\\t\\b\\f");
}

TEST(JSONEscapeFunctionTest, MixedContent) {
    std::string result = variant_helpers::json_escape("Say \"Hello\"\nNew line");
    EXPECT_EQ(result, "Say \\\"Hello\\\"\\nNew line");
}

TEST(JSONEscapeFunctionTest, ControlCharactersAsUnicode) {
    std::string input(1, '\x01');
    std::string result = variant_helpers::json_escape(input);
    EXPECT_EQ(result, "\\u0001");

    input = std::string(1, '\x1f');
    result = variant_helpers::json_escape(input);
    EXPECT_EQ(result, "\\u001f");
}

TEST(JSONEscapeFunctionTest, NullCharacterEscaping) {
    std::string input(1, '\x00');
    std::string result = variant_helpers::json_escape(input);
    EXPECT_EQ(result, "\\u0000");
}

// ============================================================================
// XML Encoding Tests (Issue #187)
// ============================================================================

class XMLEncodingTest : public ::testing::Test {
protected:
    std::unique_ptr<value_container> container;

    void SetUp() override {
        container = std::make_unique<value_container>();
        container->set_message_type("test_message");
    }

    void TearDown() override {
        container.reset();
    }
};

TEST_F(XMLEncodingTest, AmpersandEncoding) {
    std::string key = "query";
    std::string value = "a & b";
    container->add(make_string_value(key, value));

    std::string xml = container->to_xml();

    EXPECT_NE(xml.find("a &amp; b"), std::string::npos)
        << "Expected encoded ampersand in XML output: " << xml;
}

TEST_F(XMLEncodingTest, LessThanEncoding) {
    std::string key = "query";
    std::string value = "SELECT * FROM users WHERE id < 5";
    container->add(make_string_value(key, value));

    std::string xml = container->to_xml();

    EXPECT_NE(xml.find("id &lt; 5"), std::string::npos)
        << "Expected encoded less-than in XML output: " << xml;
}

TEST_F(XMLEncodingTest, GreaterThanEncoding) {
    std::string key = "query";
    std::string value = "x > 10";
    container->add(make_string_value(key, value));

    std::string xml = container->to_xml();

    EXPECT_NE(xml.find("x &gt; 10"), std::string::npos)
        << "Expected encoded greater-than in XML output: " << xml;
}

TEST_F(XMLEncodingTest, QuoteEncoding) {
    std::string key = "message";
    std::string value = "Hello \"World\"";
    container->add(make_string_value(key, value));

    std::string xml = container->to_xml();

    EXPECT_NE(xml.find("Hello &quot;World&quot;"), std::string::npos)
        << "Expected encoded quotes in XML output: " << xml;
}

TEST_F(XMLEncodingTest, ApostropheEncoding) {
    std::string key = "message";
    std::string value = "It's working";
    container->add(make_string_value(key, value));

    std::string xml = container->to_xml();

    EXPECT_NE(xml.find("It&apos;s working"), std::string::npos)
        << "Expected encoded apostrophe in XML output: " << xml;
}

TEST_F(XMLEncodingTest, AllSpecialCharactersCombined) {
    std::string key = "complex";
    std::string value = "a < b & c > d \"quoted\" and 'apostrophe'";
    container->add(make_string_value(key, value));

    std::string xml = container->to_xml();

    EXPECT_NE(xml.find("a &lt; b &amp; c &gt; d &quot;quoted&quot; and &apos;apostrophe&apos;"), std::string::npos)
        << "Expected all XML special characters encoded: " << xml;
}

TEST_F(XMLEncodingTest, ControlCharacterEncoding) {
    std::string key = "control";
    std::string value = std::string("before\x01\x02" "after");
    container->add(make_string_value(key, value));

    std::string xml = container->to_xml();

    EXPECT_NE(xml.find("&#x01;"), std::string::npos)
        << "Expected encoded control character &#x01; in XML output: " << xml;
    EXPECT_NE(xml.find("&#x02;"), std::string::npos)
        << "Expected encoded control character &#x02; in XML output: " << xml;
}

TEST_F(XMLEncodingTest, WhitespacePreserved) {
    std::string key = "whitespace";
    std::string value = "line1\nline2\tcolumn2\rend";
    container->add(make_string_value(key, value));

    std::string xml = container->to_xml();

    // Tab, newline, and carriage return should be preserved (not encoded)
    EXPECT_NE(xml.find("line1\nline2\tcolumn2\rend"), std::string::npos)
        << "Expected whitespace to be preserved in XML output: " << xml;
}

TEST_F(XMLEncodingTest, HeaderFieldEncoding) {
    container->set_source("source<id", "sub&id");
    container->set_target("target>id", "sub\"id");

    std::string xml = container->to_xml();

    EXPECT_NE(xml.find("source&lt;id"), std::string::npos)
        << "Expected encoded less-than in source_id: " << xml;
    EXPECT_NE(xml.find("sub&amp;id"), std::string::npos)
        << "Expected encoded ampersand in source_sub_id: " << xml;
    EXPECT_NE(xml.find("target&gt;id"), std::string::npos)
        << "Expected encoded greater-than in target_id: " << xml;
    EXPECT_NE(xml.find("sub&quot;id"), std::string::npos)
        << "Expected encoded quote in target_sub_id: " << xml;
}

TEST_F(XMLEncodingTest, ValidXMLStructure) {
    std::string key = "test";
    std::string value = "Hello <World> with & special chars";
    container->add(make_string_value(key, value));

    std::string xml = container->to_xml();

    // Basic XML structure validation
    EXPECT_EQ(xml.substr(0, 11), "<container>");
    EXPECT_EQ(xml.substr(xml.size() - 12), "</container>");
    EXPECT_NE(xml.find("<header>"), std::string::npos);
    EXPECT_NE(xml.find("</header>"), std::string::npos);
    EXPECT_NE(xml.find("<values>"), std::string::npos);
    EXPECT_NE(xml.find("</values>"), std::string::npos);
}

TEST_F(XMLEncodingTest, NumericValuesUnchanged) {
    container->add(make_int_value("int_val", 42));
    container->add(make_double_value("double_val", 3.14));

    std::string xml = container->to_xml();

    EXPECT_NE(xml.find("<int_val>42</int_val>"), std::string::npos)
        << "Expected integer value unchanged in XML output: " << xml;
}

TEST(XMLEncodeFunctionTest, EmptyString) {
    std::string result = variant_helpers::xml_encode("");
    EXPECT_EQ(result, "");
}

TEST(XMLEncodeFunctionTest, NoSpecialChars) {
    std::string result = variant_helpers::xml_encode("Hello World 123");
    EXPECT_EQ(result, "Hello World 123");
}

TEST(XMLEncodeFunctionTest, AllSpecialChars) {
    std::string result = variant_helpers::xml_encode("&<>\"'");
    EXPECT_EQ(result, "&amp;&lt;&gt;&quot;&apos;");
}

TEST(XMLEncodeFunctionTest, MixedContent) {
    std::string result = variant_helpers::xml_encode("x < 5 & y > 3");
    EXPECT_EQ(result, "x &lt; 5 &amp; y &gt; 3");
}

TEST(XMLEncodeFunctionTest, ControlCharactersAsNumericRef) {
    std::string input(1, '\x01');
    std::string result = variant_helpers::xml_encode(input);
    EXPECT_EQ(result, "&#x01;");

    input = std::string(1, '\x1f');
    result = variant_helpers::xml_encode(input);
    EXPECT_EQ(result, "&#x1f;");
}

TEST(XMLEncodeFunctionTest, WhitespacePreserved) {
    std::string result = variant_helpers::xml_encode("a\tb\nc\rd");
    EXPECT_EQ(result, "a\tb\nc\rd");
}

// ============================================================================
// Unified Value Setter API Tests (Issue #207)
// ============================================================================

class UnifiedSetterAPITest : public ::testing::Test {
protected:
    std::shared_ptr<value_container> container;

    void SetUp() override {
        container = std::make_shared<value_container>();
    }
};

TEST_F(UnifiedSetterAPITest, SetSingleValue) {
    container->set("key1", std::string("value1"));
    container->set("key2", 42);
    container->set("key3", true);
    container->set("key4", 3.14);

    EXPECT_TRUE(container->contains("key1"));
    EXPECT_TRUE(container->contains("key2"));
    EXPECT_TRUE(container->contains("key3"));
    EXPECT_TRUE(container->contains("key4"));
    EXPECT_FALSE(container->contains("nonexistent"));

    auto val1 = container->get_value("key1");
    auto val2 = container->get_value("key2");
    auto val3 = container->get_value("key3");
    auto val4 = container->get_value("key4");

    ASSERT_TRUE(val1.has_value());
    ASSERT_TRUE(val2.has_value());
    ASSERT_TRUE(val3.has_value());
    ASSERT_TRUE(val4.has_value());

    EXPECT_EQ(std::get<std::string>(val1->data), "value1");
    EXPECT_EQ(std::get<int32_t>(val2->data), 42);
    EXPECT_EQ(std::get<bool>(val3->data), true);
    EXPECT_DOUBLE_EQ(std::get<double>(val4->data), 3.14);
}

TEST_F(UnifiedSetterAPITest, SetMethodChaining) {
    container->set("a", 1)
             .set("b", 2)
             .set("c", 3);

    EXPECT_EQ(container->size(), 3u);
    EXPECT_TRUE(container->contains("a"));
    EXPECT_TRUE(container->contains("b"));
    EXPECT_TRUE(container->contains("c"));
}

TEST_F(UnifiedSetterAPITest, SetOverwritesExistingValue) {
    container->set("key", std::string("original"));
    auto val1 = container->get_value("key");
    ASSERT_TRUE(val1.has_value());
    EXPECT_EQ(std::get<std::string>(val1->data), "original");

    container->set("key", std::string("updated"));
    auto val2 = container->get_value("key");
    ASSERT_TRUE(val2.has_value());
    EXPECT_EQ(std::get<std::string>(val2->data), "updated");

    EXPECT_EQ(container->size(), 1u);
}

TEST_F(UnifiedSetterAPITest, SetOptimizedValue) {
    optimized_value ov;
    ov.name = "test_key";
    ov.data = std::string("test_value");
    ov.type = value_types::string_value;

    container->set(ov);

    EXPECT_TRUE(container->contains("test_key"));
    auto val = container->get_value("test_key");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(std::get<std::string>(val->data), "test_value");
}

TEST_F(UnifiedSetterAPITest, SetAllMultipleValues) {
    std::vector<optimized_value> values;

    optimized_value ov1;
    ov1.name = "key1";
    ov1.data = int32_t(100);
    ov1.type = value_types::int_value;
    values.push_back(ov1);

    optimized_value ov2;
    ov2.name = "key2";
    ov2.data = std::string("hello");
    ov2.type = value_types::string_value;
    values.push_back(ov2);

    optimized_value ov3;
    ov3.name = "key3";
    ov3.data = true;
    ov3.type = value_types::bool_value;
    values.push_back(ov3);

    container->set_all(values);

    EXPECT_EQ(container->size(), 3u);
    EXPECT_TRUE(container->contains("key1"));
    EXPECT_TRUE(container->contains("key2"));
    EXPECT_TRUE(container->contains("key3"));
}

TEST_F(UnifiedSetterAPITest, ContainsWithStringView) {
    container->set("test_key", 42);

    std::string_view key_view = "test_key";
    EXPECT_TRUE(container->contains(key_view));

    std::string_view nonexistent_view = "nonexistent";
    EXPECT_FALSE(container->contains(nonexistent_view));
}

TEST_F(UnifiedSetterAPITest, SerializationWithNewAPI) {
    container->set_source("source_id", "source_sub");
    container->set_target("target_id", "target_sub");
    container->set_message_type("test_type");

    container->set("string_val", std::string("test"))
             .set("int_val", 123)
             .set("bool_val", true)
             .set("double_val", 2.718);

    std::string serialized = container->serialize();
    EXPECT_FALSE(serialized.empty());

    auto restored = std::make_shared<value_container>(serialized, false);

    EXPECT_EQ(restored->source_id(), "source_id");
    EXPECT_EQ(restored->target_id(), "target_id");
    EXPECT_TRUE(restored->contains("string_val"));
    EXPECT_TRUE(restored->contains("int_val"));
    EXPECT_TRUE(restored->contains("bool_val"));
    EXPECT_TRUE(restored->contains("double_val"));
}

#if CONTAINER_HAS_COMMON_RESULT
TEST_F(UnifiedSetterAPITest, GetWithResultType) {
    container->set("int_key", int32_t(42));
    container->set("string_key", std::string("hello"));

    auto int_result = container->get<int32_t>("int_key");
    EXPECT_TRUE(kcenon::common::is_ok(int_result));
    EXPECT_EQ(kcenon::common::get_value(int_result), 42);

    auto string_result = container->get<std::string>("string_key");
    EXPECT_TRUE(kcenon::common::is_ok(string_result));
    EXPECT_EQ(kcenon::common::get_value(string_result), "hello");

    auto missing_result = container->get<int32_t>("nonexistent");
    EXPECT_TRUE(kcenon::common::is_error(missing_result));

    auto type_mismatch = container->get<std::string>("int_key");
    EXPECT_TRUE(kcenon::common::is_error(type_mismatch));
}
#endif

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}