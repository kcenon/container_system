/**
 * @file value_operations_bench.cpp
 * @brief Value type operations benchmarks
 * Phase 0, Task 0.2
 */

#include <benchmark/benchmark.h>
#include "container/core/value.h"

using namespace container_module;

static void BM_Value_CreateString(benchmark::State& state) {
    for (auto _ : state) {
        value v("test", value_types::string_value, "test_data");
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_Value_CreateString);

static void BM_Value_CreateNumeric(benchmark::State& state) {
    for (auto _ : state) {
        value v("num", value_types::int_value, "42");
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_Value_CreateNumeric);

static void BM_Value_GetData(benchmark::State& state) {
    value v("test", value_types::string_value, "test_data");
    for (auto _ : state) {
        auto data = v.get_data();
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_Value_GetData);

static void BM_Value_SetData(benchmark::State& state) {
    value v;
    std::string data = "test_data";

    for (auto _ : state) {
        v.set_data(reinterpret_cast<const unsigned char*>(data.data()),
                   data.size(), value_types::string_value);
    }
}
BENCHMARK(BM_Value_SetData);
