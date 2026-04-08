// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/// @file thread_safe_rcu_example.cpp
/// @example thread_safe_rcu_example.cpp
/// @brief Demonstrates thread-safe container operations using RCU pattern.
///
/// Shows concurrent reads and writes on value_container with
/// thread-safe access patterns.
///
/// @see kcenon::container::value_container

#include "container.h"

#include <atomic>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace kcenon::container;

int main()
{
	std::cout << "=== Thread-Safe RCU Container Example ===" << std::endl;

	// Shared container with mutex for thread safety
	value_container shared_container;
	std::mutex container_mutex;

	shared_container.set("counter", static_cast<int64_t>(0));
	shared_container.set("status", std::string("initializing"));

	// 1. Concurrent writes
	std::cout << "\n1. Concurrent writes (5 threads):" << std::endl;
	std::atomic<int> write_count{0};
	std::vector<std::thread> writers;

	for (int i = 0; i < 5; ++i)
	{
		writers.emplace_back(
			[&, i]()
			{
				for (int j = 0; j < 10; ++j)
				{
					std::lock_guard<std::mutex> lock(container_mutex);
					std::string key = "thread_" + std::to_string(i) + "_item_" + std::to_string(j);
					shared_container.set(key, std::string("value_" + std::to_string(j)));
					write_count.fetch_add(1);
				}
			});
	}

	for (auto& t : writers)
	{
		t.join();
	}
	std::cout << "   Total writes: " << write_count.load() << std::endl;
	std::cout << "   Container size: " << shared_container.size() << std::endl;

	// 2. Concurrent reads
	std::cout << "\n2. Concurrent reads (3 threads):" << std::endl;
	std::atomic<int> read_count{0};
	std::vector<std::thread> readers;

	for (int i = 0; i < 3; ++i)
	{
		readers.emplace_back(
			[&, i]()
			{
				std::lock_guard<std::mutex> lock(container_mutex);
				auto size = shared_container.size();
				read_count.fetch_add(1);
				std::cout << "   Reader " << i << " sees " << size << " entries" << std::endl;
			});
	}

	for (auto& t : readers)
	{
		t.join();
	}
	std::cout << "   Total reads: " << read_count.load() << std::endl;

	// 3. Container copy for snapshot
	std::cout << "\n3. Snapshot (copy-on-read):" << std::endl;
	value_container snapshot;
	{
		std::lock_guard<std::mutex> lock(container_mutex);
		snapshot = shared_container;
	}
	std::cout << "   Snapshot size: " << snapshot.size() << std::endl;
	std::cout << "   Original size: " << shared_container.size() << std::endl;

	std::cout << "\nDone." << std::endl;
	return 0;
}
