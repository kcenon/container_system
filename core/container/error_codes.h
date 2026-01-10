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
 * @file core/container/error_codes.h
 * @brief Standardized error codes for container_module Result<T> pattern
 *
 * This header defines comprehensive error codes organized by category:
 * - Value operations (1xx): key_not_found, type_mismatch, etc.
 * - Serialization (2xx): serialization_failed, deserialization_failed, etc.
 * - Validation (3xx): schema_validation_failed, constraint_violated, etc.
 * - Resource (4xx): memory_allocation_failed, file errors, etc.
 * - Thread safety (5xx): lock_acquisition_failed, concurrent_modification
 *
 * @see container_module::value_container
 * @since 2.0.0
 */

#pragma once

#include <string_view>
#include <string>

namespace container_module
{
	/**
	 * @brief Standardized error codes for container operations
	 *
	 * Error codes are organized into categories by hundreds digit:
	 * - 1xx: Value operations
	 * - 2xx: Serialization
	 * - 3xx: Validation
	 * - 4xx: Resource
	 * - 5xx: Thread safety
	 */
	namespace error_codes
	{
		// =========================================================================
		// Value operations (1xx)
		// =========================================================================

		/** @brief Requested key does not exist in the container */
		constexpr int key_not_found = 100;

		/** @brief Value type does not match the requested type */
		constexpr int type_mismatch = 101;

		/** @brief Numeric value is outside the valid range */
		constexpr int value_out_of_range = 102;

		/** @brief Value is invalid for the operation */
		constexpr int invalid_value = 103;

		/** @brief Key already exists when unique key required */
		constexpr int key_already_exists = 104;

		/** @brief Empty key name provided */
		constexpr int empty_key = 105;

		// =========================================================================
		// Serialization (2xx)
		// =========================================================================

		/** @brief Serialization operation failed */
		constexpr int serialization_failed = 200;

		/** @brief Deserialization operation failed */
		constexpr int deserialization_failed = 201;

		/** @brief Data format is invalid or unrecognized */
		constexpr int invalid_format = 202;

		/** @brief Data version does not match expected version */
		constexpr int version_mismatch = 203;

		/** @brief Data is corrupted or incomplete */
		constexpr int corrupted_data = 204;

		/** @brief Header parsing failed */
		constexpr int header_parse_failed = 205;

		/** @brief Value parsing failed */
		constexpr int value_parse_failed = 206;

		/** @brief Encoding/decoding error (e.g., invalid UTF-8) */
		constexpr int encoding_error = 207;

		// =========================================================================
		// Validation (3xx)
		// =========================================================================

		/** @brief Schema validation failed */
		constexpr int schema_validation_failed = 300;

		/** @brief Required field is missing */
		constexpr int missing_required_field = 301;

		/** @brief Constraint was violated */
		constexpr int constraint_violated = 302;

		/** @brief Type constraint not satisfied */
		constexpr int type_constraint_violated = 303;

		/** @brief Maximum size exceeded */
		constexpr int max_size_exceeded = 304;

		// =========================================================================
		// Resource (4xx)
		// =========================================================================

		/** @brief Memory allocation failed */
		constexpr int memory_allocation_failed = 400;

		/** @brief File not found */
		constexpr int file_not_found = 401;

		/** @brief File read error */
		constexpr int file_read_error = 402;

		/** @brief File write error */
		constexpr int file_write_error = 403;

		/** @brief Permission denied */
		constexpr int permission_denied = 404;

		/** @brief Resource exhausted */
		constexpr int resource_exhausted = 405;

		/** @brief I/O operation failed */
		constexpr int io_error = 406;

		// =========================================================================
		// Thread safety (5xx)
		// =========================================================================

		/** @brief Lock acquisition failed */
		constexpr int lock_acquisition_failed = 500;

		/** @brief Concurrent modification detected */
		constexpr int concurrent_modification = 501;

		/** @brief Deadlock detected or timeout */
		constexpr int lock_timeout = 502;

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
				// Value operations (1xx)
				case key_not_found: return "Key not found";
				case type_mismatch: return "Type mismatch";
				case value_out_of_range: return "Value out of range";
				case invalid_value: return "Invalid value";
				case key_already_exists: return "Key already exists";
				case empty_key: return "Empty key";

				// Serialization (2xx)
				case serialization_failed: return "Serialization failed";
				case deserialization_failed: return "Deserialization failed";
				case invalid_format: return "Invalid format";
				case version_mismatch: return "Version mismatch";
				case corrupted_data: return "Corrupted data";
				case header_parse_failed: return "Header parse failed";
				case value_parse_failed: return "Value parse failed";
				case encoding_error: return "Encoding error";

				// Validation (3xx)
				case schema_validation_failed: return "Schema validation failed";
				case missing_required_field: return "Missing required field";
				case constraint_violated: return "Constraint violated";
				case type_constraint_violated: return "Type constraint violated";
				case max_size_exceeded: return "Maximum size exceeded";

				// Resource (4xx)
				case memory_allocation_failed: return "Memory allocation failed";
				case file_not_found: return "File not found";
				case file_read_error: return "File read error";
				case file_write_error: return "File write error";
				case permission_denied: return "Permission denied";
				case resource_exhausted: return "Resource exhausted";
				case io_error: return "I/O error";

				// Thread safety (5xx)
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
		 */
		constexpr std::string_view get_category(int code) noexcept
		{
			if (code >= 100 && code < 200)
			{
				return "value_operation";
			}
			if (code >= 200 && code < 300)
			{
				return "serialization";
			}
			if (code >= 300 && code < 400)
			{
				return "validation";
			}
			if (code >= 400 && code < 500)
			{
				return "resource";
			}
			if (code >= 500 && code < 600)
			{
				return "thread_safety";
			}
			return "unknown";
		}

		/**
		 * @brief Check if error code is in a specific category
		 * @param code The error code
		 * @param category_base The category base (100, 200, 300, 400, 500)
		 * @return true if error code belongs to the category
		 */
		constexpr bool is_category(int code, int category_base) noexcept
		{
			return code >= category_base && code < category_base + 100;
		}

		/**
		 * @brief Check if error is a value operation error (1xx)
		 */
		constexpr bool is_value_error(int code) noexcept
		{
			return is_category(code, 100);
		}

		/**
		 * @brief Check if error is a serialization error (2xx)
		 */
		constexpr bool is_serialization_error(int code) noexcept
		{
			return is_category(code, 200);
		}

		/**
		 * @brief Check if error is a validation error (3xx)
		 */
		constexpr bool is_validation_error(int code) noexcept
		{
			return is_category(code, 300);
		}

		/**
		 * @brief Check if error is a resource error (4xx)
		 */
		constexpr bool is_resource_error(int code) noexcept
		{
			return is_category(code, 400);
		}

		/**
		 * @brief Check if error is a thread safety error (5xx)
		 */
		constexpr bool is_thread_error(int code) noexcept
		{
			return is_category(code, 500);
		}

		/**
		 * @brief Build detailed error message with context
		 * @param code The error code
		 * @param detail Additional context (e.g., key name, file path)
		 * @return Formatted error message string
		 */
		std::string make_message(int code, std::string_view detail = "")
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

} // namespace container_module
