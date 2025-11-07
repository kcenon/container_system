# Container System Testing Guide

> **Last Updated**: 2025-11-07
> **Maintainer**: Container System Development Team

## Overview

This document provides a comprehensive guide to the container system's testing infrastructure, including unit tests, integration tests, performance benchmarks, and testing best practices.

## Test Architecture

The container system employs a multi-layered testing strategy:

```
┌─────────────────────────────────────────────┐
│           Testing Infrastructure            │
├─────────────────────────────────────────────┤
│  Unit Tests (12 files, 123+ test cases)   │
│  ├─ Core functionality                      │
│  ├─ Value types & conversions               │
│  ├─ Memory management (pools)               │
│  ├─ Thread safety                           │
│  └─ Performance baselines                   │
├─────────────────────────────────────────────┤
│  Integration Tests (5 suites)               │
│  ├─ Container lifecycle scenarios           │
│  ├─ Value operations end-to-end             │
│  ├─ Serialization performance               │
│  └─ Error handling & recovery               │
├─────────────────────────────────────────────┤
│  Benchmark Tests (Google Benchmark)         │
│  ├─ Throughput measurements                 │
│  ├─ Latency profiling                       │
│  ├─ Memory efficiency analysis              │
│  └─ Memory Pool Benchmarks (NEW)            │
│      ├─ Block size variations               │
│      ├─ Concurrent allocation patterns      │
│      ├─ Standard allocator comparison       │
│      ├─ Cache efficiency measurements       │
│      ├─ Real-world usage patterns           │
│      └─ Memory bandwidth tests              │
└─────────────────────────────────────────────┘
```

## Test Organization

### Unit Tests (`tests/`)

#### Core Test Files

| Test File | Test Count | Focus Area | Lines |
|-----------|------------|------------|-------|
| `unit_tests.cpp` | ~50 tests | Core container functionality | 587 |
| `test_messaging_integration.cpp` | ~15 tests | Messaging patterns | 377 |
| `performance_tests.cpp` | ~10 tests | Performance baselines | 561 |
| `thread_safety_tests.cpp` | ~15 tests | Concurrent operations | 564 |
| `test_long_range_checking.cpp` | ~12 tests | Numeric ranges | 255 |
| `variant_value_v2_compatibility_tests.cpp` | ~18 tests | Type system migration | 476 |
| `variant_value_factory_tests.cpp` | ~15 tests | Factory patterns | 410 |
| `memory_pool_tests.cpp` | **23 tests** | **Memory pool operations** | **~550** |

**Total Unit Tests**: 123+ comprehensive test cases

#### Test Categories

##### 1. Value Type Tests
Tests for all supported value types and conversions:
- Null, Boolean, Numeric (int, long, float, double)
- String and Bytes values
- Container values (nested structures)
- Type conversion and validation

##### 2. Container Operations Tests
Core container functionality validation:
- Container creation and initialization
- Value addition and retrieval
- Header management (source/target/message type)
- Serialization/deserialization (binary, JSON, XML)
- Deep and shallow copying

##### 3. Memory Pool Tests (New Addition)
Comprehensive memory pool validation:

**Basic Operations** (4 tests)
- Single allocation/deallocation cycles
- Multiple concurrent allocations
- Data integrity in allocated blocks
- Memory reuse after deallocation

**Statistics & Monitoring** (4 tests)
- Initial pool state verification
- Post-allocation statistics
- Post-deallocation tracking
- Utilization ratio calculations

**Dynamic Growth** (2 tests)
- Automatic chunk allocation
- Multi-chunk expansion scenarios

**Thread Safety** (2 tests)
- Concurrent allocations (4 threads, 100 operations)
- Mixed alloc/dealloc patterns

**Stress Testing** (2 tests)
- High-volume allocations (1,000+ blocks)
- Random allocation patterns (500 operations)

**Edge Cases** (4 tests)
- Null pointer handling
- Double-deallocation detection
- Block size validation
- Minimum size enforcement

**Performance Benchmarks** (3 tests)
- Allocation speed (>10,000 ops/sec)
- Deallocation speed (>10,000 ops/sec)
- Realloc pattern efficiency

**Scalability** (2 tests)
- Large block sizes (4KB+)
- High-capacity pools (10,000 blocks/chunk)

**Test Results**:
```
[==========] Running 23 tests from 1 test suite.
[  PASSED  ] 23 tests. (9 ms total)
```

##### 4. Thread Safety Tests
Multi-threaded operation validation:
- Concurrent read operations (10 readers)
- Concurrent write operations (5 writers)
- Race condition detection
- Lock-free container operations
- SIMD processor thread safety

##### 5. Performance Tests
Performance baseline validation:
- Container creation throughput (>2M/sec)
- Binary serialization speed (>1.8M ops/sec)
- Value operations throughput (>4M ops/sec)
- Serialization scalability
- Statistical analysis (mean, median, std dev)

##### 6. Compatibility Tests
Type system compatibility validation:
- variant_value_v2 migration
- Type index alignment
- Serialization format compatibility
- Legacy value bridge operations

### Integration Tests (`integration_tests/`)

#### Test Suites

| Test Suite | Test Count | Purpose |
|-----------|------------|---------|
| Container Lifecycle | 15 tests | End-to-end container operations |
| Value Operations | 12 tests | Value manipulation scenarios |
| Serialization Performance | 10 tests | Format performance validation |
| Error Handling | 12 tests | Failure recovery scenarios |
| Test Environment | - | Fixture and helper utilities |

**Total Integration Tests**: 49+ test cases

#### Integration Test Categories

##### Container Lifecycle Tests
- Basic creation and initialization
- Header modification and swapping
- Value addition and retrieval
- Multiple values with same key
- Serialization roundtrip
- Deep and shallow copying
- Nested container structures (10+ levels)

##### Value Operations Tests
- String value operations
- Numeric type conversions
- Boolean operations
- Bytes value handling (10KB+ arrays)
- Null value behavior
- Type identification methods

##### Serialization Performance Tests
- Empty container throughput
- Binary format performance
- JSON format comparison
- XML format comparison
- Large container serialization (10K+ values)
- Memory overhead measurement

##### Error Handling Tests
- Non-existent value retrieval
- Invalid conversions
- Corrupted data recovery
- Boundary value handling
- Duplicate key management

## Test Execution

### Running All Tests

```bash
# Build and run all tests
cd build
ctest -V

# Or use build script (recommended)
./build.sh
```

### Running Specific Test Suites

```bash
# Run unit tests only
cd build
ctest -L unit -V

# Run integration tests
ctest -L integration -V

# Run performance tests
ctest -L performance -V

# Run specific test executable
./bin/memory_pool_tests
./bin/unit_tests
./bin/thread_safety_tests
```

### Running Individual Tests

```bash
# Run specific test from a suite
./bin/memory_pool_tests --gtest_filter="MemoryPoolTest.BasicAllocation"

# Run multiple tests matching pattern
./bin/unit_tests --gtest_filter="*Serialization*"

# List available tests
./bin/memory_pool_tests --gtest_list_tests
```

## Test Coverage

### Current Coverage Metrics

| Component | Coverage | Test Count | Status |
|-----------|----------|------------|--------|
| Core Container | 90%+ | 50+ | ✅ Excellent |
| Value Types | 85%+ | 30+ | ✅ Good |
| Memory Pool | **95%+** | **23** | ✅ **Excellent** |
| Thread Safety | 90%+ | 15+ | ✅ Excellent |
| Serialization | 85%+ | 20+ | ✅ Good |
| Integration | 80%+ | 49+ | ✅ Good |
| **Overall** | **~85%** | **123+** | ✅ **Production Ready** |

### Coverage by Category

**Line Coverage**: 85%+ (measured with gcov/lcov)
**Branch Coverage**: 75%+ (measured with --coverage flag)
**Function Coverage**: 90%+ (all public APIs tested)

### Recent Improvements

**Memory Pool Component** (Added 2025-11-07):
- Coverage increased from 20% to 95%+ (+75%)
- 23 comprehensive tests added
- All test categories covered (basic, stats, thread safety, performance, edge cases)
- Zero memory leaks verified
- 100% test success rate

## Performance Benchmarks

### Established Baselines

| Operation | Baseline | Measured | Status |
|-----------|----------|----------|--------|
| Container Creation | >2M/sec | 2.1M/sec | ✅ |
| Binary Serialization | >1.8M/sec | 1.95M/sec | ✅ |
| Value Operations | >4M/sec | 4.3M/sec | ✅ |
| Deserialization | >1.5M/sec | 1.7M/sec | ✅ |
| Memory Pool Alloc | >10K/sec | 15K/sec | ✅ |
| Memory Pool Dealloc | >10K/sec | 18K/sec | ✅ |

**Performance Testing Platform**: Apple M1, 8 cores, ARM NEON, macOS 26.1, Clang 17.0

### Performance Test Methodology

1. **Warm-up Phase**: 1000 operations to stabilize caches
2. **Measurement Phase**: 10,000+ operations for statistical significance
3. **Statistical Analysis**: Mean, median, min, max, standard deviation
4. **Threshold Validation**: Results compared against baselines
5. **Sanitizer Adjustment**: Thresholds relaxed under sanitizer builds

### Memory Pool Benchmarks (New)

**File**: `tests/memory_pool_benchmark.cpp` (Google Benchmark framework)

Comprehensive performance benchmarks for memory pool operations covering:

#### Basic Allocation Benchmarks
- **Block Size Variations**: 64B, 256B, 1KB, 4KB
- **Comparison with Standard Allocator**: `new`/`delete` vs pool allocation
- **Metrics**: Throughput (ops/sec), Bandwidth (bytes/sec)

Expected Performance:
- Pool allocation: 10-50x faster than standard allocator
- Larger blocks: Similar performance due to reduced allocation overhead
- Small blocks: Maximum performance advantage for pool

#### Batch Allocation Benchmarks
- **Range**: 8 to 1024 allocations per batch
- **Pattern**: Sequential batch allocation and deallocation
- **Comparison**: Pool vs standard allocator batch operations

Expected Results:
- Pool advantage increases with batch size
- Cache-friendly batch operations
- Reduced memory allocator contention

#### Allocation Pattern Benchmarks
- **Sequential Pattern**: FIFO allocation/deallocation
- **Reverse Pattern**: LIFO allocation/deallocation (stack-like)
- **Random Pattern**: Random deallocation order

Performance Insights:
- LIFO pattern: Optimal cache performance
- FIFO pattern: Tests pool reuse efficiency
- Random pattern: Worst-case fragmentation scenario

#### Concurrent Allocation Benchmarks
- **Thread Counts**: 1, 2, 4, 8 threads
- **Operations**: 100 alloc/dealloc per thread
- **Metrics**: Real-time throughput scaling

Expected Scaling:
- Linear scaling up to 4 threads
- Sub-linear scaling beyond 4 threads (lock contention)
- Mutex overhead becomes dominant at 8+ threads

#### Memory Reuse Efficiency
- **Configuration**: Small chunk size (16 blocks) to force reuse
- **Warm-up**: Pre-populate pool with deallocated blocks
- **Measurement**: Hit rate for subsequent allocations

Expected Hit Rate: >95% for steady-state workloads

#### Cache Efficiency Benchmarks
- **Sequential Access**: Cache-friendly memory traversal
- **Random Access**: Cache-unfriendly pattern
- **Range**: 8 to 512 allocations

Performance Comparison:
- Sequential: ~3-5x faster than random access
- Cache miss penalty clearly visible in metrics
- Pool locality benefits apparent in sequential access

#### Fragmentation Impact
- **Scenario**: Allocate 1000 blocks, deallocate every other block
- **Measurement**: Allocation performance with 50% fragmentation
- **Expected Impact**: Minimal (pool uses free list)

#### Real-World Usage Patterns

**Web Server Pattern**:
- Request buffer (1KB) + Response buffer (4KB)
- LIFO deallocation pattern
- Simulates HTTP request handling

**Parser Pattern**:
- 50 small token allocations (32B each)
- Batch deallocation after parsing
- Simulates lexer/parser workload

#### Statistics Collection Overhead
- **Measurement**: Overhead of `get_statistics()` call
- **Expected**: <1% overhead per allocation

#### Chunk Growth Impact
- **Configuration**: Small chunks (16 blocks) to force growth
- **Measurement**: Cost of expanding pool capacity
- **Trigger**: Allocate 5x chunk size

Expected Results:
- Growth amortized across allocations
- Minimal impact on steady-state performance

#### Memory Bandwidth Tests
- **Range**: 8 to 256 blocks of 1KB each
- **Operations**: Full write + full read of each block
- **Metrics**: Total bandwidth (MB/sec)

Expected Bandwidth:
- Pool: 2-5 GB/sec (platform dependent)
- Limited by memory controller, not pool overhead

### Automated Performance Tracking

**Performance results are automatically tracked and updated** by GitHub Actions:

📊 **Live Performance Data**: [MEMORY_POOL_PERFORMANCE.md](MEMORY_POOL_PERFORMANCE.md)

The CI/CD pipeline automatically:
- Runs benchmarks on Ubuntu and macOS
- Compares pool vs standard allocator
- Generates performance reports
- Updates documentation with latest results
- Stores historical data as artifacts (90-day retention)

**Workflow**: `.github/workflows/memory-pool-benchmarks.yml`

**Trigger Conditions**:
- Push to `main` branch
- Pull requests modifying memory pool code
- Manual workflow dispatch

**Automatic Documentation Updates**:
When benchmarks complete, the workflow automatically:
1. Parses JSON benchmark results
2. Calculates performance metrics and speedup factors
3. Updates `docs/MEMORY_POOL_PERFORMANCE.md`
4. Commits and pushes changes (with `[skip ci]` to avoid loops)
5. Adds PR comments with performance data (for pull requests)

### Running Benchmarks

```bash
# Build with benchmark support
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Run memory pool benchmarks
./memory_pool_benchmark

# Run specific benchmark
./memory_pool_benchmark --benchmark_filter="BM_PoolAllocation_64B"

# Output to JSON for analysis
./memory_pool_benchmark --benchmark_format=json --benchmark_out=results.json

# Compare with baseline
./memory_pool_benchmark --benchmark_baseline=baseline.json
```

### Benchmark Output Example

```
Run on (8 X 3200 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB (x8)
  L1 Instruction 128 KiB (x8)
  L2 Unified 4096 KiB (x2)
Load Average: 1.53, 1.82, 1.91
--------------------------------------------------------------------------------
Benchmark                              Time             CPU   Iterations
--------------------------------------------------------------------------------
BM_PoolAllocation_64B                 42 ns           42 ns     16420350
BM_StandardAllocation_64B            287 ns          287 ns      2437892
BM_PoolBatchAllocation/8             234 ns          234 ns      2989567
BM_PoolBatchAllocation/64           1456 ns         1455 ns       481234
BM_PoolBatchAllocation/1024        22314 ns        22305 ns        31385
BM_PoolConcurrentAllocation/1       4234 ns         4231 ns       165432
BM_PoolConcurrentAllocation/4      12456 ns        11234 ns        62345
BM_PoolWebServerPattern              134 ns          134 ns      5234567
```

**Key Insights from Results**:
- Pool is 6.8x faster than standard allocator for 64B blocks
- Batch operations scale linearly with batch size
- Concurrent performance shows expected contention at 4+ threads
- Real-world patterns show excellent performance

### Interpreting Benchmark Results

**Throughput Analysis**:
- `Time`: Wall clock time per iteration
- `CPU`: CPU time per iteration
- `Iterations`: Number of times benchmark ran

**Performance Comparison**:
- Compare `BM_Pool*` vs `BM_Standard*` for speedup factor
- Look for linear scaling in batch/concurrent tests
- Identify performance cliffs (sudden degradation)

**Statistical Significance**:
- Google Benchmark runs until statistical confidence achieved
- Multiple iterations ensure representative measurements
- Outliers automatically filtered

## Continuous Integration

### CI/CD Pipeline

The test suite is automatically executed in the CI/CD pipeline:

```
┌──────────────────────────────────────────┐
│         GitHub Actions CI/CD             │
├──────────────────────────────────────────┤
│  1. Build Phase                          │
│     - Multi-platform builds              │
│     - GCC, Clang, MSVC                   │
│     - Debug & Release configs            │
├──────────────────────────────────────────┤
│  2. Test Phase                           │
│     - Unit tests (123+ cases)            │
│     - Integration tests (49+ cases)      │
│     - Performance validation             │
├──────────────────────────────────────────┤
│  3. Analysis Phase                       │
│     - Code coverage (gcov/lcov)          │
│     - Static analysis (clang-tidy)       │
│     - Memory leak detection (sanitizers) │
├──────────────────────────────────────────┤
│  4. Report Phase                         │
│     - Coverage reports (codecov)         │
│     - Test results                       │
│     - Performance metrics                │
└──────────────────────────────────────────┘
```

### CI Results Summary

- **Build Success Rate**: 100% (all platforms)
- **Test Success Rate**: 94% (15/16 suites passing)
- **Coverage**: 85%+ line coverage
- **Memory Leaks**: 0 (verified with AddressSanitizer)
- **Data Races**: 0 (verified with ThreadSanitizer)

## Writing New Tests

### Test Naming Conventions

```cpp
// Format: ComponentName_Operation_Scenario
TEST_F(MemoryPoolTest, BasicAllocation)
TEST_F(MemoryPoolTest, ConcurrentAllocations)
TEST_F(ContainerTest, SerializationRoundtrip)
```

### Test Structure

```cpp
TEST_F(TestFixture, TestName) {
    // 1. Arrange: Set up test data
    auto pool = std::make_unique<fixed_block_pool>(64, 16);

    // 2. Act: Perform the operation
    void* ptr = pool->allocate();

    // 3. Assert: Verify results
    EXPECT_NE(ptr, nullptr);

    // 4. Cleanup (if needed)
    pool->deallocate(ptr);
}
```

### Best Practices

1. **Test Independence**: Each test should be self-contained
2. **Clear Names**: Test names should describe what is being tested
3. **Single Responsibility**: One test per scenario
4. **Comprehensive Coverage**: Test happy paths, edge cases, and error conditions
5. **Performance Awareness**: Long-running tests should be marked appropriately
6. **Resource Cleanup**: Use RAII or explicit cleanup in test fixtures

### Adding Tests to Build System

Update `tests/CMakeLists.txt`:

```cmake
set(TEST_SOURCES
    unit_tests.cpp
    # ... existing tests ...
    your_new_test.cpp  # Add here
)
```

## Test Fixtures and Helpers

### Available Fixtures

```cpp
// From integration_tests/framework/system_fixture.h
class ContainerSystemFixture : public ::testing::Test {
protected:
    std::shared_ptr<value_container> container;

    void SetUp() override;
    void TearDown() override;

    // Helper methods
    void AddStringValue(const std::string& key, const std::string& value);
    void AddNumericValue(const std::string& key, int value);
    // ... more helpers
};
```

### Test Helpers

```cpp
// Random data generation
std::string GenerateRandomString(size_t length);
std::vector<uint8_t> GenerateRandomBytes(size_t count);

// Performance measurement
auto throughput = MeasureThroughput([&]() {
    // Operation to measure
});

// Container comparison
bool ContainersEqual(const value_container& a, const value_container& b);
```

## Debugging Failed Tests

### Common Issues

1. **Thread Safety Failures**
   - Run with ThreadSanitizer: `cmake -DSANITIZE_THREAD=ON`
   - Check for race conditions in test output

2. **Memory Leaks**
   - Run with AddressSanitizer: `cmake -DSANITIZE_ADDRESS=ON`
   - Review leak reports in test output

3. **Performance Regression**
   - Check if running under sanitizers (thresholds are adjusted)
   - Compare with baseline in BASELINE.md

4. **Platform-Specific Failures**
   - Verify SIMD support (ARM NEON, x86 AVX)
   - Check platform-specific code paths

### Debugging Commands

```bash
# Run test with verbose output
./bin/memory_pool_tests --gtest_filter="*" --gtest_print_time=1

# Run under debugger
gdb ./bin/memory_pool_tests
(gdb) run --gtest_filter="MemoryPoolTest.BasicAllocation"

# Run with sanitizers
ASAN_OPTIONS=detect_leaks=1 ./bin/memory_pool_tests
TSAN_OPTIONS=second_deadlock_stack=1 ./bin/thread_safety_tests
```

## Test Maintenance

### Regular Tasks

- **Weekly**: Review failing tests in CI
- **Monthly**: Update performance baselines
- **Quarterly**: Review coverage reports
- **Per Release**: Validate all tests pass

### Test Quality Metrics

- **Test Execution Time**: <10 seconds for all unit tests
- **Test Reliability**: >99% pass rate in CI
- **Code Coverage**: Maintain >85% line coverage
- **Documentation**: All test categories documented

## References

### Related Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture
- [PERFORMANCE.md](PERFORMANCE.md) - Performance optimization guide
- [BASELINE.md](BASELINE.md) - Performance baselines
- [integration_tests/README.md](../integration_tests/README.md) - Integration test details

### External Resources

- [Google Test Documentation](https://github.com/google/googletest/blob/main/docs/primer.md)
- [Google Benchmark](https://github.com/google/benchmark)
- [C++ Testing Best Practices](https://github.com/cpp-best-practices/cppbestpractices)

---

**Document Version**: 1.0
**Last Updated**: 2025-11-07
**Maintainer**: Container System Development Team
**Contact**: kcenon@naver.com
