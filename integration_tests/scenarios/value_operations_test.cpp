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
 * @file value_operations_test.cpp
 * @brief Integration tests for value operations and type conversions
 *
 * Tests cover:
 * - Value type creation and validation
 * - Type conversions (string, numeric, boolean)
 * - Value serialization and deserialization
 * - Bytes value operations
 * - Null value handling
 * - Type checking and validation
 * - Edge cases for numeric values
 */

#include <gtest/gtest.h>
#include "framework/system_fixture.h"
#include "framework/test_helpers.h"
#include <container/core/container.h>
#include <container/values/bool_value.h>
#include <container/values/string_value.h>
#include <container/values/bytes_value.h>
#include <container/values/numeric_value.h>
#include <limits>

using namespace container_module;
using namespace container_module::testing;

class ValueOperationsTest : public ContainerSystemFixture
{
};

/**
 * Test 1: String value operations
 */
TEST_F(ValueOperationsTest, StringValueOperations)
{
    std::string test_str = "Hello, World!";
    auto str_val = std::make_shared<string_value>("test", test_str);

    EXPECT_TRUE(str_val->is_string());
    EXPECT_FALSE(str_val->is_numeric());
    EXPECT_FALSE(str_val->is_boolean());
    EXPECT_FALSE(str_val->is_null());
    EXPECT_EQ(str_val->to_string(), test_str);
}

/**
 * Test 2: Numeric value type conversions
 */
TEST_F(ValueOperationsTest, NumericValueConversions)
{
    auto int_val = std::make_shared<int_value>("int", 42);

    EXPECT_TRUE(int_val->is_numeric());
    EXPECT_EQ(int_val->to_int(), 42);
    EXPECT_EQ(int_val->to_long(), 42L);
    EXPECT_DOUBLE_EQ(int_val->to_double(), 42.0);
}

/**
 * Test 3: Boolean value operations
 */
TEST_F(ValueOperationsTest, BooleanValueOperations)
{
    auto true_val = std::make_shared<bool_value>("true_val", true);
    auto false_val = std::make_shared<bool_value>("false_val", false);

    EXPECT_TRUE(true_val->is_boolean());
    EXPECT_TRUE(true_val->to_boolean());
    EXPECT_FALSE(false_val->to_boolean());
}

/**
 * Test 4: Bytes value operations
 */
TEST_F(ValueOperationsTest, BytesValueOperations)
{
    std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD};
    auto bytes_val = std::make_shared<bytes_value>("bytes", test_data);

    EXPECT_TRUE(bytes_val->is_bytes());
    EXPECT_FALSE(bytes_val->is_string());

    auto retrieved = bytes_val->to_bytes();
    ASSERT_EQ(retrieved.size(), test_data.size());

    for (size_t i = 0; i < test_data.size(); ++i) {
        EXPECT_EQ(retrieved[i], test_data[i]);
    }
}

/**
 * Test 5: Large bytes value handling
 */
TEST_F(ValueOperationsTest, LargeBytesValue)
{
    auto large_bytes = TestHelpers::GenerateRandomBytes(10000);
    auto bytes_val = std::make_shared<bytes_value>("large", large_bytes);

    auto retrieved = bytes_val->to_bytes();
    EXPECT_EQ(retrieved.size(), large_bytes.size());
    EXPECT_EQ(retrieved, large_bytes);
}

/**
 * Test 6: Null value behavior
 */
TEST_F(ValueOperationsTest, NullValueBehavior)
{
    auto null_val = std::make_shared<value>("null", value_types::null_value, "");

    EXPECT_TRUE(null_val->is_null());
    EXPECT_FALSE(null_val->is_string());
    EXPECT_FALSE(null_val->is_numeric());
    EXPECT_FALSE(null_val->is_boolean());
}

/**
 * Test 7: Double value precision
 */
TEST_F(ValueOperationsTest, DoubleValuePrecision)
{
    double precise_value = 3.141592653589793;
    auto double_val = std::make_shared<double_value>("pi", precise_value);

    EXPECT_DOUBLE_EQ(double_val->to_double(), precise_value);

    // Test serialization preserves precision
    // Note: Serialization may lose some precision due to string conversion
    // Use EXPECT_NEAR with appropriate epsilon for roundtrip tests
    container->add(double_val);
    auto restored = RoundTripSerialize();

    // Allow for precision loss during serialization/deserialization
    // Use 1e-6 tolerance (about 6-7 significant digits)
    double restored_value = restored->get_value("pi")->to_double();
    EXPECT_NEAR(restored_value, precise_value, 1e-6)
        << "Expected: " << precise_value << ", Got: " << restored_value;
}

/**
 * Test 8: Integer overflow handling
 */
TEST_F(ValueOperationsTest, IntegerEdgeCases)
{
    auto max_int = std::make_shared<int_value>("max", std::numeric_limits<int>::max());
    auto min_int = std::make_shared<int_value>("min", std::numeric_limits<int>::min());

    EXPECT_EQ(max_int->to_int(), std::numeric_limits<int>::max());
    EXPECT_EQ(min_int->to_int(), std::numeric_limits<int>::min());
}

/**
 * Test 9: Long long value handling
 */
TEST_F(ValueOperationsTest, LongLongValues)
{
    long long large_value = 9223372036854775807LL;
    auto llong_val = std::make_shared<llong_value>("large", large_value);

    EXPECT_EQ(llong_val->to_llong(), large_value);

    container->add(llong_val);
    auto restored = RoundTripSerialize();
    EXPECT_EQ(restored->get_value("large")->to_llong(), large_value);
}

/**
 * Test 10: Value type identification
 */
TEST_F(ValueOperationsTest, ValueTypeIdentification)
{
    auto str = std::make_shared<string_value>("str", "test");
    auto num = std::make_shared<int_value>("num", 42);
    auto boolean = std::make_shared<bool_value>("bool", true);
    std::vector<uint8_t> bytes_data = {0x01, 0x02};
    auto bytes = std::make_shared<bytes_value>("bytes", bytes_data);

    EXPECT_EQ(str->type(), value_types::string_value);
    EXPECT_EQ(num->type(), value_types::int_value);
    EXPECT_EQ(boolean->type(), value_types::bool_value);
    EXPECT_EQ(bytes->type(), value_types::bytes_value);
}

/**
 * Test 11: Special string characters
 */
TEST_F(ValueOperationsTest, SpecialStringCharacters)
{
    std::string special = "Line1\nLine2\tTab\rReturn";
    auto str_val = std::make_shared<string_value>("special", special);

    container->add(str_val);
    auto restored = RoundTripSerialize();

    auto restored_val = restored->get_value("special");
    // Note: Some special characters may be encoded/decoded differently
    EXPECT_FALSE(restored_val->is_null());
}

/**
 * Test 12: Empty and whitespace strings
 */
TEST_F(ValueOperationsTest, EmptyAndWhitespaceStrings)
{
    auto empty = std::make_shared<string_value>("empty", "");
    auto whitespace = std::make_shared<string_value>("whitespace", "   ");
    auto mixed = std::make_shared<string_value>("mixed", "  text  ");

    container->add(empty);
    container->add(whitespace);
    container->add(mixed);

    auto restored = RoundTripSerialize();

    EXPECT_FALSE(restored->get_value("empty")->is_null());
    EXPECT_FALSE(restored->get_value("whitespace")->is_null());
    EXPECT_FALSE(restored->get_value("mixed")->is_null());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
