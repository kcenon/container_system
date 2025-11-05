/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>
#include "container/internal/variant_value_v2.h"
#include "container/integration/value_bridge.h"
#include "container/values/bool_value.h"
#include "container/values/numeric_value.h"
#include "container/values/bytes_value.h"
#include "container/values/string_value.h"
#include "container/values/container_value.h"
#include "container/values/array_value.h"
#include <vector>
#include <cstdint>

using namespace container_module;

// ============================================================================
// Type Index Alignment Tests
// ============================================================================

TEST(VariantValueV2Compatibility, TypeIndexAlignment) {
    // Verify that variant indices match value_types enum

    // Test null_value (0)
    variant_value_v2 null_val("test");
    EXPECT_EQ(null_val.type(), value_types::null_value);
    EXPECT_EQ(null_val.variant_index(), 0);

    // Test bool_value (1)
    variant_value_v2 bool_val("test", true);
    EXPECT_EQ(bool_val.type(), value_types::bool_value);
    EXPECT_EQ(bool_val.variant_index(), 1);

    // Test short_value (2)
    variant_value_v2 short_val("test", int16_t(42));
    EXPECT_EQ(short_val.type(), value_types::short_value);
    EXPECT_EQ(short_val.variant_index(), 2);

    // Test bytes_value (12)
    std::vector<uint8_t> bytes = {0xDE, 0xAD, 0xBE, 0xEF};
    variant_value_v2 bytes_val("test", bytes);
    EXPECT_EQ(bytes_val.type(), value_types::bytes_value);
    EXPECT_EQ(bytes_val.variant_index(), 12);

    // Test string_value (13)
    variant_value_v2 string_val("test", std::string("hello"));
    EXPECT_EQ(string_val.type(), value_types::string_value);
    EXPECT_EQ(string_val.variant_index(), 13);

    // Test array_value (15)
    array_variant arr;
    variant_value_v2 array_val("test", arr);
    EXPECT_EQ(array_val.type(), value_types::array_value);
    EXPECT_EQ(array_val.variant_index(), 15);
}

// ============================================================================
// Serialization Compatibility Tests
// ============================================================================

class SerializationCompatibilityTest : public ::testing::Test {
protected:
    // Helper: Create legacy value and serialize
    template<typename LegacyType, typename... Args>
    std::vector<uint8_t> serialize_legacy(const std::string& name, Args&&... args) {
        auto legacy = std::make_shared<LegacyType>(name, std::forward<Args>(args)...);
        auto str = legacy->serialize();
        return std::vector<uint8_t>(str.begin(), str.end());
    }

    // Helper: Deserialize and verify type
    void verify_deserialization(const std::vector<uint8_t>& data,
                               value_types expected_type,
                               const std::string& expected_name) {
        auto result = variant_value_v2::deserialize(data);
        ASSERT_TRUE(result.has_value()) << "Deserialization failed";
        EXPECT_EQ(result->type(), expected_type);
        EXPECT_EQ(result->name(), expected_name);
    }
};

TEST_F(SerializationCompatibilityTest, BoolValueRoundTrip) {
    // Legacy → bytes
    auto legacy_data = serialize_legacy<bool_value>("flag", true);

    // bytes → modern
    auto modern = variant_value_v2::deserialize(legacy_data);
    ASSERT_TRUE(modern.has_value());
    EXPECT_EQ(modern->type(), value_types::bool_value);

    auto bool_val = modern->get<bool>();
    ASSERT_TRUE(bool_val.has_value());
    EXPECT_TRUE(*bool_val);

    // modern → bytes
    auto modern_data = modern->serialize();

    // Verify byte-for-byte equality
    EXPECT_EQ(legacy_data, modern_data);
}

TEST_F(SerializationCompatibilityTest, IntValueRoundTrip) {
    auto legacy_data = serialize_legacy<int_value>("count", 42);

    auto modern = variant_value_v2::deserialize(legacy_data);
    ASSERT_TRUE(modern.has_value());
    EXPECT_EQ(modern->type(), value_types::int_value);

    auto int_val = modern->get<int32_t>();
    ASSERT_TRUE(int_val.has_value());
    EXPECT_EQ(*int_val, 42);

    auto modern_data = modern->serialize();
    EXPECT_EQ(legacy_data, modern_data);
}

TEST_F(SerializationCompatibilityTest, StringValueRoundTrip) {
    std::string test_string = "Hello, World! 한글 테스트";
    auto legacy_data = serialize_legacy<string_value>("message", test_string);

    auto modern = variant_value_v2::deserialize(legacy_data);
    ASSERT_TRUE(modern.has_value());
    EXPECT_EQ(modern->type(), value_types::string_value);

    auto str_val = modern->get<std::string>();
    ASSERT_TRUE(str_val.has_value());
    EXPECT_EQ(*str_val, test_string);

    auto modern_data = modern->serialize();
    EXPECT_EQ(legacy_data, modern_data);
}

TEST_F(SerializationCompatibilityTest, BytesValueRoundTrip) {
    std::vector<uint8_t> test_bytes = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
    auto legacy_data = serialize_legacy<bytes_value>("binary", test_bytes);

    auto modern = variant_value_v2::deserialize(legacy_data);
    ASSERT_TRUE(modern.has_value());
    EXPECT_EQ(modern->type(), value_types::bytes_value);

    auto bytes_val = modern->get<std::vector<uint8_t>>();
    ASSERT_TRUE(bytes_val.has_value());
    EXPECT_EQ(*bytes_val, test_bytes);

    auto modern_data = modern->serialize();
    EXPECT_EQ(legacy_data, modern_data);
}

TEST_F(SerializationCompatibilityTest, FloatValueRoundTrip) {
    auto legacy_data = serialize_legacy<float_value>("pi", 3.14159f);

    auto modern = variant_value_v2::deserialize(legacy_data);
    ASSERT_TRUE(modern.has_value());
    EXPECT_EQ(modern->type(), value_types::float_value);

    auto float_val = modern->get<float>();
    ASSERT_TRUE(float_val.has_value());
    EXPECT_FLOAT_EQ(*float_val, 3.14159f);

    auto modern_data = modern->serialize();
    EXPECT_EQ(legacy_data, modern_data);
}

TEST_F(SerializationCompatibilityTest, DoubleValueRoundTrip) {
    auto legacy_data = serialize_legacy<double_value>("e", 2.718281828459045);

    auto modern = variant_value_v2::deserialize(legacy_data);
    ASSERT_TRUE(modern.has_value());
    EXPECT_EQ(modern->type(), value_types::double_value);

    auto double_val = modern->get<double>();
    ASSERT_TRUE(double_val.has_value());
    EXPECT_DOUBLE_EQ(*double_val, 2.718281828459045);

    auto modern_data = modern->serialize();
    EXPECT_EQ(legacy_data, modern_data);
}

// ============================================================================
// Value Bridge Tests
// ============================================================================

class ValueBridgeTest : public ::testing::Test {
protected:
    void SetUp() override {
        value_bridge::reset_stats();
    }
};

TEST_F(ValueBridgeTest, BoolValueConversion) {
    // Legacy → Modern
    auto legacy = std::make_shared<bool_value>("flag", true);
    auto modern = value_bridge::to_modern(legacy);

    EXPECT_EQ(modern.type(), value_types::bool_value);
    auto val = modern.get<bool>();
    ASSERT_TRUE(val.has_value());
    EXPECT_TRUE(*val);

    // Modern → Legacy
    auto back_to_legacy = value_bridge::to_legacy(modern);
    EXPECT_EQ(back_to_legacy->type(), value_types::bool_value);
    EXPECT_TRUE(back_to_legacy->to_boolean());
}

TEST_F(ValueBridgeTest, NumericValuesConversion) {
    // Test all numeric types
    struct NumericTest {
        value_types type;
        std::shared_ptr<value> legacy;
        std::function<bool(const variant_value_v2&)> verify;
    };

    std::vector<NumericTest> tests = {
        {value_types::short_value, std::make_shared<short_value>("s", 42),
         [](const variant_value_v2& v) { return v.get<int16_t>() == 42; }},
        {value_types::int_value, std::make_shared<int_value>("i", 12345),
         [](const variant_value_v2& v) { return v.get<int32_t>() == 12345; }},
        {value_types::long_value, std::make_shared<llong_value>("l", 9876543210LL),
         [](const variant_value_v2& v) { return v.get<int64_t>() == 9876543210LL; }},
        {value_types::float_value, std::make_shared<float_value>("f", 3.14f),
         [](const variant_value_v2& v) { auto fv = v.get<float>(); return fv && std::abs(*fv - 3.14f) < 0.001f; }},
        {value_types::double_value, std::make_shared<double_value>("d", 2.718),
         [](const variant_value_v2& v) { auto dv = v.get<double>(); return dv && std::abs(*dv - 2.718) < 0.0001; }},
    };

    for (const auto& test : tests) {
        auto modern = value_bridge::to_modern(test.legacy);
        EXPECT_EQ(modern.type(), test.type);
        EXPECT_TRUE(test.verify(modern)) << "Verification failed for type "
                                         << static_cast<int>(test.type);

        auto back = value_bridge::to_legacy(modern);
        EXPECT_EQ(back->type(), test.type);
    }
}

TEST_F(ValueBridgeTest, StringValueConversion) {
    std::string test_str = "Unicode test: 한글, 日本語, Emoji: 🚀";
    auto legacy = std::make_shared<string_value>("text", test_str);

    auto modern = value_bridge::to_modern(legacy);
    EXPECT_EQ(modern.type(), value_types::string_value);

    auto str_val = modern.get<std::string>();
    ASSERT_TRUE(str_val.has_value());
    EXPECT_EQ(*str_val, test_str);

    auto back = value_bridge::to_legacy(modern);
    EXPECT_EQ(back->to_string(), test_str);
}

TEST_F(ValueBridgeTest, BytesValueConversion) {
    std::vector<uint8_t> test_bytes;
    for (int i = 0; i < 256; ++i) {
        test_bytes.push_back(static_cast<uint8_t>(i));
    }

    auto legacy = std::make_shared<bytes_value>("data", test_bytes);
    auto modern = value_bridge::to_modern(legacy);

    EXPECT_EQ(modern.type(), value_types::bytes_value);
    auto bytes_val = modern.get<std::vector<uint8_t>>();
    ASSERT_TRUE(bytes_val.has_value());
    EXPECT_EQ(*bytes_val, test_bytes);

    auto back = value_bridge::to_legacy(modern);
    EXPECT_EQ(back->to_bytes(), test_bytes);
}

TEST_F(ValueBridgeTest, ArrayValueConversion) {
    // Create legacy array with mixed types
    std::vector<std::shared_ptr<value>> elements;
    elements.push_back(std::make_shared<bool_value>("flag", true));
    elements.push_back(std::make_shared<int_value>("num", 42));
    elements.push_back(std::make_shared<string_value>("text", "hello"));

    auto legacy = std::make_shared<array_value>("mixed", elements);
    auto modern = value_bridge::to_modern(legacy);

    EXPECT_EQ(modern.type(), value_types::array_value);
    auto arr = modern.get<array_variant>();
    ASSERT_TRUE(arr.has_value());
    EXPECT_EQ(arr->values.size(), 3);

    // Verify elements
    EXPECT_EQ(arr->values[0]->type(), value_types::bool_value);
    EXPECT_EQ(arr->values[1]->type(), value_types::int_value);
    EXPECT_EQ(arr->values[2]->type(), value_types::string_value);

    auto back = value_bridge::to_legacy(modern);
    EXPECT_EQ(back->type(), value_types::array_value);
    EXPECT_EQ(back->child_count(), 3);
}

// ============================================================================
// Round-Trip Verification Tests
// ============================================================================

TEST_F(ValueBridgeTest, RoundTripAllTypes) {
    std::vector<std::shared_ptr<value>> test_values = {
        std::make_shared<value>("null"),
        std::make_shared<bool_value>("bool", false),
        std::make_shared<short_value>("short", 100),
        std::make_shared<int_value>("int", 50000),
        std::make_shared<llong_value>("long", 9999999999LL),
        std::make_shared<float_value>("float", 1.234f),
        std::make_shared<double_value>("double", 5.678),
        std::make_shared<bytes_value>("bytes", std::vector<uint8_t>{1, 2, 3, 4}),
        std::make_shared<string_value>("string", "test"),
    };

    for (const auto& original : test_values) {
        EXPECT_TRUE(value_bridge::verify_round_trip(original))
            << "Round-trip failed for type " << static_cast<int>(original->type());
    }
}

TEST_F(ValueBridgeTest, StatisticsTracking) {
    auto legacy1 = std::make_shared<int_value>("test1", 42);
    auto legacy2 = std::make_shared<string_value>("test2", "hello");

    value_bridge::to_modern(legacy1);
    value_bridge::to_modern(legacy2);

    auto stats = value_bridge::get_stats();
    EXPECT_GE(stats.successful_conversions, 2);
    EXPECT_EQ(stats.failed_conversions, 0);
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST(VariantValueV2EdgeCases, EmptyString) {
    variant_value_v2 val("empty", std::string(""));
    auto serialized = val.serialize();
    auto deserialized = variant_value_v2::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());
    auto str = deserialized->get<std::string>();
    ASSERT_TRUE(str.has_value());
    EXPECT_EQ(*str, "");
}

TEST(VariantValueV2EdgeCases, EmptyBytes) {
    variant_value_v2 val("empty", std::vector<uint8_t>());
    auto serialized = val.serialize();
    auto deserialized = variant_value_v2::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());
    auto bytes = deserialized->get<std::vector<uint8_t>>();
    ASSERT_TRUE(bytes.has_value());
    EXPECT_TRUE(bytes->empty());
}

TEST(VariantValueV2EdgeCases, LargeString) {
    std::string large(1024 * 1024, 'A');  // 1MB string
    variant_value_v2 val("large", large);
    auto serialized = val.serialize();
    auto deserialized = variant_value_v2::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());
    auto str = deserialized->get<std::string>();
    ASSERT_TRUE(str.has_value());
    EXPECT_EQ(*str, large);
}

TEST(VariantValueV2EdgeCases, InvalidDeserialization) {
    // Too small data
    std::vector<uint8_t> invalid1 = {0x01};
    EXPECT_FALSE(variant_value_v2::deserialize(invalid1).has_value());

    // Invalid type byte
    std::vector<uint8_t> invalid2 = {
        0x00, 0x00, 0x00, 0x00,  // name_len = 0
        0xFF                      // invalid type (> 15)
    };
    EXPECT_FALSE(variant_value_v2::deserialize(invalid2).has_value());
}

// ============================================================================
// Performance Comparison Tests
// ============================================================================

TEST(VariantValueV2Performance, SerializationSpeed) {
    const int iterations = 10000;

    // Modern variant_value_v2
    auto start_modern = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        variant_value_v2 val("test", i);
        auto data = val.serialize();
    }
    auto end_modern = std::chrono::high_resolution_clock::now();
    auto modern_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_modern - start_modern).count();

    // Legacy value
    auto start_legacy = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        int_value val("test", i);
        auto data = val.serialize();
    }
    auto end_legacy = std::chrono::high_resolution_clock::now();
    auto legacy_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_legacy - start_legacy).count();

    std::cout << "Serialization performance:\n"
              << "  Modern: " << modern_duration << " μs (" << iterations << " iterations)\n"
              << "  Legacy: " << legacy_duration << " μs (" << iterations << " iterations)\n"
              << "  Speedup: " << (static_cast<double>(legacy_duration) / modern_duration) << "x\n";

    // Modern should be at least as fast as legacy
    EXPECT_LE(modern_duration, legacy_duration * 1.5);  // Allow 50% slower max
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
