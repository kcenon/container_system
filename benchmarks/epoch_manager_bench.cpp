// BSD 3-Clause License
// Copyright (c) 2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file epoch_manager_bench.cpp
 * @brief Benchmarks for epoch-based memory reclamation under concurrent readers
 */

#include <benchmark/benchmark.h>
#include "internal/epoch_manager.h"

#include <thread>
#include <vector>
#include <atomic>

using namespace kcenon::container;

static void BM_EpochEnterExit(benchmark::State& state)
{
    auto& em = epoch_manager::instance();

    for (auto _ : state) {
        em.enter_critical();
        benchmark::DoNotOptimize(em.in_critical_section());
        em.exit_critical();
    }
}
BENCHMARK(BM_EpochEnterExit);

static void BM_EpochGuardRAII(benchmark::State& state)
{
    for (auto _ : state) {
        epoch_guard guard;
        benchmark::DoNotOptimize(
            epoch_manager::instance().in_critical_section());
    }
}
BENCHMARK(BM_EpochGuardRAII);

static void BM_EpochDeferAndGC(benchmark::State& state)
{
    auto& em = epoch_manager::instance();

    for (auto _ : state) {
        int* ptr = new int(42);
        em.defer_delete(ptr, [](void* p) { delete static_cast<int*>(p); });
        em.try_gc();
    }

    em.force_gc();
}
BENCHMARK(BM_EpochDeferAndGC);

static void BM_EpochGCThroughputWithReaders(benchmark::State& state)
{
    const int num_reader_threads = state.range(0);
    auto& em = epoch_manager::instance();

    std::atomic<bool> stop{false};
    std::vector<std::thread> readers;

    // Spawn reader threads that continuously enter/exit critical sections
    for (int i = 0; i < num_reader_threads; ++i) {
        readers.emplace_back([&em, &stop]() {
            while (!stop.load(std::memory_order_relaxed)) {
                epoch_guard guard;
                // Simulate reading
                benchmark::DoNotOptimize(em.current_epoch());
            }
        });
    }

    for (auto _ : state) {
        int* ptr = new int(0);
        em.defer_delete(ptr, [](void* p) { delete static_cast<int*>(p); });
        auto reclaimed = em.try_gc();
        benchmark::DoNotOptimize(reclaimed);
    }

    stop.store(true, std::memory_order_relaxed);
    for (auto& t : readers) {
        t.join();
    }

    em.force_gc();
}
BENCHMARK(BM_EpochGCThroughputWithReaders)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(8)
    ->Unit(benchmark::kNanosecond);
