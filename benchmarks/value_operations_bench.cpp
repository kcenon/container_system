/**
 * @file value_operations_bench.cpp
 * @brief Value type operations benchmarks
 * Phase 0, Task 0.2
 */

#include <benchmark/benchmark.h>
#include "core/value.h"

using namespace container_module;

static void BM_Value_CreateString(benchmark::State& state) {
    for (auto _ : state) {
        auto v = std::make_shared<string_value>("test", "test_data");
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_Value_CreateString);

static void BM_Value_CreateNumeric(benchmark::State& state) {
    for (auto _ : state) {
        auto v = std::make_shared<int_value>("num", 42);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_Value_CreateNumeric);

static void BM_Value_GetData(benchmark::State& state) {
    auto v = std::make_shared<string_value>("test", "test_data");
    for (auto _ : state) {
        auto data = v->data();
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_Value_GetData);

static void BM_Value_ToString(benchmark::State& state) {
    auto v = std::make_shared<string_value>("test", "test_data");
    for (auto _ : state) {
        auto str = v->to_string();
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_Value_ToString);
