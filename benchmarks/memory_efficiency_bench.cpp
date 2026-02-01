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

#include <benchmark/benchmark.h>
#include "core/container.h"
#include "tests/test_compat.h"

using namespace container_module;

/**
 * @brief Benchmark memory footprint of containers with different value counts
 */
static void BM_MemoryFootprint(benchmark::State& state)
{
	const size_t num_values = state.range(0);

	for (auto _ : state)
	{
		auto container = std::make_shared<value_container>();
		container->set_message_type("memory_test");

		// Add mixed value types
		for (size_t i = 0; i < num_values; ++i)
		{
			if (i % 3 == 0)
			{
				container->set("int_" + std::to_string(i), static_cast<int>(i));
			}
			else if (i % 3 == 1)
			{
				container->set("double_" + std::to_string(i), static_cast<double>(i) * 1.5);
			}
			else
			{
				container->set("string_" + std::to_string(i), "value_" + std::to_string(i));
			}
		}

		// Measure memory footprint
		size_t footprint = container->memory_footprint();
		benchmark::DoNotOptimize(footprint);
	}

	state.SetLabel("values=" + std::to_string(num_values));
}

BENCHMARK(BM_MemoryFootprint)->Arg(10)->Arg(50)->Arg(100)->Arg(500)->Arg(1000);

/**
 * @brief Benchmark Small Object Optimization effectiveness
 */
static void BM_SOO_vs_Traditional(benchmark::State& state)
{
	const bool use_soo = state.range(0) != 0;
	const size_t num_values = 100;

	for (auto _ : state)
	{
		auto container = std::make_shared<value_container>();
		container->set_soo_enabled(use_soo);
		container->set_message_type("soo_test");

		// Add primitive values that benefit from SOO
		for (size_t i = 0; i < num_values; ++i)
		{
			container->set("int_" + std::to_string(i), static_cast<int>(i));
			container->set("bool_" + std::to_string(i), i % 2 == 0);
			container->set("double_" + std::to_string(i), static_cast<double>(i));
		}

		auto [heap_allocs, stack_allocs] = container->memory_stats();
		benchmark::DoNotOptimize(heap_allocs);
		benchmark::DoNotOptimize(stack_allocs);
	}

	state.SetLabel(use_soo ? "SOO_enabled" : "Traditional");
}

BENCHMARK(BM_SOO_vs_Traditional)->Arg(0)->Arg(1);

/**
 * @brief Benchmark memory pool effectiveness
 */
static void BM_MemoryPoolEfficiency(benchmark::State& state)
{
	const size_t iterations = state.range(0);

	// Clear pool before benchmark
	value_container::clear_pool();

	for (auto _ : state)
	{
		std::vector<std::shared_ptr<value_container>> containers;
		containers.reserve(iterations);

		// Create containers (pool allocation)
		for (size_t i = 0; i < iterations; ++i)
		{
			auto container = std::make_shared<value_container>();
			container->set_message_type("pool_test");
			container->set("value", static_cast<int>(i));
			containers.push_back(container);
		}

		// Destroy containers (pool deallocation)
		containers.clear();
	}

	// Report pool statistics
	auto stats = value_container::get_pool_stats();
	state.counters["PoolHits"] = benchmark::Counter(stats.hits);
	state.counters["PoolMisses"] = benchmark::Counter(stats.misses);
	state.counters["HitRate"] = benchmark::Counter(stats.hit_rate(),
		benchmark::Counter::kAvgThreads);
}

BENCHMARK(BM_MemoryPoolEfficiency)->Arg(10)->Arg(50)->Arg(100)->Arg(500);

/**
 * @brief Benchmark container creation with different memory strategies
 */
static void BM_ContainerCreationSpeed(benchmark::State& state)
{
	const size_t num_values = state.range(0);

	for (auto _ : state)
	{
		auto container = std::make_shared<value_container>();
		container->set_source("benchmark", "test");
		container->set_target("target", "handler");
		container->set_message_type("creation_test");

		for (size_t i = 0; i < num_values; ++i)
		{
			container->set("val" + std::to_string(i), static_cast<int>(i));
		}

		benchmark::DoNotOptimize(container);
	}

	state.SetItemsProcessed(state.iterations() * num_values);
}

BENCHMARK(BM_ContainerCreationSpeed)
	->Arg(10)
	->Arg(50)
	->Arg(100)
	->Arg(500);

/**
 * @brief Benchmark memory allocation patterns
 */
static void BM_AllocationPattern(benchmark::State& state)
{
	const size_t container_count = state.range(0);

	for (auto _ : state)
	{
		std::vector<std::shared_ptr<value_container>> containers;
		containers.reserve(container_count);

		// Bulk creation
		for (size_t i = 0; i < container_count; ++i)
		{
			auto container = std::make_shared<value_container>();
			container->set("id", static_cast<int>(i));
			container->set("price", i * 10.5);
			container->set("active", true);
			containers.push_back(container);
		}

		// Measure total footprint
		size_t total_footprint = 0;
		for (const auto& c : containers)
		{
			total_footprint += c->memory_footprint();
		}

		benchmark::DoNotOptimize(total_footprint);
		state.counters["AvgFootprint"] = benchmark::Counter(
			total_footprint / container_count,
			benchmark::Counter::kAvgThreads);
	}
}

BENCHMARK(BM_AllocationPattern)
	->Arg(10)
	->Arg(100)
	->Arg(1000);

/**
 * @brief Benchmark cache locality with contiguous storage
 */
static void BM_CacheLocality(benchmark::State& state)
{
	const size_t num_values = state.range(0);

	auto container = std::make_shared<value_container>();
	container->set_soo_enabled(true);

	// Pre-populate with values
	for (size_t i = 0; i < num_values; ++i)
	{
		container->set("val" + std::to_string(i), static_cast<int>(i));
	}

	for (auto _ : state)
	{
		// Sequential access pattern (tests cache locality)
		int sum = 0;
		for (size_t i = 0; i < num_values; ++i)
		{
			auto val = container->get_value("val" + std::to_string(i));
			if (!ov_is_null(val))
			{
				sum += ov_to_int(val);
			}
		}
		benchmark::DoNotOptimize(sum);
	}

	state.SetItemsProcessed(state.iterations() * num_values);
}

BENCHMARK(BM_CacheLocality)
	->Arg(10)
	->Arg(100)
	->Arg(500);
