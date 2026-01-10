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
 * @file msgpack_tests.cpp
 * @brief Unit tests for MessagePack serialization/deserialization
 *
 * Tests cover:
 * - MessagePack encoder functionality
 * - MessagePack decoder functionality
 * - Round-trip serialization/deserialization
 * - Format detection
 * - Error handling
 */

#include <gtest/gtest.h>
#include "test_compat.h"
#include <vector>
#include <cstdint>
#include <string>

using namespace container_module;

// ============================================================================
// MessagePack Encoder Tests
// ============================================================================

class MsgpackEncoderTest : public ::testing::Test {
protected:
    msgpack_encoder encoder;
};

TEST_F(MsgpackEncoderTest, WriteNil) {
    encoder.write_nil();
    auto data = encoder.finish();

    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(data[0], 0xc0); // nil format
}

TEST_F(MsgpackEncoderTest, WriteBoolTrue) {
    encoder.write_bool(true);
    auto data = encoder.finish();

    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(data[0], 0xc3); // true format
}

TEST_F(MsgpackEncoderTest, WriteBoolFalse) {
    encoder.write_bool(false);
    auto data = encoder.finish();

    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(data[0], 0xc2); // false format
}

TEST_F(MsgpackEncoderTest, WritePositiveFixint) {
    encoder.write_uint(42);
    auto data = encoder.finish();

    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(data[0], 42); // positive fixint
}

TEST_F(MsgpackEncoderTest, WriteNegativeFixint) {
    encoder.write_int(-10);
    auto data = encoder.finish();

    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(static_cast<int8_t>(data[0]), -10); // negative fixint
}

TEST_F(MsgpackEncoderTest, WriteUint8) {
    encoder.write_uint(200);
    auto data = encoder.finish();

    ASSERT_EQ(data.size(), 2);
    EXPECT_EQ(data[0], 0xcc); // uint8 format
    EXPECT_EQ(data[1], 200);
}

TEST_F(MsgpackEncoderTest, WriteUint16) {
    encoder.write_uint(1000);
    auto data = encoder.finish();

    ASSERT_EQ(data.size(), 3);
    EXPECT_EQ(data[0], 0xcd); // uint16 format
}

TEST_F(MsgpackEncoderTest, WriteInt8) {
    encoder.write_int(-100);
    auto data = encoder.finish();

    ASSERT_EQ(data.size(), 2);
    EXPECT_EQ(data[0], 0xd0); // int8 format
}

TEST_F(MsgpackEncoderTest, WriteFloat) {
    encoder.write_float(3.14f);
    auto data = encoder.finish();

    ASSERT_EQ(data.size(), 5);
    EXPECT_EQ(data[0], 0xca); // float32 format
}

TEST_F(MsgpackEncoderTest, WriteDouble) {
    encoder.write_double(3.14159265358979);
    auto data = encoder.finish();

    ASSERT_EQ(data.size(), 9);
    EXPECT_EQ(data[0], 0xcb); // float64 format
}

TEST_F(MsgpackEncoderTest, WriteFixstr) {
    encoder.write_string("hello");
    auto data = encoder.finish();

    ASSERT_EQ(data.size(), 6); // 1 byte header + 5 bytes string
    EXPECT_EQ(data[0], 0xa5); // fixstr with length 5
    EXPECT_EQ(data[1], 'h');
    EXPECT_EQ(data[5], 'o');
}

TEST_F(MsgpackEncoderTest, WriteStr8) {
    std::string long_str(50, 'x');
    encoder.write_string(long_str);
    auto data = encoder.finish();

    EXPECT_EQ(data[0], 0xd9); // str8 format
    EXPECT_EQ(data[1], 50);   // length
}

TEST_F(MsgpackEncoderTest, WriteBinary) {
    std::vector<uint8_t> binary_data = {0x01, 0x02, 0x03, 0x04};
    encoder.write_binary(binary_data);
    auto data = encoder.finish();

    EXPECT_EQ(data[0], 0xc4); // bin8 format
    EXPECT_EQ(data[1], 4);    // length
}

TEST_F(MsgpackEncoderTest, WriteFixarray) {
    encoder.write_array_header(5);
    auto data = encoder.finish();

    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(data[0], 0x95); // fixarray with 5 elements
}

TEST_F(MsgpackEncoderTest, WriteFixmap) {
    encoder.write_map_header(3);
    auto data = encoder.finish();

    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(data[0], 0x83); // fixmap with 3 elements
}

// ============================================================================
// MessagePack Decoder Tests
// ============================================================================

class MsgpackDecoderTest : public ::testing::Test {
protected:
};

TEST_F(MsgpackDecoderTest, ReadNil) {
    std::vector<uint8_t> data = {0xc0};
    msgpack_decoder decoder(data);

    EXPECT_TRUE(decoder.read_nil());
    EXPECT_TRUE(decoder.eof());
}

TEST_F(MsgpackDecoderTest, ReadBoolTrue) {
    std::vector<uint8_t> data = {0xc3};
    msgpack_decoder decoder(data);

    auto result = decoder.read_bool();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(*result);
}

TEST_F(MsgpackDecoderTest, ReadBoolFalse) {
    std::vector<uint8_t> data = {0xc2};
    msgpack_decoder decoder(data);

    auto result = decoder.read_bool();
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(*result);
}

TEST_F(MsgpackDecoderTest, ReadPositiveFixint) {
    std::vector<uint8_t> data = {42};
    msgpack_decoder decoder(data);

    auto result = decoder.read_int();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST_F(MsgpackDecoderTest, ReadNegativeFixint) {
    std::vector<uint8_t> data = {static_cast<uint8_t>(-10)};
    msgpack_decoder decoder(data);

    auto result = decoder.read_int();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, -10);
}

TEST_F(MsgpackDecoderTest, ReadFixstr) {
    std::vector<uint8_t> data = {0xa5, 'h', 'e', 'l', 'l', 'o'};
    msgpack_decoder decoder(data);

    auto result = decoder.read_string();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
}

TEST_F(MsgpackDecoderTest, ReadMapHeader) {
    std::vector<uint8_t> data = {0x83};
    msgpack_decoder decoder(data);

    auto result = decoder.read_map_header();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 3);
}

TEST_F(MsgpackDecoderTest, ReadArrayHeader) {
    std::vector<uint8_t> data = {0x95};
    msgpack_decoder decoder(data);

    auto result = decoder.read_array_header();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 5);
}

TEST_F(MsgpackDecoderTest, PeekType) {
    std::vector<uint8_t> data = {0xc0, 0xc3, 42, 0xa5};
    msgpack_decoder decoder(data);

    EXPECT_EQ(decoder.peek_type(), msgpack_type::nil);
    decoder.read_nil();

    EXPECT_EQ(decoder.peek_type(), msgpack_type::boolean);
    decoder.read_bool();

    EXPECT_EQ(decoder.peek_type(), msgpack_type::positive_int);
    decoder.read_int();

    EXPECT_EQ(decoder.peek_type(), msgpack_type::str);
}

// ============================================================================
// Container MessagePack Integration Tests
// ============================================================================

class ContainerMsgpackTest : public ::testing::Test {
protected:
    std::shared_ptr<value_container> container;

    void SetUp() override {
        container = std::make_shared<value_container>();
    }
};

TEST_F(ContainerMsgpackTest, EmptyContainerRoundTrip) {
    auto data = container->to_msgpack();
    EXPECT_FALSE(data.empty());

    auto restored = value_container::create_from_msgpack(data);
    ASSERT_NE(restored, nullptr);

    EXPECT_EQ(restored->message_type(), container->message_type());
}

TEST_F(ContainerMsgpackTest, ContainerWithValuesRoundTrip) {
    container->set("name", std::string("Alice"));
    container->set("age", 30);
    container->set("score", 95.5);
    container->set("active", true);

    auto data = container->to_msgpack();
    EXPECT_FALSE(data.empty());

    auto restored = value_container::create_from_msgpack(data);
    ASSERT_NE(restored, nullptr);

    auto name = restored->get_value("name");
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(std::get<std::string>(name->data), "Alice");

    auto age = restored->get_value("age");
    ASSERT_TRUE(age.has_value());
    EXPECT_EQ(std::get<int>(age->data), 30);

    auto score = restored->get_value("score");
    ASSERT_TRUE(score.has_value());
    EXPECT_DOUBLE_EQ(std::get<double>(score->data), 95.5);

    auto active = restored->get_value("active");
    ASSERT_TRUE(active.has_value());
    EXPECT_TRUE(std::get<bool>(active->data));
}

TEST_F(ContainerMsgpackTest, ContainerWithBinaryData) {
    std::vector<uint8_t> binary = {0x01, 0x02, 0x03, 0x04, 0x05};
    container->set("data", binary);

    auto msgpack_data = container->to_msgpack();
    auto restored = value_container::create_from_msgpack(msgpack_data);
    ASSERT_NE(restored, nullptr);

    auto data = restored->get_value("data");
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(std::get<std::vector<uint8_t>>(data->data), binary);
}

TEST_F(ContainerMsgpackTest, ContainerWithHeader) {
    container->set_source("source_app", "instance1");
    container->set_target("target_app", "instance2");
    container->set_message_type("test_message");

    auto data = container->to_msgpack();
    auto restored = value_container::create_from_msgpack(data);
    ASSERT_NE(restored, nullptr);

    EXPECT_EQ(restored->source_id(), "source_app");
    EXPECT_EQ(restored->source_sub_id(), "instance1");
    EXPECT_EQ(restored->target_id(), "target_app");
    EXPECT_EQ(restored->target_sub_id(), "instance2");
    EXPECT_EQ(restored->message_type(), "test_message");
}

TEST_F(ContainerMsgpackTest, FromMsgpackMethod) {
    container->set("key", std::string("value"));
    auto data = container->to_msgpack();

    auto new_container = std::make_shared<value_container>();
    bool result = new_container->from_msgpack(data);
    EXPECT_TRUE(result);

    auto key = new_container->get_value("key");
    ASSERT_TRUE(key.has_value());
    EXPECT_EQ(std::get<std::string>(key->data), "value");
}

TEST_F(ContainerMsgpackTest, InvalidDataReturnsFalse) {
    std::vector<uint8_t> invalid_data = {0x00, 0x01, 0x02};
    bool result = container->from_msgpack(invalid_data);
    EXPECT_FALSE(result);
}

TEST_F(ContainerMsgpackTest, EmptyDataReturnsFalse) {
    std::vector<uint8_t> empty_data;
    bool result = container->from_msgpack(empty_data);
    EXPECT_FALSE(result);
}

TEST_F(ContainerMsgpackTest, CreateFromMsgpackWithInvalidData) {
    std::vector<uint8_t> invalid_data = {0x00, 0x01, 0x02};
    auto restored = value_container::create_from_msgpack(invalid_data);
    EXPECT_EQ(restored, nullptr);
}

// ============================================================================
// Format Detection Tests
// ============================================================================

class FormatDetectionTest : public ::testing::Test {
protected:
};

TEST_F(FormatDetectionTest, DetectMsgpackFormat) {
    // MessagePack fixmap
    std::vector<uint8_t> data = {0x82}; // fixmap with 2 elements
    auto format = value_container::detect_format(data);
    EXPECT_EQ(format, value_container::serialization_format::msgpack);
}

TEST_F(FormatDetectionTest, DetectJsonFormat) {
    std::string json = "{\"key\": \"value\"}";
    std::vector<uint8_t> data(json.begin(), json.end());
    auto format = value_container::detect_format(data);
    EXPECT_EQ(format, value_container::serialization_format::json);
}

TEST_F(FormatDetectionTest, DetectXmlFormat) {
    std::string xml = "<?xml version=\"1.0\"?><container></container>";
    std::vector<uint8_t> data(xml.begin(), xml.end());
    auto format = value_container::detect_format(data);
    EXPECT_EQ(format, value_container::serialization_format::xml);
}

TEST_F(FormatDetectionTest, DetectBinaryFormat) {
    std::string binary = "@header{{[1,test];[2,value];}};@data{{}};";
    std::vector<uint8_t> data(binary.begin(), binary.end());
    auto format = value_container::detect_format(data);
    EXPECT_EQ(format, value_container::serialization_format::binary);
}

TEST_F(FormatDetectionTest, DetectUnknownFormat) {
    std::vector<uint8_t> data = {0x00, 0x00, 0x00};
    auto format = value_container::detect_format(data);
    EXPECT_EQ(format, value_container::serialization_format::unknown);
}

TEST_F(FormatDetectionTest, DetectEmptyData) {
    std::vector<uint8_t> data;
    auto format = value_container::detect_format(data);
    EXPECT_EQ(format, value_container::serialization_format::unknown);
}

TEST_F(FormatDetectionTest, DetectFormatWithWhitespace) {
    std::string json = "  \n  {\"key\": \"value\"}";
    std::vector<uint8_t> data(json.begin(), json.end());
    auto format = value_container::detect_format(data);
    EXPECT_EQ(format, value_container::serialization_format::json);
}

// ============================================================================
// Result API Tests (if available)
// ============================================================================

#if CONTAINER_HAS_COMMON_RESULT
class MsgpackResultApiTest : public ::testing::Test {
protected:
    std::shared_ptr<value_container> container;

    void SetUp() override {
        container = std::make_shared<value_container>();
        container->set("key", std::string("value"));
    }
};

TEST_F(MsgpackResultApiTest, ToMsgpackResultSuccess) {
    auto result = container->to_msgpack_result();
    EXPECT_TRUE(result.is_ok());
    EXPECT_FALSE(result.value().empty());
}

TEST_F(MsgpackResultApiTest, FromMsgpackResultSuccess) {
    auto data = container->to_msgpack();
    auto new_container = std::make_shared<value_container>();

    auto result = new_container->from_msgpack_result(data);
    EXPECT_TRUE(result.is_ok());
}

TEST_F(MsgpackResultApiTest, FromMsgpackResultFailure) {
    std::vector<uint8_t> invalid_data = {0x00, 0x01};
    auto result = container->from_msgpack_result(invalid_data);
    EXPECT_TRUE(result.is_err());
}
#endif

// ============================================================================
// Performance Comparison Tests
// ============================================================================

class MsgpackPerformanceTest : public ::testing::Test {
protected:
    std::shared_ptr<value_container> container;

    void SetUp() override {
        container = std::make_shared<value_container>();
        // Add some test data
        for (int i = 0; i < 100; ++i) {
            container->set("key" + std::to_string(i), i * 100);
        }
    }
};

TEST_F(MsgpackPerformanceTest, CompareOutputSize) {
    // Get binary serialization size
    auto binary_data = container->serialize_array();

    // Get JSON size
    auto json_data = container->to_json();

    // Get MessagePack size
    auto msgpack_data = container->to_msgpack();

    // MessagePack should be smaller than JSON
    EXPECT_LT(msgpack_data.size(), json_data.size());

    // Log sizes for information
    SCOPED_TRACE("Binary size: " + std::to_string(binary_data.size()));
    SCOPED_TRACE("JSON size: " + std::to_string(json_data.size()));
    SCOPED_TRACE("MessagePack size: " + std::to_string(msgpack_data.size()));
}

TEST_F(MsgpackPerformanceTest, RoundTripPreservesData) {
    auto msgpack_data = container->to_msgpack();
    auto restored = value_container::create_from_msgpack(msgpack_data);
    ASSERT_NE(restored, nullptr);

    // Verify all values are preserved
    for (int i = 0; i < 100; ++i) {
        auto value = restored->get_value("key" + std::to_string(i));
        ASSERT_TRUE(value.has_value()) << "Missing key" << i;
        EXPECT_EQ(std::get<int>(value->data), i * 100) << "Mismatch at key" << i;
    }
}
