/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, All rights reserved.
*****************************************************************************/

#include "internal/variant_value_factory.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace kcenon::container;

// ============================================================================
// Modern factory API tests (factory::make, factory::make_null)
// ============================================================================

TEST(ModernFactoryTest, MakeWithBool) {
    auto v = factory::make("flag", true);
    EXPECT_EQ(v.name(), "flag");
    EXPECT_EQ(v.type(), value_types::bool_value);
    EXPECT_TRUE(v.get<bool>().value());
}

TEST(ModernFactoryTest, MakeWithInt) {
    auto v = factory::make("count", 42);
    EXPECT_EQ(v.name(), "count");
    EXPECT_EQ(v.type(), value_types::int_value);
    EXPECT_EQ(v.get<int32_t>().value(), 42);
}

TEST(ModernFactoryTest, MakeWithInt64) {
    auto v = factory::make("large", int64_t{1234567890123456LL});
    EXPECT_EQ(v.type(), value_types::long_value);
    EXPECT_EQ(v.get<int64_t>().value(), 1234567890123456LL);
}

TEST(ModernFactoryTest, MakeWithDouble) {
    auto v = factory::make("pi", 3.14159);
    EXPECT_EQ(v.type(), value_types::double_value);
    EXPECT_DOUBLE_EQ(v.get<double>().value(), 3.14159);
}

TEST(ModernFactoryTest, MakeWithString) {
    auto v = factory::make("name", std::string("John"));
    EXPECT_EQ(v.type(), value_types::string_value);
    EXPECT_EQ(v.get<std::string>().value(), "John");
}

TEST(ModernFactoryTest, MakeWithBytes) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    auto v = factory::make("binary", data);
    EXPECT_EQ(v.type(), value_types::bytes_value);
    EXPECT_EQ(v.get<std::vector<uint8_t>>().value(), data);
}

TEST(ModernFactoryTest, MakeNull) {
    auto v = factory::make_null("empty");
    EXPECT_EQ(v.name(), "empty");
    EXPECT_EQ(v.type(), value_types::null_value);
    EXPECT_TRUE(v.is_null());
}

TEST(ModernFactoryTest, MakeNullWithoutName) {
    auto v = factory::make_null();
    EXPECT_EQ(v.name(), "");
    EXPECT_TRUE(v.is_null());
}

TEST(ModernFactoryTest, MakeArray) {
    auto v = factory::make_array("items", {
        value("a", 1),
        value("b", std::string("two")),
        value("c", true)
    });
    EXPECT_EQ(v.type(), value_types::array_value);

    auto result = v.get<array_variant>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().values.size(), 3u);
}

TEST(ModernFactoryTest, MakeEmptyArray) {
    auto v = factory::make_empty_array("empty");
    EXPECT_EQ(v.type(), value_types::array_value);

    auto result = v.get<array_variant>();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value().values.empty());
}

TEST(ModernFactoryTest, MakeBytesFromRawPointer) {
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    auto v = factory::make_bytes("raw", data, sizeof(data));
    EXPECT_EQ(v.type(), value_types::bytes_value);

    auto result = v.get<std::vector<uint8_t>>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 4u);
    EXPECT_EQ(result.value()[0], 0xDE);
}

TEST(ModernFactoryTest, MakeBytesFromString) {
    auto v = factory::make_bytes_from_string("encoded", "hello");
    EXPECT_EQ(v.type(), value_types::bytes_value);

    auto result = v.get<std::vector<uint8_t>>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 5u);
}

TEST(ModernFactoryTest, DirectConstructorEquivalence) {
    // Verify that factory::make produces the same result as direct constructor
    auto v1 = factory::make("test", 42);
    auto v2 = value("test", 42);

    EXPECT_EQ(v1.name(), v2.name());
    EXPECT_EQ(v1.type(), v2.type());
    EXPECT_EQ(v1.get<int32_t>().value(), v2.get<int32_t>().value());
}

// ============================================================================
// Value constructor tests (migrated from legacy factory tests)
// ============================================================================

// ============================================================================
// Null value tests
// ============================================================================

TEST(ValueConstructorTest, NullValue) {
    auto v = value("null_test");
    EXPECT_EQ(v.name(), "null_test");
    EXPECT_EQ(v.type(), value_types::null_value);
    EXPECT_TRUE(v.is_null());
}

TEST(ValueConstructorTest, NullValueWithoutName) {
    auto v = factory::make_null();
    EXPECT_EQ(v.name(), "");
    EXPECT_TRUE(v.is_null());
}

// ============================================================================
// Boolean value tests
// ============================================================================

TEST(ValueConstructorTest, BoolValue) {
    auto v_true = value("flag", true);
    EXPECT_EQ(v_true.name(), "flag");
    EXPECT_EQ(v_true.type(), value_types::bool_value);

    auto result = v_true.get<bool>();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());

    auto v_false = value("disabled", false);
    auto result_false = v_false.get<bool>();
    ASSERT_TRUE(result_false.has_value());
    EXPECT_FALSE(result_false.value());
}

// ============================================================================
// Numeric value tests
// ============================================================================

TEST(ValueConstructorTest, ShortValue) {
    auto v = value("count", int16_t{42});
    EXPECT_EQ(v.name(), "count");
    EXPECT_EQ(v.type(), value_types::short_value);

    auto result = v.get<int16_t>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 42);
}

TEST(ValueConstructorTest, UShortValue) {
    auto v = value("port", uint16_t{8080});
    EXPECT_EQ(v.type(), value_types::ushort_value);

    auto result = v.get<uint16_t>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 8080);
}

TEST(ValueConstructorTest, IntValue) {
    auto v = value("id", int32_t{123456});
    EXPECT_EQ(v.type(), value_types::int_value);

    auto result = v.get<int32_t>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 123456);
}

TEST(ValueConstructorTest, UIntValue) {
    auto v = value("unsigned_id", uint32_t{999999});
    EXPECT_EQ(v.type(), value_types::uint_value);

    auto result = v.get<uint32_t>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 999999u);
}

TEST(ValueConstructorTest, LongValue) {
    auto v = value("timestamp", int64_t{1234567890123456LL});
    EXPECT_EQ(v.type(), value_types::long_value);

    auto result = v.get<int64_t>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 1234567890123456LL);
}

TEST(ValueConstructorTest, ULongValue) {
    auto v = value("big_number", uint64_t{18446744073709551615ULL});
    EXPECT_EQ(v.type(), value_types::ulong_value);

    auto result = v.get<uint64_t>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 18446744073709551615ULL);
}

TEST(ValueConstructorTest, FloatValue) {
    auto v = value("pi", 3.14159f);
    EXPECT_EQ(v.type(), value_types::float_value);

    auto result = v.get<float>();
    ASSERT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(result.value(), 3.14159f);
}

TEST(ValueConstructorTest, DoubleValue) {
    auto v = value("e", 2.718281828459045);
    EXPECT_EQ(v.type(), value_types::double_value);

    auto result = v.get<double>();
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result.value(), 2.718281828459045);
}

// ============================================================================
// String value tests
// ============================================================================

TEST(ValueConstructorTest, StringValueFromString) {
    auto v = value("message", std::string("Hello, World!"));
    EXPECT_EQ(v.type(), value_types::string_value);

    auto result = v.get<std::string>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "Hello, World!");
}

TEST(ValueConstructorTest, StringValueEmpty) {
    auto v = value("empty", std::string(""));
    EXPECT_EQ(v.type(), value_types::string_value);

    auto result = v.get<std::string>();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value().empty());
}

// ============================================================================
// Bytes value tests
// ============================================================================

TEST(ValueConstructorTest, BytesValueFromVector) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0xFF};
    auto v = value("binary", data);
    EXPECT_EQ(v.type(), value_types::bytes_value);

    auto result = v.get<std::vector<uint8_t>>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), data);
}

TEST(ValueConstructorTest, BytesValueFromRawPointer) {
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    auto v = factory::make_bytes("raw", data, sizeof(data));
    EXPECT_EQ(v.type(), value_types::bytes_value);

    auto result = v.get<std::vector<uint8_t>>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 4u);
    EXPECT_EQ(result.value()[0], 0xDE);
    EXPECT_EQ(result.value()[3], 0xEF);
}

TEST(ValueConstructorTest, BytesValueFromString) {
    std::string_view str = "binary\x00data";
    auto v = factory::make_bytes_from_string("encoded", str);
    EXPECT_EQ(v.type(), value_types::bytes_value);

    auto result = v.get<std::vector<uint8_t>>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), str.size());
}

// ============================================================================
// Array value tests
// ============================================================================

TEST(ValueConstructorTest, ArrayValueFromVector) {
    std::vector<std::shared_ptr<value>> items;
    items.push_back(std::make_shared<value>("item1", 1));
    items.push_back(std::make_shared<value>("item2", 2));
    items.push_back(std::make_shared<value>("item3", 3));

    auto v = factory::make_array("numbers", std::move(items));
    EXPECT_EQ(v.type(), value_types::array_value);

    auto result = v.get<array_variant>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().values.size(), 3u);
}

TEST(ValueConstructorTest, ArrayValueFromInitializerList) {
    auto v = factory::make_array("mixed", {
        value("num", 42),
        value("str", std::string("hello")),
        value("flag", true)
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

TEST(ValueConstructorTest, EmptyArrayValue) {
    auto v = factory::make_empty_array("empty_array");
    EXPECT_EQ(v.type(), value_types::array_value);

    auto result = v.get<array_variant>();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value().values.empty());
}

// ============================================================================
// Serialization round-trip tests
// ============================================================================

TEST(ValueConstructorTest, SerializationRoundTripBool) {
    auto original = value("test", true);
    auto serialized = original.serialize();
    auto deserialized = value::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized->name(), original.name());
    EXPECT_EQ(deserialized->type(), original.type());
    EXPECT_EQ(*deserialized, original);
}

TEST(ValueConstructorTest, SerializationRoundTripInt) {
    auto original = value("number", 12345);
    auto serialized = original.serialize();
    auto deserialized = value::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(*deserialized, original);
}

TEST(ValueConstructorTest, SerializationRoundTripString) {
    auto original = value("text", std::string("Hello, World!"));
    auto serialized = original.serialize();
    auto deserialized = value::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(*deserialized, original);
}

TEST(ValueConstructorTest, SerializationRoundTripBytes) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0xFF};
    auto original = value("binary", data);
    auto serialized = original.serialize();
    auto deserialized = value::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(*deserialized, original);
}

TEST(ValueConstructorTest, SerializationRoundTripArray) {
    auto original = factory::make_array("items", {
        value("a", 1),
        value("b", std::string("two")),
        value("c", 3.14)
    });

    auto serialized = original.serialize();
    auto deserialized = value::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(*deserialized, original);
}

// ============================================================================
// Utility function tests
// ============================================================================

TEST(ValueConstructorTest, SameType) {
    auto v1 = value("a", 1);
    auto v2 = value("b", 2);
    auto v3 = value("c", std::string("test"));

    EXPECT_TRUE(same_type(v1, v2));
    EXPECT_FALSE(same_type(v1, v3));
}

TEST(ValueConstructorTest, TypeName) {
    EXPECT_EQ(type_name(factory::make_null()), "null");
    EXPECT_EQ(type_name(value("", true)), "bool");
    EXPECT_EQ(type_name(value("", 42)), "int");
    EXPECT_EQ(type_name(value("", std::string("test"))), "string");
    EXPECT_EQ(type_name(value("", std::vector<uint8_t>{})), "bytes");
    EXPECT_EQ(type_name(factory::make_empty_array("")), "array");
}

// ============================================================================
// Edge case tests
// ============================================================================

TEST(ValueConstructorTest, LargeStringValue) {
    std::string large_string(10000, 'x');
    auto v = value("large", large_string);

    auto result = v.get<std::string>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 10000u);
    EXPECT_EQ(result.value(), large_string);
}

TEST(ValueConstructorTest, LargeBytesValue) {
    std::vector<uint8_t> large_data(100000, 0xFF);
    auto v = value("large_binary", large_data);

    auto result = v.get<std::vector<uint8_t>>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 100000u);
}

TEST(ValueConstructorTest, NestedArrays) {
    auto inner_array = factory::make_array("inner", {
        value("x", 1),
        value("y", 2)
    });

    auto outer_array = factory::make_array("outer", {
        inner_array,
        value("label", std::string("nested"))
    });

    EXPECT_EQ(outer_array.type(), value_types::array_value);

    auto result = outer_array.get<array_variant>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().values.size(), 2u);
    EXPECT_EQ(result.value().values[0]->type(), value_types::array_value);
}

TEST(ValueConstructorTest, NumericBoundaries) {
    auto max_int16 = value("max_short", INT16_MAX);
    auto min_int16 = value("min_short", INT16_MIN);
    auto max_uint32 = value("max_uint", UINT32_MAX);
    auto max_int64 = value("max_long", INT64_MAX);

    EXPECT_EQ(max_int16.get<int16_t>().value(), INT16_MAX);
    EXPECT_EQ(min_int16.get<int16_t>().value(), INT16_MIN);
    EXPECT_EQ(max_uint32.get<uint32_t>().value(), UINT32_MAX);
    EXPECT_EQ(max_int64.get<int64_t>().value(), INT64_MAX);
}
