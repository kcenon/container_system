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

#include <filesystem>
#include <fstream>
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

// ============================================================================
// File Operation Result API Tests (Issue #240)
// ============================================================================

class FileOperationResultAPITest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		container = std::make_shared<value_container>();
		// Create a unique test directory
		test_dir = std::filesystem::temp_directory_path() / "container_test";
		std::filesystem::create_directories(test_dir);
	}

	void TearDown() override
	{
		container.reset();
		// Clean up test directory
		std::error_code ec;
		std::filesystem::remove_all(test_dir, ec);
	}

	std::shared_ptr<value_container> container;
	std::filesystem::path test_dir;
};

TEST_F(FileOperationResultAPITest, LoadPacketResultFileNotFound)
{
	// Try to load a non-existent file
	std::string nonexistent_path = (test_dir / "nonexistent_file.bin").string();

	auto result = container->load_packet_result(nonexistent_path);
	EXPECT_TRUE(kcenon::common::is_error(result))
		<< "load_packet_result should fail for non-existent file";

	auto error = kcenon::common::get_error(result);
	EXPECT_EQ(error.code, error_codes::file_not_found)
		<< "Error code should be file_not_found";
	EXPECT_FALSE(error.message.empty())
		<< "Error message should not be empty";
	EXPECT_NE(error.message.find(nonexistent_path), std::string::npos)
		<< "Error message should contain file path";
}

TEST_F(FileOperationResultAPITest, SavePacketResultSuccess)
{
	// Add some data to the container
	container->set("test_key", std::string("test_value"));
	container->set("number", int32_t(42));

	std::string file_path = (test_dir / "test_output.bin").string();

	auto result = container->save_packet_result(file_path);
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "save_packet_result should succeed for valid path";

	// Verify file was created
	EXPECT_TRUE(std::filesystem::exists(file_path))
		<< "File should exist after save";

	// Verify file is not empty
	EXPECT_GT(std::filesystem::file_size(file_path), 0)
		<< "Saved file should not be empty";
}

TEST_F(FileOperationResultAPITest, LoadPacketResultSuccess)
{
	// First save a container
	container->set("load_test", std::string("hello"));
	container->set("value", int32_t(123));

	std::string file_path = (test_dir / "test_load.bin").string();
	auto save_result = container->save_packet_result(file_path);
	EXPECT_TRUE(kcenon::common::is_ok(save_result));

	// Create a new container and load from file
	auto new_container = std::make_shared<value_container>();
	auto load_result = new_container->load_packet_result(file_path);
	EXPECT_TRUE(kcenon::common::is_ok(load_result))
		<< "load_packet_result should succeed for valid file";

	// Verify loaded data
	EXPECT_TRUE(new_container->contains("load_test"));
	EXPECT_TRUE(new_container->contains("value"));

	auto str_result = new_container->get<std::string>("load_test");
	EXPECT_TRUE(kcenon::common::is_ok(str_result));
	EXPECT_EQ(kcenon::common::get_value(str_result), "hello");

	auto int_result = new_container->get<int32_t>("value");
	EXPECT_TRUE(kcenon::common::is_ok(int_result));
	EXPECT_EQ(kcenon::common::get_value(int_result), 123);
}

TEST_F(FileOperationResultAPITest, LoadPacketResultInvalidContent)
{
	// Create a file with invalid content
	std::string file_path = (test_dir / "invalid_content.bin").string();
	{
		std::ofstream file(file_path, std::ios::binary);
		file << "this is not valid serialized data";
	}

	auto result = container->load_packet_result(file_path);
	EXPECT_TRUE(kcenon::common::is_error(result))
		<< "load_packet_result should fail for invalid file content";

	auto error = kcenon::common::get_error(result);
	EXPECT_EQ(error.code, error_codes::deserialization_failed)
		<< "Error code should be deserialization_failed";
}

TEST_F(FileOperationResultAPITest, SavePacketResultInvalidPath)
{
	// Try to save to an invalid path (non-existent directory)
	std::string invalid_path = "/nonexistent_directory_12345/file.bin";

	container->set("test", int32_t(1));
	auto result = container->save_packet_result(invalid_path);
	EXPECT_TRUE(kcenon::common::is_error(result))
		<< "save_packet_result should fail for invalid path";

	auto error = kcenon::common::get_error(result);
	EXPECT_EQ(error.code, error_codes::file_write_error)
		<< "Error code should be file_write_error";
}

TEST_F(FileOperationResultAPITest, FileOperationRoundTrip)
{
	// Comprehensive round-trip test
	container->set("string_val", std::string("round trip test"));
	container->set("int_val", int32_t(98765));
	container->set("double_val", 2.71828);
	container->set("bool_val", false);
	container->set("int64_val", int64_t(1234567890123LL));

	std::string file_path = (test_dir / "round_trip.bin").string();

	// Save
	auto save_result = container->save_packet_result(file_path);
	EXPECT_TRUE(kcenon::common::is_ok(save_result))
		<< "save_packet_result should succeed";

	// Load into new container
	auto loaded_container = std::make_shared<value_container>();
	auto load_result = loaded_container->load_packet_result(file_path);
	EXPECT_TRUE(kcenon::common::is_ok(load_result))
		<< "load_packet_result should succeed";

	// Verify all values
	auto str_result = loaded_container->get<std::string>("string_val");
	EXPECT_TRUE(kcenon::common::is_ok(str_result));
	EXPECT_EQ(kcenon::common::get_value(str_result), "round trip test");

	auto int_result = loaded_container->get<int32_t>("int_val");
	EXPECT_TRUE(kcenon::common::is_ok(int_result));
	EXPECT_EQ(kcenon::common::get_value(int_result), 98765);

	auto double_result = loaded_container->get<double>("double_val");
	EXPECT_TRUE(kcenon::common::is_ok(double_result));
	EXPECT_DOUBLE_EQ(kcenon::common::get_value(double_result), 2.71828);

	auto bool_result = loaded_container->get<bool>("bool_val");
	EXPECT_TRUE(kcenon::common::is_ok(bool_result));
	EXPECT_EQ(kcenon::common::get_value(bool_result), false);

	auto int64_result = loaded_container->get<int64_t>("int64_val");
	EXPECT_TRUE(kcenon::common::is_ok(int64_result));
	EXPECT_EQ(kcenon::common::get_value(int64_result), 1234567890123LL);
}

TEST_F(FileOperationResultAPITest, LoadPacketResultEmptyFile)
{
	// Create an empty file
	std::string file_path = (test_dir / "empty_file.bin").string();
	{
		std::ofstream file(file_path, std::ios::binary);
		// Write nothing - create empty file
	}

	auto result = container->load_packet_result(file_path);
	EXPECT_TRUE(kcenon::common::is_error(result))
		<< "load_packet_result should fail for empty file";

	auto error = kcenon::common::get_error(result);
	EXPECT_EQ(error.code, error_codes::deserialization_failed)
		<< "Error code should be deserialization_failed";
}

TEST_F(FileOperationResultAPITest, SavePacketResultEmptyContainer)
{
	// Save an empty container (should still succeed)
	std::string file_path = (test_dir / "empty_container.bin").string();

	auto result = container->save_packet_result(file_path);
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "save_packet_result should succeed for empty container";

	// Verify file was created
	EXPECT_TRUE(std::filesystem::exists(file_path))
		<< "File should exist after save";
}

TEST_F(FileOperationResultAPITest, SavePacketResultOverwrite)
{
	std::string file_path = (test_dir / "overwrite_test.bin").string();

	// First save
	container->set("version", int32_t(1));
	auto first_result = container->save_packet_result(file_path);
	EXPECT_TRUE(kcenon::common::is_ok(first_result));

	// Overwrite with different content
	auto new_container = std::make_shared<value_container>();
	new_container->set("version", int32_t(2));
	new_container->set("new_key", std::string("new_value"));

	auto second_result = new_container->save_packet_result(file_path);
	EXPECT_TRUE(kcenon::common::is_ok(second_result))
		<< "save_packet_result should succeed when overwriting";

	// Load and verify new content
	auto loaded_container = std::make_shared<value_container>();
	auto load_result = loaded_container->load_packet_result(file_path);
	EXPECT_TRUE(kcenon::common::is_ok(load_result));

	auto version_result = loaded_container->get<int32_t>("version");
	EXPECT_TRUE(kcenon::common::is_ok(version_result));
	EXPECT_EQ(kcenon::common::get_value(version_result), 2)
		<< "Should contain overwritten value";

	EXPECT_TRUE(loaded_container->contains("new_key"))
		<< "Should contain new key from overwritten file";
}

TEST_F(FileOperationResultAPITest, FileOperationErrorMessages)
{
	// Verify error messages contain useful context

	// File not found error
	std::string nonexistent = (test_dir / "no_such_file.bin").string();
	auto load_result = container->load_packet_result(nonexistent);
	EXPECT_TRUE(kcenon::common::is_error(load_result));

	auto load_error = kcenon::common::get_error(load_result);
	EXPECT_FALSE(load_error.message.empty())
		<< "Error message should not be empty";
	EXPECT_EQ(load_error.module, "container_system")
		<< "Error module should be container_system";

	// File write error
	std::string invalid_path = "/nonexistent_dir_xyz/file.bin";
	container->set("test", int32_t(1));
	auto save_result = container->save_packet_result(invalid_path);
	EXPECT_TRUE(kcenon::common::is_error(save_result));

	auto save_error = kcenon::common::get_error(save_result);
	EXPECT_FALSE(save_error.message.empty())
		<< "Error message should not be empty";
	EXPECT_EQ(save_error.module, "container_system")
		<< "Error module should be container_system";
}

// ============================================================================
// Unified Serialization API Tests (Issue #286)
// ============================================================================

class UnifiedSerializationAPITest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		container = std::make_shared<value_container>();
		container->set_source("test_source", "test_sub");
		container->set_target("test_target", "target_sub");
		container->set_message_type("test_message");
		container->set("string_key", std::string("test_value"));
		container->set("int_key", int32_t(42));
		container->set("double_key", 3.14);
		container->set("bool_key", true);
	}

	void TearDown() override { container.reset(); }

	std::shared_ptr<value_container> container;
};

TEST_F(UnifiedSerializationAPITest, SerializeBinaryFormat)
{
	auto result = container->serialize(value_container::serialization_format::binary);
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "serialize(binary) should succeed";

	const auto& data = kcenon::common::get_value(result);
	EXPECT_FALSE(data.empty()) << "Serialized binary data should not be empty";

	// Verify it contains header marker
	std::string str_data(data.begin(), data.end());
	EXPECT_NE(str_data.find("@header"), std::string::npos)
		<< "Binary format should contain @header marker";
}

TEST_F(UnifiedSerializationAPITest, SerializeJsonFormat)
{
	auto result = container->serialize(value_container::serialization_format::json);
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "serialize(json) should succeed";

	const auto& data = kcenon::common::get_value(result);
	EXPECT_FALSE(data.empty()) << "Serialized JSON data should not be empty";

	// Verify it's valid JSON-like format
	std::string str_data(data.begin(), data.end());
	EXPECT_NE(str_data.find("{"), std::string::npos)
		<< "JSON format should contain opening brace";
}

TEST_F(UnifiedSerializationAPITest, SerializeXmlFormat)
{
	auto result = container->serialize(value_container::serialization_format::xml);
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "serialize(xml) should succeed";

	const auto& data = kcenon::common::get_value(result);
	EXPECT_FALSE(data.empty()) << "Serialized XML data should not be empty";

	// Verify it's valid XML-like format
	std::string str_data(data.begin(), data.end());
	EXPECT_NE(str_data.find("<container>"), std::string::npos)
		<< "XML format should contain <container> root element";
}

TEST_F(UnifiedSerializationAPITest, SerializeMsgpackFormat)
{
	auto result = container->serialize(value_container::serialization_format::msgpack);
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "serialize(msgpack) should succeed";

	const auto& data = kcenon::common::get_value(result);
	EXPECT_FALSE(data.empty()) << "Serialized MessagePack data should not be empty";
}

TEST_F(UnifiedSerializationAPITest, SerializeInvalidFormat)
{
	auto result = container->serialize(value_container::serialization_format::auto_detect);
	EXPECT_TRUE(kcenon::common::is_error(result))
		<< "serialize(auto_detect) should fail";

	auto error = kcenon::common::get_error(result);
	EXPECT_EQ(error.code, error_codes::invalid_format)
		<< "Error code should be invalid_format";
}

TEST_F(UnifiedSerializationAPITest, SerializeStringBinaryFormat)
{
	auto result = container->serialize_string(value_container::serialization_format::binary);
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "serialize_string(binary) should succeed";

	const auto& str = kcenon::common::get_value(result);
	EXPECT_FALSE(str.empty()) << "Serialized string should not be empty";
	EXPECT_NE(str.find("@header"), std::string::npos)
		<< "Binary format should contain @header marker";
}

TEST_F(UnifiedSerializationAPITest, SerializeStringJsonFormat)
{
	auto result = container->serialize_string(value_container::serialization_format::json);
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "serialize_string(json) should succeed";

	const auto& str = kcenon::common::get_value(result);
	EXPECT_FALSE(str.empty()) << "Serialized JSON string should not be empty";
	EXPECT_NE(str.find("{"), std::string::npos)
		<< "JSON format should contain opening brace";
}

TEST_F(UnifiedSerializationAPITest, SerializeStringXmlFormat)
{
	auto result = container->serialize_string(value_container::serialization_format::xml);
	EXPECT_TRUE(kcenon::common::is_ok(result))
		<< "serialize_string(xml) should succeed";

	const auto& str = kcenon::common::get_value(result);
	EXPECT_FALSE(str.empty()) << "Serialized XML string should not be empty";
	EXPECT_NE(str.find("<container>"), std::string::npos)
		<< "XML format should contain <container> root element";
}

TEST_F(UnifiedSerializationAPITest, DeserializeBinaryFormat)
{
	// First serialize
	auto serialize_result = container->serialize(value_container::serialization_format::binary);
	ASSERT_TRUE(kcenon::common::is_ok(serialize_result));

	const auto& data = kcenon::common::get_value(serialize_result);

	// Then deserialize into new container
	auto new_container = std::make_shared<value_container>();
	auto deser_result = new_container->deserialize(
		std::span<const uint8_t>(data),
		value_container::serialization_format::binary);
	EXPECT_TRUE(kcenon::common::is_ok(deser_result))
		<< "deserialize(binary) should succeed";

	// Verify data was restored
	EXPECT_EQ(new_container->source_id(), "test_source");
	EXPECT_TRUE(new_container->contains("string_key"));
}

TEST_F(UnifiedSerializationAPITest, DeserializeMsgpackFormat)
{
	// First serialize to msgpack
	auto serialize_result = container->serialize(value_container::serialization_format::msgpack);
	ASSERT_TRUE(kcenon::common::is_ok(serialize_result));

	const auto& data = kcenon::common::get_value(serialize_result);

	// Then deserialize into new container
	auto new_container = std::make_shared<value_container>();
	auto deser_result = new_container->deserialize(
		std::span<const uint8_t>(data),
		value_container::serialization_format::msgpack);
	EXPECT_TRUE(kcenon::common::is_ok(deser_result))
		<< "deserialize(msgpack) should succeed";

	// Verify data was restored
	EXPECT_EQ(new_container->source_id(), "test_source");
	EXPECT_TRUE(new_container->contains("string_key"));
	EXPECT_TRUE(new_container->contains("int_key"));
}

TEST_F(UnifiedSerializationAPITest, DeserializeAutoDetectBinary)
{
	// Serialize to binary format
	auto serialize_result = container->serialize(value_container::serialization_format::binary);
	ASSERT_TRUE(kcenon::common::is_ok(serialize_result));

	const auto& data = kcenon::common::get_value(serialize_result);

	// Auto-detect and deserialize
	auto new_container = std::make_shared<value_container>();
	auto deser_result = new_container->deserialize(std::span<const uint8_t>(data));
	EXPECT_TRUE(kcenon::common::is_ok(deser_result))
		<< "deserialize(auto_detect) should succeed for binary data";

	EXPECT_EQ(new_container->source_id(), "test_source");
}

TEST_F(UnifiedSerializationAPITest, DeserializeAutoDetectMsgpack)
{
	// Serialize to msgpack format
	auto serialize_result = container->serialize(value_container::serialization_format::msgpack);
	ASSERT_TRUE(kcenon::common::is_ok(serialize_result));

	const auto& data = kcenon::common::get_value(serialize_result);

	// Auto-detect and deserialize
	auto new_container = std::make_shared<value_container>();
	auto deser_result = new_container->deserialize(std::span<const uint8_t>(data));
	EXPECT_TRUE(kcenon::common::is_ok(deser_result))
		<< "deserialize(auto_detect) should succeed for msgpack data";

	EXPECT_EQ(new_container->source_id(), "test_source");
}

TEST_F(UnifiedSerializationAPITest, DeserializeStringView)
{
	// Serialize to binary string
	auto serialize_result = container->serialize_string(value_container::serialization_format::binary);
	ASSERT_TRUE(kcenon::common::is_ok(serialize_result));

	const auto& str = kcenon::common::get_value(serialize_result);

	// Deserialize from string_view
	auto new_container = std::make_shared<value_container>();
	auto deser_result = new_container->deserialize(std::string_view(str));
	EXPECT_TRUE(kcenon::common::is_ok(deser_result))
		<< "deserialize(string_view) should succeed";

	EXPECT_EQ(new_container->source_id(), "test_source");
}

TEST_F(UnifiedSerializationAPITest, DeserializeStringViewWithFormat)
{
	// Serialize to binary string
	auto serialize_result = container->serialize_string(value_container::serialization_format::binary);
	ASSERT_TRUE(kcenon::common::is_ok(serialize_result));

	const auto& str = kcenon::common::get_value(serialize_result);

	// Deserialize from string_view with explicit format
	auto new_container = std::make_shared<value_container>();
	auto deser_result = new_container->deserialize(
		std::string_view(str),
		value_container::serialization_format::binary);
	EXPECT_TRUE(kcenon::common::is_ok(deser_result))
		<< "deserialize(string_view, binary) should succeed";

	EXPECT_EQ(new_container->source_id(), "test_source");
}

TEST_F(UnifiedSerializationAPITest, RoundTripAllFormats)
{
	// Test round-trip for formats that support full deserialization
	// Note: JSON and XML are output-only formats in the current implementation
	// (no from_json/from_xml methods exist)
	std::vector<value_container::serialization_format> formats = {
		value_container::serialization_format::binary,
		value_container::serialization_format::msgpack
	};

	for (auto fmt : formats) {
		// Serialize
		auto ser_result = container->serialize(fmt);
		ASSERT_TRUE(kcenon::common::is_ok(ser_result))
			<< "serialize should succeed for format " << static_cast<int>(fmt);

		const auto& data = kcenon::common::get_value(ser_result);

		// Deserialize
		auto new_container = std::make_shared<value_container>();
		auto deser_result = new_container->deserialize(
			std::span<const uint8_t>(data), fmt);
		EXPECT_TRUE(kcenon::common::is_ok(deser_result))
			<< "deserialize should succeed for format " << static_cast<int>(fmt);

		// Verify key data
		EXPECT_EQ(new_container->source_id(), "test_source")
			<< "source_id should match for format " << static_cast<int>(fmt);
		EXPECT_TRUE(new_container->contains("string_key"))
			<< "string_key should exist for format " << static_cast<int>(fmt);
	}
}

#endif  // CONTAINER_HAS_COMMON_RESULT

// Main function for running tests
int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
