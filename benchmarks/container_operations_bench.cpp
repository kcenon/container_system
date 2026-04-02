// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file container_operations_bench.cpp
 * @brief Container operations benchmarks
 * Phase 0, Task 0.2: Baseline Performance Benchmarking
 */

#include <benchmark/benchmark.h>
#include "core/container.h"
#include "tests/test_compat.h"

using namespace kcenon::container;

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
        c->set("key", std::string("value"));
    }
}
BENCHMARK(BM_Container_AddValue);

static void BM_Container_GetValue(benchmark::State& state) {
    auto c = std::make_shared<value_container>();
    c->set("key", std::string("test_value"));
    for (auto _ : state) {
        auto val = c->get("key");
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
            c->set("key_" + std::to_string(i), i);
        }
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Container_MultipleValues)->Arg(10)->Arg(100)->Arg(1000);

static void BM_Container_Clone(benchmark::State& state) {
    auto c = std::make_shared<value_container>();
    for (int i = 0; i < 100; ++i) {
        c->set("key_" + std::to_string(i), i);
    }

    for (auto _ : state) {
        auto result = c->serialize_string(value_container::serialization_format::binary);
        auto serialized = result.is_ok() ? result.value() : "";
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
            c->set("key_" + std::to_string(i), i);
        }
        state.ResumeTiming();

        c->clear_value();
    }
}
BENCHMARK(BM_Container_Clear);
