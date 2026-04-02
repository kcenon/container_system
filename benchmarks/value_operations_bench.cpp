// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file value_operations_bench.cpp
 * @brief Value type operations benchmarks
 * Phase 0, Task 0.2
 */

#include <benchmark/benchmark.h>
#include "core/container.h"

using namespace kcenon::container;

static void BM_Value_CreateString(benchmark::State& state) {
    for (auto _ : state) {
        optimized_value v;
        v.name = "test";
        v.data = std::string("test_data");
        v.type = value_types::string_value;
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_Value_CreateString);

static void BM_Value_CreateNumeric(benchmark::State& state) {
    for (auto _ : state) {
        optimized_value v;
        v.name = "num";
        v.data = int32_t(42);
        v.type = value_types::int_value;
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_Value_CreateNumeric);

static void BM_Value_GetData(benchmark::State& state) {
    optimized_value v;
    v.name = "test";
    v.data = std::string("test_data");
    v.type = value_types::string_value;
    for (auto _ : state) {
        auto data = variant_helpers::to_string(v.data, v.type);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_Value_GetData);

static void BM_Value_ToString(benchmark::State& state) {
    optimized_value v;
    v.name = "test";
    v.data = std::string("test_data");
    v.type = value_types::string_value;
    for (auto _ : state) {
        auto str = variant_helpers::to_string(v.data, v.type);
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_Value_ToString);
