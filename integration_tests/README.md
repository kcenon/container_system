# Container System Integration Tests

## Overview

This directory contains comprehensive integration tests for the container_system, covering real-world usage scenarios, performance validation, and error handling. The test suite follows the pattern established in common_system, thread_system, logger_system, and monitoring_system.

## Test Structure

```
integration_tests/
├── framework/
│   ├── system_fixture.h       # Base test fixture with container setup/teardown
│   └── test_helpers.h          # Utility functions for testing
├── scenarios/
│   ├── container_lifecycle_test.cpp    # Container lifecycle scenarios (15 tests)
│   └── value_operations_test.cpp       # Value type operations (12 tests)
├── performance/
│   └── serialization_performance_test.cpp  # Performance benchmarks (10 tests)
└── failures/
    └── error_handling_test.cpp         # Error scenarios and edge cases (12 tests)
```

## Test Suites

### 1. Container Lifecycle Tests (`scenarios/container_lifecycle_test.cpp`)

Tests container creation, initialization, and state transitions.

**Test Coverage (15 tests)**:
- Container creation and initialization
- Header modification and swapping
- Value addition and retrieval
- Multiple values with same key
- Serialization roundtrip (binary and array formats)
- Deep and shallow copying
- Nested container structures
- Multi-level nesting
- Value removal and clearing
- Empty container handling
- Mixed value types lifecycle

**Run individually**:
```bash
cd build
./bin/scenarios_container_lifecycle_test
```

### 2. Value Operations Tests (`scenarios/value_operations_test.cpp`)

Tests value type creation, conversions, and operations.

**Test Coverage (12 tests)**:
- String value operations
- Numeric type conversions
- Boolean value operations
- Bytes value operations
- Large bytes value handling
- Null value behavior
- Double precision preservation
- Integer edge cases and overflow
- Long long value handling
- Value type identification
- Special string characters
- Empty and whitespace strings

**Run individually**:
```bash
cd build
./bin/scenarios_value_operations_test
```

### 3. Serialization Performance Tests (`performance/serialization_performance_test.cpp`)

Validates performance against established baselines.

**Test Coverage (10 tests)**:
- Empty container creation throughput
- Binary serialization throughput
- Deserialization throughput
- Value addition throughput
- Serialization scalability (varying sizes)
- Memory overhead measurement
- JSON serialization performance
- XML serialization performance
- Large container serialization
- Nested container performance

**Performance Baselines**:
- Container creation: > 2M containers/second
- Binary serialization: > 1.8M operations/second
- Deserialization: > 1.5M operations/second
- Value operations: > 4M operations/second
- Memory per container: < 500 bytes baseline

**Run individually**:
```bash
cd build
./bin/performance_serialization_performance_test
```

### 4. Error Handling Tests (`failures/error_handling_test.cpp`)

Tests error scenarios and edge cases.

**Test Coverage (12 tests)**:
- Non-existent value retrieval
- Empty key operations
- Null value type conversions
- String to numeric conversion failures
- Invalid serialization data
- Corrupted header data
- Very long string values
- Numeric boundary values
- Rapid serialization stress
- Many duplicate keys
- Zero-length bytes values
- Special characters in keys

**Run individually**:
```bash
cd build
./bin/failures_error_handling_test
```

## Running Tests

### Run All Integration Tests

```bash
# From project root
mkdir build && cd build
cmake .. -DCONTAINER_BUILD_INTEGRATION_TESTS=ON
cmake --build .
ctest -L integration --output-on-failure
```

### Run by Category

```bash
# Scenario tests
ctest -L scenarios -V

# Performance tests
ctest -L performance -V

# Failure handling tests
ctest -L failures -V
```

### Run Individual Test Executable

```bash
cd build

# Run specific test suite
./bin/scenarios_container_lifecycle_test
./bin/scenarios_value_operations_test
./bin/performance_serialization_performance_test
./bin/failures_error_handling_test

# Run combined integration tests
./bin/all_integration_tests
```

### Run with Google Test Filters

```bash
# Run specific test case
./bin/scenarios_container_lifecycle_test --gtest_filter=*SerializationRoundtrip*

# Run multiple test cases
./bin/scenarios_value_operations_test --gtest_filter=*Numeric*:*Boolean*

# List all tests
./bin/scenarios_container_lifecycle_test --gtest_list_tests
```

## Build Options

### Enable Integration Tests

```bash
cmake .. -DCONTAINER_BUILD_INTEGRATION_TESTS=ON
```

### Enable Coverage

```bash
cmake .. -DCONTAINER_BUILD_INTEGRATION_TESTS=ON -DENABLE_COVERAGE=ON
cmake --build .
ctest -L integration
# Generate coverage report
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

### Debug Build

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCONTAINER_BUILD_INTEGRATION_TESTS=ON
```

### Release Build (for performance testing)

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DCONTAINER_BUILD_INTEGRATION_TESTS=ON
```

## Framework Components

### ContainerSystemFixture

Base test fixture providing:
- Automatic container setup and teardown
- Helper methods for value creation (string, numeric, bool, bytes)
- Serialization utilities
- Performance measurement functions
- Header verification helpers

**Usage Example**:
```cpp
class MyContainerTest : public ContainerSystemFixture
{
    // Test implementation
};

TEST_F(MyContainerTest, TestCase) {
    AddStringValue("key", "value");
    auto restored = RoundTripSerialize(true);
    // Assertions...
}
```

### TestHelpers

Utility functions for:
- Random data generation (strings, bytes)
- Value existence checking
- Container equality comparison
- Nested container creation
- Throughput measurement
- Serialization validation
- Performance overhead calculation

**Usage Example**:
```cpp
auto random_str = TestHelpers::GenerateRandomString(1000);
auto mixed_container = TestHelpers::CreateMixedTypeContainer();
double throughput = TestHelpers::MeasureThroughput(operation, 1000);
```

## CI/CD Integration

### GitHub Actions Workflow

The integration tests run automatically on:
- Push to `main` or `feat/phase5-*` branches
- Pull requests to `main`
- Manual workflow dispatch

**Matrix Strategy**:
- Ubuntu 22.04 (Debug and Release)
- macOS 13 (Debug and Release)

**Coverage Reporting**:
- Generated for Debug builds on Linux
- Uploaded to Codecov
- HTML reports available as artifacts

### Performance Baseline Validation

Separate job validates performance baselines:
- Runs in Release mode for accurate measurements
- Compares against documented baselines
- Reports deviations

## Test Metrics

### Total Test Count: 49 Tests

- **Scenarios**: 27 tests (15 lifecycle + 12 value operations)
- **Performance**: 10 tests
- **Failures**: 12 tests

### Coverage Targets

- Line coverage: > 80%
- Branch coverage: > 70%
- Function coverage: > 85%

### Performance Targets

Based on BASELINE.md:
- Container creation: > 2M/sec
- Binary serialization: > 1.8M ops/sec
- Deserialization: > 1.5M ops/sec
- Value operations: > 4M ops/sec

## Troubleshooting

### Test Timeouts

Individual tests have 300 second timeout. Performance tests have 600 second timeout.

If tests timeout:
1. Check system load
2. Run in Release mode for performance tests
3. Reduce iteration counts for debugging

### Compilation Errors

Ensure all dependencies are installed:
```bash
# Ubuntu
sudo apt-get install libgtest-dev libgmock-dev

# macOS
brew install googletest
```

### Test Failures

Common issues:
1. **Serialization format changes**: Update test expectations
2. **Performance regressions**: Check for debug build or system load
3. **Platform-specific behavior**: Review platform-specific test cases

### Coverage Generation Issues

If lcov fails:
```bash
# Install lcov
sudo apt-get install lcov

# Use ignore-errors flag
lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch
```

## Contributing

When adding new integration tests:

1. Choose the appropriate category (scenarios/performance/failures)
2. Follow the established naming convention
3. Use ContainerSystemFixture base class
4. Update test count in this README
5. Ensure tests are platform-independent
6. Document any new baselines or expectations
7. Add appropriate test labels in CMakeLists.txt

## Related Documentation

- [docs/performance/BASELINE.md](../docs/performance/BASELINE.md) - Performance baselines
- [README.md](../README.md) - Container system overview
- [tests/](../tests/) - Unit tests

## License

BSD 3-Clause License (see project root LICENSE file)
