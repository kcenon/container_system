// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file error_codes_tests.cpp
 * @brief Unit tests for kcenon::container error codes and message mapping
 *
 * Tests cover:
 * - Error code value verification (negative, within common's reserved band)
 * - Error message retrieval via get_message()
 * - Category classification via get_category()
 * - Category check functions (is_value_error, is_serialization_error, etc.)
 * - Detailed message construction via make_message()
 * - Cross-system classification by kcenon::common (get_category_name == "ContainerSystem")
 */

#include <gtest/gtest.h>
#include <kcenon/container/error_codes.h>
#include <kcenon/container/schema.h>

#if defined(KCENON_HAS_COMMON_SYSTEM) && __has_include(<kcenon/common/error/error_codes.h>)
#include <kcenon/common/error/error_codes.h>
#define CONTAINER_TEST_HAS_COMMON_ERROR 1
#endif

using namespace kcenon::container;
using namespace kcenon::container::error_codes;

// ============================================================================
// Error Code Value Tests
//
// All container error codes must be NEGATIVE and live inside common's reserved
// container_system band [-499, -400], partitioned into 10-wide sub-bands.
// ============================================================================

TEST(ErrorCodesTest, ValueOperationCodes) {
	// Value operations sub-band: -400 to -409
	EXPECT_EQ(type_mismatch, -400);       // aligned with common value_type_mismatch
	EXPECT_EQ(invalid_value, -401);       // aligned with common invalid_value_type
	EXPECT_EQ(key_not_found, -402);
	EXPECT_EQ(value_out_of_range, -403);
	EXPECT_EQ(key_already_exists, -404);
	EXPECT_EQ(empty_key, -405);
}

TEST(ErrorCodesTest, ValidationCodes) {
	// Validation sub-band: -410 to -419
	EXPECT_EQ(schema_validation_failed, -410);
	EXPECT_EQ(missing_required_field, -411);
	EXPECT_EQ(constraint_violated, -412);
	EXPECT_EQ(type_constraint_violated, -413);
	EXPECT_EQ(max_size_exceeded, -414);
}

TEST(ErrorCodesTest, SerializationCodes) {
	// Serialization sub-band: -420 to -429
	EXPECT_EQ(serialization_failed, -420);   // aligned with common serialization_failed
	EXPECT_EQ(deserialization_failed, -421); // aligned with common deserialization_failed
	EXPECT_EQ(invalid_format, -422);         // aligned with common invalid_format
	EXPECT_EQ(version_mismatch, -423);
	EXPECT_EQ(corrupted_data, -424);
	EXPECT_EQ(header_parse_failed, -425);
	EXPECT_EQ(value_parse_failed, -426);
	EXPECT_EQ(encoding_error, -427);
}

TEST(ErrorCodesTest, ResourceCodes) {
	// Resource sub-band: -430 to -439
	EXPECT_EQ(memory_allocation_failed, -430);
	EXPECT_EQ(file_not_found, -431);
	EXPECT_EQ(file_read_error, -432);
	EXPECT_EQ(file_write_error, -433);
	EXPECT_EQ(permission_denied, -434);
	EXPECT_EQ(resource_exhausted, -435);
	EXPECT_EQ(io_error, -436);
}

TEST(ErrorCodesTest, ThreadSafetyCodes) {
	// Thread safety sub-band: -440 to -449
	EXPECT_EQ(lock_acquisition_failed, -440);
	EXPECT_EQ(concurrent_modification, -441);
	EXPECT_EQ(lock_timeout, -442);
}

TEST(ErrorCodesTest, AllCodesWithinCommonReservedBand) {
	// Every container error code must be in common's reserved [-499, -400].
	const int all_codes[] = {
		type_mismatch, invalid_value, key_not_found, value_out_of_range,
		key_already_exists, empty_key,
		serialization_failed, deserialization_failed, invalid_format,
		version_mismatch, corrupted_data, header_parse_failed,
		value_parse_failed, encoding_error,
		schema_validation_failed, missing_required_field, constraint_violated,
		type_constraint_violated, max_size_exceeded,
		memory_allocation_failed, file_not_found, file_read_error,
		file_write_error, permission_denied, resource_exhausted, io_error,
		lock_acquisition_failed, concurrent_modification, lock_timeout,
		// schema-level validation codes (-450 to -459)
		validation_codes::missing_required, validation_codes::type_mismatch,
		validation_codes::out_of_range, validation_codes::invalid_length,
		validation_codes::pattern_mismatch, validation_codes::not_in_allowed_values,
		validation_codes::custom_validation_failed,
		validation_codes::nested_validation_failed,
	};
	for (int code : all_codes) {
		EXPECT_GE(code, -499) << "code " << code << " below container band";
		EXPECT_LE(code, -400) << "code " << code << " above container band";
	}
}

TEST(ErrorCodesTest, SchemaValidationCodesSubBand) {
	// Schema-level validation codes occupy -450 to -459 (distinct from
	// error_codes' validation sub-band -420 to -429).
	EXPECT_EQ(validation_codes::missing_required, -450);
	EXPECT_EQ(validation_codes::type_mismatch, -451);
	EXPECT_EQ(validation_codes::out_of_range, -452);
	EXPECT_EQ(validation_codes::invalid_length, -453);
	EXPECT_EQ(validation_codes::pattern_mismatch, -454);
	EXPECT_EQ(validation_codes::not_in_allowed_values, -455);
	EXPECT_EQ(validation_codes::custom_validation_failed, -456);
	EXPECT_EQ(validation_codes::nested_validation_failed, -457);
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
// Category Tests (negative sub-bands)
// ============================================================================

TEST(ErrorCodesTest, GetCategoryValueOperation) {
	EXPECT_EQ(get_category(-400), "value_operation");
	EXPECT_EQ(get_category(-405), "value_operation");
	EXPECT_EQ(get_category(-409), "value_operation");
}

TEST(ErrorCodesTest, GetCategoryValidation) {
	EXPECT_EQ(get_category(-410), "validation");
	EXPECT_EQ(get_category(-414), "validation");
	EXPECT_EQ(get_category(-419), "validation");
}

TEST(ErrorCodesTest, GetCategorySerialization) {
	EXPECT_EQ(get_category(-420), "serialization");
	EXPECT_EQ(get_category(-427), "serialization");
	EXPECT_EQ(get_category(-429), "serialization");
}

TEST(ErrorCodesTest, GetCategoryResource) {
	EXPECT_EQ(get_category(-430), "resource");
	EXPECT_EQ(get_category(-436), "resource");
	EXPECT_EQ(get_category(-439), "resource");
}

TEST(ErrorCodesTest, GetCategoryThreadSafety) {
	EXPECT_EQ(get_category(-440), "thread_safety");
	EXPECT_EQ(get_category(-442), "thread_safety");
	EXPECT_EQ(get_category(-449), "thread_safety");
}

TEST(ErrorCodesTest, GetCategoryUnknown) {
	EXPECT_EQ(get_category(0), "unknown");
	EXPECT_EQ(get_category(-99), "unknown");
	EXPECT_EQ(get_category(-399), "unknown");   // just outside container band
	EXPECT_EQ(get_category(-450), "unknown");   // schema sub-band not classified here
	EXPECT_EQ(get_category(-500), "unknown");
	EXPECT_EQ(get_category(100), "unknown");    // old positive code no longer valid
}

// ============================================================================
// Representative per-band get_category() checks (named codes)
// ============================================================================

TEST(ErrorCodesTest, GetCategoryRepresentativePerBand) {
	EXPECT_EQ(get_category(type_mismatch), "value_operation");
	EXPECT_EQ(get_category(deserialization_failed), "serialization");
	EXPECT_EQ(get_category(schema_validation_failed), "validation");
	EXPECT_EQ(get_category(file_not_found), "resource");
	EXPECT_EQ(get_category(lock_timeout), "thread_safety");
}

// ============================================================================
// Category Check Function Tests (negative bands)
// ============================================================================

TEST(ErrorCodesTest, IsCategoryFunction) {
	// 10-wide band ending at the base value (inclusive).
	EXPECT_TRUE(is_category(-400, -400));
	EXPECT_TRUE(is_category(-405, -400));
	EXPECT_TRUE(is_category(-409, -400));
	EXPECT_FALSE(is_category(-410, -400));
	EXPECT_FALSE(is_category(-399, -400));
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
	constexpr auto msg = get_message(key_not_found);
	EXPECT_EQ(msg, "Key not found");
}

TEST(ErrorCodesTest, ConstexprGetCategory) {
	constexpr auto cat = get_category(serialization_failed);
	EXPECT_EQ(cat, "serialization");
}

TEST(ErrorCodesTest, ConstexprIsCategory) {
	constexpr bool result = is_category(-405, -400);
	EXPECT_TRUE(result);
}

TEST(ErrorCodesTest, ConstexprIsCategoryHelpers) {
	constexpr bool is_val = is_value_error(type_mismatch);
	constexpr bool is_ser = is_serialization_error(serialization_failed);
	constexpr bool is_vld = is_validation_error(schema_validation_failed);
	constexpr bool is_res = is_resource_error(memory_allocation_failed);
	constexpr bool is_thr = is_thread_error(lock_acquisition_failed);

	EXPECT_TRUE(is_val);
	EXPECT_TRUE(is_ser);
	EXPECT_TRUE(is_vld);
	EXPECT_TRUE(is_res);
	EXPECT_TRUE(is_thr);
}

// ============================================================================
// Edge Case Tests (boundary between negative sub-bands)
// ============================================================================

TEST(ErrorCodesTest, BoundaryValues) {
	EXPECT_TRUE(is_value_error(-409));
	EXPECT_FALSE(is_value_error(-410));

	EXPECT_TRUE(is_validation_error(-419));
	EXPECT_FALSE(is_validation_error(-420));

	EXPECT_TRUE(is_serialization_error(-429));
	EXPECT_FALSE(is_serialization_error(-430));

	EXPECT_TRUE(is_resource_error(-439));
	EXPECT_FALSE(is_resource_error(-440));

	EXPECT_TRUE(is_thread_error(-449));
	EXPECT_FALSE(is_thread_error(-450));
}

TEST(ErrorCodesTest, PositiveCodeHandling) {
	// Legacy positive codes are no longer valid container error codes.
	EXPECT_EQ(get_category(100), "unknown");
	EXPECT_EQ(get_category(500), "unknown");
	EXPECT_FALSE(is_value_error(100));
	EXPECT_FALSE(is_serialization_error(200));
}

// ============================================================================
// Cross-System Classification Tests
//
// The whole point of this issue: container codes, when handed to common's
// error registry, must classify as "ContainerSystem" (not Success/Invalid).
// ============================================================================

#ifdef CONTAINER_TEST_HAS_COMMON_ERROR
TEST(ErrorCodesTest, CommonClassifiesAsContainerSystem) {
	using kcenon::common::error::get_category_name;

	// One representative code from each sub-band, plus a schema code.
	const int representatives[] = {
		type_mismatch,            // value
		serialization_failed,     // serialization
		schema_validation_failed, // validation
		file_not_found,           // resource
		lock_timeout,             // thread safety
		validation_codes::out_of_range, // schema-level
	};
	for (int code : representatives) {
		EXPECT_EQ(get_category_name(code), "ContainerSystem")
			<< "common misclassified container code " << code;
	}
}

TEST(ErrorCodesTest, AlignedWithCommonContainerCodes) {
	namespace cc = kcenon::common::error::codes::container_system;
	// Obvious alignments requested by the issue.
	EXPECT_EQ(type_mismatch, cc::value_type_mismatch);
	EXPECT_EQ(invalid_value, cc::invalid_value_type);
	EXPECT_EQ(serialization_failed, cc::serialization_failed);
	EXPECT_EQ(deserialization_failed, cc::deserialization_failed);
	EXPECT_EQ(invalid_format, cc::invalid_format);
}
#endif // CONTAINER_TEST_HAS_COMMON_ERROR
