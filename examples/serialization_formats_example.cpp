// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/// @file serialization_formats_example.cpp
/// @example serialization_formats_example.cpp
/// @brief Compares binary, JSON, and XML serialization strategies.
///
/// Demonstrates serializer_factory, format auto-detection, and
/// roundtrip serialization with size comparison.
///
/// @see kcenon::container::serializer_factory

#include "container.h"

#include <kcenon/container/serializers/serializer_factory.h>
#include <kcenon/container/serializers/binary_serializer.h>
#include <kcenon/container/serializers/json_serializer.h>

#include <iostream>
#include <string>

using namespace kcenon::container;

int main()
{
	std::cout << "=== Serialization Formats Example ===" << std::endl;

	// Create a container with sample data
	value_container container;
	container.add("name", "Alice");
	container.add("age", static_cast<int64_t>(30));
	container.add("score", 95.5);
	container.add("active", true);

	std::cout << "\n1. Original container:" << std::endl;
	std::cout << "   Fields: name, age, score, active" << std::endl;

	// Binary serialization
	std::cout << "\n2. Binary serialization:" << std::endl;
	auto binary = serializer_factory::create(serialization_format::binary);
	if (binary)
	{
		auto data = binary->serialize(container);
		std::cout << "   Size: " << data.size() << " bytes" << std::endl;
		std::cout << "   Format: " << binary->name() << std::endl;
	}

	// JSON serialization
	std::cout << "\n3. JSON serialization:" << std::endl;
	auto json = serializer_factory::create(serialization_format::json);
	if (json)
	{
		auto str = json->serialize_to_string(container);
		std::cout << "   Size: " << str.size() << " bytes" << std::endl;
		std::cout << "   Preview: " << str.substr(0, 80) << "..." << std::endl;
	}

	// Format support check
	std::cout << "\n4. Format support:" << std::endl;
	std::cout << "   Binary: "
			  << (serializer_factory::is_supported(serialization_format::binary) ? "yes" : "no")
			  << std::endl;
	std::cout << "   JSON: "
			  << (serializer_factory::is_supported(serialization_format::json) ? "yes" : "no")
			  << std::endl;

	std::cout << "\nDone." << std::endl;
	return 0;
}
