# container_system Performance Benchmarks

Phase 0, Task 0.2: Baseline Performance Benchmarking

## Overview

This directory contains comprehensive performance benchmarks for the container_system, measuring:

- **Container Operations**: Creation, get/set, clone, clear performance
- **Serialization**: Binary serialization and deserialization speed
- **Value Operations**: Value type creation and manipulation
- **SIMD Performance**: SIMD acceleration benefits (if available)

## Building

### Prerequisites

```bash
# macOS (Homebrew)
brew install google-benchmark

# Ubuntu/Debian
sudo apt-get install libbenchmark-dev

# From source
git clone https://github.com/google/benchmark.git
cd benchmark
cmake -E make_directory build
cmake -E chdir build cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DCMAKE_BUILD_TYPE=Release ../
cmake --build build --config Release
sudo cmake --build build --config Release --target install
```

### Build Benchmarks

```bash
cd container_system
cmake -B build -S . -DCONTAINER_BUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Or use the build target
cd build
make container_benchmarks
```

## Running Benchmarks

### Run All Benchmarks

```bash
./build/benchmarks/container_benchmarks
```

### Run Specific Benchmark Categories

```bash
# Container operations only
./build/benchmarks/container_benchmarks --benchmark_filter=Container

# Serialization benchmarks only
./build/benchmarks/container_benchmarks --benchmark_filter=Serialize

# Value operations only
./build/benchmarks/container_benchmarks --benchmark_filter=Value
```

### Output Formats

```bash
# Console output (default)
./build/benchmarks/container_benchmarks

# JSON output
./build/benchmarks/container_benchmarks --benchmark_format=json

# CSV output
./build/benchmarks/container_benchmarks --benchmark_format=csv

# Save to file
./build/benchmarks/container_benchmarks --benchmark_format=json --benchmark_out=results.json
```

### Advanced Options

```bash
# Run for minimum time (for stable results)
./build/benchmarks/container_benchmarks --benchmark_min_time=5.0

# Specify number of iterations
./build/benchmarks/container_benchmarks --benchmark_repetitions=10

# Show all statistics
./build/benchmarks/container_benchmarks --benchmark_report_aggregates_only=false
```

## Benchmark Categories

### 1. Container Operations Benchmarks

**File**: `container_operations_bench.cpp`

Measures container performance:

- Container creation
- Set value operations
- Get value operations
- Multiple values (10, 100, 1000 items)
- Clone operations
- Clear operations

**Target Metrics**:
- Container creation: < 1μs
- Set value: < 500ns
- Get value: < 100ns
- Clone (100 items): < 100μs

### 2. Serialization Benchmarks

**File**: `serialization_bench.cpp`

Measures serialization performance:

- Small container serialization (2 values)
- Large container serialization (10, 100, 1000 values)
- Deserialization performance
- Round-trip serialization + deserialization

**Target Metrics**:
- Serialization: < 1μs per field
- Deserialization: < 1μs per field
- Round-trip overhead: < 2x single operation

### 3. Value Operations Benchmarks

**File**: `value_operations_bench.cpp`

Measures value type performance:

- String value creation
- Numeric value creation
- Value data retrieval (get_data)
- Value data modification (set_data)

**Target Metrics**:
- Value creation: < 100ns
- Data retrieval: < 50ns
- Data modification: < 200ns

## Baseline Results

**To be measured**: Run benchmarks and record results in `docs/BASELINE.md`

Expected baseline ranges (to be confirmed):

| Metric | Target | Acceptable |
|--------|--------|------------|
| Container Creation | < 1μs | < 10μs |
| Set Value | < 500ns | < 5μs |
| Get Value | < 100ns | < 1μs |
| Serialization (per field) | < 1μs | < 10μs |
| Deserialization (per field) | < 1μs | < 10μs |
| Clone (100 items) | < 100μs | < 1ms |

## Interpreting Results

### Understanding Benchmark Output

```
---------------------------------------------------------------
Benchmark                         Time           CPU Iterations
---------------------------------------------------------------
BM_Container_Create             456 ns        455 ns    1534891
```

- **Time**: Wall clock time per iteration
- **CPU**: CPU time per iteration
- **Iterations**: Number of times the benchmark was run

### Percentiles

For latency-sensitive benchmarks, focus on p95 and p99:

```bash
# Run with statistics
./build/benchmarks/container_benchmarks --benchmark_enable_random_interleaving=true
```

### Comparison

To compare before/after performance:

```bash
# Baseline
./build/benchmarks/container_benchmarks --benchmark_out=baseline.json --benchmark_out_format=json

# After changes
./build/benchmarks/container_benchmarks --benchmark_out=after.json --benchmark_out_format=json

# Compare (requires benchmark tools)
compare.py baseline.json after.json
```

## CI Integration

Benchmarks are run in CI on every PR to detect performance regressions.

See `.github/workflows/benchmarks.yml` for configuration.

## Troubleshooting

### Google Benchmark Not Found

```bash
# Check installation
find /usr -name "*benchmark*" 2>/dev/null

# Try pkg-config
pkg-config --modversion benchmark

# Reinstall
brew reinstall google-benchmark  # macOS
```

### Build Errors

```bash
# Clean build
rm -rf build
cmake -B build -S . -DCONTAINER_BUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
```

### Unstable Results

```bash
# Increase minimum time
./build/benchmarks/container_benchmarks --benchmark_min_time=10.0

# Disable CPU frequency scaling (Linux)
sudo cpupower frequency-set --governor performance
```

## Contributing

When adding new benchmarks:

1. Follow existing naming conventions (`BM_Category_SpecificTest`)
2. Use appropriate benchmark types (Fixture, Threaded, etc.)
3. Set meaningful labels and counters
4. Document target metrics in file header
5. Clean up resources in TearDown/after benchmark
6. Update this README with new benchmark description

## References

- [Google Benchmark Documentation](https://github.com/google/benchmark)
- [Benchmark Best Practices](https://github.com/google/benchmark/blob/main/docs/user_guide.md)
- [container_system Architecture](../README.md)

---

**Last Updated**: 2025-10-07
**Phase**: 0 - Foundation and Tooling
**Status**: Baseline measurement in progress
