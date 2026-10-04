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

#include <kcenon/container/error_codes.h>

#include <iostream>
#include <string>

using namespace kcenon::container;

int main()
{
	std::cout << "=== Error Handling Example ===" << std::endl;

	// 1. Error code message lookup
	std::cout << "\n1. Error code messages:" << std::endl;
	std::cout << "   type_mismatch: " << error_codes::get_message(error_codes::type_mismatch)
			  << std::endl;
	std::cout << "   deserialization_failed: "
			  << error_codes::get_message(error_codes::deserialization_failed) << std::endl;
	std::cout << "   schema_validation_failed: "
			  << error_codes::get_message(error_codes::schema_validation_failed) << std::endl;

	// 2. Error categories
	std::cout << "\n2. Error categories:" << std::endl;
	std::cout << "   type_mismatch is value error: "
			  << (error_codes::is_value_error(error_codes::type_mismatch) ? "yes" : "no")
			  << std::endl;
	std::cout << "   deserialization_failed is serialization: "
			  << (error_codes::is_serialization_error(error_codes::deserialization_failed) ? "yes" : "no")
			  << std::endl;
	std::cout << "   schema_validation_failed is validation: "
			  << (error_codes::is_validation_error(error_codes::schema_validation_failed) ? "yes" : "no")
			  << std::endl;
	std::cout << "   file_not_found is resource: "
			  << (error_codes::is_resource_error(error_codes::file_not_found) ? "yes" : "no")
			  << std::endl;
	std::cout << "   lock_timeout is thread: "
			  << (error_codes::is_thread_error(error_codes::lock_timeout) ? "yes" : "no")
			  << std::endl;

	// 3. Category lookup
	std::cout << "\n3. Category names:" << std::endl;
	std::cout << "   type_mismatch category: "
			  << error_codes::get_category(error_codes::type_mismatch) << std::endl;
	std::cout << "   deserialization_failed category: "
			  << error_codes::get_category(error_codes::deserialization_failed) << std::endl;
	std::cout << "   schema_validation_failed category: "
			  << error_codes::get_category(error_codes::schema_validation_failed) << std::endl;

	// 4. Formatted error messages
	std::cout << "\n4. Formatted messages:" << std::endl;
	auto msg = error_codes::make_message(error_codes::type_mismatch, "username");
	std::cout << "   " << msg << std::endl;

	std::cout << "\nDone." << std::endl;
	return 0;
}
