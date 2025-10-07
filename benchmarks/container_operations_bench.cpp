/**
 * @file container_operations_bench.cpp
 * @brief Container operations benchmarks
 * Phase 0, Task 0.2: Baseline Performance Benchmarking
 */

#include <benchmark/benchmark.h>
#include "container/core/container.h"
#include "container/core/value.h"

using namespace container_module;

static void BM_Container_Create(benchmark::State& state) {
    for (auto _ : state) {
        container c;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_Container_Create);

static void BM_Container_SetValue(benchmark::State& state) {
    container c;
    for (auto _ : state) {
        c.set_value("key", "value");
    }
}
BENCHMARK(BM_Container_SetValue);

static void BM_Container_GetValue(benchmark::State& state) {
    container c;
    c.set_value("key", "test_value");
    for (auto _ : state) {
        auto val = c.get_value("key");
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_Container_GetValue);

static void BM_Container_MultipleValues(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        container c;
        state.ResumeTiming();

        for (int i = 0; i < state.range(0); ++i) {
            c.set_value("key_" + std::to_string(i), i);
        }
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Container_MultipleValues)->Arg(10)->Arg(100)->Arg(1000);

static void BM_Container_Clone(benchmark::State& state) {
    container c;
    for (int i = 0; i < 100; ++i) {
        c.set_value("key_" + std::to_string(i), i);
    }

    for (auto _ : state) {
        auto cloned = c.clone();
        benchmark::DoNotOptimize(cloned);
    }
}
BENCHMARK(BM_Container_Clone);

static void BM_Container_Clear(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        container c;
        for (int i = 0; i < 100; ++i) {
            c.set_value("key_" + std::to_string(i), i);
        }
        state.ResumeTiming();

        c.clear();
    }
}
BENCHMARK(BM_Container_Clear);
