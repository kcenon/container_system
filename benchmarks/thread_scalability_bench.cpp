/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, kcenon
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
 * @file thread_scalability_bench.cpp
 * @brief Thread scalability benchmarks for container operations
 *
 * Measures how container create, set/get, serialize, and deserialize
 * operations scale with 1/2/4/8/16 concurrent threads.
 */

#include <benchmark/benchmark.h>
#include "core/container.h"

#include <string>
#include <vector>

using namespace kcenon::container;

// =============================================================================
// Container creation scaling
// =============================================================================

static void BM_ContainerCreateScaling(benchmark::State& state)
{
    for (auto _ : state) {
        auto c = std::make_shared<value_container>();
        c->set("key", std::string("value"));
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_ContainerCreateScaling)
    ->Threads(1)->Threads(2)->Threads(4)->Threads(8)->Threads(16)
    ->Unit(benchmark::kNanosecond);

// =============================================================================
// Value set scaling (each thread operates on its own container)
// =============================================================================

static void BM_ContainerSetScaling(benchmark::State& state)
{
    auto c = std::make_shared<value_container>();
    int counter = 0;

    for (auto _ : state) {
        c->set("key_" + std::to_string(counter++), counter);
    }

    benchmark::DoNotOptimize(c);
}
BENCHMARK(BM_ContainerSetScaling)
    ->Threads(1)->Threads(2)->Threads(4)->Threads(8)->Threads(16)
    ->Unit(benchmark::kNanosecond);

// =============================================================================
// Value get scaling (each thread reads from its own container)
// =============================================================================

static void BM_ContainerGetScaling(benchmark::State& state)
{
    auto c = std::make_shared<value_container>();
    for (int i = 0; i < 100; ++i) {
        c->set("key_" + std::to_string(i), i);
    }

    int idx = 0;
    for (auto _ : state) {
        auto val = c->get("key_" + std::to_string(idx % 100));
        benchmark::DoNotOptimize(val);
        ++idx;
    }
}
BENCHMARK(BM_ContainerGetScaling)
    ->Threads(1)->Threads(2)->Threads(4)->Threads(8)->Threads(16)
    ->Unit(benchmark::kNanosecond);

// =============================================================================
// Serialization scaling (each thread serializes its own container)
// =============================================================================

static void BM_ContainerSerializeScaling(benchmark::State& state)
{
    const int num_values = state.range(0);
    auto c = std::make_shared<value_container>();
    for (int i = 0; i < num_values; ++i) {
        c->set("key_" + std::to_string(i), std::string("value_" + std::to_string(i)));
    }

    for (auto _ : state) {
        auto result = c->serialize(value_container::serialization_format::binary);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ContainerSerializeScaling)
    ->Arg(10)->Arg(100)
    ->Threads(1)->Threads(2)->Threads(4)->Threads(8)->Threads(16)
    ->Unit(benchmark::kMicrosecond);

// =============================================================================
// Deserialization scaling (each thread deserializes independently)
// =============================================================================

static void BM_ContainerDeserializeScaling(benchmark::State& state)
{
    const int num_values = state.range(0);
    auto c = std::make_shared<value_container>();
    for (int i = 0; i < num_values; ++i) {
        c->set("key_" + std::to_string(i), std::string("value_" + std::to_string(i)));
    }

    auto result = c->serialize(value_container::serialization_format::binary);
    if (!result.is_ok()) {
        state.SkipWithError("serialization failed");
        return;
    }
    const auto& data = result.value();

    for (auto _ : state) {
        auto target = std::make_shared<value_container>(data, false);
        benchmark::DoNotOptimize(target);
    }
}
BENCHMARK(BM_ContainerDeserializeScaling)
    ->Arg(10)->Arg(100)
    ->Threads(1)->Threads(2)->Threads(4)->Threads(8)->Threads(16)
    ->Unit(benchmark::kMicrosecond);

// =============================================================================
// Mixed read/write throughput (simulates async_container concurrent usage)
// =============================================================================

static void BM_ContainerMixedReadWriteScaling(benchmark::State& state)
{
    auto c = std::make_shared<value_container>();
    for (int i = 0; i < 50; ++i) {
        c->set("key_" + std::to_string(i), std::string("initial_" + std::to_string(i)));
    }

    int counter = 0;
    for (auto _ : state) {
        if (counter % 4 == 0) {
            // Write (25% of operations)
            c->set("key_" + std::to_string(counter % 50),
                    std::string("updated_" + std::to_string(counter)));
        } else {
            // Read (75% of operations)
            auto val = c->get("key_" + std::to_string(counter % 50));
            benchmark::DoNotOptimize(val);
        }
        ++counter;
    }
}
BENCHMARK(BM_ContainerMixedReadWriteScaling)
    ->Threads(1)->Threads(2)->Threads(4)->Threads(8)->Threads(16)
    ->Unit(benchmark::kNanosecond);
