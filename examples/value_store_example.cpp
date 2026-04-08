// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/// @file value_store_example.cpp
/// @example value_store_example.cpp
/// @brief Demonstrates thread-safe value_store with statistics.
///
/// Shows CRUD operations, thread-safe access, serialization,
/// and read/write statistics tracking.
///
/// @see kcenon::container::value_store

#include <kcenon/container/value_store.h>

#include <iostream>
#include <string>

using namespace kcenon::container;

int main()
{
	std::cout << "=== Value Store Example ===" << std::endl;

	value_store store;

	// 1. Add values
	std::cout << "\n1. Adding values:" << std::endl;
	store.add("config.host", "localhost");
	store.add("config.port", "8080");
	store.add("config.debug", "true");
	std::cout << "   Size: " << store.size() << std::endl;

	// 2. Get values
	std::cout << "\n2. Reading values:" << std::endl;
	auto host = store.get("config.host");
	std::cout << "   config.host: " << host << std::endl;

	// 3. Check existence
	std::cout << "\n3. Contains check:" << std::endl;
	std::cout << "   'config.host': " << (store.contains("config.host") ? "yes" : "no")
			  << std::endl;
	std::cout << "   'config.missing': " << (store.contains("config.missing") ? "yes" : "no")
			  << std::endl;

	// 4. Remove
	std::cout << "\n4. Remove 'config.debug':" << std::endl;
	store.remove("config.debug");
	std::cout << "   Size after remove: " << store.size() << std::endl;

	// 5. Serialization
	std::cout << "\n5. Serialization:" << std::endl;
	auto json_str = store.serialize();
	std::cout << "   JSON: " << json_str.substr(0, 60) << "..." << std::endl;

	auto binary_data = store.serialize_binary();
	std::cout << "   Binary: " << binary_data.size() << " bytes" << std::endl;

	// 6. Statistics
	std::cout << "\n6. Access statistics:" << std::endl;
	std::cout << "   Read count: " << store.get_read_count() << std::endl;
	std::cout << "   Write count: " << store.get_write_count() << std::endl;

	store.reset_statistics();
	std::cout << "   After reset - reads: " << store.get_read_count()
			  << ", writes: " << store.get_write_count() << std::endl;

	// 7. Clear
	store.clear();
	std::cout << "\n7. After clear: empty=" << (store.empty() ? "yes" : "no") << std::endl;

	std::cout << "\nDone." << std::endl;
	return 0;
}
