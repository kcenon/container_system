/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

/**
 * @file core/container/types.h
 * @brief Core type definitions for container module
 *
 * This header defines the fundamental types used throughout the container
 * module, including:
 * - value_variant: Small Object Optimization storage type
 * - optimized_value: Value wrapper with SOO support
 * - pool_stats: Memory pool statistics
 */

#pragma once

#include "core/value_types.h"

// Note: When included from container.h, these paths resolve correctly
// via the container -> . symlink in project root

#include <variant>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace container_module
{
	// Forward declaration
	class value_container;

	/**
	 * @brief Small Object Optimization (SOO) for value storage
	 *
	 * This variant-based storage allows small primitive values to be stored
	 * on the stack rather than heap-allocated, significantly reducing memory
	 * overhead and improving cache locality.
	 */
	using value_variant = std::variant<
		std::monostate,                          // null_value (0 bytes)
		bool,                                    // bool_value (1 byte)
		short,                                   // short_value (2 bytes)
		unsigned short,                          // ushort_value (2 bytes)
		int,                                     // int_value (4 bytes)
		unsigned int,                            // uint_value (4 bytes)
		long,                                    // long_value (4/8 bytes)
		unsigned long,                           // ulong_value (4/8 bytes)
		long long,                               // llong_value (8 bytes)
		unsigned long long,                      // ullong_value (8 bytes)
		float,                                   // float_value (4 bytes)
		double,                                  // double_value (8 bytes)
		std::string,                             // string_value (dynamic)
		std::vector<uint8_t>,                    // bytes_value (dynamic)
		std::shared_ptr<value_container>         // container_value (pointer only)
	>;

	/**
	 * @brief Optimized value storage with Small Object Optimization
	 */
	struct optimized_value
	{
		std::string name;              ///< Value identifier
		value_types type;              ///< Type enumeration
		value_variant data;            ///< Variant storage (stack-allocated for primitives)

		optimized_value()
			: name("")
			, type(value_types::null_value)
			, data(std::monostate{})
		{}

		optimized_value(const std::string& n, value_types t)
			: name(n)
			, type(t)
			, data(std::monostate{})
		{}

		size_t memory_footprint() const
		{
			size_t base = sizeof(optimized_value);
			base += name.capacity();
			if (std::holds_alternative<std::string>(data)) {
				base += std::get<std::string>(data).capacity();
			} else if (std::holds_alternative<std::vector<uint8_t>>(data)) {
				base += std::get<std::vector<uint8_t>>(data).capacity();
			}
			return base;
		}

		bool is_stack_allocated() const
		{
			return type != value_types::string_value &&
				   type != value_types::bytes_value &&
				   type != value_types::container_value;
		}
	};

	/**
	 * @brief Pool statistics structure for memory pool monitoring
	 */
	struct pool_stats
	{
		size_t hits{0};                // Pool allocations satisfied
		size_t misses{0};              // Heap allocations (pool bypassed)
		size_t small_pool_allocs{0};   // Small pool (<=64 bytes) allocations
		size_t medium_pool_allocs{0};  // Medium pool (<=256 bytes) allocations
		size_t deallocations{0};       // Total deallocations
		size_t available{0};           // Free blocks available

		pool_stats() = default;

		pool_stats(size_t h, size_t m, size_t a)
			: hits(h), misses(m), available(a)
		{}

		pool_stats(size_t h, size_t m, size_t sm, size_t med, size_t deallocs, size_t avail)
			: hits(h), misses(m), small_pool_allocs(sm), medium_pool_allocs(med)
			, deallocations(deallocs), available(avail)
		{}

		[[nodiscard]] double hit_rate() const noexcept {
			auto total = hits + misses;
			return total > 0 ? static_cast<double>(hits) / static_cast<double>(total) : 0.0;
		}
	};

} // namespace container_module
