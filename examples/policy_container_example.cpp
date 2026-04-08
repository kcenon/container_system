// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/// @file policy_container_example.cpp
/// @example policy_container_example.cpp
/// @brief Demonstrates policy-based container with pluggable storage.
///
/// Shows dynamic_storage_policy and indexed_storage_policy for
/// different access patterns and performance characteristics.
///
/// @see kcenon::container::basic_value_container
/// @see kcenon::container::policy::dynamic_storage_policy

#include <kcenon/container/policy_container.h>
#include <kcenon/container/storage_policy.h>

#include <iostream>
#include <string>

using namespace kcenon::container;

int main()
{
	std::cout << "=== Policy Container Example ===" << std::endl;

	// 1. Dynamic storage policy (default, flexible)
	std::cout << "\n1. Dynamic storage policy:" << std::endl;
	basic_value_container<policy::dynamic_storage_policy> dynamic_container;

	dynamic_container.add("name", "Alice");
	dynamic_container.add("score", 95.5);
	dynamic_container.add("level", static_cast<int64_t>(42));

	std::cout << "   Added 3 fields to dynamic container" << std::endl;
	std::cout << "   Size: " << dynamic_container.size() << std::endl;

	// 2. Indexed storage policy (optimized for key lookup)
	std::cout << "\n2. Indexed storage policy:" << std::endl;
	basic_value_container<policy::indexed_storage_policy> indexed_container;

	indexed_container.add("id", static_cast<int64_t>(1001));
	indexed_container.add("status", "active");

	std::cout << "   Added 2 fields to indexed container" << std::endl;
	std::cout << "   Size: " << indexed_container.size() << std::endl;

	// 3. StoragePolicy concept check
	std::cout << "\n3. Storage policy traits:" << std::endl;
	std::cout << "   dynamic_storage_policy satisfies StoragePolicy: "
			  << policy::StoragePolicy<policy::dynamic_storage_policy> << std::endl;
	std::cout << "   indexed_storage_policy satisfies StoragePolicy: "
			  << policy::StoragePolicy<policy::indexed_storage_policy> << std::endl;

	std::cout << "\nDone." << std::endl;
	return 0;
}
