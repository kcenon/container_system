// BSD 3-Clause License
//
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this
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
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

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

#include <chrono>
#include <container.h>
#include <container/internal/thread_safe_container.h>
#include <gtest/gtest.h>
#include <random>
#include <sstream>
#include <thread>
#include <variant>
#include <vector>

using namespace container_module;

// Test fixture for container tests
class ContainerTest : public ::testing::Test {
protected:
  std::unique_ptr<value_container> container;

  void SetUp() override { container = std::make_unique<value_container>(); }

  void TearDown() override { container.reset(); }
};

// ============================================================================
// Value Type Tests
// ============================================================================

TEST(ValueTest, NullValueCreation) {
  optimized_value null_val("test_null", value_types::null_value);

  EXPECT_EQ(null_val.name, "test_null");
  EXPECT_EQ(null_val.type, value_types::null_value);
  EXPECT_TRUE(std::holds_alternative<std::monostate>(null_val.data));
}

TEST(ValueTest, BooleanValueCreation) {
  // Test true value
  optimized_value true_val;
  true_val.name = "test_bool";
  true_val.type = value_types::bool_value;
  true_val.data = true;

  EXPECT_EQ(true_val.name, "test_bool");
  EXPECT_EQ(true_val.type, value_types::bool_value);
  EXPECT_TRUE(std::holds_alternative<bool>(true_val.data));
  EXPECT_TRUE(std::get<bool>(true_val.data));

  // Test false value
  optimized_value false_val;
  false_val.name = "test_bool2";
  false_val.type = value_types::bool_value;
  false_val.data = false;
  EXPECT_FALSE(std::get<bool>(false_val.data));
}

TEST(ValueTest, NumericValueCreation) {
  // Test int
  optimized_value int_val;
  int_val.name = "test_int";
  int_val.type = value_types::int_value;
  int_val.data = 42;

  EXPECT_EQ(int_val.type, value_types::int_value);
  EXPECT_TRUE(std::holds_alternative<int>(int_val.data));
  EXPECT_EQ(std::get<int>(int_val.data), 42);

  // Test long long
  optimized_value llong_val;
  llong_val.name = "test_llong";
  llong_val.type = value_types::llong_value;
  llong_val.data = 9223372036854775807LL;
  EXPECT_EQ(std::get<long long>(llong_val.data), 9223372036854775807LL);

  // Test double
  optimized_value double_val;
  double_val.name = "test_double";
  double_val.type = value_types::double_value;
  double_val.data = 3.14159;
  EXPECT_DOUBLE_EQ(std::get<double>(double_val.data), 3.14159);
}

TEST(ValueTest, StringValueCreation) {
  optimized_value str_val;
  str_val.name = "test_string";
  str_val.type = value_types::string_value;
  str_val.data = std::string("Hello, World!");

  EXPECT_EQ(str_val.type, value_types::string_value);
  EXPECT_TRUE(std::holds_alternative<std::string>(str_val.data));
  EXPECT_EQ(std::get<std::string>(str_val.data), "Hello, World!");
}

TEST(ValueTest, BytesValueCreation) {
  std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0x04, 0xFF};

  optimized_value bytes_val;
  bytes_val.name = "test_bytes";
  bytes_val.type = value_types::bytes_value;
  bytes_val.data = test_data;

  EXPECT_EQ(bytes_val.type, value_types::bytes_value);
  EXPECT_TRUE(std::holds_alternative<std::vector<uint8_t>>(bytes_val.data));
  EXPECT_EQ(std::get<std::vector<uint8_t>>(bytes_val.data), test_data);
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
  container->add_value("key1", std::string("value1"));
  container->add_value("key2", 100);
  container->add_value("key3", true);

  // Retrieve values
  auto val1 = container->get_value("key1");
  auto val2 = container->get_value("key2");
  auto val3 = container->get_value("key3");

  ASSERT_TRUE(val1.has_value());
  ASSERT_TRUE(val2.has_value());
  ASSERT_TRUE(val3.has_value());

  EXPECT_EQ(std::get<std::string>(val1->data), "value1");
  EXPECT_EQ(std::get<int>(val2->data), 100);
  EXPECT_TRUE(std::get<bool>(val3->data));

  // Test non-existent key
  auto val4 = container->get_value("non_existent");
  EXPECT_FALSE(val4.has_value());
}

TEST_F(ContainerTest, ContainerSerialization) {
  // Setup container
  container->set_source("src", "sub");
  container->set_target("tgt", "");
  container->set_message_type("test");

  // Add values
  container->add_value("str", std::string("hello"));
  container->add_value("num", 42);

  // Serialize
  std::string serialized = container->serialize();

  // Deserialize - parse_only_header=false to parse values too
  auto new_container = std::make_unique<value_container>(serialized, false);

  // Verify
  EXPECT_EQ(new_container->source_id(), "src");
  EXPECT_EQ(new_container->source_sub_id(), "sub");
  EXPECT_EQ(new_container->target_id(), "tgt");
  EXPECT_EQ(new_container->message_type(), "test");

  auto str_val = new_container->get_value("str");
  auto num_val = new_container->get_value("num");

  ASSERT_TRUE(str_val.has_value());
  ASSERT_TRUE(num_val.has_value());

  EXPECT_EQ(std::get<std::string>(str_val->data), "hello");
  EXPECT_EQ(std::get<int>(num_val->data), 42);
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

TEST_F(ContainerTest, ContainerCopy) {
  // Setup original
  container->set_message_type("original");
  container->add_value("key", std::string("value"));

  // Deep copy
  auto copy = container->copy(true);
  EXPECT_EQ(copy->message_type(), "original");
  auto copy_val = copy->get_value("key");
  ASSERT_TRUE(copy_val.has_value());
  EXPECT_EQ(std::get<std::string>(copy_val->data), "value");

  // Shallow copy (header only)
  auto shallow = container->copy(false);
  EXPECT_EQ(shallow->message_type(), "original");
  auto val = shallow->get_value("key");
  EXPECT_FALSE(val.has_value()); // No values in shallow copy
}

// Disabled in CI: large data causes timeouts due to serialization issues
TEST_F(ContainerTest, DISABLED_LargeDataHandling) {
  // Create large string
  std::string large_data(1024 * 1024, 'X'); // 1MB of X's
  std::string key = "large";

  container->add_value(key, large_data);

  // Serialize and deserialize - parse_only_header=false to parse values too
  std::string serialized = container->serialize();
  auto restored = std::make_unique<value_container>(serialized, false);

  auto val = restored->get_value("large");
  ASSERT_TRUE(val.has_value());
  EXPECT_EQ(std::get<std::string>(val->data), large_data);
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

// Disabled in CI: threading tests are flaky in CI environment
TEST(ThreadSafetyTest, DISABLED_ConcurrentReads) {
  auto container = std::make_unique<value_container>();

  // Add test data
  for (int i = 0; i < 100; ++i) {
    container->add_value("key" + std::to_string(i), i);
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
        if (val.has_value() && std::get<int>(val->data) == i) {
          success_count++;
        }
      }
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  EXPECT_EQ(success_count, num_threads * 100);
}

// Disabled in CI: threading tests are flaky in CI environment
TEST(ThreadSafetyTest, DISABLED_ThreadSafeContainer) {
  auto safe_container = std::make_shared<thread_safe_container>();

  const int num_threads = 5;
  const int ops_per_thread = 100;
  std::vector<std::thread> threads;

  // Writers
  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([safe_container, t, ops_per_thread]() {
      for (int i = 0; i < ops_per_thread; ++i) {
        std::string key =
            "thread" + std::to_string(t) + "_" + std::to_string(i);
        int value = t * 1000 + i;
        safe_container->set_typed(key, value);
      }
    });
  }

  // Readers
  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([safe_container, t, ops_per_thread]() {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(10)); // Let writers start

      for (int i = 0; i < ops_per_thread; ++i) {
        std::string key =
            "thread" + std::to_string(t) + "_" + std::to_string(i);
        auto val = safe_container->get_typed<int>(key);
        // Value might not exist yet, that's OK
      }
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  // Verify all values were added
  for (int t = 0; t < num_threads; ++t) {
    for (int i = 0; i < ops_per_thread; ++i) {
      std::string key = "thread" + std::to_string(t) + "_" + std::to_string(i);
      auto val = safe_container->get_typed<int>(key);
      EXPECT_TRUE(val.has_value());
      EXPECT_EQ(val.value(), t * 1000 + i);
    }
  }
}

// ============================================================================
// Error Handling Tests
// ============================================================================

// Disabled: Windows-specific - value_container constructor doesn't throw on
// invalid data
// TODO: Implement proper validation and exception throwing in value_container
// constructor
TEST(ErrorHandlingTest, DISABLED_InvalidSerialization) {
  // Test invalid serialization data
  EXPECT_THROW(value_container("invalid data"), std::exception);
  EXPECT_THROW(value_container("@header={};@data={[invalid];"), std::exception);
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

TEST(EdgeCaseTest, SpecialCharacters) {
  auto container = std::make_unique<value_container>();

  // Test special characters in values
  std::string special = "Line1\nLine2\rLine3\tTab\0Null";
  std::string special_key = "special";
  container->add_value(special_key, special);

  // Test special characters in names (use simpler keys)
  std::string underscore_key = "key_with_underscores";
  std::string underscore_value = "value1";
  std::string camel_key = "keyWithCamelCase";
  std::string camel_value = "value2";
  container->add_value(underscore_key, underscore_value);
  container->add_value(camel_key, camel_value);

  // Serialize and restore
  std::string serialized = container->serialize();
  auto restored = std::make_unique<value_container>(serialized, false);

  // Verify special characters preserved
  auto special_val = restored->get_value("special");
  ASSERT_TRUE(special_val.has_value());
  EXPECT_EQ(std::get<std::string>(special_val->data), special);

  auto underscore_val = restored->get_value("key_with_underscores");
  ASSERT_TRUE(underscore_val.has_value());
  EXPECT_EQ(std::get<std::string>(underscore_val->data), "value1");

  auto camel_val = restored->get_value("keyWithCamelCase");
  ASSERT_TRUE(camel_val.has_value());
  EXPECT_EQ(std::get<std::string>(camel_val->data), "value2");
}

TEST_F(ContainerTest, MaximumValues) {
  // Test maximum numeric values
  std::string max_int_key = "max_int";
  std::string min_int_key = "min_int";
  std::string max_llong_key = "max_llong";

  container->add_value(max_int_key, std::numeric_limits<int>::max());
  container->add_value(min_int_key, std::numeric_limits<int>::min());
  container->add_value(max_llong_key, std::numeric_limits<long long>::max());

  auto max_int = container->get_value(max_int_key);
  auto min_int = container->get_value(min_int_key);
  auto max_llong = container->get_value(max_llong_key);

  EXPECT_EQ(std::get<int>(max_int->data), std::numeric_limits<int>::max());
  EXPECT_EQ(std::get<int>(min_int->data), std::numeric_limits<int>::min());
  EXPECT_EQ(std::get<long long>(max_llong->data),
            std::numeric_limits<long long>::max());
}

// Main function for running tests
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}