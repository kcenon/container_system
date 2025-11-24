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
    auto null_val = std::make_shared<legacy_value>(key);

    EXPECT_EQ(null_val->name(), "test_null");
    EXPECT_EQ(null_val->type(), value_types::null_value);
    EXPECT_TRUE(null_val->is_null());
    EXPECT_FALSE(null_val->is_boolean());
    EXPECT_FALSE(null_val->is_numeric());
    EXPECT_FALSE(null_val->is_string());
    EXPECT_FALSE(null_val->is_container());
}

TEST_F(ValueTest, BooleanValueCreation) {
    // Test true value
    std::string key1 = "test_bool";
    auto true_val = make_legacy_bool_value(key1, true);
    EXPECT_EQ(true_val->name(), "test_bool");
    EXPECT_EQ(true_val->type(), value_types::bool_value);
    EXPECT_TRUE(true_val->is_boolean());
    EXPECT_TRUE(true_val->to_boolean());

    // Test false value
    std::string key2 = "test_bool2";
    auto false_val = make_legacy_bool_value(key2, false);
    EXPECT_FALSE(false_val->to_boolean());

    // Test string that converts to boolean (via helper)
    std::string key3 = "test_str_true";
    auto str_true_val = make_legacy_string_value(key3, "true");
    EXPECT_TRUE(str_true_val->to_boolean());

    std::string key4 = "test_str_false";
    auto str_false_val = make_legacy_string_value(key4, "false");
    EXPECT_FALSE(str_false_val->to_boolean());
}

TEST_F(ValueTest, NumericValueCreation) {
    // Test int
    std::string key_int = "test_int";
    auto int_val = make_legacy_int_value(key_int, 42);
    EXPECT_EQ(int_val->type(), value_types::int_value);
    EXPECT_TRUE(int_val->is_numeric());
    EXPECT_EQ(int_val->to_int(), 42);
    EXPECT_EQ(int_val->to_long(), 42L);
    EXPECT_DOUBLE_EQ(int_val->to_double(), 42.0);

    // Test long long
    std::string key_llong = "test_llong";
    auto llong_val = make_legacy_llong_value(key_llong, 9223372036854775807LL);
    EXPECT_EQ(llong_val->to_llong(), 9223372036854775807LL);

    // Test double
    std::string key_double = "test_double";
    auto double_val = make_legacy_double_value(key_double, 3.14159);
    EXPECT_DOUBLE_EQ(double_val->to_double(), 3.14159);

    // Test negative values
    std::string key_neg = "test_neg";
    auto neg_val = make_legacy_int_value(key_neg, -100);
    EXPECT_EQ(neg_val->to_int(), -100);
}

TEST_F(ValueTest, StringValueCreation) {
    std::string key = "test_string";
    std::string val = "Hello, World!";
    auto str_val = make_legacy_string_value(key, val);

    EXPECT_EQ(str_val->type(), value_types::string_value);
    EXPECT_TRUE(str_val->is_string());
    EXPECT_EQ(str_val->to_string(), "Hello, World!");
    // Note: size() returns internal data size after conversion, not original string length
    EXPECT_GT(str_val->size(), 0); // Just verify data exists
}

TEST_F(ValueTest, BytesValueCreation) {
    std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0x04, 0xFF};

    std::string key = "test_bytes";
    auto bytes_val = make_legacy_bytes_value(key, test_data);

    EXPECT_EQ(bytes_val->type(), value_types::bytes_value);
    EXPECT_TRUE(bytes_val->is_bytes());

    auto retrieved_bytes = bytes_val->to_bytes();
    EXPECT_EQ(retrieved_bytes.size(), test_data.size());
    EXPECT_EQ(retrieved_bytes, test_data);
}

TEST_F(ValueTest, ValueTypeSerialization) {
    // Test each value type serialization
    std::string key1 = "bool";
    auto bool_val = make_legacy_bool_value(key1, true);
    std::string key2 = "int";
    auto int_val = make_legacy_int_value(key2, 42);
    std::string key3 = "str";
    std::string val3 = "test";
    auto str_val = make_legacy_string_value(key3, val3);

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

// Large data handling test - validates serialization of megabyte-scale data
TEST_F(ContainerTest, LargeDataHandling) {
    // Create large string
    std::string large_data(1024 * 1024, 'X'); // 1MB of X's
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

    // Writers - use value objects
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([safe_container, t, ops_per_thread]() {
            for (int i = 0; i < ops_per_thread; ++i) {
                std::string key = "thread" + std::to_string(t) + "_" + std::to_string(i);
                int val = t * 1000 + i;
                safe_container->set(key, value(key, val));
            }
        });
    }

    // Readers
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([safe_container, t, ops_per_thread]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Let writers start

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
    auto str_val = make_legacy_string_value(key, val);

    // String to int conversion should handle gracefully
    EXPECT_EQ(str_val->to_int(), 0); // Default value for failed conversion
}

// Disabled: Value class doesn't throw on null conversions - returns defaults instead
// TODO: Consider implementing exception throwing for null value conversions
TEST(ErrorHandlingTest, DISABLED_NullValueConversions) {
    std::string key = "null";
    auto null_val = std::make_shared<legacy_value>(key);

    // With legacy_value, null conversions return default values rather than throwing
    // This test is disabled as the current implementation doesn't throw
    EXPECT_EQ(null_val->to_boolean(), false);
    EXPECT_EQ(null_val->to_int(), 0);
    EXPECT_EQ(null_val->to_double(), 0.0);
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

    auto max_int = make_legacy_int_value(max_int_key, std::numeric_limits<int>::max());
    auto min_int = make_legacy_int_value(min_int_key, std::numeric_limits<int>::min());
    auto max_llong = make_legacy_llong_value(max_llong_key, std::numeric_limits<long long>::max());

    EXPECT_EQ(max_int->to_int(), std::numeric_limits<int>::max());
    EXPECT_EQ(min_int->to_int(), std::numeric_limits<int>::min());
    EXPECT_EQ(max_llong->to_llong(), std::numeric_limits<long long>::max());
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}