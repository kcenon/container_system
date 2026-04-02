// BSD 3-Clause License
// Copyright (c) 2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file serializer_factory_tests.cpp
 * @brief Unit tests for serializer_factory and serializer_strategy
 * @since 4.0.0
 */

#include <gtest/gtest.h>

#include <core/serializers/serializer_factory.h>
#include <core/serializers/serializer_strategy.h>
#include <container.h>

using namespace kcenon::container;

// =============================================================================
// serialization_format enum tests
// =============================================================================

TEST(SerializationFormatTest, EnumValuesAreDifferent) {
    EXPECT_NE(static_cast<int>(serialization_format::binary),
              static_cast<int>(serialization_format::json));
    EXPECT_NE(static_cast<int>(serialization_format::json),
              static_cast<int>(serialization_format::xml));
    EXPECT_NE(static_cast<int>(serialization_format::xml),
              static_cast<int>(serialization_format::msgpack));
    EXPECT_NE(static_cast<int>(serialization_format::msgpack),
              static_cast<int>(serialization_format::auto_detect));
    EXPECT_NE(static_cast<int>(serialization_format::auto_detect),
              static_cast<int>(serialization_format::unknown));
}

// =============================================================================
// serializer_factory::create tests
// =============================================================================

TEST(SerializerFactoryTest, CreateBinarySerializer) {
    auto serializer = serializer_factory::create(serialization_format::binary);
    ASSERT_NE(serializer, nullptr);
    EXPECT_EQ(serializer->format(), serialization_format::binary);
}

TEST(SerializerFactoryTest, CreateJsonSerializer) {
    auto serializer = serializer_factory::create(serialization_format::json);
    ASSERT_NE(serializer, nullptr);
    EXPECT_EQ(serializer->format(), serialization_format::json);
}

TEST(SerializerFactoryTest, CreateAutoDetectReturnsNull) {
    auto serializer = serializer_factory::create(serialization_format::auto_detect);
    EXPECT_EQ(serializer, nullptr);
}

TEST(SerializerFactoryTest, CreateUnknownReturnsNull) {
    auto serializer = serializer_factory::create(serialization_format::unknown);
    EXPECT_EQ(serializer, nullptr);
}

// =============================================================================
// serializer_factory::is_supported tests
// =============================================================================

TEST(SerializerFactoryTest, BinaryIsSupported) {
    EXPECT_TRUE(serializer_factory::is_supported(serialization_format::binary));
}

TEST(SerializerFactoryTest, JsonIsSupported) {
    EXPECT_TRUE(serializer_factory::is_supported(serialization_format::json));
}

TEST(SerializerFactoryTest, AutoDetectNotSupported) {
    EXPECT_FALSE(serializer_factory::is_supported(serialization_format::auto_detect));
}

TEST(SerializerFactoryTest, UnknownNotSupported) {
    EXPECT_FALSE(serializer_factory::is_supported(serialization_format::unknown));
}

// =============================================================================
// serializer_strategy name() tests
// =============================================================================

TEST(SerializerFactoryTest, BinarySerializerName) {
    auto serializer = serializer_factory::create(serialization_format::binary);
    ASSERT_NE(serializer, nullptr);
    EXPECT_FALSE(serializer->name().empty());
}

TEST(SerializerFactoryTest, JsonSerializerName) {
    auto serializer = serializer_factory::create(serialization_format::json);
    ASSERT_NE(serializer, nullptr);
    EXPECT_FALSE(serializer->name().empty());
}

// =============================================================================
// Serialization round-trip tests (if Result API available)
// =============================================================================

#if CONTAINER_HAS_RESULT

TEST(SerializerFactoryTest, BinarySerializeEmptyContainer) {
    auto serializer = serializer_factory::create(serialization_format::binary);
    ASSERT_NE(serializer, nullptr);

    value_container container;
    auto result = serializer->serialize(container);
    EXPECT_TRUE(result.is_ok());
    EXPECT_FALSE(result.value().empty());
}

TEST(SerializerFactoryTest, JsonSerializeEmptyContainer) {
    auto serializer = serializer_factory::create(serialization_format::json);
    ASSERT_NE(serializer, nullptr);

    value_container container;
    auto result = serializer->serialize(container);
    EXPECT_TRUE(result.is_ok());
    EXPECT_FALSE(result.value().empty());
}

TEST(SerializerFactoryTest, BinarySerializeWithValues) {
    auto serializer = serializer_factory::create(serialization_format::binary);
    ASSERT_NE(serializer, nullptr);

    value_container container;
    container.set("name", std::string("test"));
    container.set("count", static_cast<int32_t>(42));

    auto result = serializer->serialize(container);
    EXPECT_TRUE(result.is_ok());
    EXPECT_GT(result.value().size(), 0u);
}

TEST(SerializerFactoryTest, JsonSerializeWithValues) {
    auto serializer = serializer_factory::create(serialization_format::json);
    ASSERT_NE(serializer, nullptr);

    value_container container;
    container.set("name", std::string("test"));
    container.set("count", static_cast<int32_t>(42));

    auto result = serializer->serialize(container);
    EXPECT_TRUE(result.is_ok());
    EXPECT_GT(result.value().size(), 0u);
}

#endif // CONTAINER_HAS_RESULT

// =============================================================================
// Factory consistency tests
// =============================================================================

TEST(SerializerFactoryTest, MultipleCreatesReturnIndependentInstances) {
    auto s1 = serializer_factory::create(serialization_format::binary);
    auto s2 = serializer_factory::create(serialization_format::binary);
    ASSERT_NE(s1, nullptr);
    ASSERT_NE(s2, nullptr);
    EXPECT_NE(s1.get(), s2.get());
    EXPECT_EQ(s1->format(), s2->format());
}

TEST(SerializerFactoryTest, SupportedFormatsProduceNonNullSerializers) {
    const serialization_format supported_formats[] = {
        serialization_format::binary,
        serialization_format::json,
    };

    for (auto fmt : supported_formats) {
        EXPECT_TRUE(serializer_factory::is_supported(fmt));
        auto serializer = serializer_factory::create(fmt);
        EXPECT_NE(serializer, nullptr) << "Format should produce non-null serializer";
    }
}

TEST(SerializerFactoryTest, UnsupportedFormatsReturnNull) {
    const serialization_format unsupported_formats[] = {
        serialization_format::auto_detect,
        serialization_format::unknown,
    };

    for (auto fmt : unsupported_formats) {
        EXPECT_FALSE(serializer_factory::is_supported(fmt));
        auto serializer = serializer_factory::create(fmt);
        EXPECT_EQ(serializer, nullptr) << "Unsupported format should return nullptr";
    }
}
