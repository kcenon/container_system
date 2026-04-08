// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/// @file schema_validation_example.cpp
/// @example schema_validation_example.cpp
/// @brief Demonstrates container schema validation with constraints.
///
/// Shows defining required/optional fields, range/length/pattern
/// validators, and validating containers against schemas.
///
/// @see kcenon::container::container_schema

#include "container.h"

#include <kcenon/container/container/schema.h>

#include <iostream>
#include <string>

using namespace kcenon::container;

int main()
{
	std::cout << "=== Schema Validation Example ===" << std::endl;

	// 1. Define a schema
	std::cout << "\n1. Defining schema:" << std::endl;
	container_schema schema;
	schema.require("username")
		.length(3, 20);
	schema.require("email");
	schema.require("age")
		.range(0, 150);
	schema.optional("nickname");

	std::cout << "   Fields: " << schema.field_count() << std::endl;
	std::cout << "   'username' required: " << (schema.is_required("username") ? "yes" : "no")
			  << std::endl;
	std::cout << "   'nickname' required: " << (schema.is_required("nickname") ? "yes" : "no")
			  << std::endl;

	// 2. Validate a valid container
	std::cout << "\n2. Validating valid container:" << std::endl;
	value_container valid;
	valid.add("username", "alice");
	valid.add("email", "alice@example.com");
	valid.add("age", static_cast<int64_t>(25));

	auto result = schema.validate(valid);
	std::cout << "   Valid: " << (result ? "yes" : "no") << std::endl;

	// 3. Validate an invalid container (missing required field)
	std::cout << "\n3. Validating invalid container (missing email):" << std::endl;
	value_container invalid;
	invalid.add("username", "bob");
	invalid.add("age", static_cast<int64_t>(30));

	auto errors = schema.validate_all(invalid);
	std::cout << "   Errors: " << errors.size() << std::endl;
	for (const auto& err : errors)
	{
		std::cout << "   - " << err.field << ": " << err.message << std::endl;
	}

	// 4. Schema inspection
	std::cout << "\n4. Schema inspection:" << std::endl;
	std::cout << "   Has 'username': " << (schema.has_field("username") ? "yes" : "no")
			  << std::endl;
	std::cout << "   Has 'address': " << (schema.has_field("address") ? "yes" : "no")
			  << std::endl;

	std::cout << "\nDone." << std::endl;
	return 0;
}
