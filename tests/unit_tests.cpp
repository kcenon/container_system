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
    container->set(key1, value1);
    container->set(key2, 100);
    container->set(key3, true);

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
    container->set(str_key, str_val);
    container->set(num_key, 42);

    // Serialize
    std::string serialized = container->serialize_string(value_container::serialization_format::binary).value();

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
    nested->set(nested_key, nested_value);

    // Serialize nested container
    std::string nested_data = nested->serialize_string(value_container::serialization_format::binary).value();

    // Add to main container as string value (containing serialized container data)
    std::string child_key = "child";
    container->set(child_key, nested_data);

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
    container->set(key, val1);
    container->set(key, val2);
    container->set(key, val3);

    // Get first value with name (current API behavior)
    auto item = container->get_value("item");
    EXPECT_TRUE(item.has_value());
}

TEST_F(ContainerTest, ContainerCopy) {
    // Setup original
    container->set_message_type("original");
    std::string key = "key";
    std::string val = "value";
    container->set(key, val);

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

    container->set(key, large_data);

    // Serialize and deserialize - parse_only_header=false to parse values too
    std::string serialized = container->serialize_string(value_container::serialization_format::binary).value();
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
        container->set(std::string("key") + std::to_string(i), i);
    }

    // Serialize once
    std::string serialized = container->serialize_string(value_container::serialization_format::binary).value();

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
        container->set(key, value);
    }

    // Measure serialization time
    auto start = std::chrono::high_resolution_clock::now();
    std::string serialized = container->serialize_string(value_container::serialization_format::binary).value();
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
        container->set(key, value);
    }
    std::string serialized = container->serialize_string(value_container::serialization_format::binary).value();

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
    std::string serialized = container->serialize_string(value_container::serialization_format::binary).value();
    
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

    container->set(underscore_key, underscore_value);
    container->set(camel_key, camel_value);
    container->set(numeric_key, numeric_value);

    // Serialize and restore
    std::string serialized = container->serialize_string(value_container::serialization_format::binary).value();
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
    // Thread safety is always enabled since v0.2.0

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
    container->set(key, value);

    std::string json = container->serialize_string(value_container::serialization_format::json).value();

    // Verify that quotes are escaped
    EXPECT_NE(json.find("Hello \\\"World\\\""), std::string::npos)
        << "Expected escaped quotes in JSON output: " << json;
}

TEST_F(JSONEscapingTest, BackslashEscaping) {
    std::string key = "path";
    std::string value = "C:\\Users\\test";
    container->set(key, value);

    std::string json = container->serialize_string(value_container::serialization_format::json).value();

    // Verify that backslashes are escaped
    EXPECT_NE(json.find("C:\\\\Users\\\\test"), std::string::npos)
        << "Expected escaped backslashes in JSON output: " << json;
}

TEST_F(JSONEscapingTest, NewlineEscaping) {
    std::string key = "multiline";
    std::string value = "line1\nline2\r\nline3";
    container->set(key, value);

    std::string json = container->serialize_string(value_container::serialization_format::json).value();

    // Verify that newlines are escaped
    EXPECT_NE(json.find("line1\\nline2\\r\\nline3"), std::string::npos)
        << "Expected escaped newlines in JSON output: " << json;
}

TEST_F(JSONEscapingTest, TabEscaping) {
    std::string key = "tabbed";
    std::string value = "col1\tcol2\tcol3";
    container->set(key, value);

    std::string json = container->serialize_string(value_container::serialization_format::json).value();

    // Verify that tabs are escaped
    EXPECT_NE(json.find("col1\\tcol2\\tcol3"), std::string::npos)
        << "Expected escaped tabs in JSON output: " << json;
}

TEST_F(JSONEscapingTest, ControlCharacterEscaping) {
    std::string key = "control";
    std::string value = std::string("before\x01\x02\x03" "after");
    container->set(key, value);

    std::string json = container->serialize_string(value_container::serialization_format::json).value();

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
    container->set(key, value);

    std::string json = container->serialize_string(value_container::serialization_format::json).value();

    // Verify that form feed and backspace are escaped
    EXPECT_NE(json.find("form\\ffeed\\bbackspace"), std::string::npos)
        << "Expected escaped form feed and backspace in JSON output: " << json;
}

TEST_F(JSONEscapingTest, AllSpecialCharactersCombined) {
    std::string key = "complex";
    std::string value = "Quote: \" Backslash: \\ Newline:\n Tab:\t End";
    container->set(key, value);

    std::string json = container->serialize_string(value_container::serialization_format::json).value();

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

    std::string json = container->serialize_string(value_container::serialization_format::json).value();

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
    container->set(key, value);

    std::string json = container->serialize_string(value_container::serialization_format::json).value();

    // Verify field name is escaped
    EXPECT_NE(json.find("field\\\"name"), std::string::npos)
        << "Expected escaped field name in JSON output: " << json;
}

TEST_F(JSONEscapingTest, ValidJSONOutput) {
    std::string key = "test";
    std::string value = "Hello \"World\" with \\ and \n special chars";
    container->set(key, value);

    std::string json = container->serialize_string(value_container::serialization_format::json).value();

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
    container->set(key, value);

    std::string json = container->serialize_string(value_container::serialization_format::json).value();

    // Empty string should produce valid JSON with empty value
    EXPECT_NE(json.find("\"empty\":\"\""), std::string::npos)
        << "Expected empty string value in JSON output: " << json;
}

TEST_F(JSONEscapingTest, NumericValuesUnchanged) {
    std::string key1 = "int_val";
    std::string key2 = "double_val";
    container->set(key1, 42);
    container->set(key2, 3.14);

    std::string json = container->serialize_string(value_container::serialization_format::json).value();

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
    container->set(key, value);

    std::string xml = container->serialize_string(value_container::serialization_format::xml).value();

    EXPECT_NE(xml.find("a &amp; b"), std::string::npos)
        << "Expected encoded ampersand in XML output: " << xml;
}

TEST_F(XMLEncodingTest, LessThanEncoding) {
    std::string key = "query";
    std::string value = "SELECT * FROM users WHERE id < 5";
    container->set(key, value);

    std::string xml = container->serialize_string(value_container::serialization_format::xml).value();

    EXPECT_NE(xml.find("id &lt; 5"), std::string::npos)
        << "Expected encoded less-than in XML output: " << xml;
}

TEST_F(XMLEncodingTest, GreaterThanEncoding) {
    std::string key = "query";
    std::string value = "x > 10";
    container->set(key, value);

    std::string xml = container->serialize_string(value_container::serialization_format::xml).value();

    EXPECT_NE(xml.find("x &gt; 10"), std::string::npos)
        << "Expected encoded greater-than in XML output: " << xml;
}

TEST_F(XMLEncodingTest, QuoteEncoding) {
    std::string key = "message";
    std::string value = "Hello \"World\"";
    container->set(key, value);

    std::string xml = container->serialize_string(value_container::serialization_format::xml).value();

    EXPECT_NE(xml.find("Hello &quot;World&quot;"), std::string::npos)
        << "Expected encoded quotes in XML output: " << xml;
}

TEST_F(XMLEncodingTest, ApostropheEncoding) {
    std::string key = "message";
    std::string value = "It's working";
    container->set(key, value);

    std::string xml = container->serialize_string(value_container::serialization_format::xml).value();

    EXPECT_NE(xml.find("It&apos;s working"), std::string::npos)
        << "Expected encoded apostrophe in XML output: " << xml;
}

TEST_F(XMLEncodingTest, AllSpecialCharactersCombined) {
    std::string key = "complex";
    std::string value = "a < b & c > d \"quoted\" and 'apostrophe'";
    container->set(key, value);

    std::string xml = container->serialize_string(value_container::serialization_format::xml).value();

    EXPECT_NE(xml.find("a &lt; b &amp; c &gt; d &quot;quoted&quot; and &apos;apostrophe&apos;"), std::string::npos)
        << "Expected all XML special characters encoded: " << xml;
}

TEST_F(XMLEncodingTest, ControlCharacterEncoding) {
    std::string key = "control";
    std::string value = std::string("before\x01\x02" "after");
    container->set(key, value);

    std::string xml = container->serialize_string(value_container::serialization_format::xml).value();

    EXPECT_NE(xml.find("&#x01;"), std::string::npos)
        << "Expected encoded control character &#x01; in XML output: " << xml;
    EXPECT_NE(xml.find("&#x02;"), std::string::npos)
        << "Expected encoded control character &#x02; in XML output: " << xml;
}

TEST_F(XMLEncodingTest, WhitespacePreserved) {
    std::string key = "whitespace";
    std::string value = "line1\nline2\tcolumn2\rend";
    container->set(key, value);

    std::string xml = container->serialize_string(value_container::serialization_format::xml).value();

    // Tab, newline, and carriage return should be preserved (not encoded)
    EXPECT_NE(xml.find("line1\nline2\tcolumn2\rend"), std::string::npos)
        << "Expected whitespace to be preserved in XML output: " << xml;
}

TEST_F(XMLEncodingTest, HeaderFieldEncoding) {
    container->set_source("source<id", "sub&id");
    container->set_target("target>id", "sub\"id");

    std::string xml = container->serialize_string(value_container::serialization_format::xml).value();

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
    container->set(key, value);

    std::string xml = container->serialize_string(value_container::serialization_format::xml).value();

    // Basic XML structure validation
    EXPECT_EQ(xml.substr(0, 11), "<container>");
    EXPECT_EQ(xml.substr(xml.size() - 12), "</container>");
    EXPECT_NE(xml.find("<header>"), std::string::npos);
    EXPECT_NE(xml.find("</header>"), std::string::npos);
    EXPECT_NE(xml.find("<values>"), std::string::npos);
    EXPECT_NE(xml.find("</values>"), std::string::npos);
}

TEST_F(XMLEncodingTest, NumericValuesUnchanged) {
    container->set(std::string("int_val"), 42);
    container->set(std::string("double_val"), 3.14);

    std::string xml = container->serialize_string(value_container::serialization_format::xml).value();

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

    std::string serialized = container->serialize_string(value_container::serialization_format::binary).value();
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

// ============================================================================
// Zero-Copy Deserialization Tests (Issue #226)
// ============================================================================

class ZeroCopyTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ZeroCopyTest, ZeroCopyModeEnabledOnParseOnlyHeader) {
    // Create a container with some data
    auto original = std::make_shared<value_container>();
    original->set_message_type("test_message");
    original->set("name", std::string("test_value"));
    original->set("count", 42);

    // Serialize it
    std::string serialized = original->serialize_string(value_container::serialization_format::binary).value();

    // Deserialize with parse_only_header = true
    auto restored = std::make_shared<value_container>(serialized, true);

    // Should be in zero-copy mode
    EXPECT_TRUE(restored->is_zero_copy_mode());
}

TEST_F(ZeroCopyTest, ZeroCopyModeDisabledOnFullParse) {
    // Create a container with some data
    auto original = std::make_shared<value_container>();
    original->set_message_type("test_message");
    original->set("name", std::string("test_value"));

    // Serialize it
    std::string serialized = original->serialize_string(value_container::serialization_format::binary).value();

    // Deserialize with parse_only_header = false
    auto restored = std::make_shared<value_container>(serialized, false);

    // Should NOT be in zero-copy mode when full parsing
    EXPECT_FALSE(restored->is_zero_copy_mode());
}

TEST_F(ZeroCopyTest, GetViewReturnsValueInZeroCopyMode) {
    // Create a container with string data
    auto original = std::make_shared<value_container>();
    original->set_message_type("test_message");
    original->set("greeting", std::string("Hello World"));
    original->set("number", 42);

    // Serialize and restore with lazy parsing
    std::string serialized = original->serialize_string(value_container::serialization_format::binary).value();
    auto restored = std::make_shared<value_container>(serialized, true);

    ASSERT_TRUE(restored->is_zero_copy_mode());

    // Get view for string value
    auto view = restored->get_view("greeting");
    ASSERT_TRUE(view.has_value());
    EXPECT_EQ(view->as_string_view(), "Hello World");
    EXPECT_EQ(view->type(), value_types::string_value);
}

TEST_F(ZeroCopyTest, GetViewReturnsNulloptForMissingKey) {
    auto original = std::make_shared<value_container>();
    original->set_message_type("test_message");
    original->set("existing", std::string("value"));

    std::string serialized = original->serialize_string(value_container::serialization_format::binary).value();
    auto restored = std::make_shared<value_container>(serialized, true);

    ASSERT_TRUE(restored->is_zero_copy_mode());

    // Get view for non-existent key
    auto view = restored->get_view("nonexistent");
    EXPECT_FALSE(view.has_value());
}

TEST_F(ZeroCopyTest, GetViewReturnsNulloptWhenNotInZeroCopyMode) {
    auto original = std::make_shared<value_container>();
    original->set_message_type("test_message");
    original->set("key", std::string("value"));

    std::string serialized = original->serialize_string(value_container::serialization_format::binary).value();
    // Full parse (not zero-copy mode)
    auto restored = std::make_shared<value_container>(serialized, false);

    ASSERT_FALSE(restored->is_zero_copy_mode());

    // get_view should return nullopt when not in zero-copy mode
    auto view = restored->get_view("key");
    EXPECT_FALSE(view.has_value());
}

TEST_F(ZeroCopyTest, ValueViewAsStringReturnsOwnedCopy) {
    auto original = std::make_shared<value_container>();
    original->set("text", std::string("test string data"));

    std::string serialized = original->serialize_string(value_container::serialization_format::binary).value();
    auto restored = std::make_shared<value_container>(serialized, true);

    auto view = restored->get_view("text");
    ASSERT_TRUE(view.has_value());

    // as_string() should return a copy
    std::string owned = view->as_string();
    EXPECT_EQ(owned, "test string data");
}

TEST_F(ZeroCopyTest, ValueViewNumericParsing) {
    auto original = std::make_shared<value_container>();
    original->set("int_val", 12345);
    original->set("float_val", 3.14f);

    std::string serialized = original->serialize_string(value_container::serialization_format::binary).value();
    auto restored = std::make_shared<value_container>(serialized, true);

    // Integer value
    auto int_view = restored->get_view("int_val");
    ASSERT_TRUE(int_view.has_value());
    auto int_opt = int_view->as<int>();
    ASSERT_TRUE(int_opt.has_value());
    EXPECT_EQ(*int_opt, 12345);

    // Float value
    auto float_view = restored->get_view("float_val");
    ASSERT_TRUE(float_view.has_value());
    auto float_opt = float_view->as<float>();
    ASSERT_TRUE(float_opt.has_value());
    EXPECT_NEAR(*float_opt, 3.14f, 0.01f);
}

TEST_F(ZeroCopyTest, EnsureIndexBuiltDoesNotCrash) {
    auto original = std::make_shared<value_container>();
    original->set("key1", std::string("value1"));
    original->set("key2", 100);

    std::string serialized = original->serialize_string(value_container::serialization_format::binary).value();
    auto restored = std::make_shared<value_container>(serialized, true);

    // Should not crash
    restored->ensure_index_built();

    // Should still work after explicit build
    auto view = restored->get_view("key1");
    EXPECT_TRUE(view.has_value());
}

TEST_F(ZeroCopyTest, MultipleValuesIndexing) {
    auto original = std::make_shared<value_container>();
    original->set("first", std::string("one"));
    original->set("second", std::string("two"));
    original->set("third", std::string("three"));
    original->set("fourth", 4);

    std::string serialized = original->serialize_string(value_container::serialization_format::binary).value();
    auto restored = std::make_shared<value_container>(serialized, true);

    // All values should be accessible
    auto v1 = restored->get_view("first");
    ASSERT_TRUE(v1.has_value());
    EXPECT_EQ(v1->as_string_view(), "one");

    auto v2 = restored->get_view("second");
    ASSERT_TRUE(v2.has_value());
    EXPECT_EQ(v2->as_string_view(), "two");

    auto v3 = restored->get_view("third");
    ASSERT_TRUE(v3.has_value());
    EXPECT_EQ(v3->as_string_view(), "three");

    auto v4 = restored->get_view("fourth");
    ASSERT_TRUE(v4.has_value());
}

// ============================================================================
// Batch Operation Tests (Issue #229)
// ============================================================================

class BatchOperationTest : public ::testing::Test {
protected:
    std::unique_ptr<value_container> container;

    void SetUp() override {
        container = std::make_unique<value_container>();
    }

    void TearDown() override {
        container.reset();
    }
};

TEST_F(BatchOperationTest, BulkInsertMoveSemantics) {
    std::vector<optimized_value> values;
    values.reserve(5);

    optimized_value v1;
    v1.name = "name";
    v1.data = std::string("Alice");
    v1.type = value_types::string_value;
    values.push_back(std::move(v1));

    optimized_value v2;
    v2.name = "age";
    v2.data = 30;
    v2.type = value_types::int_value;
    values.push_back(std::move(v2));

    optimized_value v3;
    v3.name = "score";
    v3.data = 95.5;
    v3.type = value_types::double_value;
    values.push_back(std::move(v3));

    container->bulk_insert(std::move(values));

    EXPECT_EQ(container->size(), 3);

    auto name_val = container->get_value("name");
    ASSERT_TRUE(name_val.has_value());
    EXPECT_EQ(std::get<std::string>(name_val->data), "Alice");

    auto age_val = container->get_value("age");
    ASSERT_TRUE(age_val.has_value());
    EXPECT_EQ(std::get<int>(age_val->data), 30);

    auto score_val = container->get_value("score");
    ASSERT_TRUE(score_val.has_value());
    EXPECT_DOUBLE_EQ(std::get<double>(score_val->data), 95.5);
}

TEST_F(BatchOperationTest, BulkInsertWithSpan) {
    std::vector<optimized_value> values;

    optimized_value v1;
    v1.name = "key1";
    v1.data = 100;
    v1.type = value_types::int_value;
    values.push_back(v1);

    optimized_value v2;
    v2.name = "key2";
    v2.data = 200;
    v2.type = value_types::int_value;
    values.push_back(v2);

    container->bulk_insert(std::span<const optimized_value>(values), 10);

    EXPECT_EQ(container->size(), 2);
    EXPECT_TRUE(container->contains("key1"));
    EXPECT_TRUE(container->contains("key2"));
}

TEST_F(BatchOperationTest, BulkInsertEmpty) {
    std::vector<optimized_value> empty_values;
    container->bulk_insert(std::move(empty_values));

    EXPECT_EQ(container->size(), 0);
    EXPECT_TRUE(container->empty());
}

TEST_F(BatchOperationTest, GetBatchBasic) {
    container->set("name", std::string("Bob"));
    container->set("age", 25);
    container->set("city", std::string("Seattle"));

    std::vector<std::string_view> keys = {"name", "age", "missing_key"};
    auto results = container->get_batch(std::span<const std::string_view>(keys));

    ASSERT_EQ(results.size(), 3);
    EXPECT_TRUE(results[0].has_value());
    EXPECT_EQ(std::get<std::string>(results[0]->data), "Bob");
    EXPECT_TRUE(results[1].has_value());
    EXPECT_EQ(std::get<int>(results[1]->data), 25);
    EXPECT_FALSE(results[2].has_value());  // missing_key
}

TEST_F(BatchOperationTest, GetBatchMap) {
    container->set("a", 1);
    container->set("b", 2);
    container->set("c", 3);

    std::vector<std::string_view> keys = {"a", "c", "nonexistent"};
    auto result_map = container->get_batch_map(std::span<const std::string_view>(keys));

    EXPECT_EQ(result_map.size(), 2);
    EXPECT_EQ(std::get<int>(result_map["a"].data), 1);
    EXPECT_EQ(std::get<int>(result_map["c"].data), 3);
    EXPECT_EQ(result_map.find("nonexistent"), result_map.end());
}

TEST_F(BatchOperationTest, ContainsBatch) {
    container->set("exists1", 1);
    container->set("exists2", 2);

    std::vector<std::string_view> keys = {"exists1", "missing", "exists2"};
    auto results = container->contains_batch(std::span<const std::string_view>(keys));

    ASSERT_EQ(results.size(), 3);
    EXPECT_TRUE(results[0]);   // exists1
    EXPECT_FALSE(results[1]);  // missing
    EXPECT_TRUE(results[2]);   // exists2
}

TEST_F(BatchOperationTest, RemoveBatch) {
    container->set("keep1", 1);
    container->set("remove1", 2);
    container->set("keep2", 3);
    container->set("remove2", 4);
    container->set("remove3", 5);

    std::vector<std::string_view> keys_to_remove = {"remove1", "remove2", "remove3", "nonexistent"};
    size_t removed = container->remove_batch(std::span<const std::string_view>(keys_to_remove));

    EXPECT_EQ(removed, 3);
    EXPECT_EQ(container->size(), 2);
    EXPECT_TRUE(container->contains("keep1"));
    EXPECT_TRUE(container->contains("keep2"));
    EXPECT_FALSE(container->contains("remove1"));
    EXPECT_FALSE(container->contains("remove2"));
    EXPECT_FALSE(container->contains("remove3"));
}

TEST_F(BatchOperationTest, RemoveBatchEmpty) {
    container->set("key1", 1);

    std::vector<std::string_view> empty_keys;
    size_t removed = container->remove_batch(std::span<const std::string_view>(empty_keys));

    EXPECT_EQ(removed, 0);
    EXPECT_EQ(container->size(), 1);
}

TEST_F(BatchOperationTest, UpdateIfSuccess) {
    container->set("counter", 10);

    bool updated = container->update_if("counter", value_variant{10}, value_variant{20});

    EXPECT_TRUE(updated);
    auto val = container->get_value("counter");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(std::get<int>(val->data), 20);
}

TEST_F(BatchOperationTest, UpdateIfFailureMismatch) {
    container->set("counter", 10);

    bool updated = container->update_if("counter", value_variant{99}, value_variant{20});

    EXPECT_FALSE(updated);
    auto val = container->get_value("counter");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(std::get<int>(val->data), 10);  // Value unchanged
}

TEST_F(BatchOperationTest, UpdateIfKeyNotFound) {
    container->set("counter", 10);

    bool updated = container->update_if("nonexistent", value_variant{10}, value_variant{20});

    EXPECT_FALSE(updated);
}

TEST_F(BatchOperationTest, UpdateBatchIf) {
    container->set("a", 1);
    container->set("b", 2);
    container->set("c", 3);

    std::vector<value_container::update_spec> updates = {
        {"a", value_variant{1}, value_variant{10}},   // Should succeed
        {"b", value_variant{99}, value_variant{20}},  // Should fail (mismatch)
        {"c", value_variant{3}, value_variant{30}},   // Should succeed
        {"d", value_variant{4}, value_variant{40}}    // Should fail (not found)
    };

    auto results = container->update_batch_if(std::span<const value_container::update_spec>(updates));

    ASSERT_EQ(results.size(), 4);
    EXPECT_TRUE(results[0]);   // a updated
    EXPECT_FALSE(results[1]);  // b not updated
    EXPECT_TRUE(results[2]);   // c updated
    EXPECT_FALSE(results[3]);  // d not found

    EXPECT_EQ(std::get<int>(container->get_value("a")->data), 10);
    EXPECT_EQ(std::get<int>(container->get_value("b")->data), 2);  // unchanged
    EXPECT_EQ(std::get<int>(container->get_value("c")->data), 30);
}

TEST_F(BatchOperationTest, BatchOperationsPreserveOrder) {
    // Verify that get_batch returns results in the same order as input keys
    container->set("z", 1);
    container->set("a", 2);
    container->set("m", 3);

    std::vector<std::string_view> keys = {"m", "z", "a", "x"};
    auto results = container->get_batch(std::span<const std::string_view>(keys));

    ASSERT_EQ(results.size(), 4);
    EXPECT_TRUE(results[0].has_value());
    EXPECT_EQ(std::get<int>(results[0]->data), 3);  // m
    EXPECT_TRUE(results[1].has_value());
    EXPECT_EQ(std::get<int>(results[1]->data), 1);  // z
    EXPECT_TRUE(results[2].has_value());
    EXPECT_EQ(std::get<int>(results[2]->data), 2);  // a
    EXPECT_FALSE(results[3].has_value());            // x (not found)
}

TEST_F(BatchOperationTest, BatchOperationsWithDuplicateKeys) {
    container->set("key", 100);

    // get_batch with duplicate keys
    std::vector<std::string_view> keys = {"key", "key", "key"};
    auto results = container->get_batch(std::span<const std::string_view>(keys));

    ASSERT_EQ(results.size(), 3);
    for (const auto& r : results) {
        EXPECT_TRUE(r.has_value());
        EXPECT_EQ(std::get<int>(r->data), 100);
    }
}

TEST_F(BatchOperationTest, MethodChaining) {
    std::vector<optimized_value> values1;
    optimized_value v1;
    v1.name = "x";
    v1.data = 1;
    v1.type = value_types::int_value;
    values1.push_back(v1);

    std::vector<optimized_value> values2;
    optimized_value v2;
    v2.name = "y";
    v2.data = 2;
    v2.type = value_types::int_value;
    values2.push_back(v2);

    container->bulk_insert(std::move(values1))
             .set("z", 3);

    EXPECT_EQ(container->size(), 2);
    EXPECT_TRUE(container->contains("x"));
    EXPECT_TRUE(container->contains("z"));
}

// ============================================================================
// Schema Validation Tests (Issue #228)
// ============================================================================

#include "container/core/container/schema.h"

class SchemaValidationTest : public ::testing::Test {
protected:
    std::shared_ptr<value_container> container;

    void SetUp() override {
        container = std::make_shared<value_container>();
    }
};

TEST_F(SchemaValidationTest, RequiredFieldPresent) {
    container->set("user_id", std::string("U12345"));

    auto schema = container_schema()
        .require("user_id", value_types::string_value);

    auto result = schema.validate(*container);
    EXPECT_FALSE(result.has_value()) << "Validation should pass when required field is present";
}

TEST_F(SchemaValidationTest, RequiredFieldMissing) {
    auto schema = container_schema()
        .require("user_id", value_types::string_value);

    auto result = schema.validate(*container);
    ASSERT_TRUE(result.has_value()) << "Validation should fail when required field is missing";
    EXPECT_EQ(result->code, validation_codes::missing_required);
    EXPECT_EQ(result->field, "user_id");
}

TEST_F(SchemaValidationTest, OptionalFieldMissing) {
    auto schema = container_schema()
        .optional("phone", value_types::string_value);

    auto result = schema.validate(*container);
    EXPECT_FALSE(result.has_value()) << "Validation should pass when optional field is missing";
}

TEST_F(SchemaValidationTest, TypeMismatch) {
    container->set("age", std::string("not_a_number"));

    auto schema = container_schema()
        .require("age", value_types::int_value);

    auto result = schema.validate(*container);
    ASSERT_TRUE(result.has_value()) << "Validation should fail on type mismatch";
    EXPECT_EQ(result->code, validation_codes::type_mismatch);
}

TEST_F(SchemaValidationTest, IntegerRangeValid) {
    container->set("age", 25);

    auto schema = container_schema()
        .require("age", value_types::int_value)
        .range("age", 0LL, 150LL);

    auto result = schema.validate(*container);
    EXPECT_FALSE(result.has_value()) << "Validation should pass for value in range";
}

TEST_F(SchemaValidationTest, IntegerRangeInvalid) {
    container->set("age", 200);

    auto schema = container_schema()
        .require("age", value_types::int_value)
        .range("age", 0LL, 150LL);

    auto result = schema.validate(*container);
    ASSERT_TRUE(result.has_value()) << "Validation should fail for value out of range";
    EXPECT_EQ(result->code, validation_codes::out_of_range);
}

TEST_F(SchemaValidationTest, DoubleRangeValid) {
    container->set("price", 99.99);

    auto schema = container_schema()
        .require("price", value_types::double_value)
        .range("price", 0.0, 1000.0);

    auto result = schema.validate(*container);
    EXPECT_FALSE(result.has_value()) << "Validation should pass for double in range";
}

TEST_F(SchemaValidationTest, DoubleRangeInvalid) {
    container->set("price", 1500.0);

    auto schema = container_schema()
        .require("price", value_types::double_value)
        .range("price", 0.0, 1000.0);

    auto result = schema.validate(*container);
    ASSERT_TRUE(result.has_value()) << "Validation should fail for double out of range";
    EXPECT_EQ(result->code, validation_codes::out_of_range);
}

TEST_F(SchemaValidationTest, StringLengthValid) {
    container->set("username", std::string("john_doe"));

    auto schema = container_schema()
        .require("username", value_types::string_value)
        .length("username", 3, 20);

    auto result = schema.validate(*container);
    EXPECT_FALSE(result.has_value()) << "Validation should pass for valid string length";
}

TEST_F(SchemaValidationTest, StringLengthTooShort) {
    container->set("username", std::string("ab"));

    auto schema = container_schema()
        .require("username", value_types::string_value)
        .length("username", 3, 20);

    auto result = schema.validate(*container);
    ASSERT_TRUE(result.has_value()) << "Validation should fail for too short string";
    EXPECT_EQ(result->code, validation_codes::invalid_length);
}

TEST_F(SchemaValidationTest, StringLengthTooLong) {
    container->set("username", std::string("this_is_a_very_long_username"));

    auto schema = container_schema()
        .require("username", value_types::string_value)
        .length("username", 3, 20);

    auto result = schema.validate(*container);
    ASSERT_TRUE(result.has_value()) << "Validation should fail for too long string";
    EXPECT_EQ(result->code, validation_codes::invalid_length);
}

TEST_F(SchemaValidationTest, PatternValid) {
    container->set("email", std::string("user@example.com"));

    auto schema = container_schema()
        .require("email", value_types::string_value)
        .pattern("email", R"(^[\w\.-]+@[\w\.-]+\.\w+$)");

    auto result = schema.validate(*container);
    EXPECT_FALSE(result.has_value()) << "Validation should pass for valid email pattern";
}

TEST_F(SchemaValidationTest, PatternInvalid) {
    container->set("email", std::string("not-an-email"));

    auto schema = container_schema()
        .require("email", value_types::string_value)
        .pattern("email", R"(^[\w\.-]+@[\w\.-]+\.\w+$)");

    auto result = schema.validate(*container);
    ASSERT_TRUE(result.has_value()) << "Validation should fail for invalid email pattern";
    EXPECT_EQ(result->code, validation_codes::pattern_mismatch);
}

TEST_F(SchemaValidationTest, OneOfValid) {
    container->set("role", std::string("admin"));

    auto schema = container_schema()
        .require("role", value_types::string_value)
        .one_of("role", {"admin", "user", "guest"});

    auto result = schema.validate(*container);
    EXPECT_FALSE(result.has_value()) << "Validation should pass for allowed value";
}

TEST_F(SchemaValidationTest, OneOfInvalid) {
    container->set("role", std::string("superuser"));

    auto schema = container_schema()
        .require("role", value_types::string_value)
        .one_of("role", {"admin", "user", "guest"});

    auto result = schema.validate(*container);
    ASSERT_TRUE(result.has_value()) << "Validation should fail for non-allowed value";
    EXPECT_EQ(result->code, validation_codes::not_in_allowed_values);
}

TEST_F(SchemaValidationTest, CustomValidatorSuccess) {
    container->set("quantity", 10);

    auto schema = container_schema()
        .require("quantity", value_types::int_value)
        .custom("quantity", [](const optimized_value& val) -> std::optional<std::string> {
            if (auto* p = std::get_if<int>(&val.data)) {
                if (*p > 0) return std::nullopt;  // Valid
            }
            return "Quantity must be positive";
        });

    auto result = schema.validate(*container);
    EXPECT_FALSE(result.has_value()) << "Validation should pass for valid custom validation";
}

TEST_F(SchemaValidationTest, CustomValidatorFailure) {
    container->set("quantity", -5);

    auto schema = container_schema()
        .require("quantity", value_types::int_value)
        .custom("quantity", [](const optimized_value& val) -> std::optional<std::string> {
            if (auto* p = std::get_if<int>(&val.data)) {
                if (*p > 0) return std::nullopt;
            }
            return "Quantity must be positive";
        });

    auto result = schema.validate(*container);
    ASSERT_TRUE(result.has_value()) << "Validation should fail for custom validation";
    EXPECT_EQ(result->code, validation_codes::custom_validation_failed);
}

TEST_F(SchemaValidationTest, ValidateAllReturnsMultipleErrors) {
    container->set("age", std::string("not_a_number"));  // Type mismatch
    // "name" is missing (required)

    auto schema = container_schema()
        .require("name", value_types::string_value)
        .require("age", value_types::int_value);

    auto errors = schema.validate_all(*container);
    EXPECT_EQ(errors.size(), 2) << "Should have 2 validation errors";
}

TEST_F(SchemaValidationTest, NestedSchemaValid) {
    auto nested_container = std::make_shared<value_container>();
    nested_container->set("street", std::string("123 Main St"));
    nested_container->set("city", std::string("Boston"));

    container->set("address", nested_container);

    auto address_schema = container_schema()
        .require("street", value_types::string_value)
        .require("city", value_types::string_value);

    auto schema = container_schema()
        .require("address", value_types::container_value, address_schema);

    auto result = schema.validate(*container);
    EXPECT_FALSE(result.has_value()) << "Validation should pass for valid nested schema";
}

TEST_F(SchemaValidationTest, NestedSchemaInvalid) {
    auto nested_container = std::make_shared<value_container>();
    nested_container->set("street", std::string("123 Main St"));
    // Missing "city"

    container->set("address", nested_container);

    auto address_schema = container_schema()
        .require("street", value_types::string_value)
        .require("city", value_types::string_value);

    auto schema = container_schema()
        .require("address", value_types::container_value, address_schema);

    auto result = schema.validate(*container);
    ASSERT_TRUE(result.has_value()) << "Validation should fail for invalid nested schema";
    EXPECT_EQ(result->code, validation_codes::nested_validation_failed);
}

TEST_F(SchemaValidationTest, SchemaFieldCount) {
    auto schema = container_schema()
        .require("id", value_types::string_value)
        .require("name", value_types::string_value)
        .optional("email", value_types::string_value);

    EXPECT_EQ(schema.field_count(), 3);
}

TEST_F(SchemaValidationTest, HasFieldAndIsRequired) {
    auto schema = container_schema()
        .require("id", value_types::string_value)
        .optional("email", value_types::string_value);

    EXPECT_TRUE(schema.has_field("id"));
    EXPECT_TRUE(schema.has_field("email"));
    EXPECT_FALSE(schema.has_field("phone"));

    EXPECT_TRUE(schema.is_required("id"));
    EXPECT_FALSE(schema.is_required("email"));
    EXPECT_FALSE(schema.is_required("phone"));
}

TEST_F(SchemaValidationTest, ComplexSchemaValidation) {
    container->set("user_id", std::string("U12345"))
             .set("age", 25)
             .set("email", std::string("user@example.com"))
             .set("role", std::string("admin"));

    auto schema = container_schema()
        .require("user_id", value_types::string_value)
        .require("age", value_types::int_value)
        .range("age", 0LL, 150LL)
        .require("email", value_types::string_value)
        .pattern("email", R"(^[\w\.-]+@[\w\.-]+\.\w+$)")
        .optional("phone", value_types::string_value)
        .length("phone", 10, 15)
        .require("role", value_types::string_value)
        .one_of("role", {"admin", "user", "guest"});

    auto result = schema.validate(*container);
    EXPECT_FALSE(result.has_value()) << "Complex validation should pass for valid data";
}

#if SCHEMA_HAS_COMMON_RESULT
TEST_F(SchemaValidationTest, ValidateResultSuccess) {
    container->set("id", std::string("123"));

    auto schema = container_schema()
        .require("id", value_types::string_value);

    auto result = schema.validate_result(*container);
    EXPECT_TRUE(kcenon::common::is_ok(result)) << "validate_result should return ok for valid data";
}

TEST_F(SchemaValidationTest, ValidateResultFailure) {
    auto schema = container_schema()
        .require("id", value_types::string_value);

    auto result = schema.validate_result(*container);
    EXPECT_TRUE(kcenon::common::is_error(result)) << "validate_result should return error for invalid data";
}
#endif

// ===========================================================================
// Detailed Observability Metrics Tests (Issue #230)
// ===========================================================================

class MetricsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset metrics before each test
        value_container::reset_metrics();
        value_container::set_metrics_enabled(true);
    }

    void TearDown() override {
        value_container::set_metrics_enabled(false);
    }
};

TEST_F(MetricsTest, MetricsEnabledDisabled) {
    EXPECT_TRUE(value_container::is_metrics_enabled());
    value_container::set_metrics_enabled(false);
    EXPECT_FALSE(value_container::is_metrics_enabled());
    value_container::set_metrics_enabled(true);
    EXPECT_TRUE(value_container::is_metrics_enabled());
}

TEST_F(MetricsTest, OperationCountersIncrement) {
    auto container = std::make_shared<value_container>();

    // Perform operations
    container->set("key1", 42);
    container->set("key2", "hello");
    container->get_value("key1");
    container->get_value("key2");
    container->serialize_string(value_container::serialization_format::binary).value();

    auto metrics = value_container::get_detailed_metrics();

    // Check operation counts
    EXPECT_GE(metrics.operations.writes.load(), 2U);
    EXPECT_GE(metrics.operations.reads.load(), 2U);
    EXPECT_GE(metrics.operations.serializations.load(), 1U);
}

TEST_F(MetricsTest, DeserializationCounter) {
    auto container = std::make_shared<value_container>();
    container->set("test", 123);
    std::string serialized = container->serialize_string(value_container::serialization_format::binary).value();

    auto container2 = std::make_shared<value_container>();
    container2->deserialize_result(serialized, false);

    auto metrics = value_container::get_detailed_metrics();
    EXPECT_GE(metrics.operations.deserializations.load(), 1U);
}

TEST_F(MetricsTest, TimingMetricsAccumulate) {
    auto container = std::make_shared<value_container>();

    // Perform several operations
    for (int i = 0; i < 10; ++i) {
        container->set("key" + std::to_string(i), i);
        container->serialize_string(value_container::serialization_format::binary).value();
    }

    auto metrics = value_container::get_detailed_metrics();

    // Timing should be greater than 0 after operations
    EXPECT_GT(metrics.timing.total_serialize_ns.load(), 0U);
    EXPECT_GT(metrics.timing.total_write_ns.load(), 0U);
}

TEST_F(MetricsTest, MetricsReset) {
    auto container = std::make_shared<value_container>();
    container->set("key", 42);
    container->serialize_string(value_container::serialization_format::binary).value();

    // Verify counters are non-zero
    auto metrics1 = value_container::get_detailed_metrics();
    EXPECT_GT(metrics1.operations.writes.load(), 0U);

    // Reset and verify counters are zero
    value_container::reset_metrics();
    auto metrics2 = value_container::get_detailed_metrics();
    EXPECT_EQ(metrics2.operations.writes.load(), 0U);
    EXPECT_EQ(metrics2.operations.reads.load(), 0U);
    EXPECT_EQ(metrics2.operations.serializations.load(), 0U);
}

TEST_F(MetricsTest, MetricsToJsonFormat) {
    auto container = std::make_shared<value_container>();
    container->set("key", 42);
    container->serialize_string(value_container::serialization_format::binary).value();

    std::string json = container->metrics_to_json();

    // Check JSON structure
    EXPECT_NE(json.find("\"operations\""), std::string::npos);
    EXPECT_NE(json.find("\"timing\""), std::string::npos);
    EXPECT_NE(json.find("\"latency\""), std::string::npos);
    EXPECT_NE(json.find("\"simd\""), std::string::npos);
    EXPECT_NE(json.find("\"cache\""), std::string::npos);
    EXPECT_NE(json.find("\"reads\""), std::string::npos);
    EXPECT_NE(json.find("\"writes\""), std::string::npos);
}

TEST_F(MetricsTest, MetricsToPrometheusFormat) {
    auto container = std::make_shared<value_container>();
    container->set("key", 42);
    container->serialize_string(value_container::serialization_format::binary).value();

    std::string prom = container->metrics_to_prometheus();

    // Check Prometheus format
    EXPECT_NE(prom.find("# HELP"), std::string::npos);
    EXPECT_NE(prom.find("# TYPE"), std::string::npos);
    EXPECT_NE(prom.find("container_operations_total"), std::string::npos);
    EXPECT_NE(prom.find("container_serialize_latency_nanoseconds"), std::string::npos);
}

TEST_F(MetricsTest, LatencyHistogramRecording) {
    auto container = std::make_shared<value_container>();

    // Perform enough operations to populate histogram
    for (int i = 0; i < 100; ++i) {
        container->set("key" + std::to_string(i), i);
    }

    auto metrics = value_container::get_detailed_metrics();

    // Sample count should match number of operations
    EXPECT_GE(metrics.write_latency.sample_count.load(), 100U);

    // Percentiles should be calculated
    EXPECT_GE(metrics.write_latency.p50(), 0U);
}

TEST_F(MetricsTest, MetricsDisabledNoOverhead) {
    value_container::set_metrics_enabled(false);
    value_container::reset_metrics();

    auto container = std::make_shared<value_container>();
    container->set("key", 42);
    container->serialize_string(value_container::serialization_format::binary).value();

    auto metrics = value_container::get_detailed_metrics();

    // With metrics disabled, counters should remain zero
    EXPECT_EQ(metrics.operations.writes.load(), 0U);
    EXPECT_EQ(metrics.operations.serializations.load(), 0U);
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}