// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/// @file error_handling_example.cpp
/// @example error_handling_example.cpp
/// @brief Demonstrates container error codes and error handling.
///
/// Shows error code categories, message lookup, and integration
/// with Result<T> pattern from common_system.
///
/// @see kcenon::container::error_codes

#include "container.h"

#include <kcenon/container/container/error_codes.h>

#include <iostream>
#include <string>

using namespace kcenon::container;

int main()
{
	std::cout << "=== Error Handling Example ===" << std::endl;

	// 1. Error code message lookup
	std::cout << "\n1. Error code messages:" << std::endl;
	std::cout << "   Code 101: " << error_codes::get_message(101) << std::endl;
	std::cout << "   Code 201: " << error_codes::get_message(201) << std::endl;
	std::cout << "   Code 301: " << error_codes::get_message(301) << std::endl;

	// 2. Error categories
	std::cout << "\n2. Error categories:" << std::endl;
	std::cout << "   101 is value error: " << (error_codes::is_value_error(101) ? "yes" : "no")
			  << std::endl;
	std::cout << "   201 is serialization: "
			  << (error_codes::is_serialization_error(201) ? "yes" : "no") << std::endl;
	std::cout << "   301 is validation: " << (error_codes::is_validation_error(301) ? "yes" : "no")
			  << std::endl;
	std::cout << "   401 is resource: " << (error_codes::is_resource_error(401) ? "yes" : "no")
			  << std::endl;
	std::cout << "   501 is thread: " << (error_codes::is_thread_error(501) ? "yes" : "no")
			  << std::endl;

	// 3. Category lookup
	std::cout << "\n3. Category names:" << std::endl;
	std::cout << "   Code 101 category: " << error_codes::get_category(101) << std::endl;
	std::cout << "   Code 201 category: " << error_codes::get_category(201) << std::endl;
	std::cout << "   Code 301 category: " << error_codes::get_category(301) << std::endl;

	// 4. Formatted error messages
	std::cout << "\n4. Formatted messages:" << std::endl;
	auto msg = error_codes::make_message(101, "username");
	std::cout << "   " << msg << std::endl;

	std::cout << "\nDone." << std::endl;
	return 0;
}
