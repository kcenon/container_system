// BSD 3-Clause License
//
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * @file serialization_bench.cpp
 * @brief Serialization/deserialization benchmarks
 * Phase 0, Task 0.2
 */

#include <benchmark/benchmark.h>
#include "container/core/container.h"
#include "tests/test_compat.h"

using namespace container_module;

static void BM_Serialize_Small(benchmark::State& state) {
    auto c = std::make_shared<value_container>();
    c->add(std::make_shared<string_value>("key1", "value1"));
    c->add(std::make_shared<int_value>("key2", 42));

    for (auto _ : state) {
        auto data = c->serialize();
        benchmark::DoNotOptimize(data);
        state.SetBytesProcessed(data.size());
    }
}
BENCHMARK(BM_Serialize_Small);

static void BM_Serialize_Large(benchmark::State& state) {
    auto c = std::make_shared<value_container>();
    for (int i = 0; i < state.range(0); ++i) {
        c->add(std::make_shared<string_value>("key_" + std::to_string(i), std::string(100, 'x')));
    }

    for (auto _ : state) {
        auto data = c->serialize();
        benchmark::DoNotOptimize(data);
        state.SetBytesProcessed(data.size());
    }
}
BENCHMARK(BM_Serialize_Large)->Arg(10)->Arg(100)->Arg(1000);

static void BM_Deserialize(benchmark::State& state) {
    auto c = std::make_shared<value_container>();
    for (int i = 0; i < 100; ++i) {
        c->add(std::make_shared<int_value>("key_" + std::to_string(i), i));
    }
    auto data = c->serialize();

    for (auto _ : state) {
        auto deserialized = std::make_shared<value_container>(data);
        benchmark::DoNotOptimize(deserialized);
        state.SetBytesProcessed(data.size());
    }
}
BENCHMARK(BM_Deserialize);

static void BM_SerializeDeserialize_RoundTrip(benchmark::State& state) {
    for (auto _ : state) {
        auto c = std::make_shared<value_container>();
        c->add(std::make_shared<string_value>("test", "data"));

        auto data = c->serialize();
        auto c2 = std::make_shared<value_container>(data);

        benchmark::DoNotOptimize(c2);
    }
}
BENCHMARK(BM_SerializeDeserialize_RoundTrip);
