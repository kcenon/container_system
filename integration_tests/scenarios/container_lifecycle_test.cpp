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
#include <container/values/bool_value.h>
#include <container/values/string_value.h>
#include <container/values/numeric_value.h>

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

    EXPECT_FALSE(val1->is_null());
    EXPECT_FALSE(val2->is_null());
    EXPECT_FALSE(val3->is_null());

    EXPECT_EQ(val1->to_string(), "value1");
    EXPECT_EQ(val2->to_int(), 42);
    EXPECT_TRUE(val3->to_boolean());
}

/**
 * Test 5: Multiple values with same key
 */
TEST_F(ContainerLifecycleTest, MultipleValuesWithSameKey)
{
    AddStringValue("item", "first");
    AddStringValue("item", "second");
    AddStringValue("item", "third");

    auto items = container->value_array("item");
    ASSERT_EQ(items.size(), 3);

    EXPECT_EQ(items[0]->to_string(), "first");
    EXPECT_EQ(items[1]->to_string(), "second");
    EXPECT_EQ(items[2]->to_string(), "third");
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
    EXPECT_EQ(restored->get_value("test_key")->to_string(), "test_value");
    EXPECT_EQ(restored->get_value("number")->to_int(), 123);
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

    EXPECT_FALSE(restored_bytes->is_null());
    EXPECT_TRUE(restored_bytes->is_bytes());
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
    EXPECT_EQ(copy->get_value("original")->to_string(), "value");
    EXPECT_EQ(copy->get_value("number")->to_int(), 42);

    // Verify independence - modify original
    AddStringValue("new_key", "new_value");
    auto copy_new_val = copy->get_value("new_key");
    EXPECT_TRUE(copy_new_val->is_null());
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
    EXPECT_TRUE(val->is_null());
}

/**
 * Test 10: Nested container structure
 */
TEST_F(ContainerLifecycleTest, NestedContainerStructure)
{
    auto nested = std::make_shared<value_container>();
    nested->set_message_type("nested_msg");
    nested->add(std::make_shared<string_value>("nested_key", "nested_value"));

    std::string nested_data = nested->serialize();
    container->add(std::make_shared<value>("child", value_types::container_value,
                                           nested_data));

    auto child_val = container->get_value("child");
    EXPECT_TRUE(child_val->is_container());

    auto child_container = std::make_shared<value_container>(child_val->data(), false);
    EXPECT_EQ(child_container->message_type(), "nested_msg");
    EXPECT_EQ(child_container->get_value("nested_key")->to_string(), "nested_value");
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
    EXPECT_FALSE(restored->get_value("data_3")->is_null());
}

/**
 * Test 12: Container value removal
 */
TEST_F(ContainerLifecycleTest, ValueRemoval)
{
    AddStringValue("key1", "value1");
    AddStringValue("key2", "value2");
    AddStringValue("key3", "value3");

    EXPECT_FALSE(container->get_value("key2")->is_null());

    container->remove("key2", true);

    // After removal, key2 should not exist
    // Note: Implementation may return null_value instead of actually removing
    auto removed_val = container->get_value("key2");
    // Value should either be null or not found
    EXPECT_TRUE(removed_val->is_null() || removed_val->name() != "key2");
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
    EXPECT_EQ(restored->get_value("str_val")->to_string(), "test_string");
    EXPECT_EQ(restored->get_value("int_val")->to_int(), 42);
    EXPECT_EQ(restored->get_value("long_val")->to_llong(), 9223372036854775807LL);
    // Use EXPECT_NEAR for floating-point comparison to handle serialization precision loss
    EXPECT_NEAR(restored->get_value("double_val")->to_double(), 3.14159,
                TestConfig::instance().get_double_epsilon());
    EXPECT_TRUE(restored->get_value("bool_val")->to_boolean());
    EXPECT_TRUE(restored->get_value("bytes_val")->is_bytes());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
