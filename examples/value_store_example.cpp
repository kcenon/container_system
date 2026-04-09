// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/// @file value_store_example.cpp
/// @example value_store_example.cpp
/// @brief Demonstrates thread-safe value_store with typed values.
///
/// Shows CRUD operations with properly typed values,
/// thread-safe access, and read/write statistics.
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

	// 1. Add typed values
	std::cout << "\n1. Adding values:" << std::endl;
	store.add("host", value("host", std::string("localhost")));
	store.add("port", value("port", static_cast<int32_t>(8080)));
	store.add("debug", value("debug", true));
	std::cout << "   Size: " << store.size() << std::endl;

	// 2. Get values
	std::cout << "\n2. Reading values:" << std::endl;
	auto host = store.get("host");
	if (host.has_value())
	{
		std::cout << "   host found: yes" << std::endl;
	}

	// 3. Check existence
	std::cout << "\n3. Contains check:" << std::endl;
	std::cout << "   'host': " << (store.contains("host") ? "yes" : "no") << std::endl;
	std::cout << "   'missing': " << (store.contains("missing") ? "yes" : "no") << std::endl;

	// 4. Remove
	std::cout << "\n4. Remove 'debug':" << std::endl;
	store.remove("debug");
	std::cout << "   Size after remove: " << store.size() << std::endl;

	// 5. Statistics
	std::cout << "\n5. Access statistics:" << std::endl;
	std::cout << "   Read count: " << store.get_read_count() << std::endl;
	std::cout << "   Write count: " << store.get_write_count() << std::endl;

	store.reset_statistics();
	std::cout << "   After reset - reads: " << store.get_read_count()
			  << ", writes: " << store.get_write_count() << std::endl;

	// 6. Clear
	store.clear();
	std::cout << "\n6. After clear: empty=" << (store.empty() ? "yes" : "no") << std::endl;

	std::cout << "\nDone." << std::endl;
	return 0;
}
