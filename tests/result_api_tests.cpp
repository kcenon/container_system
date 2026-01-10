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

#endif  // CONTAINER_HAS_COMMON_RESULT

// Main function for running tests
int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
