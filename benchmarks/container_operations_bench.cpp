/**
 * @file container_operations_bench.cpp
 * @brief Container operations benchmarks
 * Phase 0, Task 0.2: Baseline Performance Benchmarking
 */

#include <benchmark/benchmark.h>
#include "core/container.h"
#include "core/value.h"
#include "values/string_value.h"
#include "values/numeric_value.h"

using namespace container_module;

static void BM_Container_Create(benchmark::State& state) {
    for (auto _ : state) {
        auto c = std::make_shared<value_container>();
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_Container_Create);

static void BM_Container_AddValue(benchmark::State& state) {
    auto c = std::make_shared<value_container>();
    for (auto _ : state) {
        c->add(std::make_shared<string_value>("key", "value"));
    }
}
BENCHMARK(BM_Container_AddValue);

static void BM_Container_GetValue(benchmark::State& state) {
    auto c = std::make_shared<value_container>();
    c->add(std::make_shared<string_value>("key", "test_value"));
    for (auto _ : state) {
        auto val = c->get_value("key");
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_Container_GetValue);

static void BM_Container_MultipleValues(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto c = std::make_shared<value_container>();
        state.ResumeTiming();

        for (int i = 0; i < state.range(0); ++i) {
            c->add(std::make_shared<int_value>("key_" + std::to_string(i), i));
        }
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Container_MultipleValues)->Arg(10)->Arg(100)->Arg(1000);

static void BM_Container_Clone(benchmark::State& state) {
    auto c = std::make_shared<value_container>();
    for (int i = 0; i < 100; ++i) {
        c->add(std::make_shared<int_value>("key_" + std::to_string(i), i));
    }

    for (auto _ : state) {
        auto serialized = c->serialize();
        auto cloned = std::make_shared<value_container>(serialized);
        benchmark::DoNotOptimize(cloned);
    }
}
BENCHMARK(BM_Container_Clone);

static void BM_Container_Clear(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto c = std::make_shared<value_container>();
        for (int i = 0; i < 100; ++i) {
            c->add(std::make_shared<int_value>("key_" + std::to_string(i), i));
        }
        state.ResumeTiming();

        c->clear_value();
    }
}
BENCHMARK(BM_Container_Clear);
