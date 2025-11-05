/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "container/internal/variant_value_factory.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace container_module;

// ============================================================================
// Null value tests
// ============================================================================

TEST(VariantValueFactoryTest, MakeNullValue) {
    auto v = make_null_value("null_test");
    EXPECT_EQ(v.name(), "null_test");
    EXPECT_EQ(v.type(), value_types::null_value);
    EXPECT_TRUE(v.is_null());
}

TEST(VariantValueFactoryTest, MakeNullValueWithoutName) {
    auto v = make_null_value();
    EXPECT_EQ(v.name(), "");
    EXPECT_TRUE(v.is_null());
}

// ============================================================================
// Boolean value tests
// ============================================================================

TEST(VariantValueFactoryTest, MakeBoolValue) {
    auto v_true = make_bool_value("flag", true);
    EXPECT_EQ(v_true.name(), "flag");
    EXPECT_EQ(v_true.type(), value_types::bool_value);

    auto result = v_true.get<bool>();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());

    auto v_false = make_bool_value("disabled", false);
    auto result_false = v_false.get<bool>();
    ASSERT_TRUE(result_false.has_value());
    EXPECT_FALSE(result_false.value());
}

// ============================================================================
// Numeric value tests
// ============================================================================

TEST(VariantValueFactoryTest, MakeShortValue) {
    auto v = make_short_value("count", 42);
    EXPECT_EQ(v.name(), "count");
    EXPECT_EQ(v.type(), value_types::short_value);

    auto result = v.get<int16_t>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 42);
}

TEST(VariantValueFactoryTest, MakeUShortValue) {
    auto v = make_ushort_value("port", 8080);
    EXPECT_EQ(v.type(), value_types::ushort_value);

    auto result = v.get<uint16_t>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 8080);
}

TEST(VariantValueFactoryTest, MakeIntValue) {
    auto v = make_int_value("id", 123456);
    EXPECT_EQ(v.type(), value_types::int_value);

    auto result = v.get<int32_t>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 123456);
}

TEST(VariantValueFactoryTest, MakeUIntValue) {
    auto v = make_uint_value("unsigned_id", 999999);
    EXPECT_EQ(v.type(), value_types::uint_value);

    auto result = v.get<uint32_t>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 999999u);
}

TEST(VariantValueFactoryTest, MakeLongValue) {
    auto v = make_long_value("timestamp", 1234567890123456LL);
    EXPECT_EQ(v.type(), value_types::long_value);

    auto result = v.get<int64_t>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 1234567890123456LL);
}

TEST(VariantValueFactoryTest, MakeULongValue) {
    auto v = make_ulong_value("big_number", 18446744073709551615ULL);
    EXPECT_EQ(v.type(), value_types::ulong_value);

    auto result = v.get<uint64_t>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 18446744073709551615ULL);
}

TEST(VariantValueFactoryTest, MakeFloatValue) {
    auto v = make_float_value("pi", 3.14159f);
    EXPECT_EQ(v.type(), value_types::float_value);

    auto result = v.get<float>();
    ASSERT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(result.value(), 3.14159f);
}

TEST(VariantValueFactoryTest, MakeDoubleValue) {
    auto v = make_double_value("e", 2.718281828459045);
    EXPECT_EQ(v.type(), value_types::double_value);

    auto result = v.get<double>();
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result.value(), 2.718281828459045);
}

// ============================================================================
// Generic numeric factory tests
// ============================================================================

TEST(VariantValueFactoryTest, MakeNumericValueBool) {
    auto v = make_numeric_value("flag", true);
    EXPECT_EQ(v.type(), value_types::bool_value);
}

TEST(VariantValueFactoryTest, MakeNumericValueInt) {
    auto v = make_numeric_value("number", 42);
    EXPECT_EQ(v.type(), value_types::int_value);

    auto result = v.get<int32_t>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 42);
}

TEST(VariantValueFactoryTest, MakeNumericValueDouble) {
    auto v = make_numeric_value("pi", 3.14159);
    EXPECT_EQ(v.type(), value_types::double_value);

    auto result = v.get<double>();
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result.value(), 3.14159);
}

// ============================================================================
// String value tests
// ============================================================================

TEST(VariantValueFactoryTest, MakeStringValueFromString) {
    auto v = make_string_value("message", std::string("Hello, World!"));
    EXPECT_EQ(v.type(), value_types::string_value);

    auto result = v.get<std::string>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "Hello, World!");
}

TEST(VariantValueFactoryTest, MakeStringValueFromStringView) {
    std::string_view sv = "Test string view";
    auto v = make_string_value("text", sv);
    EXPECT_EQ(v.type(), value_types::string_value);

    auto result = v.get<std::string>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "Test string view");
}

TEST(VariantValueFactoryTest, MakeStringValueFromCString) {
    auto v = make_string_value("literal", "C-style string");
    EXPECT_EQ(v.type(), value_types::string_value);

    auto result = v.get<std::string>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "C-style string");
}

TEST(VariantValueFactoryTest, MakeStringValueEmpty) {
    auto v = make_string_value("empty", "");
    EXPECT_EQ(v.type(), value_types::string_value);

    auto result = v.get<std::string>();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value().empty());
}

// ============================================================================
// Bytes value tests
// ============================================================================

TEST(VariantValueFactoryTest, MakeBytesValueFromVector) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0xFF};
    auto v = make_bytes_value("binary", data);
    EXPECT_EQ(v.type(), value_types::bytes_value);

    auto result = v.get<std::vector<uint8_t>>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), data);
}

TEST(VariantValueFactoryTest, MakeBytesValueFromRawPointer) {
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    auto v = make_bytes_value("raw", data, sizeof(data));
    EXPECT_EQ(v.type(), value_types::bytes_value);

    auto result = v.get<std::vector<uint8_t>>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 4u);
    EXPECT_EQ(result.value()[0], 0xDE);
    EXPECT_EQ(result.value()[3], 0xEF);
}

TEST(VariantValueFactoryTest, MakeBytesValueFromString) {
    std::string_view str = "binary\x00data";
    auto v = make_bytes_from_string("encoded", str);
    EXPECT_EQ(v.type(), value_types::bytes_value);

    auto result = v.get<std::vector<uint8_t>>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), str.size());
}

// ============================================================================
// Array value tests
// ============================================================================

TEST(VariantValueFactoryTest, MakeArrayValueFromVector) {
    std::vector<std::shared_ptr<variant_value_v2>> items;
    items.push_back(std::make_shared<variant_value_v2>(make_int_value("item1", 1)));
    items.push_back(std::make_shared<variant_value_v2>(make_int_value("item2", 2)));
    items.push_back(std::make_shared<variant_value_v2>(make_int_value("item3", 3)));

    auto v = make_array_value("numbers", std::move(items));
    EXPECT_EQ(v.type(), value_types::array_value);

    auto result = v.get<array_variant>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().values.size(), 3u);
}

TEST(VariantValueFactoryTest, MakeArrayValueFromInitializerList) {
    auto v = make_array_value("mixed", {
        make_int_value("num", 42),
        make_string_value("str", "hello"),
        make_bool_value("flag", true)
    });

    EXPECT_EQ(v.type(), value_types::array_value);

    auto result = v.get<array_variant>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().values.size(), 3u);

    // Check types of elements
    EXPECT_EQ(result.value().values[0]->type(), value_types::int_value);
    EXPECT_EQ(result.value().values[1]->type(), value_types::string_value);
    EXPECT_EQ(result.value().values[2]->type(), value_types::bool_value);
}

TEST(VariantValueFactoryTest, MakeEmptyArrayValue) {
    auto v = make_empty_array_value("empty_array");
    EXPECT_EQ(v.type(), value_types::array_value);

    auto result = v.get<array_variant>();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value().values.empty());
}

// ============================================================================
// Serialization round-trip tests
// ============================================================================

TEST(VariantValueFactoryTest, SerializationRoundTripBool) {
    auto original = make_bool_value("test", true);
    auto serialized = original.serialize();
    auto deserialized = variant_value_v2::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized->name(), original.name());
    EXPECT_EQ(deserialized->type(), original.type());
    EXPECT_EQ(*deserialized, original);
}

TEST(VariantValueFactoryTest, SerializationRoundTripInt) {
    auto original = make_int_value("number", 12345);
    auto serialized = original.serialize();
    auto deserialized = variant_value_v2::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(*deserialized, original);
}

TEST(VariantValueFactoryTest, SerializationRoundTripString) {
    auto original = make_string_value("text", "Hello, World!");
    auto serialized = original.serialize();
    auto deserialized = variant_value_v2::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(*deserialized, original);
}

TEST(VariantValueFactoryTest, SerializationRoundTripBytes) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0xFF};
    auto original = make_bytes_value("binary", data);
    auto serialized = original.serialize();
    auto deserialized = variant_value_v2::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(*deserialized, original);
}

TEST(VariantValueFactoryTest, SerializationRoundTripArray) {
    auto original = make_array_value("items", {
        make_int_value("a", 1),
        make_string_value("b", "two"),
        make_double_value("c", 3.14)
    });

    auto serialized = original.serialize();
    auto deserialized = variant_value_v2::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(*deserialized, original);
}

// ============================================================================
// Utility function tests
// ============================================================================

TEST(VariantValueFactoryTest, SameType) {
    auto v1 = make_int_value("a", 1);
    auto v2 = make_int_value("b", 2);
    auto v3 = make_string_value("c", "test");

    EXPECT_TRUE(same_type(v1, v2));
    EXPECT_FALSE(same_type(v1, v3));
}

TEST(VariantValueFactoryTest, TypeName) {
    EXPECT_EQ(type_name(make_null_value()), "null");
    EXPECT_EQ(type_name(make_bool_value("", true)), "bool");
    EXPECT_EQ(type_name(make_int_value("", 42)), "int");
    EXPECT_EQ(type_name(make_string_value("", "test")), "string");
    EXPECT_EQ(type_name(make_bytes_value("", std::vector<uint8_t>{})), "bytes");
    EXPECT_EQ(type_name(make_empty_array_value("")), "array");
}

// ============================================================================
// Edge case tests
// ============================================================================

TEST(VariantValueFactoryTest, LargeStringValue) {
    std::string large_string(10000, 'x');
    auto v = make_string_value("large", large_string);

    auto result = v.get<std::string>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 10000u);
    EXPECT_EQ(result.value(), large_string);
}

TEST(VariantValueFactoryTest, LargeBytesValue) {
    std::vector<uint8_t> large_data(100000, 0xFF);
    auto v = make_bytes_value("large_binary", large_data);

    auto result = v.get<std::vector<uint8_t>>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 100000u);
}

TEST(VariantValueFactoryTest, NestedArrays) {
    auto inner_array = make_array_value("inner", {
        make_int_value("x", 1),
        make_int_value("y", 2)
    });

    auto outer_array = make_array_value("outer", {
        inner_array,
        make_string_value("label", "nested")
    });

    EXPECT_EQ(outer_array.type(), value_types::array_value);

    auto result = outer_array.get<array_variant>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().values.size(), 2u);
    EXPECT_EQ(result.value().values[0]->type(), value_types::array_value);
}

TEST(VariantValueFactoryTest, NumericBoundaries) {
    // Test numeric type boundaries
    auto max_int16 = make_short_value("max_short", INT16_MAX);
    auto min_int16 = make_short_value("min_short", INT16_MIN);
    auto max_uint32 = make_uint_value("max_uint", UINT32_MAX);
    auto max_int64 = make_long_value("max_long", INT64_MAX);

    EXPECT_EQ(max_int16.get<int16_t>().value(), INT16_MAX);
    EXPECT_EQ(min_int16.get<int16_t>().value(), INT16_MIN);
    EXPECT_EQ(max_uint32.get<uint32_t>().value(), UINT32_MAX);
    EXPECT_EQ(max_int64.get<int64_t>().value(), INT64_MAX);
}
