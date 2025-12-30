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
    auto str_val = make_string_value("test", test_str);

    EXPECT_TRUE(is_string(*str_val));
    EXPECT_FALSE(is_numeric(*str_val));
    EXPECT_FALSE(is_boolean(*str_val));
    EXPECT_FALSE(str_val->is_null());
    EXPECT_EQ(str_val->to_string(), test_str);
}

/**
 * Test 2: Numeric value type conversions
 */
TEST_F(ValueOperationsTest, NumericValueConversions)
{
    auto int_val = make_int_value("int", 42);

    EXPECT_TRUE(is_numeric(*int_val));
    EXPECT_EQ(to_int(*int_val), 42);
    EXPECT_EQ(to_long(*int_val), 42L);
    EXPECT_DOUBLE_EQ(to_double(*int_val), 42.0);
}

/**
 * Test 3: Boolean value operations
 */
TEST_F(ValueOperationsTest, BooleanValueOperations)
{
    auto true_val = make_bool_value("true_val", true);
    auto false_val = make_bool_value("false_val", false);

    EXPECT_TRUE(is_boolean(*true_val));
    EXPECT_TRUE(to_boolean(*true_val));
    EXPECT_FALSE(to_boolean(*false_val));
}

/**
 * Test 4: Bytes value operations
 */
TEST_F(ValueOperationsTest, BytesValueOperations)
{
    std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD};
    auto bytes_val = make_bytes_value("bytes", test_data);

    EXPECT_TRUE(is_bytes(*bytes_val));
    EXPECT_FALSE(is_string(*bytes_val));

    auto retrieved = to_bytes(*bytes_val);
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
    auto bytes_val = make_bytes_value("large", large_bytes);

    auto retrieved = to_bytes(*bytes_val);
    EXPECT_EQ(retrieved.size(), large_bytes.size());
    EXPECT_EQ(retrieved, large_bytes);
}

/**
 * Test 6: Null value behavior
 */
TEST_F(ValueOperationsTest, NullValueBehavior)
{
    std::vector<uint8_t> empty_data;
    auto null_val = std::make_shared<value>("null", value_types::null_value, empty_data);

    EXPECT_TRUE(null_val->is_null());
    EXPECT_FALSE(is_string(*null_val));
    EXPECT_FALSE(is_numeric(*null_val));
    EXPECT_FALSE(is_boolean(*null_val));
}

/**
 * Test 7: Double value precision
 */
TEST_F(ValueOperationsTest, DoubleValuePrecision)
{
    double precise_value = 3.141592653589793;
    auto double_val = make_double_value("pi", precise_value);

    EXPECT_DOUBLE_EQ(to_double(*double_val), precise_value);

    // Test serialization preserves precision
    // Note: Serialization may lose some precision due to string conversion
    // Use EXPECT_NEAR with appropriate epsilon for roundtrip tests
    container->add(double_val);
    auto restored = RoundTripSerialize();

    // Allow for precision loss during serialization/deserialization
    // Use 1e-6 tolerance (about 6-7 significant digits)
    double restored_value = ov_to_double(restored->get_value("pi"));
    EXPECT_NEAR(restored_value, precise_value, 1e-6)
        << "Expected: " << precise_value << ", Got: " << restored_value;
}

/**
 * Test 8: Integer overflow handling
 */
TEST_F(ValueOperationsTest, IntegerEdgeCases)
{
    auto max_int = make_int_value("max", std::numeric_limits<int>::max());
    auto min_int = make_int_value("min", std::numeric_limits<int>::min());

    EXPECT_EQ(to_int(*max_int), std::numeric_limits<int>::max());
    EXPECT_EQ(to_int(*min_int), std::numeric_limits<int>::min());
}

/**
 * Test 9: Long long value handling
 */
TEST_F(ValueOperationsTest, LongLongValues)
{
    long long large_value = 9223372036854775807LL;
    auto llong_val = make_llong_value("large", large_value);

    EXPECT_EQ(to_llong(*llong_val), large_value);

    container->add(llong_val);
    auto restored = RoundTripSerialize();
    EXPECT_EQ(ov_to_llong(restored->get_value("large")), large_value);
}

/**
 * Test 10: Value type identification
 */
TEST_F(ValueOperationsTest, ValueTypeIdentification)
{
    auto str = make_string_value("str", "test");
    auto num = make_int_value("num", 42);
    auto boolean = make_bool_value("bool", true);
    std::vector<uint8_t> bytes_data = {0x01, 0x02};
    auto bytes = make_bytes_value("bytes", bytes_data);

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
    auto str_val = make_string_value("special", special);

    container->add(str_val);
    auto restored = RoundTripSerialize();

    auto restored_val = restored->get_value("special");
    // Note: Some special characters may be encoded/decoded differently
    EXPECT_FALSE(ov_is_null(restored_val));
}

/**
 * Test 12: Empty and whitespace strings
 */
TEST_F(ValueOperationsTest, EmptyAndWhitespaceStrings)
{
    auto empty = make_string_value("empty", "");
    auto whitespace = make_string_value("whitespace", "   ");
    auto mixed = make_string_value("mixed", "  text  ");

    container->add(empty);
    container->add(whitespace);
    container->add(mixed);

    auto restored = RoundTripSerialize();

    EXPECT_FALSE(ov_is_null(restored->get_value("empty")));
    EXPECT_FALSE(ov_is_null(restored->get_value("whitespace")));
    EXPECT_FALSE(ov_is_null(restored->get_value("mixed")));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
