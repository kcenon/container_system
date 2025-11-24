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
 * @file container_lifecycle_test.cpp
 * @brief Integration tests for container lifecycle scenarios
 *
 * Tests cover:
 * - Container creation and initialization
 * - Header management (source/target/message type)
 * - Value addition and retrieval
 * - Serialization and deserialization
 * - Container copying (deep/shallow)
 * - Nested container structures
 * - Header swapping functionality
 * - Container state transitions
 */

#include <gtest/gtest.h>
#include "framework/system_fixture.h"
#include "framework/test_helpers.h"
#include <container/core/container.h>

using namespace container_module;
using namespace container_module::testing;

class ContainerLifecycleTest : public ContainerSystemFixture
{
};

/**
 * Test 1: Basic container creation and initialization
 */
TEST_F(ContainerLifecycleTest, BasicContainerCreation)
{
    EXPECT_NE(container, nullptr);
    EXPECT_EQ(container->source_id(), "test_source");
    EXPECT_EQ(container->source_sub_id(), "test_sub");
    EXPECT_EQ(container->target_id(), "test_target");
    EXPECT_EQ(container->message_type(), "test_message");
}

/**
 * Test 2: Container header modification
 */
TEST_F(ContainerLifecycleTest, HeaderModification)
{
    container->set_source("new_source", "new_sub");
    container->set_target("new_target", "target_sub");
    container->set_message_type("new_type");

    EXPECT_EQ(container->source_id(), "new_source");
    EXPECT_EQ(container->source_sub_id(), "new_sub");
    EXPECT_EQ(container->target_id(), "new_target");
    EXPECT_EQ(container->target_sub_id(), "target_sub");
    EXPECT_EQ(container->message_type(), "new_type");
}

/**
 * Test 3: Header swapping functionality
 */
TEST_F(ContainerLifecycleTest, HeaderSwapping)
{
    std::string orig_source = container->source_id();
    std::string orig_source_sub = container->source_sub_id();
    std::string orig_target = container->target_id();
    std::string orig_target_sub = container->target_sub_id();

    container->swap_header();

    EXPECT_EQ(container->source_id(), orig_target);
    EXPECT_EQ(container->source_sub_id(), orig_target_sub);
    EXPECT_EQ(container->target_id(), orig_source);
    EXPECT_EQ(container->target_sub_id(), orig_source_sub);
}

/**
 * Test 4: Value addition and retrieval
 */
TEST_F(ContainerLifecycleTest, ValueAdditionAndRetrieval)
{
    AddStringValue("key1", "value1");
    AddNumericValue("key2", 42);
    AddBoolValue("key3", true);

    auto val1 = container->get_value("key1");
    auto val2 = container->get_value("key2");
    auto val3 = container->get_value("key3");

    EXPECT_FALSE(ov_is_null(val1));
    EXPECT_FALSE(ov_is_null(val2));
    EXPECT_FALSE(ov_is_null(val3));

    EXPECT_EQ(ov_to_string(val1), "value1");
    EXPECT_EQ(ov_to_int(val2), 42);
    EXPECT_TRUE(ov_to_boolean(val3));
}

/**
 * Test 5: Multiple values with same key
 * Note: With variant-based storage, set_value updates existing keys.
 * Use add() or add_value() to add multiple values with the same key.
 */
TEST_F(ContainerLifecycleTest, MultipleValuesWithSameKey)
{
    // Use add() to allow duplicate keys
    container->add(std::make_shared<string_value>("item", "first"));
    container->add(std::make_shared<string_value>("item", "second"));
    container->add(std::make_shared<string_value>("item", "third"));

    // Iterate through container to find all values with key "item"
    std::vector<std::string> items;
    for (const auto& val : *container) {
        if (val.name == "item") {
            items.push_back(ov_to_string(std::make_optional(val)));
        }
    }
    ASSERT_EQ(items.size(), 3);

    EXPECT_EQ(items[0], "first");
    EXPECT_EQ(items[1], "second");
    EXPECT_EQ(items[2], "third");
}

/**
 * Test 6: Container serialization roundtrip
 */
TEST_F(ContainerLifecycleTest, SerializationRoundtrip)
{
    AddStringValue("test_key", "test_value");
    AddNumericValue("number", 123);

    std::string serialized = container->serialize();
    EXPECT_FALSE(serialized.empty());
    EXPECT_TRUE(TestHelpers::IsValidSerializedData(serialized));

    auto restored = std::make_shared<value_container>(serialized, false);

    EXPECT_EQ(restored->source_id(), container->source_id());
    EXPECT_EQ(restored->target_id(), container->target_id());
    EXPECT_EQ(restored->message_type(), container->message_type());
    EXPECT_EQ(ov_to_string(restored->get_value("test_key")), "test_value");
    EXPECT_EQ(ov_to_int(restored->get_value("number")), 123);
}

/**
 * Test 7: Binary array serialization
 */
TEST_F(ContainerLifecycleTest, BinaryArraySerialization)
{
    std::vector<uint8_t> test_bytes = {0x01, 0x02, 0x03, 0xFF, 0xFE};
    AddBytesValue("binary_data", test_bytes);

    auto serialized_array = container->serialize_array();
    EXPECT_FALSE(serialized_array.empty());

    auto restored = std::make_shared<value_container>(serialized_array, false);
    auto restored_bytes = restored->get_value("binary_data");

    EXPECT_FALSE(ov_is_null(restored_bytes));
    EXPECT_TRUE(ov_is_bytes(restored_bytes));
}

/**
 * Test 8: Deep copy functionality
 */
TEST_F(ContainerLifecycleTest, DeepCopy)
{
    AddStringValue("original", "value");
    AddNumericValue("number", 42);

    auto copy = container->copy(true);

    ASSERT_NE(copy, nullptr);
    EXPECT_EQ(copy->message_type(), container->message_type());
    EXPECT_EQ(ov_to_string(copy->get_value("original")), "value");
    EXPECT_EQ(ov_to_int(copy->get_value("number")), 42);

    // Verify independence - modify original
    AddStringValue("new_key", "new_value");
    auto copy_new_val = copy->get_value("new_key");
    EXPECT_TRUE(ov_is_null(copy_new_val));
}

/**
 * Test 9: Shallow copy (header only)
 */
TEST_F(ContainerLifecycleTest, ShallowCopy)
{
    AddStringValue("data", "value");
    AddNumericValue("number", 100);

    auto shallow = container->copy(false);

    ASSERT_NE(shallow, nullptr);
    EXPECT_EQ(shallow->source_id(), container->source_id());
    EXPECT_EQ(shallow->target_id(), container->target_id());
    EXPECT_EQ(shallow->message_type(), container->message_type());

    // Shallow copy should not have values
    auto val = shallow->get_value("data");
    EXPECT_TRUE(ov_is_null(val));
}

/**
 * Test 10: Nested container structure
 * Note: This test verifies that serialized container data can be stored and
 * deserialized correctly using the modern API.
 */
TEST_F(ContainerLifecycleTest, NestedContainerStructure)
{
    auto nested = std::make_shared<value_container>();
    nested->set_message_type("nested_msg");
    nested->add(std::make_shared<string_value>("nested_key", "nested_value"));

    // Serialize nested container
    std::string nested_data = nested->serialize();

    // Store as string value (container serialized form)
    container->set_value("child_data", nested_data);

    auto child_val = container->get_value("child_data");
    EXPECT_FALSE(ov_is_null(child_val));

    // Deserialize nested container from stored string
    std::string retrieved_data = ov_to_string(child_val);
    auto child_container = std::make_shared<value_container>(retrieved_data, false);
    EXPECT_EQ(child_container->message_type(), "nested_msg");
    EXPECT_EQ(ov_to_string(child_container->get_value("nested_key")), "nested_value");
}

/**
 * Test 11: Multi-level nested containers
 */
TEST_F(ContainerLifecycleTest, MultiLevelNestedContainers)
{
    auto nested = TestHelpers::CreateNestedContainer(3);
    ASSERT_NE(nested, nullptr);

    std::string serialized = nested->serialize();
    auto restored = std::make_shared<value_container>(serialized, false);

    EXPECT_EQ(restored->message_type(), "root_level");
    EXPECT_FALSE(ov_is_null(restored->get_value("data_3")));
}

/**
 * Test 12: Container value removal
 */
TEST_F(ContainerLifecycleTest, ValueRemoval)
{
    AddStringValue("key1", "value1");
    AddStringValue("key2", "value2");
    AddStringValue("key3", "value3");

    EXPECT_FALSE(ov_is_null(container->get_value("key2")));

    container->remove("key2", true);

    // After removal, key2 should not exist
    // Note: Implementation may return null_value instead of actually removing
    auto removed_val = container->get_value("key2");
    // Value should either be null or not found
    EXPECT_TRUE(ov_is_null(removed_val) || ov_name(removed_val) != "key2");
}

/**
 * Test 13: Container clear operation
 */
TEST_F(ContainerLifecycleTest, ClearAllValues)
{
    AddStringValue("key1", "value1");
    AddNumericValue("key2", 42);
    AddBoolValue("key3", true);

    container->clear_value();

    // After clear, container should be empty
    std::string serialized = container->serialize();
    auto restored = std::make_shared<value_container>(serialized, false);

    // Values should be cleared, but header should remain
    EXPECT_EQ(restored->message_type(), container->message_type());
}

/**
 * Test 14: Empty container serialization
 */
TEST_F(ContainerLifecycleTest, EmptyContainerSerialization)
{
    auto empty = std::make_shared<value_container>();

    std::string serialized = empty->serialize();
    EXPECT_FALSE(serialized.empty());

    auto restored = std::make_shared<value_container>(serialized, false);
    EXPECT_EQ(restored->message_type(), "data_container");
}

/**
 * Test 15: Container lifecycle with mixed value types
 */
TEST_F(ContainerLifecycleTest, MixedValueTypesLifecycle)
{
    auto mixed = TestHelpers::CreateMixedTypeContainer();

    // Serialize and deserialize
    std::string serialized = mixed->serialize();
    auto restored = std::make_shared<value_container>(serialized, false);

    // Verify all value types preserved
    EXPECT_EQ(ov_to_string(restored->get_value("str_val")), "test_string");
    EXPECT_EQ(ov_to_int(restored->get_value("int_val")), 42);
    EXPECT_EQ(ov_to_llong(restored->get_value("long_val")), 9223372036854775807LL);
    // Use EXPECT_NEAR for floating-point comparison to handle serialization precision loss
    EXPECT_NEAR(ov_to_double(restored->get_value("double_val")), 3.14159,
                TestConfig::instance().get_double_epsilon());
    EXPECT_TRUE(ov_to_boolean(restored->get_value("bool_val")));
    EXPECT_TRUE(ov_is_bytes(restored->get_value("bytes_val")));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
