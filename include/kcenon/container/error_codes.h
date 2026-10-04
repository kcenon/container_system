// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file core/container/error_codes.h
 * @brief Standardized error codes for kcenon::container Result<T> pattern
 *
 * These error codes are cast into the shared kcenon::common error_info.code
 * field. common reserves the band [-499, -400] for container_system and
 * classifies any code >= 0 as "Success"/"Invalid", so container error codes
 * MUST be negative and inside common's reserved band to be recognised as
 * "ContainerSystem" by kcenon::common::error::get_category_name(code).
 *
 * Negative sub-bands (all within [-499, -400], distinct, room to grow):
 * - Value operations   (-400 to -409): key_not_found, type_mismatch, etc.
 * - Validation         (-410 to -419): schema_validation_failed, etc.
 * - Serialization      (-420 to -429): serialization_failed, etc.
 * - Resource           (-430 to -439): memory_allocation_failed, file errors
 * - Thread safety      (-440 to -449): lock_acquisition_failed, etc.
 * (Schema-level validation_codes occupy -450 to -459, see schema.h.)
 *
 * Where common defines a matching code in kcenon::common::error::codes::
 * container_system the values are aligned for the obvious cases
 * (type_mismatch == value_type_mismatch == -400, invalid_value ==
 * invalid_value_type == -401, and the serialization band -420..-422 matching
 * serialization_failed / deserialization_failed / invalid_format).
 *
 * @see kcenon::container::value_container
 * @see kcenon::common::error::codes::container_system
 * @since 2.0.0
 */

#pragma once

#include <string_view>
#include <string>

namespace kcenon::container
{
	/**
	 * @brief Standardized error codes for container operations
	 *
	 * Error codes are negative and organised into sub-bands within common's
	 * reserved container_system range [-499, -400]:
	 * - -400 to -409: Value operations
	 * - -410 to -419: Validation
	 * - -420 to -429: Serialization
	 * - -430 to -439: Resource
	 * - -440 to -449: Thread safety
	 */
	namespace error_codes
	{
		// =========================================================================
		// Value operations (-400 to -409)
		// =========================================================================

		/** @brief Requested key does not exist in the container */
		constexpr int key_not_found = -402;

		/** @brief Value type does not match the requested type
		 *  (aligned with kcenon::common::error::codes::container_system::value_type_mismatch) */
		constexpr int type_mismatch = -400;

		/** @brief Numeric value is outside the valid range */
		constexpr int value_out_of_range = -403;

		/** @brief Value is invalid for the operation
		 *  (aligned with kcenon::common::error::codes::container_system::invalid_value_type) */
		constexpr int invalid_value = -401;

		/** @brief Key already exists when unique key required */
		constexpr int key_already_exists = -404;

		/** @brief Empty key name provided */
		constexpr int empty_key = -405;

		// =========================================================================
		// Validation (-410 to -419)
		// =========================================================================

		/** @brief Schema validation failed */
		constexpr int schema_validation_failed = -410;

		/** @brief Required field is missing */
		constexpr int missing_required_field = -411;

		/** @brief Constraint was violated */
		constexpr int constraint_violated = -412;

		/** @brief Type constraint not satisfied */
		constexpr int type_constraint_violated = -413;

		/** @brief Maximum size exceeded */
		constexpr int max_size_exceeded = -414;

		// =========================================================================
		// Serialization (-420 to -429)
		// =========================================================================

		/** @brief Serialization operation failed
		 *  (aligned with kcenon::common::error::codes::container_system::serialization_failed) */
		constexpr int serialization_failed = -420;

		/** @brief Deserialization operation failed
		 *  (aligned with kcenon::common::error::codes::container_system::deserialization_failed) */
		constexpr int deserialization_failed = -421;

		/** @brief Data format is invalid or unrecognized
		 *  (aligned with kcenon::common::error::codes::container_system::invalid_format) */
		constexpr int invalid_format = -422;

		/** @brief Data version does not match expected version */
		constexpr int version_mismatch = -423;

		/** @brief Data is corrupted or incomplete */
		constexpr int corrupted_data = -424;

		/** @brief Header parsing failed */
		constexpr int header_parse_failed = -425;

		/** @brief Value parsing failed */
		constexpr int value_parse_failed = -426;

		/** @brief Encoding/decoding error (e.g., invalid UTF-8) */
		constexpr int encoding_error = -427;

		// =========================================================================
		// Resource (-430 to -439)
		// =========================================================================

		/** @brief Memory allocation failed */
		constexpr int memory_allocation_failed = -430;

		/** @brief File not found */
		constexpr int file_not_found = -431;

		/** @brief File read error */
		constexpr int file_read_error = -432;

		/** @brief File write error */
		constexpr int file_write_error = -433;

		/** @brief Permission denied */
		constexpr int permission_denied = -434;

		/** @brief Resource exhausted */
		constexpr int resource_exhausted = -435;

		/** @brief I/O operation failed */
		constexpr int io_error = -436;

		// =========================================================================
		// Thread safety (-440 to -449)
		// =========================================================================

		/** @brief Lock acquisition failed */
		constexpr int lock_acquisition_failed = -440;

		/** @brief Concurrent modification detected */
		constexpr int concurrent_modification = -441;

		/** @brief Deadlock detected or timeout */
		constexpr int lock_timeout = -442;

		// =========================================================================
		// Error message utilities
		// =========================================================================

		/**
		 * @brief Get human-readable message for an error code
		 * @param code The error code
		 * @return String view containing the error message
		 */
		constexpr std::string_view get_message(int code) noexcept
		{
			switch (code)
			{
				// Value operations (-400 to -409)
				case key_not_found: return "Key not found";
				case type_mismatch: return "Type mismatch";
				case value_out_of_range: return "Value out of range";
				case invalid_value: return "Invalid value";
				case key_already_exists: return "Key already exists";
				case empty_key: return "Empty key";

				// Validation (-410 to -419)
				case schema_validation_failed: return "Schema validation failed";
				case missing_required_field: return "Missing required field";
				case constraint_violated: return "Constraint violated";
				case type_constraint_violated: return "Type constraint violated";
				case max_size_exceeded: return "Maximum size exceeded";

				// Serialization (-420 to -429)
				case serialization_failed: return "Serialization failed";
				case deserialization_failed: return "Deserialization failed";
				case invalid_format: return "Invalid format";
				case version_mismatch: return "Version mismatch";
				case corrupted_data: return "Corrupted data";
				case header_parse_failed: return "Header parse failed";
				case value_parse_failed: return "Value parse failed";
				case encoding_error: return "Encoding error";

				// Resource (-430 to -439)
				case memory_allocation_failed: return "Memory allocation failed";
				case file_not_found: return "File not found";
				case file_read_error: return "File read error";
				case file_write_error: return "File write error";
				case permission_denied: return "Permission denied";
				case resource_exhausted: return "Resource exhausted";
				case io_error: return "I/O error";

				// Thread safety (-440 to -449)
				case lock_acquisition_failed: return "Lock acquisition failed";
				case concurrent_modification: return "Concurrent modification";
				case lock_timeout: return "Lock timeout";

				default: return "Unknown error";
			}
		}

		/**
		 * @brief Get the category name for an error code
		 * @param code The error code
		 * @return String view containing the category name
		 *
		 * @note Sub-band boundaries are inclusive on the band start (e.g. -400)
		 *       and exclusive on the next band start (e.g. -410). For negatives
		 *       this means `code <= -400 && code > -410` selects [-409, -400].
		 */
		constexpr std::string_view get_category(int code) noexcept
		{
			if (code <= -400 && code > -410)
			{
				return "value_operation";
			}
			if (code <= -410 && code > -420)
			{
				return "validation";
			}
			if (code <= -420 && code > -430)
			{
				return "serialization";
			}
			if (code <= -430 && code > -440)
			{
				return "resource";
			}
			if (code <= -440 && code > -450)
			{
				return "thread_safety";
			}
			return "unknown";
		}

		/**
		 * @brief Check if error code is in a specific category
		 * @param code The error code
		 * @param category_base The category base (-400, -410, -420, -430, -440)
		 * @return true if error code belongs to the 10-wide sub-band
		 *
		 * @note For negative bands the band covers [category_base - 9, category_base],
		 *       i.e. `code <= category_base && code > category_base - 10`.
		 */
		constexpr bool is_category(int code, int category_base) noexcept
		{
			return code <= category_base && code > category_base - 10;
		}

		/**
		 * @brief Check if error is a value operation error (-400 to -409)
		 */
		constexpr bool is_value_error(int code) noexcept
		{
			return is_category(code, -400);
		}

		/**
		 * @brief Check if error is a validation error (-410 to -419)
		 */
		constexpr bool is_validation_error(int code) noexcept
		{
			return is_category(code, -410);
		}

		/**
		 * @brief Check if error is a serialization error (-420 to -429)
		 */
		constexpr bool is_serialization_error(int code) noexcept
		{
			return is_category(code, -420);
		}

		/**
		 * @brief Check if error is a resource error (-430 to -439)
		 */
		constexpr bool is_resource_error(int code) noexcept
		{
			return is_category(code, -430);
		}

		/**
		 * @brief Check if error is a thread safety error (-440 to -449)
		 */
		constexpr bool is_thread_error(int code) noexcept
		{
			return is_category(code, -440);
		}

		/**
		 * @brief Build detailed error message with context
		 * @param code The error code
		 * @param detail Additional context (e.g., key name, file path)
		 * @return Formatted error message string
		 */
		inline std::string make_message(int code, std::string_view detail = "")
		{
			std::string msg{get_message(code)};
			if (!detail.empty())
			{
				msg += ": ";
				msg += detail;
			}
			return msg;
		}

	} // namespace error_codes

} // namespace kcenon::container
