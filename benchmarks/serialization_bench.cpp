// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file serialization_bench.cpp
 * @brief Serialization/deserialization benchmarks
 * Phase 0, Task 0.2
 */

#include <benchmark/benchmark.h>
#include "core/container.h"
#include "tests/test_compat.h"

using namespace kcenon::container;

static void BM_Serialize_Small(benchmark::State& state) {
    auto c = std::make_shared<value_container>();
    c->set("key1", std::string("value1"));
    c->set("key2", 42);

    for (auto _ : state) {
        auto result = c->serialize_string(value_container::serialization_format::binary);
        auto data = result.is_ok() ? result.value() : "";
        benchmark::DoNotOptimize(data);
        state.SetBytesProcessed(data.size());
    }
}
BENCHMARK(BM_Serialize_Small);

static void BM_Serialize_Large(benchmark::State& state) {
    auto c = std::make_shared<value_container>();
    for (int i = 0; i < state.range(0); ++i) {
        c->set("key_" + std::to_string(i), std::string(100, 'x'));
    }

    for (auto _ : state) {
        auto result = c->serialize_string(value_container::serialization_format::binary);
        auto data = result.is_ok() ? result.value() : "";
        benchmark::DoNotOptimize(data);
        state.SetBytesProcessed(data.size());
    }
}
BENCHMARK(BM_Serialize_Large)->Arg(10)->Arg(100)->Arg(1000);

static void BM_Deserialize(benchmark::State& state) {
    auto c = std::make_shared<value_container>();
    for (int i = 0; i < 100; ++i) {
        c->set("key_" + std::to_string(i), i);
    }
    auto result = c->serialize_string(value_container::serialization_format::binary);
    auto data = result.is_ok() ? result.value() : "";

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
        c->set("test", std::string("data"));

        auto result = c->serialize_string(value_container::serialization_format::binary);
        auto data = result.is_ok() ? result.value() : "";
        auto c2 = std::make_shared<value_container>(data);

        benchmark::DoNotOptimize(c2);
    }
}
BENCHMARK(BM_SerializeDeserialize_RoundTrip);
