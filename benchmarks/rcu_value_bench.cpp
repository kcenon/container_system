// BSD 3-Clause License
// Copyright (c) 2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file rcu_value_bench.cpp
 * @brief Benchmarks for rcu_value read/update throughput scaling with threads
 */

#include <benchmark/benchmark.h>
#include "internal/rcu_value.h"

#include <thread>
#include <vector>
#include <atomic>
#include <string>

using namespace kcenon::container;

static void BM_RcuValueReadSingleThread(benchmark::State& state)
{
    rcu_value<int> val{42};

    for (auto _ : state) {
        auto snapshot = val.read();
        benchmark::DoNotOptimize(*snapshot);
    }
}
BENCHMARK(BM_RcuValueReadSingleThread);

static void BM_RcuValueUpdateSingleThread(benchmark::State& state)
{
    rcu_value<int> val{0};
    int counter = 0;

    for (auto _ : state) {
        val.update(++counter);
    }

    benchmark::DoNotOptimize(val.read());
}
BENCHMARK(BM_RcuValueUpdateSingleThread);

static void BM_RcuValueReadThroughputMultiThread(benchmark::State& state)
{
    static rcu_value<int> shared_val{42};
    const int num_threads = state.range(0);

    std::atomic<bool> stop{false};
    std::vector<std::thread> threads;

    // Spawn reader threads
    for (int i = 0; i < num_threads - 1; ++i) {
        threads.emplace_back([&stop]() {
            while (!stop.load(std::memory_order_relaxed)) {
                auto snapshot = shared_val.read();
                benchmark::DoNotOptimize(*snapshot);
            }
        });
    }

    // Main thread does the benchmarking
    for (auto _ : state) {
        auto snapshot = shared_val.read();
        benchmark::DoNotOptimize(*snapshot);
    }

    stop.store(true, std::memory_order_relaxed);
    for (auto& t : threads) {
        t.join();
    }
}
BENCHMARK(BM_RcuValueReadThroughputMultiThread)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(8)->Arg(16)
    ->Unit(benchmark::kNanosecond);

static void BM_RcuValueMixedReadWrite(benchmark::State& state)
{
    static rcu_value<std::string> shared_str{"initial"};
    const int num_threads = state.range(0);

    std::atomic<bool> stop{false};
    std::vector<std::thread> threads;

    // Spawn writer threads (1 writer per 4 readers)
    int writers = std::max(1, num_threads / 4);
    int readers = num_threads - writers;

    for (int i = 0; i < readers; ++i) {
        threads.emplace_back([&stop]() {
            while (!stop.load(std::memory_order_relaxed)) {
                auto snapshot = shared_str.read();
                benchmark::DoNotOptimize(snapshot->size());
            }
        });
    }

    for (int i = 0; i < writers - 1; ++i) {
        threads.emplace_back([&stop, id = i]() {
            int counter = 0;
            while (!stop.load(std::memory_order_relaxed)) {
                shared_str.update("writer_" + std::to_string(id)
                                  + "_" + std::to_string(counter++));
            }
        });
    }

    // Main thread is the last writer doing benchmarked iterations
    int counter = 0;
    for (auto _ : state) {
        shared_str.update("main_" + std::to_string(counter++));
    }

    stop.store(true, std::memory_order_relaxed);
    for (auto& t : threads) {
        t.join();
    }
}
BENCHMARK(BM_RcuValueMixedReadWrite)
    ->Arg(2)->Arg(4)->Arg(8)->Arg(16)
    ->Unit(benchmark::kNanosecond);

static void BM_RcuValueCASContention(benchmark::State& state)
{
    static rcu_value<int> shared_counter{0};
    const int num_threads = state.range(0);

    std::atomic<bool> stop{false};
    std::vector<std::thread> threads;

    // Spawn CAS contention threads
    for (int i = 0; i < num_threads - 1; ++i) {
        threads.emplace_back([&stop]() {
            while (!stop.load(std::memory_order_relaxed)) {
                auto expected = shared_counter.read();
                shared_counter.compare_and_update(expected, *expected + 1);
            }
        });
    }

    for (auto _ : state) {
        auto expected = shared_counter.read();
        bool success = shared_counter.compare_and_update(expected, *expected + 1);
        benchmark::DoNotOptimize(success);
    }

    stop.store(true, std::memory_order_relaxed);
    for (auto& t : threads) {
        t.join();
    }
}
BENCHMARK(BM_RcuValueCASContention)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(8)
    ->Unit(benchmark::kNanosecond);
