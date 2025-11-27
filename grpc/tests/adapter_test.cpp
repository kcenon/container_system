/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, kcenon
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
 * @file adapter_test.cpp
 * @brief Unit tests for container_adapter conversion layer
 *
 * Tests verify bidirectional conversion between container_module types
 * and gRPC proto messages with data integrity preservation.
 */

#include <gtest/gtest.h>

#include "adapters/container_adapter.h"
#include "adapters/value_mapper.h"
#include "container_service.pb.h"
#include "core/container.h"

#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <vector>

using namespace container_grpc;
using namespace container_module;

/**
 * Test fixture for container adapter tests
 */
class ContainerAdapterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a sample container for testing
        sample_container_ = std::make_shared<value_container>();
        sample_container_->set_source("test_source", "test_sub_source");
        sample_container_->set_target("test_target", "test_sub_target");
        sample_container_->set_message_type("test_message");
    }

    void TearDown() override {
        sample_container_.reset();
    }

    std::shared_ptr<value_container> sample_container_;
};

// =============================================================================
// Round-Trip Conversion Tests
// =============================================================================

TEST_F(ContainerAdapterTest, RoundTripEmptyContainer) {
    auto container = std::make_shared<value_container>();
    container->set_message_type("empty");

    // Convert to gRPC
    auto grpc = container_adapter::to_grpc(*container);

    // Convert back
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->message_type(), "empty");
}

TEST_F(ContainerAdapterTest, RoundTripWithHeaderFields) {
    sample_container_->set_source("source_id", "source_sub_id");
    sample_container_->set_target("target_id", "target_sub_id");
    sample_container_->set_message_type("test_type");

    auto grpc = container_adapter::to_grpc(*sample_container_);
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->source_id(), "source_id");
    EXPECT_EQ(restored->source_sub_id(), "source_sub_id");
    EXPECT_EQ(restored->target_id(), "target_id");
    EXPECT_EQ(restored->target_sub_id(), "target_sub_id");
    EXPECT_EQ(restored->message_type(), "test_type");
}

// =============================================================================
// Primitive Type Conversion Tests
// =============================================================================

TEST_F(ContainerAdapterTest, ConvertBoolValue) {
    sample_container_->add_value("bool_true", value_types::bool_value, true);
    sample_container_->add_value("bool_false", value_types::bool_value, false);

    auto grpc = container_adapter::to_grpc(*sample_container_);
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);

    auto val_true = restored->get_variant_value("bool_true");
    auto val_false = restored->get_variant_value("bool_false");

    ASSERT_TRUE(val_true.has_value());
    ASSERT_TRUE(val_false.has_value());

    EXPECT_TRUE(std::get<bool>(val_true->data));
    EXPECT_FALSE(std::get<bool>(val_false->data));
}

TEST_F(ContainerAdapterTest, ConvertIntegerTypes) {
    // Test various integer types
    sample_container_->add_value("short_val", value_types::short_value,
                                  static_cast<short>(32767));
    sample_container_->add_value("ushort_val", value_types::ushort_value,
                                  static_cast<unsigned short>(65535));
    sample_container_->add_value("int_val", value_types::int_value, 2147483647);
    sample_container_->add_value("uint_val", value_types::uint_value, 4294967295U);
    sample_container_->add_value("long_val", value_types::long_value,
                                  static_cast<long>(2147483647L));
    sample_container_->add_value("llong_val", value_types::llong_value,
                                  9223372036854775807LL);

    auto grpc = container_adapter::to_grpc(*sample_container_);
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);

    auto short_v = restored->get_variant_value("short_val");
    ASSERT_TRUE(short_v.has_value());
    EXPECT_EQ(std::get<short>(short_v->data), 32767);

    auto int_v = restored->get_variant_value("int_val");
    ASSERT_TRUE(int_v.has_value());
    EXPECT_EQ(std::get<int>(int_v->data), 2147483647);

    auto llong_v = restored->get_variant_value("llong_val");
    ASSERT_TRUE(llong_v.has_value());
    EXPECT_EQ(std::get<long long>(llong_v->data), 9223372036854775807LL);
}

TEST_F(ContainerAdapterTest, ConvertFloatingPointTypes) {
    sample_container_->add_value("float_val", value_types::float_value, 3.14159f);
    sample_container_->add_value("double_val", value_types::double_value,
                                  3.141592653589793);

    auto grpc = container_adapter::to_grpc(*sample_container_);
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);

    auto float_v = restored->get_variant_value("float_val");
    ASSERT_TRUE(float_v.has_value());
    EXPECT_NEAR(std::get<float>(float_v->data), 3.14159f, 0.00001f);

    auto double_v = restored->get_variant_value("double_val");
    ASSERT_TRUE(double_v.has_value());
    EXPECT_NEAR(std::get<double>(double_v->data), 3.141592653589793, 0.0000000001);
}

TEST_F(ContainerAdapterTest, ConvertStringValue) {
    std::string test_string = "Hello, gRPC World!";
    sample_container_->add_value("string_val", value_types::string_value,
                                  test_string);

    auto grpc = container_adapter::to_grpc(*sample_container_);
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);

    auto str_v = restored->get_variant_value("string_val");
    ASSERT_TRUE(str_v.has_value());
    EXPECT_EQ(std::get<std::string>(str_v->data), test_string);
}

TEST_F(ContainerAdapterTest, ConvertEmptyString) {
    sample_container_->add_value("empty_string", value_types::string_value,
                                  std::string(""));

    auto grpc = container_adapter::to_grpc(*sample_container_);
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);

    auto str_v = restored->get_variant_value("empty_string");
    ASSERT_TRUE(str_v.has_value());
    EXPECT_EQ(std::get<std::string>(str_v->data), "");
}

TEST_F(ContainerAdapterTest, ConvertBytesValue) {
    std::vector<uint8_t> test_bytes = {0x00, 0x01, 0x02, 0xFF, 0xFE, 0xFD};
    sample_container_->add_value("bytes_val", value_types::bytes_value, test_bytes);

    auto grpc = container_adapter::to_grpc(*sample_container_);
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);

    auto bytes_v = restored->get_variant_value("bytes_val");
    ASSERT_TRUE(bytes_v.has_value());
    EXPECT_EQ(std::get<std::vector<uint8_t>>(bytes_v->data), test_bytes);
}

TEST_F(ContainerAdapterTest, ConvertNullValue) {
    sample_container_->add_value("null_val", value_types::null_value,
                                  std::monostate{});

    auto grpc = container_adapter::to_grpc(*sample_container_);
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);

    auto null_v = restored->get_variant_value("null_val");
    ASSERT_TRUE(null_v.has_value());
    EXPECT_EQ(null_v->type, value_types::null_value);
}

// =============================================================================
// Nested Container Tests
// =============================================================================

TEST_F(ContainerAdapterTest, ConvertNestedContainer) {
    auto nested = std::make_shared<value_container>();
    nested->set_message_type("nested_type");
    nested->add_value("nested_int", value_types::int_value, 42);
    nested->add_value("nested_string", value_types::string_value,
                       std::string("nested value"));

    sample_container_->add_value("nested_container", value_types::container_value,
                                  nested);

    auto grpc = container_adapter::to_grpc(*sample_container_);
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);

    auto nested_v = restored->get_variant_value("nested_container");
    ASSERT_TRUE(nested_v.has_value());
    ASSERT_EQ(nested_v->type, value_types::container_value);

    auto restored_nested =
        std::get<std::shared_ptr<value_container>>(nested_v->data);
    ASSERT_NE(restored_nested, nullptr);
    EXPECT_EQ(restored_nested->message_type(), "nested_type");

    auto nested_int = restored_nested->get_variant_value("nested_int");
    ASSERT_TRUE(nested_int.has_value());
    EXPECT_EQ(std::get<int>(nested_int->data), 42);
}

TEST_F(ContainerAdapterTest, ConvertDeeplyNestedContainer) {
    // Create 3 levels of nesting
    auto level3 = std::make_shared<value_container>();
    level3->set_message_type("level3");
    level3->add_value("depth", value_types::int_value, 3);

    auto level2 = std::make_shared<value_container>();
    level2->set_message_type("level2");
    level2->add_value("depth", value_types::int_value, 2);
    level2->add_value("child", value_types::container_value, level3);

    auto level1 = std::make_shared<value_container>();
    level1->set_message_type("level1");
    level1->add_value("depth", value_types::int_value, 1);
    level1->add_value("child", value_types::container_value, level2);

    sample_container_->add_value("root_child", value_types::container_value, level1);

    auto grpc = container_adapter::to_grpc(*sample_container_);
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);

    // Navigate through nested structure
    auto l1 = restored->get_variant_value("root_child");
    ASSERT_TRUE(l1.has_value());
    auto l1_container = std::get<std::shared_ptr<value_container>>(l1->data);
    ASSERT_NE(l1_container, nullptr);

    auto l2 = l1_container->get_variant_value("child");
    ASSERT_TRUE(l2.has_value());
    auto l2_container = std::get<std::shared_ptr<value_container>>(l2->data);
    ASSERT_NE(l2_container, nullptr);

    auto l3 = l2_container->get_variant_value("child");
    ASSERT_TRUE(l3.has_value());
    auto l3_container = std::get<std::shared_ptr<value_container>>(l3->data);
    ASSERT_NE(l3_container, nullptr);

    auto depth = l3_container->get_variant_value("depth");
    ASSERT_TRUE(depth.has_value());
    EXPECT_EQ(std::get<int>(depth->data), 3);
}

// =============================================================================
// Edge Cases and Error Handling
// =============================================================================

TEST_F(ContainerAdapterTest, ConvertSpecialFloatValues) {
    sample_container_->add_value("infinity", value_types::double_value,
                                  std::numeric_limits<double>::infinity());
    sample_container_->add_value("neg_infinity", value_types::double_value,
                                  -std::numeric_limits<double>::infinity());

    auto grpc = container_adapter::to_grpc(*sample_container_);
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);

    auto inf_v = restored->get_variant_value("infinity");
    ASSERT_TRUE(inf_v.has_value());
    EXPECT_TRUE(std::isinf(std::get<double>(inf_v->data)));
    EXPECT_GT(std::get<double>(inf_v->data), 0);

    auto neg_inf_v = restored->get_variant_value("neg_infinity");
    ASSERT_TRUE(neg_inf_v.has_value());
    EXPECT_TRUE(std::isinf(std::get<double>(neg_inf_v->data)));
    EXPECT_LT(std::get<double>(neg_inf_v->data), 0);
}

TEST_F(ContainerAdapterTest, ConvertLargeBinaryData) {
    // Test with 1MB of binary data
    std::vector<uint8_t> large_data(1024 * 1024);
    for (size_t i = 0; i < large_data.size(); ++i) {
        large_data[i] = static_cast<uint8_t>(i % 256);
    }

    sample_container_->add_value("large_bytes", value_types::bytes_value, large_data);

    auto grpc = container_adapter::to_grpc(*sample_container_);
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);

    auto bytes_v = restored->get_variant_value("large_bytes");
    ASSERT_TRUE(bytes_v.has_value());
    EXPECT_EQ(std::get<std::vector<uint8_t>>(bytes_v->data).size(),
              large_data.size());
    EXPECT_EQ(std::get<std::vector<uint8_t>>(bytes_v->data), large_data);
}

TEST_F(ContainerAdapterTest, ConvertUnicodeString) {
    std::string unicode_str = u8"Hello \u4e16\u754c \U0001F600"; // "Hello 世界 😀"
    sample_container_->add_value("unicode", value_types::string_value, unicode_str);

    auto grpc = container_adapter::to_grpc(*sample_container_);
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);

    auto str_v = restored->get_variant_value("unicode");
    ASSERT_TRUE(str_v.has_value());
    EXPECT_EQ(std::get<std::string>(str_v->data), unicode_str);
}

TEST_F(ContainerAdapterTest, ConvertMultipleValuesPreservesOrder) {
    for (int i = 0; i < 10; ++i) {
        sample_container_->add_value("value_" + std::to_string(i),
                                      value_types::int_value, i * 10);
    }

    auto grpc = container_adapter::to_grpc(*sample_container_);
    auto restored = container_adapter::from_grpc(grpc);

    ASSERT_NE(restored, nullptr);

    for (int i = 0; i < 10; ++i) {
        auto val = restored->get_variant_value("value_" + std::to_string(i));
        ASSERT_TRUE(val.has_value()) << "Missing value_" << i;
        EXPECT_EQ(std::get<int>(val->data), i * 10);
    }
}

// =============================================================================
// Validation Tests
// =============================================================================

TEST_F(ContainerAdapterTest, CanConvertValidContainer) {
    sample_container_->add_value("int", value_types::int_value, 42);
    sample_container_->add_value("string", value_types::string_value,
                                  std::string("test"));

    EXPECT_TRUE(container_adapter::can_convert(*sample_container_));
}

TEST_F(ContainerAdapterTest, CanConvertEmptyContainer) {
    auto empty = std::make_shared<value_container>();
    EXPECT_TRUE(container_adapter::can_convert(*empty));
}

// =============================================================================
// Type Mapping Tests
// =============================================================================

TEST_F(ContainerAdapterTest, TypeMappingRoundTrip) {
    // Test that all supported types map correctly
    std::vector<value_types> types = {
        value_types::null_value,   value_types::bool_value,
        value_types::short_value,  value_types::ushort_value,
        value_types::int_value,    value_types::uint_value,
        value_types::long_value,   value_types::ulong_value,
        value_types::llong_value,  value_types::ullong_value,
        value_types::float_value,  value_types::double_value,
        value_types::string_value, value_types::bytes_value,
        value_types::container_value};

    for (auto type : types) {
        auto grpc_type = container_adapter::to_grpc_type(type);
        auto restored_type = container_adapter::from_grpc_type(grpc_type);
        EXPECT_EQ(type, restored_type)
            << "Type mismatch for " << value_mapper::type_name(type);
    }
}

TEST_F(ContainerAdapterTest, ValueMapperIsSupportedCheck) {
    EXPECT_TRUE(value_mapper::is_supported(value_types::null_value));
    EXPECT_TRUE(value_mapper::is_supported(value_types::bool_value));
    EXPECT_TRUE(value_mapper::is_supported(value_types::int_value));
    EXPECT_TRUE(value_mapper::is_supported(value_types::string_value));
    EXPECT_TRUE(value_mapper::is_supported(value_types::container_value));
}

TEST_F(ContainerAdapterTest, ValueMapperTypeNames) {
    EXPECT_STREQ(value_mapper::type_name(value_types::null_value), "null");
    EXPECT_STREQ(value_mapper::type_name(value_types::bool_value), "bool");
    EXPECT_STREQ(value_mapper::type_name(value_types::int_value), "int");
    EXPECT_STREQ(value_mapper::type_name(value_types::string_value), "string");
    EXPECT_STREQ(value_mapper::type_name(value_types::container_value), "container");
}

// =============================================================================
// Size Calculator Tests
// =============================================================================

TEST_F(ContainerAdapterTest, SizeCalculatorEstimations) {
    // Verify size estimates are reasonable
    auto null_size =
        size_calculator::estimate_proto_size(value_types::null_value, 0);
    EXPECT_GT(null_size, 0u);

    auto string_size =
        size_calculator::estimate_proto_size(value_types::string_value, 100);
    EXPECT_GE(string_size, 100u);

    auto container_size =
        size_calculator::estimate_container_size(10, 20, 50);
    EXPECT_GT(container_size, 0u);
}

// =============================================================================
// Main
// =============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
