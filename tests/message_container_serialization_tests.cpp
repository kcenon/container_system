// BSD 3-Clause License
// Copyright (c) 2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file message_container_serialization_tests.cpp
 * @brief Round-trip tests for message_container binary and JSON serialization
 *
 * Covers the in-place binary deserialization path for the value_store
 * payload (kcenon/container_system#545) and the in-place JSON
 * deserialization path (kcenon/container_system#546).
 */

#include <gtest/gtest.h>

#include <kcenon/container/messaging/message_container.h>
#include <kcenon/container/value_store.h>
#include <kcenon/container/internal/value.h>

#include <stdexcept>
#include <string>
#include <vector>

using namespace kcenon::container;

// =============================================================================
// message_container binary round-trip
// =============================================================================

TEST(MessageContainerBinaryTest, RoundTripPreservesHeader) {
    message_container source;
    source.set_source("src_id", "src_sub");
    source.set_target("tgt_id", "tgt_sub");
    source.set_message_type("request");
    source.set_version("1.0");

    auto binary = source.serialize_binary();
    auto restored = message_container::deserialize_binary(binary);
    ASSERT_NE(restored, nullptr);

    EXPECT_EQ(restored->source_id(), "src_id");
    EXPECT_EQ(restored->source_sub_id(), "src_sub");
    EXPECT_EQ(restored->target_id(), "tgt_id");
    EXPECT_EQ(restored->target_sub_id(), "tgt_sub");
    EXPECT_EQ(restored->message_type(), "request");
    EXPECT_EQ(restored->version(), "1.0");
}

TEST(MessageContainerBinaryTest, RoundTripPreservesPayload) {
    message_container source;
    source.set_message_type("data");
    source.payload().add("integer", value("integer", int32_t(42)));
    source.payload().add("text", value("text", std::string("hello world")));
    source.payload().add("flag", value("flag", true));
    source.payload().add("decimal", value("decimal", double(3.14)));

    auto binary = source.serialize_binary();
    auto restored = message_container::deserialize_binary(binary);
    ASSERT_NE(restored, nullptr);

    EXPECT_EQ(restored->payload().size(), 4u);

    auto int_val = restored->payload().get("integer");
    ASSERT_TRUE(int_val.has_value());
    auto int_opt = int_val->get<int32_t>();
    ASSERT_TRUE(int_opt.has_value());
    EXPECT_EQ(*int_opt, 42);

    auto str_val = restored->payload().get("text");
    ASSERT_TRUE(str_val.has_value());
    auto str_opt = str_val->get<std::string>();
    ASSERT_TRUE(str_opt.has_value());
    EXPECT_EQ(*str_opt, "hello world");

    auto bool_val = restored->payload().get("flag");
    ASSERT_TRUE(bool_val.has_value());
    auto bool_opt = bool_val->get<bool>();
    ASSERT_TRUE(bool_opt.has_value());
    EXPECT_EQ(*bool_opt, true);

    auto dbl_val = restored->payload().get("decimal");
    ASSERT_TRUE(dbl_val.has_value());
    auto dbl_opt = dbl_val->get<double>();
    ASSERT_TRUE(dbl_opt.has_value());
    EXPECT_DOUBLE_EQ(*dbl_opt, 3.14);
}

TEST(MessageContainerBinaryTest, RoundTripEmptyPayload) {
    message_container source;
    source.set_message_type("ping");

    auto binary = source.serialize_binary();
    auto restored = message_container::deserialize_binary(binary);
    ASSERT_NE(restored, nullptr);

    EXPECT_TRUE(restored->payload().empty());
    EXPECT_EQ(restored->message_type(), "ping");
}

// =============================================================================
// value_store in-place deserialization
// =============================================================================

TEST(ValueStoreInPlaceDeserializeTest, PopulatesExistingStore) {
    value_store source;
    source.add("key1", value("key1", int32_t(100)));
    source.add("key2", value("key2", std::string("test")));

    auto binary = source.serialize_binary();

    value_store target;
    value_store::deserialize_binary_into(target, binary);

    EXPECT_EQ(target.size(), 2u);
    EXPECT_TRUE(target.contains("key1"));
    EXPECT_TRUE(target.contains("key2"));
}

TEST(ValueStoreInPlaceDeserializeTest, ReplacesPreviousContents) {
    value_store source;
    source.add("new_key", value("new_key", int32_t(7)));
    auto binary = source.serialize_binary();

    value_store target;
    target.add("stale_key", value("stale_key", int32_t(1)));

    value_store::deserialize_binary_into(target, binary);

    EXPECT_EQ(target.size(), 1u);
    EXPECT_FALSE(target.contains("stale_key"));
    EXPECT_TRUE(target.contains("new_key"));
}

TEST(ValueStoreInPlaceDeserializeTest, MalformedInputLeavesStoreUnmodified) {
    value_store target;
    target.add("existing", value("existing", int32_t(99)));

    std::vector<uint8_t> too_small = {1};
    EXPECT_THROW(value_store::deserialize_binary_into(target, too_small),
                 std::runtime_error);

    std::vector<uint8_t> bad_version = {99, 0, 0, 0, 0};
    EXPECT_THROW(value_store::deserialize_binary_into(target, bad_version),
                 std::runtime_error);

    // Truncated: claims one entry but provides no entry data
    std::vector<uint8_t> truncated = {1, 1, 0, 0, 0};
    EXPECT_THROW(value_store::deserialize_binary_into(target, truncated),
                 std::runtime_error);

    // Store must retain its original state after each failure
    EXPECT_EQ(target.size(), 1u);
    EXPECT_TRUE(target.contains("existing"));
}

TEST(ValueStoreInPlaceDeserializeTest, FactoryStillWorks) {
    value_store source;
    source.add("k", value("k", int32_t(5)));
    auto binary = source.serialize_binary();

    auto restored = value_store::deserialize_binary(binary);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->size(), 1u);
    EXPECT_TRUE(restored->contains("k"));
}

// =============================================================================
// value_store JSON deserialization
// =============================================================================

TEST(ValueStoreJsonDeserializeTest, RoundTripPreservesScalarTypes) {
    value_store source;
    source.add("integer", value("integer", int32_t(42)));
    source.add("text", value("text", std::string("hello world")));
    source.add("flag", value("flag", true));
    source.add("decimal", value("decimal", double(3.14)));

    auto json = source.serialize();
    auto restored = value_store::deserialize(json);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->size(), 4u);

    auto int_val = restored->get("integer");
    ASSERT_TRUE(int_val.has_value());
    ASSERT_TRUE(int_val->get<int32_t>().has_value());
    EXPECT_EQ(*int_val->get<int32_t>(), 42);

    auto str_val = restored->get("text");
    ASSERT_TRUE(str_val.has_value());
    ASSERT_TRUE(str_val->get<std::string>().has_value());
    EXPECT_EQ(*str_val->get<std::string>(), "hello world");

    auto bool_val = restored->get("flag");
    ASSERT_TRUE(bool_val.has_value());
    ASSERT_TRUE(bool_val->get<bool>().has_value());
    EXPECT_EQ(*bool_val->get<bool>(), true);

    auto dbl_val = restored->get("decimal");
    ASSERT_TRUE(dbl_val.has_value());
    ASSERT_TRUE(dbl_val->get<double>().has_value());
    EXPECT_DOUBLE_EQ(*dbl_val->get<double>(), 3.14);
}

TEST(ValueStoreJsonDeserializeTest, InPlacePopulatesExistingStore) {
    value_store source;
    source.add("a", value("a", int32_t(1)));
    source.add("b", value("b", std::string("two")));

    auto json = source.serialize();

    value_store target;
    value_store::deserialize_into(target, json);

    EXPECT_EQ(target.size(), 2u);
    EXPECT_TRUE(target.contains("a"));
    EXPECT_TRUE(target.contains("b"));
}

TEST(ValueStoreJsonDeserializeTest, InPlaceReplacesPreviousContents) {
    value_store source;
    source.add("new_key", value("new_key", int32_t(7)));
    auto json = source.serialize();

    value_store target;
    target.add("stale_key", value("stale_key", int32_t(1)));

    value_store::deserialize_into(target, json);

    EXPECT_EQ(target.size(), 1u);
    EXPECT_FALSE(target.contains("stale_key"));
    EXPECT_TRUE(target.contains("new_key"));
}

TEST(ValueStoreJsonDeserializeTest, MalformedInputLeavesStoreUnmodified) {
    value_store target;
    target.add("existing", value("existing", int32_t(99)));

    EXPECT_ANY_THROW(value_store::deserialize_into(target, "{not valid json"));
    EXPECT_ANY_THROW(value_store::deserialize_into(target, "[1,2,3]"));
    EXPECT_ANY_THROW(
        value_store::deserialize_into(target, "{\"k\":{\"type\":4}}"));

    EXPECT_EQ(target.size(), 1u);
    EXPECT_TRUE(target.contains("existing"));
}

TEST(ValueStoreJsonDeserializeTest, EmptyObjectYieldsEmptyStore) {
    value_store target;
    target.add("stale", value("stale", int32_t(1)));

    value_store::deserialize_into(target, "{}");

    EXPECT_TRUE(target.empty());
}

// =============================================================================
// message_container JSON round-trip
// =============================================================================

TEST(MessageContainerJsonTest, RoundTripPreservesHeader) {
    message_container source;
    source.set_source("src_id", "src_sub");
    source.set_target("tgt_id", "tgt_sub");
    source.set_message_type("request");
    source.set_version("1.0");

    auto json = source.serialize();
    auto restored = message_container::deserialize(json);
    ASSERT_NE(restored, nullptr);

    EXPECT_EQ(restored->source_id(), "src_id");
    EXPECT_EQ(restored->source_sub_id(), "src_sub");
    EXPECT_EQ(restored->target_id(), "tgt_id");
    EXPECT_EQ(restored->target_sub_id(), "tgt_sub");
    EXPECT_EQ(restored->message_type(), "request");
    EXPECT_EQ(restored->version(), "1.0");
}

TEST(MessageContainerJsonTest, RoundTripPreservesPayload) {
    message_container source;
    source.set_message_type("data");
    source.payload().add("integer", value("integer", int32_t(42)));
    source.payload().add("text", value("text", std::string("hello world")));
    source.payload().add("flag", value("flag", true));
    source.payload().add("decimal", value("decimal", double(3.14)));

    auto json = source.serialize();
    auto restored = message_container::deserialize(json);
    ASSERT_NE(restored, nullptr);

    EXPECT_EQ(restored->payload().size(), 4u);

    auto int_val = restored->payload().get("integer");
    ASSERT_TRUE(int_val.has_value());
    ASSERT_TRUE(int_val->get<int32_t>().has_value());
    EXPECT_EQ(*int_val->get<int32_t>(), 42);

    auto str_val = restored->payload().get("text");
    ASSERT_TRUE(str_val.has_value());
    ASSERT_TRUE(str_val->get<std::string>().has_value());
    EXPECT_EQ(*str_val->get<std::string>(), "hello world");

    auto bool_val = restored->payload().get("flag");
    ASSERT_TRUE(bool_val.has_value());
    ASSERT_TRUE(bool_val->get<bool>().has_value());
    EXPECT_EQ(*bool_val->get<bool>(), true);

    auto dbl_val = restored->payload().get("decimal");
    ASSERT_TRUE(dbl_val.has_value());
    ASSERT_TRUE(dbl_val->get<double>().has_value());
    EXPECT_DOUBLE_EQ(*dbl_val->get<double>(), 3.14);
}

TEST(MessageContainerJsonTest, RoundTripEmptyPayload) {
    message_container source;
    source.set_message_type("ping");

    auto json = source.serialize();
    auto restored = message_container::deserialize(json);
    ASSERT_NE(restored, nullptr);

    EXPECT_TRUE(restored->payload().empty());
    EXPECT_EQ(restored->message_type(), "ping");
}
