// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file value_types.h
 * @brief Enumeration of the 16 supported value types in the container system.
 *
 * Defines value_types enum and compile-time type name mapping used by
 * value_store, serializers, and container schema validation.
 *
 * @see value_store.h For the storage that uses these types
 */

#pragma once

#include <array>
#include <string>
#include <string_view>

namespace kcenon::container
{
	/**
	 * @brief Enumeration of available value types in the container system.
	 */
	enum class value_types
	{
		null_value,
		bool_value,
		short_value,
		ushort_value,
		int_value,
		uint_value,
		long_value,
		ulong_value,
		llong_value,
		ullong_value,
		float_value,
		double_value,
		string_value,
		bytes_value,
		container_value,
		array_value
	};

	// Compile-time type mapping
	constexpr std::array<std::pair<std::string_view, value_types>, 16> type_map{
		{{"0", value_types::null_value},
		 {"1", value_types::bool_value},
		 {"2", value_types::short_value},
		 {"3", value_types::ushort_value},
		 {"4", value_types::int_value},
		 {"5", value_types::uint_value},
		 {"6", value_types::long_value},
		 {"7", value_types::ulong_value},
		 {"8", value_types::llong_value},
		 {"9", value_types::ullong_value},
		 {"10", value_types::float_value},
		 {"11", value_types::double_value},
		 {"12", value_types::string_value},
		 {"13", value_types::bytes_value},
		 {"14", value_types::container_value},
		 {"15", value_types::array_value}}};

	/**
	 * @brief Compile-time conversion from string to value_types
	 */
	constexpr value_types get_type_from_string(std::string_view str) noexcept
	{
		for (const auto& [key, type] : type_map)
		{
			if (key == str)
				return type;
		}
		return value_types::null_value;
	}

	/**
	 * @brief Compile-time conversion from value_types to string
	 */
	constexpr std::string_view get_string_from_type(value_types type) noexcept
	{
		for (const auto& [key, val] : type_map)
		{
			if (val == type)
				return key;
		}
		return "0";
	}

	// Non-constexpr conversion functions (implemented in value_types.cpp)
	value_types convert_value_type(const std::string& target);
	std::string convert_value_type(const value_types& target);

} // namespace kcenon::container
