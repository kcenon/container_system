/**
 * @file serialization_bench.cpp
 * @brief Serialization/deserialization benchmarks
 * Phase 0, Task 0.2
 */

#include <benchmark/benchmark.h>
#include "core/container.h"
#include "core/value.h"

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
