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
 * @file core/container_schema.cpp
 * @brief Implementation of container_schema class
 */

#include "core/container/schema.h"
#include "core/container.h"

#include <algorithm>
#include <cmath>

namespace container_module
{
	// =========================================================================
	// Field Definition API
	// =========================================================================

	container_schema& container_schema::require(std::string_view key, value_types type)
	{
		field_def field(key, type, true);
		fields_.push_back(std::move(field));
		return *this;
	}

	container_schema& container_schema::optional(std::string_view key, value_types type)
	{
		field_def field(key, type, false);
		fields_.push_back(std::move(field));
		return *this;
	}

	container_schema& container_schema::require(std::string_view key, value_types type,
												const container_schema& nested_schema)
	{
		field_def field(key, type, true);
		field.nested_schema = std::make_unique<container_schema>(nested_schema);
		fields_.push_back(std::move(field));
		return *this;
	}

	container_schema& container_schema::optional(std::string_view key, value_types type,
												 const container_schema& nested_schema)
	{
		field_def field(key, type, false);
		field.nested_schema = std::make_unique<container_schema>(nested_schema);
		fields_.push_back(std::move(field));
		return *this;
	}

	// =========================================================================
	// Constraint API
	// =========================================================================

	container_schema& container_schema::range_int64(std::string_view key, int64_t min, int64_t max)
	{
		if (auto* field = find_field(key))
		{
			field->min_int = min;
			field->max_int = max;
		}
		return *this;
	}

	container_schema& container_schema::range_double(std::string_view key, double min, double max)
	{
		if (auto* field = find_field(key))
		{
			field->min_double = min;
			field->max_double = max;
		}
		return *this;
	}

	container_schema& container_schema::length(std::string_view key, size_t min, size_t max)
	{
		if (auto* field = find_field(key))
		{
			field->min_length = min;
			field->max_length = max;
		}
		return *this;
	}

	container_schema& container_schema::pattern(std::string_view key, std::string_view regex_pattern)
	{
		if (auto* field = find_field(key))
		{
			field->pattern_str = std::string(regex_pattern);
			try
			{
				field->compiled_pattern = std::regex(std::string(regex_pattern));
			}
			catch (const std::regex_error&)
			{
				// Invalid regex pattern - will fail validation
				field->compiled_pattern = std::nullopt;
			}
		}
		return *this;
	}

	container_schema& container_schema::one_of(std::string_view key, std::vector<std::string> allowed)
	{
		if (auto* field = find_field(key))
		{
			field->allowed_values = std::move(allowed);
		}
		return *this;
	}

	container_schema& container_schema::custom(std::string_view key, validator_fn validator)
	{
		if (auto* field = find_field(key))
		{
			field->custom_validators.push_back(std::move(validator));
		}
		return *this;
	}

	// =========================================================================
	// Validation API
	// =========================================================================

	std::optional<validation_error> container_schema::validate(
		const value_container& container) const noexcept
	{
		auto errors = validate_all(container);
		if (errors.empty())
		{
			return std::nullopt;
		}
		return errors.front();
	}

	std::vector<validation_error> container_schema::validate_all(
		const value_container& container) const noexcept
	{
		std::vector<validation_error> errors;

		// Check each field definition
		for (const auto& field : fields_)
		{
			auto value_opt = container.get_value(field.name);

			if (!value_opt.has_value())
			{
				// Field not found
				if (field.required)
				{
					errors.push_back(validation_error::missing_required(field.name));
				}
				continue;
			}

			// Field exists - validate it
			validate_field(field, value_opt.value(), errors);
		}

		return errors;
	}

#if SCHEMA_HAS_COMMON_RESULT
	kcenon::common::VoidResult container_schema::validate_result(
		const value_container& container) const noexcept
	{
		auto error_opt = validate(container);
		if (!error_opt.has_value())
		{
			return kcenon::common::ok();
		}

		const auto& err = error_opt.value();
		return kcenon::common::VoidResult(
			kcenon::common::error_info{
				err.code,
				err.message,
				"container_schema"});
	}
#endif

	bool container_schema::has_field(std::string_view key) const noexcept
	{
		return find_field(key) != nullptr;
	}

	bool container_schema::is_required(std::string_view key) const noexcept
	{
		const auto* field = find_field(key);
		return field != nullptr && field->required;
	}

	// =========================================================================
	// Private Implementation
	// =========================================================================

	container_schema::field_def* container_schema::find_field(std::string_view key) noexcept
	{
		auto it = std::find_if(fields_.begin(), fields_.end(),
			[key](const field_def& f) { return f.name == key; });
		return it != fields_.end() ? &(*it) : nullptr;
	}

	const container_schema::field_def* container_schema::find_field(std::string_view key) const noexcept
	{
		auto it = std::find_if(fields_.begin(), fields_.end(),
			[key](const field_def& f) { return f.name == key; });
		return it != fields_.end() ? &(*it) : nullptr;
	}

	bool container_schema::validate_field(const field_def& field,
										  const optimized_value& value,
										  std::vector<validation_error>& errors) const noexcept
	{
		bool valid = true;

		// Type validation
		if (!validate_type(field, value, errors))
		{
			valid = false;
		}

		// Range validation
		if (!validate_range(field, value, errors))
		{
			valid = false;
		}

		// Length validation
		if (!validate_length(field, value, errors))
		{
			valid = false;
		}

		// Pattern validation
		if (!validate_pattern(field, value, errors))
		{
			valid = false;
		}

		// Allowed values validation
		if (!validate_allowed(field, value, errors))
		{
			valid = false;
		}

		// Custom validation
		if (!validate_custom(field, value, errors))
		{
			valid = false;
		}

		// Nested schema validation
		if (!validate_nested(field, value, errors))
		{
			valid = false;
		}

		return valid;
	}

	bool container_schema::validate_type(const field_def& field,
										 const optimized_value& value,
										 std::vector<validation_error>& errors) const noexcept
	{
		if (value.type != field.type)
		{
			errors.push_back(validation_error::type_mismatch(
				field.name, field.type, value.type));
			return false;
		}
		return true;
	}

	bool container_schema::validate_range(const field_def& field,
										  const optimized_value& value,
										  std::vector<validation_error>& errors) const noexcept
	{
		// Integer range check
		if (field.min_int.has_value() && field.max_int.has_value())
		{
			int64_t val = 0;
			bool has_value = false;

			switch (value.type)
			{
				case value_types::short_value:
					if (auto* p = std::get_if<short>(&value.data))
					{
						val = *p;
						has_value = true;
					}
					break;
				case value_types::ushort_value:
					if (auto* p = std::get_if<unsigned short>(&value.data))
					{
						val = *p;
						has_value = true;
					}
					break;
				case value_types::int_value:
					if (auto* p = std::get_if<int>(&value.data))
					{
						val = *p;
						has_value = true;
					}
					break;
				case value_types::uint_value:
					if (auto* p = std::get_if<unsigned int>(&value.data))
					{
						val = *p;
						has_value = true;
					}
					break;
				case value_types::long_value:
					if (auto* p = std::get_if<long>(&value.data))
					{
						val = *p;
						has_value = true;
					}
					break;
				case value_types::ulong_value:
					if (auto* p = std::get_if<unsigned long>(&value.data))
					{
						val = static_cast<int64_t>(*p);
						has_value = true;
					}
					break;
				case value_types::llong_value:
					if (auto* p = std::get_if<long long>(&value.data))
					{
						val = *p;
						has_value = true;
					}
					break;
				case value_types::ullong_value:
					if (auto* p = std::get_if<unsigned long long>(&value.data))
					{
						val = static_cast<int64_t>(*p);
						has_value = true;
					}
					break;
				default:
					break;
			}

			if (has_value)
			{
				if (val < field.min_int.value() || val > field.max_int.value())
				{
					errors.push_back(validation_error::out_of_range(
						field.name, val, field.min_int.value(), field.max_int.value()));
					return false;
				}
			}
		}

		// Double range check
		if (field.min_double.has_value() && field.max_double.has_value())
		{
			double val = 0.0;
			bool has_value = false;

			if (value.type == value_types::float_value)
			{
				if (auto* p = std::get_if<float>(&value.data))
				{
					val = static_cast<double>(*p);
					has_value = true;
				}
			}
			else if (value.type == value_types::double_value)
			{
				if (auto* p = std::get_if<double>(&value.data))
				{
					val = *p;
					has_value = true;
				}
			}

			if (has_value)
			{
				if (val < field.min_double.value() || val > field.max_double.value())
				{
					errors.push_back(validation_error::out_of_range(
						field.name, val, field.min_double.value(), field.max_double.value()));
					return false;
				}
			}
		}

		return true;
	}

	bool container_schema::validate_length(const field_def& field,
										   const optimized_value& value,
										   std::vector<validation_error>& errors) const noexcept
	{
		if (!field.min_length.has_value() || !field.max_length.has_value())
		{
			return true;
		}

		size_t len = 0;
		bool has_length = false;

		if (value.type == value_types::string_value)
		{
			if (auto* p = std::get_if<std::string>(&value.data))
			{
				len = p->length();
				has_length = true;
			}
		}
		else if (value.type == value_types::bytes_value)
		{
			if (auto* p = std::get_if<std::vector<uint8_t>>(&value.data))
			{
				len = p->size();
				has_length = true;
			}
		}

		if (has_length)
		{
			if (len < field.min_length.value() || len > field.max_length.value())
			{
				errors.push_back(validation_error::invalid_length(
					field.name, len, field.min_length.value(), field.max_length.value()));
				return false;
			}
		}

		return true;
	}

	bool container_schema::validate_pattern(const field_def& field,
											const optimized_value& value,
											std::vector<validation_error>& errors) const noexcept
	{
		if (!field.compiled_pattern.has_value())
		{
			return true;
		}

		if (value.type != value_types::string_value)
		{
			return true;
		}

		auto* str_ptr = std::get_if<std::string>(&value.data);
		if (!str_ptr)
		{
			return true;
		}

		try
		{
			if (!std::regex_match(*str_ptr, field.compiled_pattern.value()))
			{
				errors.push_back(validation_error::pattern_mismatch(
					field.name, field.pattern_str.value_or("")));
				return false;
			}
		}
		catch (const std::regex_error&)
		{
			// Regex matching failed - treat as pattern mismatch
			errors.push_back(validation_error::pattern_mismatch(
				field.name, field.pattern_str.value_or("")));
			return false;
		}

		return true;
	}

	bool container_schema::validate_allowed(const field_def& field,
											const optimized_value& value,
											std::vector<validation_error>& errors) const noexcept
	{
		if (!field.allowed_values.has_value())
		{
			return true;
		}

		if (value.type != value_types::string_value)
		{
			return true;
		}

		auto* str_ptr = std::get_if<std::string>(&value.data);
		if (!str_ptr)
		{
			return true;
		}

		const auto& allowed = field.allowed_values.value();
		auto it = std::find(allowed.begin(), allowed.end(), *str_ptr);
		if (it == allowed.end())
		{
			errors.push_back(validation_error::not_allowed(field.name, *str_ptr));
			return false;
		}

		return true;
	}

	bool container_schema::validate_custom(const field_def& field,
										   const optimized_value& value,
										   std::vector<validation_error>& errors) const noexcept
	{
		for (const auto& validator : field.custom_validators)
		{
			try
			{
				auto result = validator(value);
				if (result.has_value())
				{
					errors.push_back(validation_error::custom_failed(
						field.name, result.value()));
					return false;
				}
			}
			catch (...)
			{
				errors.push_back(validation_error::custom_failed(
					field.name, "Validator threw an exception"));
				return false;
			}
		}
		return true;
	}

	bool container_schema::validate_nested(const field_def& field,
										   const optimized_value& value,
										   std::vector<validation_error>& errors) const noexcept
	{
		if (!field.nested_schema)
		{
			return true;
		}

		if (value.type != value_types::container_value)
		{
			return true;
		}

		const auto* container_ptr = std::get_if<std::shared_ptr<value_container>>(&value.data);
		if (container_ptr == nullptr || !(*container_ptr))
		{
			errors.push_back(validation_error::nested_failed(field.name, {}));
			return false;
		}

		auto nested_errors = field.nested_schema->validate_all(**container_ptr);
		if (!nested_errors.empty())
		{
			errors.push_back(validation_error::nested_failed(field.name, nested_errors));
			// Also append all nested errors with prefixed field names
			for (auto& err : nested_errors)
			{
				err.field = field.name + "." + err.field;
				errors.push_back(std::move(err));
			}
			return false;
		}

		return true;
	}

} // namespace container_module
