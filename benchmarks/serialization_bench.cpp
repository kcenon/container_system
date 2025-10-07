/**
 * @file serialization_bench.cpp
 * @brief Serialization/deserialization benchmarks
 * Phase 0, Task 0.2
 */

#include <benchmark/benchmark.h>
#include "container/core/container.h"

using namespace container_module;

static void BM_Serialize_Small(benchmark::State& state) {
    container c;
    c.set_value("key1", "value1");
    c.set_value("key2", 42);

    for (auto _ : state) {
        auto data = c.serialize_to_binary();
        benchmark::DoNotOptimize(data);
        state.SetBytesProcessed(data.size());
    }
}
BENCHMARK(BM_Serialize_Small);

static void BM_Serialize_Large(benchmark::State& state) {
    container c;
    for (int i = 0; i < state.range(0); ++i) {
        c.set_value("key_" + std::to_string(i), std::string(100, 'x'));
    }

    for (auto _ : state) {
        auto data = c.serialize_to_binary();
        benchmark::DoNotOptimize(data);
        state.SetBytesProcessed(data.size());
    }
}
BENCHMARK(BM_Serialize_Large)->Arg(10)->Arg(100)->Arg(1000);

static void BM_Deserialize(benchmark::State& state) {
    container c;
    for (int i = 0; i < 100; ++i) {
        c.set_value("key_" + std::to_string(i), i);
    }
    auto data = c.serialize_to_binary();

    for (auto _ : state) {
        container deserialized;
        deserialized.deserialize_from_binary(data);
        benchmark::DoNotOptimize(deserialized);
        state.SetBytesProcessed(data.size());
    }
}
BENCHMARK(BM_Deserialize);

static void BM_SerializeDeserialize_RoundTrip(benchmark::State& state) {
    for (auto _ : state) {
        container c;
        c.set_value("test", "data");

        auto data = c.serialize_to_binary();
        container c2;
        c2.deserialize_from_binary(data);

        benchmark::DoNotOptimize(c2);
    }
}
BENCHMARK(BM_SerializeDeserialize_RoundTrip);
