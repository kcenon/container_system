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

#include <gtest/gtest.h>
#include "framework/system_fixture.h"
#include "framework/test_helpers.h"
#include <container/core/container.h>
#include <container/values/string_value.h>
#include <container/values/numeric_value.h>
#include <limits>
#include <stdexcept>

using namespace container_module;
using namespace container_module::testing;

class ErrorHandlingTest : public ContainerSystemFixture
{
};

/**
 * Test 1: Non-existent value retrieval
 */
TEST_F(ErrorHandlingTest, NonExistentValueRetrieval)
{
    AddStringValue("exists", "value");

    auto missing = container->get_value("does_not_exist");
    EXPECT_TRUE(missing->is_null());
    EXPECT_EQ(missing->type(), value_types::null_value);
}

/**
 * Test 2: Empty key value operations
 */
TEST_F(ErrorHandlingTest, EmptyKeyOperations)
{
    AddStringValue("", "empty_key_value");

    auto val = container->get_value("");
    // Behavior may vary - either null or returns the value
    // Just verify it doesn't crash
    EXPECT_NO_THROW({
        auto result = container->get_value("");
    });
}

/**
 * Test 3: Null value type conversions
 */
TEST_F(ErrorHandlingTest, NullValueConversions)
{
    auto null_val = std::make_shared<value>("null", value_types::null_value, "");
    container->add(null_val);

    auto retrieved = container->get_value("null");
    EXPECT_TRUE(retrieved->is_null());

    // Base value conversion helpers throw std::logic_error for null values.
    // Verify that the integration surface preserves this contract.
    EXPECT_THROW(retrieved->to_int(), std::logic_error);
    EXPECT_THROW(retrieved->to_double(), std::logic_error);
    EXPECT_THROW(retrieved->to_boolean(), std::logic_error);
}

/**
 * Test 4: String to numeric conversion failures
 */
TEST_F(ErrorHandlingTest, StringToNumericConversionFailures)
{
    AddStringValue("not_a_number", "abc123xyz");

    auto val = container->get_value("not_a_number");
    EXPECT_FALSE(val->is_null());

    // Should handle conversion gracefully (return 0 or default)
    int result = val->to_int();
    EXPECT_EQ(result, 0);
}

/**
 * Test 5: Invalid serialization data
 */
TEST_F(ErrorHandlingTest, InvalidSerializationData)
{
    // Test various invalid inputs
    std::vector<std::string> invalid_inputs = {
        "",
        "random garbage",
        "@header",
        "@data",
        "@header={};",
        "incomplete@data"
    };

    for (const auto& invalid : invalid_inputs) {
        EXPECT_NO_THROW({
            auto c = std::make_shared<value_container>(invalid, true);
            // Container might be created but in invalid state
        });
    }
}

/**
 * Test 6: Corrupted header data
 */
TEST_F(ErrorHandlingTest, CorruptedHeaderData)
{
    container->set_source("source", "sub");
    container->set_target("target", "");
    AddStringValue("key", "value");

    std::string serialized = container->serialize();

    // Corrupt the header portion
    if (serialized.size() > 50) {
        serialized[20] = 'X';
        serialized[21] = 'X';
    }

    EXPECT_NO_THROW({
        auto c = std::make_shared<value_container>(serialized, true);
    });
}

/**
 * Test 7: Very long string values
 */
TEST_F(ErrorHandlingTest, VeryLongStringValues)
{
    std::string long_string = TestHelpers::GenerateRandomString(100000);
    AddStringValue("long", long_string);

    EXPECT_NO_THROW({
        std::string serialized = container->serialize();
        auto restored = std::make_shared<value_container>(serialized, false);
    });
}

/**
 * Test 8: Maximum numeric value boundaries
 */
TEST_F(ErrorHandlingTest, NumericBoundaryValues)
{
    AddNumericValue("max_int", std::numeric_limits<int>::max());
    AddNumericValue("min_int", std::numeric_limits<int>::min());
    AddNumericValue("max_llong", std::numeric_limits<long long>::max());

    auto restored = RoundTripSerialize();

    EXPECT_EQ(restored->get_value("max_int")->to_int(),
              std::numeric_limits<int>::max());
    EXPECT_EQ(restored->get_value("min_int")->to_int(),
              std::numeric_limits<int>::min());
}

/**
 * Test 9: Multiple rapid serializations
 */
TEST_F(ErrorHandlingTest, RapidSerializationStress)
{
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
TEST_F(ErrorHandlingTest, ManyDuplicateKeys)
{
    for (int i = 0; i < 100; ++i) {
        AddStringValue("duplicate", "value_" + std::to_string(i));
    }

    auto values = container->value_array("duplicate");
    EXPECT_EQ(values.size(), 100);

    // Serialize and deserialize
    EXPECT_NO_THROW({
        auto restored = RoundTripSerialize();
        auto restored_values = restored->value_array("duplicate");
    });
}

/**
 * Test 11: Zero-length bytes value
 */
TEST_F(ErrorHandlingTest, ZeroLengthBytesValue)
{
    std::vector<uint8_t> empty_bytes;
    AddBytesValue("empty_bytes", empty_bytes);

    auto val = container->get_value("empty_bytes");
    EXPECT_FALSE(val->is_null());

    auto restored = RoundTripSerialize();
    auto restored_bytes = restored->get_value("empty_bytes");
    EXPECT_FALSE(restored_bytes->is_null());
}

/**
 * Test 12: Special characters in keys
 */
TEST_F(ErrorHandlingTest, SpecialCharactersInKeys)
{
    // Test various special characters in key names
    std::vector<std::string> special_keys = {
        "key_with_underscore",
        "key-with-dash",
        "key.with.dot",
        "KeyWithCamelCase",
        "key123numbers",
        "key@special",
        "key#hash"
    };

    for (const auto& key : special_keys) {
        EXPECT_NO_THROW({
            AddStringValue(key, "test_value");
        });
    }

    // Verify serialization handles special keys
    EXPECT_NO_THROW({
        std::string serialized = container->serialize();
        auto restored = std::make_shared<value_container>(serialized, false);
    });
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
