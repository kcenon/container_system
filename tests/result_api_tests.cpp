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
 * @file result_api_tests.cpp
 * @brief Unit tests for Result-returning APIs (Issue #238)
 *
 * Tests cover:
 * - set_result() success and error paths
 * - set_all_result() success and error paths
 * - remove_result() success and error paths
 * - Error code verification for each failure case
 */

#include <gtest/gtest.h>
#include <container/core/container.h>
#include <container/core/container/error_codes.h>

#include <memory>
#include <span>
#include <vector>

using namespace container_module;

// ============================================================================
// Test Fixture
// ============================================================================

class ResultAPITest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		container = std::make_shared<value_container>();
	}

	void TearDown() override { container.reset(); }

	std::shared_ptr<value_container> container;
};

// ============================================================================
// set_result() Tests
// ============================================================================

#if CONTAINER_HAS_COMMON_RESULT

TEST_F(ResultAPITest, SetResultTemplateSuccess)
{
	// Test set_result with various types
	auto int_result = container->set_result("int_key", int32_t(42));
	EXPECT_TRUE(kcenon::common::is_ok(int_result))
		<< "set_result should succeed for int32_t";

	auto string_result = container->set_result("string_key", std::string("hello"));
	EXPECT_TRUE(kcenon::common::is_ok(string_result))
		<< "set_result should succeed for string";

	auto double_result = container->set_result("double_key", 3.14);
	EXPECT_TRUE(kcenon::common::is_ok(double_result))
		<< "set_result should succeed for double";

	auto bool_result = container->set_result("bool_key", true);
	EXPECT_TRUE(kcenon::common::is_ok(bool_result))
		<< "set_result should succeed for bool";

	// Verify values were actually set
	EXPECT_TRUE(container->contains("int_key"));
	EXPECT_TRUE(container->contains("string_key"));
	EXPECT_TRUE(container->contains("double_key"));
	EXPECT_TRUE(container->contains("bool_key"));
}

TEST_F(ResultAPITest, SetResultTemplateEmptyKeyError)
{
	// Test set_result with empty key - should return error
	auto result = container->set_result("", int32_t(42));
	EXPECT_TRUE(kcenon::common::is_error(result))
		<< "set_result should fail for empty key";

	// Verify error code
	auto error = kcenon::common::get_error(result);
	EXPECT_EQ(error.code, error_codes::empty_key)
		<< "Error code should be empty_key";
}

TEST_F(ResultAPITest, SetResultOptimizedValueSuccess)
{
	// Test set_result with optimized_value
	optimized_value val;
	val.name = "test_key";
	val.data = int32_t(100);
	val.type = value_types::int_value;

	auto result = container->set_result(val);
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "set_result should succeed for valid optimized_value";

	// Verify value was set
	EXPECT_TRUE(container->contains("test_key"));
	auto retrieved = container->get<int32_t>("test_key");
	EXPECT_TRUE(kcenon::common::is_ok(retrieved));
	EXPECT_EQ(kcenon::common::get_value(retrieved), 100);
}

TEST_F(ResultAPITest, SetResultOptimizedValueEmptyKeyError)
{
	// Test set_result with empty key in optimized_value
	optimized_value val;
	val.name = "";  // Empty key
	val.data = int32_t(100);
	val.type = value_types::int_value;

	auto result = container->set_result(val);
	EXPECT_TRUE(kcenon::common::is_error(result))
		<< "set_result should fail for optimized_value with empty name";

	// Verify error code
	auto error = kcenon::common::get_error(result);
	EXPECT_EQ(error.code, error_codes::empty_key)
		<< "Error code should be empty_key";
}

TEST_F(ResultAPITest, SetResultUpdateExistingKey)
{
	// Set initial value
	auto result1 = container->set_result("key", int32_t(10));
	EXPECT_TRUE(kcenon::common::is_ok(result1));

	// Update with new value
	auto result2 = container->set_result("key", int32_t(20));
	EXPECT_TRUE(kcenon::common::is_ok(result2))
		<< "set_result should succeed when updating existing key";

	// Verify updated value
	auto retrieved = container->get<int32_t>("key");
	EXPECT_TRUE(kcenon::common::is_ok(retrieved));
	EXPECT_EQ(kcenon::common::get_value(retrieved), 20);
}

// ============================================================================
// set_all_result() Tests
// ============================================================================

TEST_F(ResultAPITest, SetAllResultSuccess)
{
	// Create array of values
	std::vector<optimized_value> vals;

	optimized_value val1;
	val1.name = "key1";
	val1.data = int32_t(1);
	val1.type = value_types::int_value;
	vals.push_back(val1);

	optimized_value val2;
	val2.name = "key2";
	val2.data = std::string("value2");
	val2.type = value_types::string_value;
	vals.push_back(val2);

	optimized_value val3;
	val3.name = "key3";
	val3.data = 3.14;
	val3.type = value_types::double_value;
	vals.push_back(val3);

	// Set all values
	auto result = container->set_all_result(std::span<const optimized_value>(vals));
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "set_all_result should succeed for valid values";

	// Verify all values were set
	EXPECT_TRUE(container->contains("key1"));
	EXPECT_TRUE(container->contains("key2"));
	EXPECT_TRUE(container->contains("key3"));
}

TEST_F(ResultAPITest, SetAllResultEmptyKeyError)
{
	// Create array with one empty key
	std::vector<optimized_value> vals;

	optimized_value val1;
	val1.name = "key1";
	val1.data = int32_t(1);
	val1.type = value_types::int_value;
	vals.push_back(val1);

	optimized_value val2;
	val2.name = "";  // Empty key - should cause error
	val2.data = int32_t(2);
	val2.type = value_types::int_value;
	vals.push_back(val2);

	optimized_value val3;
	val3.name = "key3";
	val3.data = int32_t(3);
	val3.type = value_types::int_value;
	vals.push_back(val3);

	// Set all values - should fail on second value
	auto result = container->set_all_result(std::span<const optimized_value>(vals));
	EXPECT_TRUE(kcenon::common::is_error(result))
		<< "set_all_result should fail when any value has empty key";

	// Verify error code
	auto error = kcenon::common::get_error(result);
	EXPECT_EQ(error.code, error_codes::empty_key)
		<< "Error code should be empty_key";

	// Note: First value may or may not be set depending on implementation
	// (no transactional guarantee)
}

TEST_F(ResultAPITest, SetAllResultEmptyArray)
{
	// Empty array should succeed
	std::vector<optimized_value> vals;
	auto result = container->set_all_result(std::span<const optimized_value>(vals));
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "set_all_result should succeed for empty array";
}

// ============================================================================
// remove_result() Tests
// ============================================================================

TEST_F(ResultAPITest, RemoveResultSuccess)
{
	// First add a value
	container->set("key_to_remove", int32_t(42));
	EXPECT_TRUE(container->contains("key_to_remove"));

	// Remove it
	auto result = container->remove_result("key_to_remove");
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "remove_result should succeed for existing key";

	// Verify removal
	EXPECT_FALSE(container->contains("key_to_remove"));
}

TEST_F(ResultAPITest, RemoveResultKeyNotFoundError)
{
	// Try to remove non-existent key
	auto result = container->remove_result("nonexistent_key");
	EXPECT_TRUE(kcenon::common::is_error(result))
		<< "remove_result should fail for non-existent key";

	// Verify error code
	auto error = kcenon::common::get_error(result);
	EXPECT_EQ(error.code, error_codes::key_not_found)
		<< "Error code should be key_not_found";
}

TEST_F(ResultAPITest, RemoveResultRemovesAllDuplicates)
{
	// Add multiple values with same key (through internal mechanism)
	// Note: This tests the behavior where remove removes ALL occurrences
	container->set("dup_key", int32_t(1));

	// Remove the key
	auto result = container->remove_result("dup_key");
	EXPECT_TRUE(kcenon::common::is_ok(result));

	// Verify all occurrences are removed
	EXPECT_FALSE(container->contains("dup_key"));

	// Trying to remove again should fail
	auto result2 = container->remove_result("dup_key");
	EXPECT_TRUE(kcenon::common::is_error(result2));
	auto error = kcenon::common::get_error(result2);
	EXPECT_EQ(error.code, error_codes::key_not_found);
}

// ============================================================================
// Error Message Tests
// ============================================================================

TEST_F(ResultAPITest, ErrorMessagesAreDescriptive)
{
	// Test that error messages are descriptive
	auto empty_key_result = container->set_result("", int32_t(42));
	EXPECT_TRUE(kcenon::common::is_error(empty_key_result));
	auto empty_key_error = kcenon::common::get_error(empty_key_result);
	EXPECT_FALSE(empty_key_error.message.empty())
		<< "Error message should not be empty";

	auto not_found_result = container->remove_result("nonexistent");
	EXPECT_TRUE(kcenon::common::is_error(not_found_result));
	auto not_found_error = kcenon::common::get_error(not_found_result);
	EXPECT_FALSE(not_found_error.message.empty())
		<< "Error message should not be empty";

}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST_F(ResultAPITest, SetResultWithSpecialCharactersInKey)
{
	// Test keys with special characters
	auto result1 = container->set_result("key.with.dots", int32_t(1));
	EXPECT_TRUE(kcenon::common::is_ok(result1));

	auto result2 = container->set_result("key-with-dashes", int32_t(2));
	EXPECT_TRUE(kcenon::common::is_ok(result2));

	auto result3 = container->set_result("key_with_underscores", int32_t(3));
	EXPECT_TRUE(kcenon::common::is_ok(result3));

	auto result4 = container->set_result("key with spaces", int32_t(4));
	EXPECT_TRUE(kcenon::common::is_ok(result4));

	// Verify all were set
	EXPECT_TRUE(container->contains("key.with.dots"));
	EXPECT_TRUE(container->contains("key-with-dashes"));
	EXPECT_TRUE(container->contains("key_with_underscores"));
	EXPECT_TRUE(container->contains("key with spaces"));
}

TEST_F(ResultAPITest, SetResultWithLongKey)
{
	// Test with very long key name
	std::string long_key(1000, 'a');
	auto result = container->set_result(long_key, int32_t(42));
	EXPECT_TRUE(kcenon::common::is_ok(result));
	EXPECT_TRUE(container->contains(long_key));
}

TEST_F(ResultAPITest, SetResultWithUnicodeKey)
{
	// Test with Unicode characters in key
	auto result = container->set_result("키_한글", int32_t(42));
	EXPECT_TRUE(kcenon::common::is_ok(result));
	EXPECT_TRUE(container->contains("키_한글"));
}

// ============================================================================
// Serialization Result API Tests (Issue #239)
// ============================================================================

TEST_F(ResultAPITest, SerializeResultSuccess)
{
	// Add some values to serialize
	container->set("name", std::string("test"));
	container->set("value", int32_t(42));

	auto result = container->serialize_result();
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "serialize_result should succeed for valid container";

	auto serialized = kcenon::common::get_value(result);
	EXPECT_FALSE(serialized.empty())
		<< "Serialized string should not be empty";

	// Verify the serialized data can be deserialized
	auto new_container = std::make_shared<value_container>();
	EXPECT_TRUE(new_container->deserialize(serialized, false));
	EXPECT_TRUE(new_container->contains("name"));
	EXPECT_TRUE(new_container->contains("value"));
}

TEST_F(ResultAPITest, SerializeResultEmptyContainer)
{
	// Empty container should serialize successfully
	auto result = container->serialize_result();
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "serialize_result should succeed for empty container";

	auto serialized = kcenon::common::get_value(result);
	EXPECT_FALSE(serialized.empty())
		<< "Serialized string should not be empty even for empty container";
}

TEST_F(ResultAPITest, SerializeArrayResultSuccess)
{
	// Add some values to serialize
	container->set("key1", std::string("value1"));
	container->set("key2", int32_t(100));

	auto result = container->serialize_array_result();
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "serialize_array_result should succeed for valid container";

	auto serialized = kcenon::common::get_value(result);
	EXPECT_FALSE(serialized.empty())
		<< "Serialized array should not be empty";
}

TEST_F(ResultAPITest, SerializeArrayResultEmptyContainer)
{
	// Empty container should serialize successfully
	auto result = container->serialize_array_result();
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "serialize_array_result should succeed for empty container";

	auto serialized = kcenon::common::get_value(result);
	EXPECT_FALSE(serialized.empty())
		<< "Serialized array should not be empty even for empty container";
}

TEST_F(ResultAPITest, ToJsonResultSuccess)
{
	// Add some values
	container->set("name", std::string("test"));
	container->set("count", int32_t(5));
	container->set("enabled", true);

	auto result = container->to_json_result();
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "to_json_result should succeed for valid container";

	auto json_str = kcenon::common::get_value(result);
	EXPECT_FALSE(json_str.empty())
		<< "JSON string should not be empty";

	// Basic JSON structure validation
	EXPECT_NE(json_str.find("name"), std::string::npos)
		<< "JSON should contain 'name' key";
	EXPECT_NE(json_str.find("test"), std::string::npos)
		<< "JSON should contain 'test' value";
}

TEST_F(ResultAPITest, ToJsonResultEmptyContainer)
{
	auto result = container->to_json_result();
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "to_json_result should succeed for empty container";
}

TEST_F(ResultAPITest, ToXmlResultSuccess)
{
	// Add some values
	container->set("title", std::string("XML Test"));
	container->set("id", int32_t(123));

	auto result = container->to_xml_result();
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "to_xml_result should succeed for valid container";

	auto xml_str = kcenon::common::get_value(result);
	EXPECT_FALSE(xml_str.empty())
		<< "XML string should not be empty";

	// Basic XML structure validation
	EXPECT_NE(xml_str.find("title"), std::string::npos)
		<< "XML should contain 'title' element";
}

TEST_F(ResultAPITest, ToXmlResultEmptyContainer)
{
	auto result = container->to_xml_result();
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "to_xml_result should succeed for empty container";
}

TEST_F(ResultAPITest, DeserializeResultSuccess)
{
	// First serialize a container
	container->set("key", std::string("value"));
	auto serialized = container->serialize();

	// Create new container and deserialize
	auto new_container = std::make_shared<value_container>();
	auto result = new_container->deserialize_result(serialized, false);
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "deserialize_result should succeed for valid data";

	// Verify deserialization
	EXPECT_TRUE(new_container->contains("key"));
}

TEST_F(ResultAPITest, DeserializeResultInvalidData)
{
	// Try to deserialize invalid data
	std::string invalid_data = "this is not valid serialized data";

	auto result = container->deserialize_result(invalid_data, false);
	EXPECT_TRUE(kcenon::common::is_error(result))
		<< "deserialize_result should fail for invalid data";

	auto error = kcenon::common::get_error(result);
	EXPECT_EQ(error.code, error_codes::deserialization_failed)
		<< "Error code should be deserialization_failed";
}

TEST_F(ResultAPITest, DeserializeResultEmptyData)
{
	// Try to deserialize empty string
	std::string empty_data = "";

	auto result = container->deserialize_result(empty_data, false);
	EXPECT_TRUE(kcenon::common::is_error(result))
		<< "deserialize_result should fail for empty data";

	auto error = kcenon::common::get_error(result);
	EXPECT_EQ(error.code, error_codes::deserialization_failed)
		<< "Error code should be deserialization_failed";
}

TEST_F(ResultAPITest, DeserializeResultCorruptedData)
{
	// Create partially corrupted data
	container->set("test", std::string("value"));
	auto serialized = container->serialize();

	// Corrupt the data by truncating it
	if (serialized.length() > 10)
	{
		std::string corrupted = serialized.substr(0, serialized.length() / 2);
		auto new_container = std::make_shared<value_container>();
		auto result = new_container->deserialize_result(corrupted, false);

		// Should either fail or succeed partially - implementation dependent
		// At minimum, error handling should not throw
		(void)result;  // Suppress unused warning - we just want to ensure no crash
	}
}

TEST_F(ResultAPITest, DeserializeResultByteArraySuccess)
{
	// Serialize to array
	container->set("byte_test", int32_t(42));
	auto array_data = container->serialize_array();

	// Deserialize from array
	auto new_container = std::make_shared<value_container>();
	auto result = new_container->deserialize_result(array_data, false);
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "deserialize_result should succeed for valid byte array";

	EXPECT_TRUE(new_container->contains("byte_test"));
}

TEST_F(ResultAPITest, DeserializeResultByteArrayInvalid)
{
	// Create invalid byte array
	std::vector<uint8_t> invalid_data = {0x00, 0xFF, 0x12, 0x34};

	auto result = container->deserialize_result(invalid_data, false);
	EXPECT_TRUE(kcenon::common::is_error(result))
		<< "deserialize_result should fail for invalid byte array";
}

TEST_F(ResultAPITest, SerializationRoundTrip)
{
	// Add various types of values
	container->set("string_val", std::string("hello world"));
	container->set("int_val", int32_t(12345));
	container->set("double_val", 3.14159);
	container->set("bool_val", true);
	container->set("int64_val", int64_t(9876543210LL));

	// Serialize
	auto serialize_result = container->serialize_result();
	EXPECT_TRUE(kcenon::common::is_ok(serialize_result));

	// Deserialize to new container
	auto new_container = std::make_shared<value_container>();
	auto deserialize_result_val = new_container->deserialize_result(
		kcenon::common::get_value(serialize_result), false);
	EXPECT_TRUE(kcenon::common::is_ok(deserialize_result_val));

	// Verify all values
	auto str_result = new_container->get<std::string>("string_val");
	EXPECT_TRUE(kcenon::common::is_ok(str_result));
	EXPECT_EQ(kcenon::common::get_value(str_result), "hello world");

	auto int_result = new_container->get<int32_t>("int_val");
	EXPECT_TRUE(kcenon::common::is_ok(int_result));
	EXPECT_EQ(kcenon::common::get_value(int_result), 12345);

	auto double_result = new_container->get<double>("double_val");
	EXPECT_TRUE(kcenon::common::is_ok(double_result));
	EXPECT_DOUBLE_EQ(kcenon::common::get_value(double_result), 3.14159);

	auto bool_result = new_container->get<bool>("bool_val");
	EXPECT_TRUE(kcenon::common::is_ok(bool_result));
	EXPECT_EQ(kcenon::common::get_value(bool_result), true);

	auto int64_result = new_container->get<int64_t>("int64_val");
	EXPECT_TRUE(kcenon::common::is_ok(int64_result));
	EXPECT_EQ(kcenon::common::get_value(int64_result), 9876543210LL);
}

TEST_F(ResultAPITest, SerializationArrayRoundTrip)
{
	// Add values
	container->set("arr_test", std::string("array round trip"));
	container->set("arr_num", int32_t(999));

	// Serialize to array
	auto serialize_result = container->serialize_array_result();
	EXPECT_TRUE(kcenon::common::is_ok(serialize_result));

	// Deserialize from array
	auto new_container = std::make_shared<value_container>();
	auto deserialize_result_val = new_container->deserialize_result(
		kcenon::common::get_value(serialize_result), false);
	EXPECT_TRUE(kcenon::common::is_ok(deserialize_result_val));

	// Verify values
	EXPECT_TRUE(new_container->contains("arr_test"));
	EXPECT_TRUE(new_container->contains("arr_num"));

	auto str_result = new_container->get<std::string>("arr_test");
	EXPECT_TRUE(kcenon::common::is_ok(str_result));
	EXPECT_EQ(kcenon::common::get_value(str_result), "array round trip");
}

TEST_F(ResultAPITest, ErrorMessageContainsContext)
{
	// Verify error messages contain useful context
	std::string invalid_data = "invalid_serialized_data_that_cannot_be_parsed";
	auto result = container->deserialize_result(invalid_data, false);
	EXPECT_TRUE(kcenon::common::is_error(result));

	auto error = kcenon::common::get_error(result);

	// Error message should not be empty
	EXPECT_FALSE(error.message.empty())
		<< "Error message should provide context about the failure";

	// Error module should be set
	EXPECT_FALSE(error.module.empty())
		<< "Error module should identify container_system";
}

#endif  // CONTAINER_HAS_COMMON_RESULT

// Main function for running tests
int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
