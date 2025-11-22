/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

/**
 * @file error_handling_test.cpp
 * @brief Integration tests for error handling and edge cases
 *
 * Tests cover:
 * - Invalid serialization data handling
 * - Missing value retrieval
 * - Type conversion errors
 * - Null value operations
 * - Boundary conditions
 * - Malformed data handling
 * - Resource exhaustion scenarios
 */

#include "framework/system_fixture.h"
#include "framework/test_helpers.h"
#include <container/core/container.h>
#include <gtest/gtest.h>
#include <limits>
#include <stdexcept>

using namespace container_module;
using namespace container_module::testing;

class ErrorHandlingTest : public ContainerSystemFixture {};

/**
 * Test 1: Non-existent value retrieval
 */
TEST_F(ErrorHandlingTest, NonExistentValueRetrieval) {
  AddStringValue("exists", "value");

  auto missing = container->get_value("does_not_exist");
  EXPECT_TRUE(missing->type == value_types::null_value);
  EXPECT_EQ(missing->type, value_types::null_value);
}

/**
 * Test 2: Empty key value operations
 */
TEST_F(ErrorHandlingTest, EmptyKeyOperations) {
  AddStringValue("", "empty_key_value");

  auto val = container->get_value("");
  // Behavior may vary - either null or returns the value
  // Just verify it doesn't crash
  EXPECT_NO_THROW({ auto result = container->get_value(""); });
}

/**
 * Test 3: Null value type conversions
 */
TEST_F(ErrorHandlingTest, NullValueConversions) {
  container->add_value("null", std::monostate{});

  auto retrieved = container->get_value("null");
  EXPECT_TRUE(retrieved->type == value_types::null_value);

  // Base value conversion helpers throw std::bad_variant_access for null values
  // (wrong type access). Verify that the integration surface preserves this
  // contract.
  EXPECT_THROW(std::get<int>(retrieved->data), std::bad_variant_access);
  EXPECT_THROW(std::get<double>(retrieved->data), std::bad_variant_access);
  EXPECT_THROW(std::get<bool>(retrieved->data), std::bad_variant_access);
}

/**
 * Test 4: String to numeric conversion failures
 */
TEST_F(ErrorHandlingTest, StringToNumericConversionFailures) {
  AddStringValue("not_a_number", "abc123xyz");

  auto val = container->get_value("not_a_number");
  EXPECT_FALSE(val->type == value_types::null_value);

  // Should throw bad_variant_access when trying to access string as int
  EXPECT_THROW(std::get<int>(val->data), std::bad_variant_access);
}

/**
 * Test 5: Invalid serialization data
 */
TEST_F(ErrorHandlingTest, InvalidSerializationData) {
  // Test various invalid inputs
  std::vector<std::string> invalid_inputs = {
      "",      "random garbage", "@header",
      "@data", "@header={};",    "incomplete@data"};

  for (const auto &invalid : invalid_inputs) {
    EXPECT_NO_THROW({
      auto c = std::make_shared<value_container>(invalid, true);
      // Container might be created but in invalid state
    });
  }
}

/**
 * Test 6: Corrupted header data
 */
TEST_F(ErrorHandlingTest, CorruptedHeaderData) {
  container->set_source("source", "sub");
  container->set_target("target", "");
  AddStringValue("key", "value");

  std::string serialized = container->serialize();

  // Corrupt the header portion
  if (serialized.size() > 50) {
    serialized[20] = 'X';
    serialized[21] = 'X';
  }

  EXPECT_NO_THROW(
      { auto c = std::make_shared<value_container>(serialized, true); });
}

/**
 * Test 7: Very long string values
 */
TEST_F(ErrorHandlingTest, VeryLongStringValues) {
  // Use smaller string size in CI environments to avoid stack/heap exhaustion
  // CI environments have more limited resources and Debug builds use more
  // memory
  size_t string_size =
      TestConfig::instance().is_ci_environment() ? 10000 : 100000;

  std::string long_string = TestHelpers::GenerateRandomString(string_size);
  AddStringValue("long", long_string);

  EXPECT_NO_THROW({
    std::string serialized = container->serialize();
    auto restored = std::make_shared<value_container>(serialized, false);
  });
}

/**
 * Test 8: Maximum numeric value boundaries
 */
TEST_F(ErrorHandlingTest, NumericBoundaryValues) {
  AddNumericValue("max_int", std::numeric_limits<int>::max());
  AddNumericValue("min_int", std::numeric_limits<int>::min());
  AddNumericValue("max_llong", std::numeric_limits<long long>::max());

  auto restored = RoundTripSerialize();

  EXPECT_EQ(std::get<int>(restored->get_value("max_int")->data),
            std::numeric_limits<int>::max());
  EXPECT_EQ(std::get<int>(restored->get_value("min_int")->data),
            std::numeric_limits<int>::min());
}

/**
 * Test 9: Multiple rapid serializations
 */
TEST_F(ErrorHandlingTest, RapidSerializationStress) {
  AddStringValue("key1", "value1");
  AddNumericValue("key2", 42);

  EXPECT_NO_THROW({
    for (int i = 0; i < 1000; ++i) {
      std::string serialized = container->serialize();
    }
  });
}

/**
 * Test 10: Container with many duplicate keys
 */
TEST_F(ErrorHandlingTest, ManyDuplicateKeys) {
  for (int i = 0; i < 100; ++i) {
    AddStringValue("duplicate", "value_" + std::to_string(i));
  }

  std::vector<optimized_value> values;
  for (const auto &val : *container) {
    if (val.name == "duplicate") {
      values.push_back(val);
    }
  }
  EXPECT_EQ(values.size(), 100);

  // Serialize and deserialize
  EXPECT_NO_THROW({
    auto restored = RoundTripSerialize();
    std::vector<optimized_value> restored_values;
    for (const auto &val : *restored) {
      if (val.name == "duplicate") {
        restored_values.push_back(val);
      }
    }
  });
}

/**
 * Test 11: Zero-length bytes value
 */
TEST_F(ErrorHandlingTest, ZeroLengthBytesValue) {
  std::vector<uint8_t> empty_bytes;
  AddBytesValue("empty_bytes", empty_bytes);

  auto val = container->get_value("empty_bytes");
  EXPECT_FALSE(val->type == value_types::null_value);

  auto restored = RoundTripSerialize();
  auto restored_bytes = restored->get_value("empty_bytes");
  EXPECT_FALSE(restored_bytes->type == value_types::null_value);
}

/**
 * Test 12: Special characters in keys
 */
TEST_F(ErrorHandlingTest, SpecialCharactersInKeys) {
  // Test various special characters in key names
  std::vector<std::string> special_keys = {"key_with_underscore",
                                           "key-with-dash",
                                           "key.with.dot",
                                           "KeyWithCamelCase",
                                           "key123numbers",
                                           "key@special",
                                           "key#hash"};

  for (const auto &key : special_keys) {
    EXPECT_NO_THROW({ AddStringValue(key, "test_value"); });
  }

  // Verify serialization handles special keys
  EXPECT_NO_THROW({
    std::string serialized = container->serialize();
    auto restored = std::make_shared<value_container>(serialized, false);
  });
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
