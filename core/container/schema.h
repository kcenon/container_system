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
 * @file core/container/schema.h
 * @brief Runtime schema validation for container data integrity
 *
 * This header provides schema validation capabilities for value_container,
 * allowing validation of container contents against predefined schemas.
 *
 * @code
 * // Define schema
 * auto user_schema = container_schema()
 *     .require("user_id", value_types::string_value)
 *     .require("age", value_types::int_value)
 *     .range("age", 0, 150)
 *     .optional("phone", value_types::string_value);
 *
 * // Validate container
 * auto result = user_schema.validate(*container);
 * if (!result) {
 *     // Handle validation failure
 * }
 * @endcode
 *
 * @see container_module::value_container
 * @since 2.0.0
 */

#pragma once

#include "container/core/container/types.h"
#include "container/core/container/error_codes.h"
#include "container/core/value_types.h"

#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <functional>
#include <memory>
#include <regex>

// Optional common system integration
#if __has_include(<kcenon/common/config/feature_flags.h>)
	#include <kcenon/common/config/feature_flags.h>
#endif

#if KCENON_HAS_COMMON_SYSTEM
	#if __has_include(<kcenon/common/patterns/result.h>)
		#include <kcenon/common/patterns/result.h>
		#ifndef SCHEMA_HAS_COMMON_RESULT
		#define SCHEMA_HAS_COMMON_RESULT 1
		#endif
	#elif __has_include(<common/patterns/result.h>)
		#include <common/patterns/result.h>
		#ifndef SCHEMA_HAS_COMMON_RESULT
		#define SCHEMA_HAS_COMMON_RESULT 1
		#endif
	#else
		#define SCHEMA_HAS_COMMON_RESULT 0
	#endif
#else
	#define SCHEMA_HAS_COMMON_RESULT 0
#endif

namespace container_module
{
	// Forward declarations
	class value_container;
	class container_schema;

	/**
	 * @brief Error codes specific to schema validation
	 */
	namespace validation_codes
	{
		/** @brief Required field is missing from container */
		constexpr int missing_required = 310;

		/** @brief Field type does not match schema definition */
		constexpr int type_mismatch = 311;

		/** @brief Numeric value is outside specified range */
		constexpr int out_of_range = 312;

		/** @brief String/bytes length is outside specified bounds */
		constexpr int invalid_length = 313;

		/** @brief String does not match specified regex pattern */
		constexpr int pattern_mismatch = 314;

		/** @brief Value is not in the allowed values list */
		constexpr int not_in_allowed_values = 315;

		/** @brief Custom validator returned failure */
		constexpr int custom_validation_failed = 316;

		/** @brief Nested container schema validation failed */
		constexpr int nested_validation_failed = 317;

		/**
		 * @brief Get human-readable message for validation error code
		 * @param code The validation error code
		 * @return String view containing the error message
		 */
		constexpr std::string_view get_message(int code) noexcept
		{
			switch (code)
			{
				case missing_required: return "Required field is missing";
				case type_mismatch: return "Field type mismatch";
				case out_of_range: return "Value is out of range";
				case invalid_length: return "Invalid length";
				case pattern_mismatch: return "Pattern mismatch";
				case not_in_allowed_values: return "Value not in allowed list";
				case custom_validation_failed: return "Custom validation failed";
				case nested_validation_failed: return "Nested schema validation failed";
				default: return error_codes::get_message(code);
			}
		}
	} // namespace validation_codes

	/**
	 * @brief Validation error structure containing detailed error information
	 */
	struct validation_error
	{
		std::string field;        ///< Field name that failed validation
		std::string message;      ///< Human-readable error message
		int code;                 ///< Error code from validation_codes

		validation_error() : code(0) {}

		validation_error(std::string_view f, std::string_view msg, int c)
			: field(f), message(msg), code(c) {}

		/**
		 * @brief Create error for missing required field
		 */
		static validation_error missing_required(std::string_view field_name)
		{
			return validation_error(
				field_name,
				std::string("Required field '") + std::string(field_name) + "' is missing",
				validation_codes::missing_required);
		}

		/**
		 * @brief Create error for type mismatch
		 */
		static validation_error type_mismatch(std::string_view field_name,
											  value_types expected,
											  value_types actual)
		{
			return validation_error(
				field_name,
				std::string("Field '") + std::string(field_name) + "' type mismatch: expected " +
					std::string(get_string_from_type(expected)) + ", got " +
					std::string(get_string_from_type(actual)),
				validation_codes::type_mismatch);
		}

		/**
		 * @brief Create error for out of range value
		 */
		template<typename T>
		static validation_error out_of_range(std::string_view field_name,
											 T value, T min_val, T max_val)
		{
			return validation_error(
				field_name,
				std::string("Field '") + std::string(field_name) + "' value " +
					std::to_string(value) + " is out of range [" +
					std::to_string(min_val) + ", " + std::to_string(max_val) + "]",
				validation_codes::out_of_range);
		}

		/**
		 * @brief Create error for invalid length
		 */
		static validation_error invalid_length(std::string_view field_name,
											   size_t actual, size_t min_len, size_t max_len)
		{
			return validation_error(
				field_name,
				std::string("Field '") + std::string(field_name) + "' length " +
					std::to_string(actual) + " is outside bounds [" +
					std::to_string(min_len) + ", " + std::to_string(max_len) + "]",
				validation_codes::invalid_length);
		}

		/**
		 * @brief Create error for pattern mismatch
		 */
		static validation_error pattern_mismatch(std::string_view field_name,
												 std::string_view pattern)
		{
			return validation_error(
				field_name,
				std::string("Field '") + std::string(field_name) +
					"' does not match pattern '" + std::string(pattern) + "'",
				validation_codes::pattern_mismatch);
		}

		/**
		 * @brief Create error for value not in allowed list
		 */
		static validation_error not_allowed(std::string_view field_name,
											std::string_view value)
		{
			return validation_error(
				field_name,
				std::string("Field '") + std::string(field_name) + "' value '" +
					std::string(value) + "' is not in allowed values",
				validation_codes::not_in_allowed_values);
		}

		/**
		 * @brief Create error for custom validation failure
		 */
		static validation_error custom_failed(std::string_view field_name,
											  std::string_view reason)
		{
			return validation_error(
				field_name,
				std::string("Field '") + std::string(field_name) +
					"' custom validation failed: " + std::string(reason),
				validation_codes::custom_validation_failed);
		}

		/**
		 * @brief Create error for nested validation failure
		 */
		static validation_error nested_failed(std::string_view field_name,
											  const std::vector<validation_error>& nested_errors)
		{
			std::string msg = std::string("Field '") + std::string(field_name) +
				"' nested validation failed with " +
				std::to_string(nested_errors.size()) + " error(s)";
			return validation_error(field_name, msg, validation_codes::nested_validation_failed);
		}
	};

	/**
	 * @brief Custom validator function type
	 *
	 * Returns std::nullopt on success, or an error message on failure.
	 */
	using validator_fn = std::function<std::optional<std::string>(const optimized_value&)>;

	/**
	 * @brief Schema definition for container validation
	 *
	 * Provides a fluent API for defining validation rules including:
	 * - Required and optional fields with type checking
	 * - Numeric range constraints
	 * - String length constraints
	 * - Regex pattern matching
	 * - Enum-style allowed values
	 * - Custom validators
	 * - Nested container schemas
	 *
	 * @code
	 * auto schema = container_schema()
	 *     .require("id", value_types::string_value)
	 *     .require("age", value_types::int_value)
	 *     .range("age", 0, 150)
	 *     .optional("email", value_types::string_value)
	 *     .pattern("email", R"(^[\w\.-]+@[\w\.-]+\.\w+$)");
	 *
	 * auto result = schema.validate(container);
	 * @endcode
	 */
	class container_schema
	{
	public:
		container_schema() = default;
		~container_schema() = default;

		container_schema(const container_schema&) = default;
		container_schema& operator=(const container_schema&) = default;
		container_schema(container_schema&&) noexcept = default;
		container_schema& operator=(container_schema&&) noexcept = default;

		// =====================================================================
		// Field Definition API
		// =====================================================================

		/**
		 * @brief Define a required field
		 * @param key Field name
		 * @param type Expected value type
		 * @return Reference to this schema for chaining
		 */
		container_schema& require(std::string_view key, value_types type);

		/**
		 * @brief Define an optional field
		 * @param key Field name
		 * @param type Expected value type (validated if present)
		 * @return Reference to this schema for chaining
		 */
		container_schema& optional(std::string_view key, value_types type);

		/**
		 * @brief Define a required field with nested schema
		 * @param key Field name
		 * @param type Expected value type (must be container_value)
		 * @param nested_schema Schema for the nested container
		 * @return Reference to this schema for chaining
		 */
		container_schema& require(std::string_view key, value_types type,
								  const container_schema& nested_schema);

		/**
		 * @brief Define an optional field with nested schema
		 * @param key Field name
		 * @param type Expected value type (must be container_value)
		 * @param nested_schema Schema for the nested container
		 * @return Reference to this schema for chaining
		 */
		container_schema& optional(std::string_view key, value_types type,
								   const container_schema& nested_schema);

		// =====================================================================
		// Constraint API
		// =====================================================================

		/**
		 * @brief Add integer range constraint
		 * @param key Field name (must be numeric type)
		 * @param min Minimum value (inclusive)
		 * @param max Maximum value (inclusive)
		 * @return Reference to this schema for chaining
		 */
		container_schema& range(std::string_view key, int64_t min, int64_t max);

		/**
		 * @brief Add floating-point range constraint
		 * @param key Field name (must be float or double type)
		 * @param min Minimum value (inclusive)
		 * @param max Maximum value (inclusive)
		 * @return Reference to this schema for chaining
		 */
		container_schema& range(std::string_view key, double min, double max);

		/**
		 * @brief Add string/bytes length constraint
		 * @param key Field name (must be string or bytes type)
		 * @param min Minimum length (inclusive)
		 * @param max Maximum length (inclusive)
		 * @return Reference to this schema for chaining
		 */
		container_schema& length(std::string_view key, size_t min, size_t max);

		/**
		 * @brief Add regex pattern constraint
		 * @param key Field name (must be string type)
		 * @param regex_pattern Regular expression pattern
		 * @return Reference to this schema for chaining
		 */
		container_schema& pattern(std::string_view key, std::string_view regex_pattern);

		/**
		 * @brief Add allowed values constraint (enum-style)
		 * @param key Field name (must be string type)
		 * @param allowed Vector of allowed string values
		 * @return Reference to this schema for chaining
		 */
		container_schema& one_of(std::string_view key, std::vector<std::string> allowed);

		/**
		 * @brief Add custom validator
		 * @param key Field name
		 * @param validator Custom validation function
		 * @return Reference to this schema for chaining
		 */
		container_schema& custom(std::string_view key, validator_fn validator);

		// =====================================================================
		// Validation API
		// =====================================================================

		/**
		 * @brief Validate a container against this schema
		 * @param container The container to validate
		 * @return std::nullopt on success, or first validation_error on failure
		 * @exception_safety No-throw guarantee
		 *
		 * Stops at the first validation error for efficiency.
		 */
		[[nodiscard]] std::optional<validation_error> validate(
			const value_container& container) const noexcept;

		/**
		 * @brief Validate a container and collect all errors
		 * @param container The container to validate
		 * @return Vector of all validation errors (empty if valid)
		 * @exception_safety No-throw guarantee
		 */
		[[nodiscard]] std::vector<validation_error> validate_all(
			const value_container& container) const noexcept;

#if SCHEMA_HAS_COMMON_RESULT
		/**
		 * @brief Validate with Result return type
		 * @param container The container to validate
		 * @return VoidResult indicating success or error with details
		 * @exception_safety No-throw guarantee
		 */
		[[nodiscard]] kcenon::common::VoidResult validate_result(
			const value_container& container) const noexcept;
#endif

		/**
		 * @brief Get field count in schema
		 * @return Number of fields defined
		 */
		[[nodiscard]] size_t field_count() const noexcept { return fields_.size(); }

		/**
		 * @brief Check if a field is defined in schema
		 * @param key Field name to check
		 * @return true if field is defined
		 */
		[[nodiscard]] bool has_field(std::string_view key) const noexcept;

		/**
		 * @brief Check if a field is required
		 * @param key Field name to check
		 * @return true if field is required, false if optional or not defined
		 */
		[[nodiscard]] bool is_required(std::string_view key) const noexcept;

	private:
		/**
		 * @brief Field definition structure
		 */
		struct field_def
		{
			std::string name;                              ///< Field name
			value_types type;                              ///< Expected type
			bool required;                                 ///< Required or optional

			// Constraints (all optional)
			std::optional<int64_t> min_int;                ///< Integer minimum
			std::optional<int64_t> max_int;                ///< Integer maximum
			std::optional<double> min_double;              ///< Float/double minimum
			std::optional<double> max_double;              ///< Float/double maximum
			std::optional<size_t> min_length;              ///< Minimum string/bytes length
			std::optional<size_t> max_length;              ///< Maximum string/bytes length
			std::optional<std::string> pattern_str;        ///< Regex pattern string
			std::optional<std::regex> compiled_pattern;    ///< Compiled regex
			std::optional<std::vector<std::string>> allowed_values;  ///< Enum values
			std::unique_ptr<container_schema> nested_schema; ///< Nested container schema
			std::vector<validator_fn> custom_validators;   ///< Custom validators

			field_def()
				: name("")
				, type(value_types::null_value)
				, required(false)
			{}

			field_def(std::string_view n, value_types t, bool req)
				: name(n)
				, type(t)
				, required(req)
			{}

			// Copy constructor - deep copy nested_schema
			field_def(const field_def& other)
				: name(other.name)
				, type(other.type)
				, required(other.required)
				, min_int(other.min_int)
				, max_int(other.max_int)
				, min_double(other.min_double)
				, max_double(other.max_double)
				, min_length(other.min_length)
				, max_length(other.max_length)
				, pattern_str(other.pattern_str)
				, compiled_pattern(other.compiled_pattern)
				, allowed_values(other.allowed_values)
				, nested_schema(other.nested_schema ?
					std::make_unique<container_schema>(*other.nested_schema) : nullptr)
				, custom_validators(other.custom_validators)
			{}

			// Copy assignment - deep copy nested_schema
			field_def& operator=(const field_def& other)
			{
				if (this != &other)
				{
					name = other.name;
					type = other.type;
					required = other.required;
					min_int = other.min_int;
					max_int = other.max_int;
					min_double = other.min_double;
					max_double = other.max_double;
					min_length = other.min_length;
					max_length = other.max_length;
					pattern_str = other.pattern_str;
					compiled_pattern = other.compiled_pattern;
					allowed_values = other.allowed_values;
					nested_schema = other.nested_schema ?
						std::make_unique<container_schema>(*other.nested_schema) : nullptr;
					custom_validators = other.custom_validators;
				}
				return *this;
			}

			field_def(field_def&&) noexcept = default;
			field_def& operator=(field_def&&) noexcept = default;
		};

		/**
		 * @brief Find field definition by name
		 * @param key Field name
		 * @return Pointer to field_def or nullptr if not found
		 */
		field_def* find_field(std::string_view key) noexcept;
		const field_def* find_field(std::string_view key) const noexcept;

		/**
		 * @brief Validate a single field
		 * @param field Field definition
		 * @param value Value to validate
		 * @param errors Vector to append errors to
		 * @return true if field is valid
		 */
		bool validate_field(const field_def& field,
							const optimized_value& value,
							std::vector<validation_error>& errors) const noexcept;

		/**
		 * @brief Validate type constraint
		 */
		bool validate_type(const field_def& field,
						   const optimized_value& value,
						   std::vector<validation_error>& errors) const noexcept;

		/**
		 * @brief Validate range constraint
		 */
		bool validate_range(const field_def& field,
							const optimized_value& value,
							std::vector<validation_error>& errors) const noexcept;

		/**
		 * @brief Validate length constraint
		 */
		bool validate_length(const field_def& field,
							 const optimized_value& value,
							 std::vector<validation_error>& errors) const noexcept;

		/**
		 * @brief Validate pattern constraint
		 */
		bool validate_pattern(const field_def& field,
							  const optimized_value& value,
							  std::vector<validation_error>& errors) const noexcept;

		/**
		 * @brief Validate allowed values constraint
		 */
		bool validate_allowed(const field_def& field,
							  const optimized_value& value,
							  std::vector<validation_error>& errors) const noexcept;

		/**
		 * @brief Validate custom validators
		 */
		bool validate_custom(const field_def& field,
							 const optimized_value& value,
							 std::vector<validation_error>& errors) const noexcept;

		/**
		 * @brief Validate nested schema
		 */
		bool validate_nested(const field_def& field,
							 const optimized_value& value,
							 std::vector<validation_error>& errors) const noexcept;

		std::vector<field_def> fields_;  ///< Field definitions
	};

} // namespace container_module
