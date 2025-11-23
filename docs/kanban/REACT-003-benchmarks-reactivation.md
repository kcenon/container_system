# REACT-003: Benchmarks Reactivation

## Metadata
- **ID**: REACT-003
- **Priority**: MEDIUM
- **Estimated Duration**: 2 days
- **Dependencies**: REACT-001
- **Status**: TODO
- **Assignee**: TBD
- **Related Documents**: [REACTIVATION_PLAN.md](../advanced/REACTIVATION_PLAN.md)

---

## Overview

### What are we changing?

Completely rewrite the benchmark code that referenced the deleted `core/value.h` to the variant-based system to restore the performance measurement infrastructure.

**Current State**:
- Disabled at `CMakeLists.txt:483-487`
- Conditionally skipped at `.github/workflows/benchmarks.yml:99-107`

**Files Requiring Rewrite**:
- `benchmarks/container_operations_bench.cpp`
- `benchmarks/value_operations_bench.cpp`
- `benchmarks/serialization_bench.cpp`
- `benchmarks/memory_efficiency_bench.cpp`

---

## Changes

### How are we changing it?

#### 1. Rewrite Container Operations Benchmark (0.5 days)

**Metrics to Measure**:
- Container creation/deletion performance
- Value addition performance (`set_value()`)
- Value retrieval performance (`get_optimized_value()`)
- Batch operations performance

```cpp
// New benchmark pattern
static void BM_ContainerSetValue(benchmark::State& state) {
    thread_safe_container container;
    for (auto _ : state) {
        container.set_value("key", 42);
    }
}
BENCHMARK(BM_ContainerSetValue);
```

#### 2. Rewrite Value Operations Benchmark (0.5 days)

**Metrics to Measure**:
- `optimized_value` creation performance
- Type conversion performance
- Copy/Move performance
- Variant access performance

```cpp
static void BM_OptimizedValueCreation(benchmark::State& state) {
    for (auto _ : state) {
        optimized_value val{"test", value_types::int_value, 42};
        benchmark::DoNotOptimize(val);
    }
}
```

#### 3. Rewrite Serialization Benchmark (0.5 days)

**Metrics to Measure**:
- Binary serialization performance
- JSON serialization performance
- Deserialization performance
- Round-trip performance

**Expected Targets**:
- 3x or more serialization improvement over legacy
- 3.2x or more deserialization improvement over legacy

#### 4. Rewrite Memory Efficiency Benchmark (0.5 days)

**Metrics to Measure**:
- Memory usage per value
- Stack allocation efficiency
- Heap allocation frequency
- Cache hit rate

**Expected Targets**:
- Legacy 88 bytes -> Modern 56 bytes (36% reduction)
- 50-100% reduction in heap allocations

---

## Test Plan

### How will we test it?

#### 1. Local Benchmark Execution
```bash
cmake -B build -DBUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/benchmarks/container_system_benchmarks
```

#### 2. Performance Comparison Verification

| Metric | Legacy Baseline | Target | Pass Criteria |
|--------|-----------------|--------|---------------|
| Serialization | 1.8M ops/s | 5.4M ops/s | >= 3x |
| Deserialization | 2.1M ops/s | 6.8M ops/s | >= 3x |
| Memory/Value | 88 bytes | 56 bytes | <= 60% |
| Type Check | 15 ns | 2 ns | >= 7x |

#### 3. CI Benchmark Workflow
- Enable `.github/workflows/benchmarks.yml`
- Set up automatic performance regression detection

#### Success Criteria
- [ ] Benchmark suite compiles and runs
- [ ] Performance equals or exceeds legacy baseline
- [ ] Memory usage <= legacy (stack allocation benefits)
- [ ] CI benchmark workflow enabled

---

## Technical Details

### New Benchmarks to Add

**Legacy vs. Variant Comparison Benchmark**:
```cpp
static void BM_LegacyVsVariant_Serialization(benchmark::State& state) {
    // Compare both systems with identical data
    std::vector<variant_value_v2> modern_values;
    // ... populate

    for (auto _ : state) {
        for (const auto& val : modern_values) {
            auto bytes = val.serialize();
            benchmark::DoNotOptimize(bytes);
        }
    }
}
```

**Stack vs. Heap Comparison Benchmark**:
```cpp
static void BM_StackAllocation(benchmark::State& state) {
    for (auto _ : state) {
        optimized_value val{"key", value_types::int_value, 42};
        // Stack allocated, no heap
    }
}

static void BM_HeapAllocation(benchmark::State& state) {
    for (auto _ : state) {
        auto val = std::make_unique<optimized_value>("key", value_types::int_value, 42);
    }
}
```

---

## Risks and Mitigations

| Risk | Probability | Impact | Mitigation |
|--------|------|------|-----------|
| Performance regression | Low | High | Hot path optimization |
| Measurement variance | Medium | Low | Multiple runs, average values |
| Platform differences | Medium | Low | Test on all platforms |

---

## Checklist

- [ ] Rewrite container_operations_bench.cpp
- [ ] Rewrite value_operations_bench.cpp
- [ ] Rewrite serialization_bench.cpp
- [ ] Rewrite memory_efficiency_bench.cpp
- [ ] Add Legacy vs. Modern comparison benchmark
- [ ] Enable benchmarks in CMakeLists.txt
- [ ] Remove skip condition from benchmarks.yml
- [ ] Verify local benchmark execution
- [ ] Confirm performance targets achieved
- [ ] Confirm CI passes

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-23
