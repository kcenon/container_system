// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

#include "value_types.h"

namespace kcenon::container
{
	value_types convert_value_type(const std::string& target)
	{
		// Use the constexpr function for better performance
		return get_type_from_string(target);
	}

	std::string convert_value_type(const value_types& target)
	{
		// Use the constexpr function and convert to std::string
		return std::string(get_string_from_type(target));
	}
} // namespace kcenon::container
