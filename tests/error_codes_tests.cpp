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
 * @file error_codes_tests.cpp
 * @brief Unit tests for container_module error codes and message mapping
 *
 * Tests cover:
 * - Error code value verification
 * - Error message retrieval via get_message()
 * - Category classification via get_category()
 * - Category check functions (is_value_error, is_serialization_error, etc.)
 * - Detailed message construction via make_message()
 */

#include <gtest/gtest.h>
#include <container/core/container/error_codes.h>

using namespace container_module;
using namespace container_module::error_codes;

// ============================================================================
// Error Code Value Tests
// ============================================================================

TEST(ErrorCodesTest, ValueOperationCodes) {
	// Value operations should be in 1xx range
	EXPECT_EQ(key_not_found, 100);
	EXPECT_EQ(type_mismatch, 101);
	EXPECT_EQ(value_out_of_range, 102);
	EXPECT_EQ(invalid_value, 103);
	EXPECT_EQ(key_already_exists, 104);
	EXPECT_EQ(empty_key, 105);
}

TEST(ErrorCodesTest, SerializationCodes) {
	// Serialization errors should be in 2xx range
	EXPECT_EQ(serialization_failed, 200);
	EXPECT_EQ(deserialization_failed, 201);
	EXPECT_EQ(invalid_format, 202);
	EXPECT_EQ(version_mismatch, 203);
	EXPECT_EQ(corrupted_data, 204);
	EXPECT_EQ(header_parse_failed, 205);
	EXPECT_EQ(value_parse_failed, 206);
	EXPECT_EQ(encoding_error, 207);
}

TEST(ErrorCodesTest, ValidationCodes) {
	// Validation errors should be in 3xx range
	EXPECT_EQ(schema_validation_failed, 300);
	EXPECT_EQ(missing_required_field, 301);
	EXPECT_EQ(constraint_violated, 302);
	EXPECT_EQ(type_constraint_violated, 303);
	EXPECT_EQ(max_size_exceeded, 304);
}

TEST(ErrorCodesTest, ResourceCodes) {
	// Resource errors should be in 4xx range
	EXPECT_EQ(memory_allocation_failed, 400);
	EXPECT_EQ(file_not_found, 401);
	EXPECT_EQ(file_read_error, 402);
	EXPECT_EQ(file_write_error, 403);
	EXPECT_EQ(permission_denied, 404);
	EXPECT_EQ(resource_exhausted, 405);
	EXPECT_EQ(io_error, 406);
}

TEST(ErrorCodesTest, ThreadSafetyCodes) {
	// Thread safety errors should be in 5xx range
	EXPECT_EQ(lock_acquisition_failed, 500);
	EXPECT_EQ(concurrent_modification, 501);
	EXPECT_EQ(lock_timeout, 502);
}

// ============================================================================
// Error Message Tests
// ============================================================================

TEST(ErrorCodesTest, GetMessageValueOperations) {
	EXPECT_EQ(get_message(key_not_found), "Key not found");
	EXPECT_EQ(get_message(type_mismatch), "Type mismatch");
	EXPECT_EQ(get_message(value_out_of_range), "Value out of range");
	EXPECT_EQ(get_message(invalid_value), "Invalid value");
	EXPECT_EQ(get_message(key_already_exists), "Key already exists");
	EXPECT_EQ(get_message(empty_key), "Empty key");
}

TEST(ErrorCodesTest, GetMessageSerialization) {
	EXPECT_EQ(get_message(serialization_failed), "Serialization failed");
	EXPECT_EQ(get_message(deserialization_failed), "Deserialization failed");
	EXPECT_EQ(get_message(invalid_format), "Invalid format");
	EXPECT_EQ(get_message(version_mismatch), "Version mismatch");
	EXPECT_EQ(get_message(corrupted_data), "Corrupted data");
	EXPECT_EQ(get_message(header_parse_failed), "Header parse failed");
	EXPECT_EQ(get_message(value_parse_failed), "Value parse failed");
	EXPECT_EQ(get_message(encoding_error), "Encoding error");
}

TEST(ErrorCodesTest, GetMessageValidation) {
	EXPECT_EQ(get_message(schema_validation_failed), "Schema validation failed");
	EXPECT_EQ(get_message(missing_required_field), "Missing required field");
	EXPECT_EQ(get_message(constraint_violated), "Constraint violated");
	EXPECT_EQ(get_message(type_constraint_violated), "Type constraint violated");
	EXPECT_EQ(get_message(max_size_exceeded), "Maximum size exceeded");
}

TEST(ErrorCodesTest, GetMessageResource) {
	EXPECT_EQ(get_message(memory_allocation_failed), "Memory allocation failed");
	EXPECT_EQ(get_message(file_not_found), "File not found");
	EXPECT_EQ(get_message(file_read_error), "File read error");
	EXPECT_EQ(get_message(file_write_error), "File write error");
	EXPECT_EQ(get_message(permission_denied), "Permission denied");
	EXPECT_EQ(get_message(resource_exhausted), "Resource exhausted");
	EXPECT_EQ(get_message(io_error), "I/O error");
}

TEST(ErrorCodesTest, GetMessageThreadSafety) {
	EXPECT_EQ(get_message(lock_acquisition_failed), "Lock acquisition failed");
	EXPECT_EQ(get_message(concurrent_modification), "Concurrent modification");
	EXPECT_EQ(get_message(lock_timeout), "Lock timeout");
}

TEST(ErrorCodesTest, GetMessageUnknownCode) {
	EXPECT_EQ(get_message(999), "Unknown error");
	EXPECT_EQ(get_message(-1), "Unknown error");
	EXPECT_EQ(get_message(0), "Unknown error");
}

// ============================================================================
// Category Tests
// ============================================================================

TEST(ErrorCodesTest, GetCategoryValueOperation) {
	EXPECT_EQ(get_category(100), "value_operation");
	EXPECT_EQ(get_category(105), "value_operation");
	EXPECT_EQ(get_category(199), "value_operation");
}

TEST(ErrorCodesTest, GetCategorySerialization) {
	EXPECT_EQ(get_category(200), "serialization");
	EXPECT_EQ(get_category(207), "serialization");
	EXPECT_EQ(get_category(299), "serialization");
}

TEST(ErrorCodesTest, GetCategoryValidation) {
	EXPECT_EQ(get_category(300), "validation");
	EXPECT_EQ(get_category(304), "validation");
	EXPECT_EQ(get_category(399), "validation");
}

TEST(ErrorCodesTest, GetCategoryResource) {
	EXPECT_EQ(get_category(400), "resource");
	EXPECT_EQ(get_category(406), "resource");
	EXPECT_EQ(get_category(499), "resource");
}

TEST(ErrorCodesTest, GetCategoryThreadSafety) {
	EXPECT_EQ(get_category(500), "thread_safety");
	EXPECT_EQ(get_category(502), "thread_safety");
	EXPECT_EQ(get_category(599), "thread_safety");
}

TEST(ErrorCodesTest, GetCategoryUnknown) {
	EXPECT_EQ(get_category(0), "unknown");
	EXPECT_EQ(get_category(99), "unknown");
	EXPECT_EQ(get_category(600), "unknown");
	EXPECT_EQ(get_category(-1), "unknown");
}

// ============================================================================
// Category Check Function Tests
// ============================================================================

TEST(ErrorCodesTest, IsCategoryFunction) {
	EXPECT_TRUE(is_category(100, 100));
	EXPECT_TRUE(is_category(150, 100));
	EXPECT_TRUE(is_category(199, 100));
	EXPECT_FALSE(is_category(200, 100));
	EXPECT_FALSE(is_category(99, 100));
}

TEST(ErrorCodesTest, IsValueError) {
	EXPECT_TRUE(is_value_error(key_not_found));
	EXPECT_TRUE(is_value_error(type_mismatch));
	EXPECT_TRUE(is_value_error(empty_key));
	EXPECT_FALSE(is_value_error(serialization_failed));
	EXPECT_FALSE(is_value_error(file_not_found));
}

TEST(ErrorCodesTest, IsSerializationError) {
	EXPECT_TRUE(is_serialization_error(serialization_failed));
	EXPECT_TRUE(is_serialization_error(deserialization_failed));
	EXPECT_TRUE(is_serialization_error(encoding_error));
	EXPECT_FALSE(is_serialization_error(key_not_found));
	EXPECT_FALSE(is_serialization_error(schema_validation_failed));
}

TEST(ErrorCodesTest, IsValidationError) {
	EXPECT_TRUE(is_validation_error(schema_validation_failed));
	EXPECT_TRUE(is_validation_error(missing_required_field));
	EXPECT_TRUE(is_validation_error(max_size_exceeded));
	EXPECT_FALSE(is_validation_error(serialization_failed));
	EXPECT_FALSE(is_validation_error(memory_allocation_failed));
}

TEST(ErrorCodesTest, IsResourceError) {
	EXPECT_TRUE(is_resource_error(memory_allocation_failed));
	EXPECT_TRUE(is_resource_error(file_not_found));
	EXPECT_TRUE(is_resource_error(io_error));
	EXPECT_FALSE(is_resource_error(schema_validation_failed));
	EXPECT_FALSE(is_resource_error(lock_acquisition_failed));
}

TEST(ErrorCodesTest, IsThreadError) {
	EXPECT_TRUE(is_thread_error(lock_acquisition_failed));
	EXPECT_TRUE(is_thread_error(concurrent_modification));
	EXPECT_TRUE(is_thread_error(lock_timeout));
	EXPECT_FALSE(is_thread_error(memory_allocation_failed));
	EXPECT_FALSE(is_thread_error(key_not_found));
}

// ============================================================================
// Make Message Tests
// ============================================================================

TEST(ErrorCodesTest, MakeMessageWithoutDetail) {
	auto msg = make_message(key_not_found);
	EXPECT_EQ(msg, "Key not found");
}

TEST(ErrorCodesTest, MakeMessageWithDetail) {
	auto msg = make_message(key_not_found, "user_id");
	EXPECT_EQ(msg, "Key not found: user_id");
}

TEST(ErrorCodesTest, MakeMessageWithEmptyDetail) {
	auto msg = make_message(file_not_found, "");
	EXPECT_EQ(msg, "File not found");
}

TEST(ErrorCodesTest, MakeMessageWithLongDetail) {
	std::string long_detail = "/path/to/some/very/long/file/path/that/might/be/used/in/error/messages.txt";
	auto msg = make_message(file_read_error, long_detail);
	EXPECT_EQ(msg, "File read error: " + long_detail);
}

TEST(ErrorCodesTest, MakeMessageUnknownWithDetail) {
	auto msg = make_message(999, "some context");
	EXPECT_EQ(msg, "Unknown error: some context");
}

// ============================================================================
// Constexpr Tests (compile-time evaluation)
// ============================================================================

TEST(ErrorCodesTest, ConstexprGetMessage) {
	// Verify get_message can be used in constexpr context
	constexpr auto msg = get_message(key_not_found);
	EXPECT_EQ(msg, "Key not found");
}

TEST(ErrorCodesTest, ConstexprGetCategory) {
	// Verify get_category can be used in constexpr context
	constexpr auto cat = get_category(200);
	EXPECT_EQ(cat, "serialization");
}

TEST(ErrorCodesTest, ConstexprIsCategory) {
	// Verify is_category can be used in constexpr context
	constexpr bool result = is_category(150, 100);
	EXPECT_TRUE(result);
}

TEST(ErrorCodesTest, ConstexprIsCategoryHelpers) {
	// Verify category helper functions can be used in constexpr context
	constexpr bool is_val = is_value_error(100);
	constexpr bool is_ser = is_serialization_error(200);
	constexpr bool is_vld = is_validation_error(300);
	constexpr bool is_res = is_resource_error(400);
	constexpr bool is_thr = is_thread_error(500);

	EXPECT_TRUE(is_val);
	EXPECT_TRUE(is_ser);
	EXPECT_TRUE(is_vld);
	EXPECT_TRUE(is_res);
	EXPECT_TRUE(is_thr);
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST(ErrorCodesTest, BoundaryValues) {
	// Test boundary values between categories
	EXPECT_TRUE(is_value_error(199));
	EXPECT_FALSE(is_value_error(200));

	EXPECT_TRUE(is_serialization_error(299));
	EXPECT_FALSE(is_serialization_error(300));

	EXPECT_TRUE(is_validation_error(399));
	EXPECT_FALSE(is_validation_error(400));

	EXPECT_TRUE(is_resource_error(499));
	EXPECT_FALSE(is_resource_error(500));

	EXPECT_TRUE(is_thread_error(599));
	EXPECT_FALSE(is_thread_error(600));
}

TEST(ErrorCodesTest, NegativeCodeHandling) {
	// Negative codes should be classified as unknown
	EXPECT_EQ(get_category(-1), "unknown");
	EXPECT_EQ(get_category(-100), "unknown");
	EXPECT_FALSE(is_value_error(-100));
	EXPECT_FALSE(is_serialization_error(-200));
}
