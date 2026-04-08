// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/// @file simd_batch_example.cpp
/// @example simd_batch_example.cpp
/// @brief Demonstrates SIMD-optimized batch operations on containers.
///
/// Shows simd_batch for high-throughput data collection with
/// trivially copyable types.
///
/// @see kcenon::container::core::simd_batch

#include <kcenon/container/simd_batch.h>

#include <chrono>
#include <cstdint>
#include <iostream>

using namespace kcenon::container;

int main()
{
	std::cout << "=== SIMD Batch Example ===" << std::endl;

	// 1. Basic batch operations with integers
	std::cout << "\n1. Integer batch:" << std::endl;
	core::simd_batch<int32_t> int_batch;
	int_batch.reserve(1000);

	for (int32_t i = 0; i < 100; ++i)
	{
		int_batch.push(i * i);
	}

	std::cout << "   Size: " << int_batch.size() << std::endl;
	std::cout << "   Empty: " << (int_batch.empty() ? "yes" : "no") << std::endl;

	const auto& values = int_batch.values();
	std::cout << "   First 5: ";
	for (size_t i = 0; i < 5 && i < values.size(); ++i)
	{
		std::cout << values[i] << " ";
	}
	std::cout << std::endl;

	// 2. Double-precision batch
	std::cout << "\n2. Double batch:" << std::endl;
	core::simd_batch<double> double_batch;

	for (int i = 0; i < 50; ++i)
	{
		double_batch.push(i * 0.1);
	}
	std::cout << "   Size: " << double_batch.size() << std::endl;

	// 3. Performance measurement
	std::cout << "\n3. Batch insert performance:" << std::endl;
	core::simd_batch<int64_t> perf_batch;
	perf_batch.reserve(100000);

	auto start = std::chrono::high_resolution_clock::now();
	for (int64_t i = 0; i < 100000; ++i)
	{
		perf_batch.push(i);
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	std::cout << "   100K inserts: " << us << " us" << std::endl;
	std::cout << "   Final size: " << perf_batch.size() << std::endl;

	// 4. Clear and reuse
	perf_batch.clear();
	std::cout << "\n4. After clear: size=" << perf_batch.size()
			  << ", empty=" << (perf_batch.empty() ? "yes" : "no") << std::endl;

	std::cout << "\nDone." << std::endl;
	return 0;
}
