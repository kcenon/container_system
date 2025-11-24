# REACT-004: Samples and Examples Reactivation

## Metadata
- **ID**: REACT-004
- **Priority**: LOW
- **Estimated Duration**: 2 days
- **Dependencies**: REACT-001, REACT-002
- **Status**: DONE
- **Assignee**: TBD
- **Related Documents**: [REACTIVATION_PLAN.md](../advanced/REACTIVATION_PLAN.md)

---

## Overview

### What are we changing?

Migrate the code in `samples/` and `examples/` directories that used the legacy API to the variant-based API to restore user-facing example code.

**Current State**:
- ~~Disabled at `CMakeLists.txt:451` samples~~ ENABLED
- ~~Disabled at `CMakeLists.txt:459` examples~~ ENABLED

**Purpose**:
- Provide examples for users to learn library usage
- Demonstrate new variant-based API patterns

---

## Changes

### How are we changing it?

#### 1. Samples Migration (1 day)

**Before (Legacy)**:
```cpp
#include "container/values/int_value.h"
#include "container/core/container.h"

int main() {
    container c;
    c.add(std::make_shared<int_value>("count", 42));
    c.add(std::make_shared<string_value>("name", "example"));

    auto val = c.get("count");
    std::cout << val->to_int() << std::endl;
}
```

**After (Modern)**:
```cpp
#include "container/internal/thread_safe_container.h"
#include "container/internal/variant_value_v2.h"

int main() {
    thread_safe_container c;
    c.set_value("count", 42);
    c.set_value("name", std::string("example"));

    if (auto val = c.get_optimized_value("count")) {
        if (auto* v = std::get_if<int32_t>(&val->data)) {
            std::cout << *v << std::endl;
        }
    }
}
```

**Additional Samples**:
- [x] SOO (Small Object Optimization) demo
- [x] Stack allocation benefits demonstration
- [x] Thread-safe usage example

#### 2. Examples (Tutorial) Migration (1 day)

**Tutorial List**:
1. **Basic Usage** - Fundamentals
   - Container creation
   - Value add/get/delete
   - Type conversion

2. **Type-Safe Operations** - Type-safe operations
   - `std::optional` pattern
   - Visitor pattern (exhaustive handling)
   - Compile-time type checking

3. **Performance Optimization** - Performance optimization
   - Stack vs. Heap allocation
   - Batch operations
   - Memory-efficient patterns

4. **Serialization** - Serialization
   - Binary serialization
   - JSON serialization
   - Round-trip examples

---

## Test Plan

### How will we test it?

#### 1. Build Verification
```bash
cmake -B build -DBUILD_CONTAINER_SAMPLES=ON -DBUILD_CONTAINER_EXAMPLES=ON
cmake --build build
```

#### 2. Execution Verification
```bash
# Run all samples
for sample in build/samples/*; do
    echo "Running $sample"
    $sample
done

# Run all examples
for example in build/examples/*; do
    echo "Running $example"
    $example
done
```

#### 3. Output Verification
- Compare expected output with actual output
- Confirm clean exit without errors

#### Success Criteria
- [x] All samples compile and run
- [x] All examples demonstrate new features
- [x] Integration with migration guide complete

---

## New Examples to Add

### SOO (Small Object Optimization) Example
```cpp
// soo_demo.cpp
#include "container/internal/optimized_value.h"

int main() {
    // Stack-allocated value (no heap)
    optimized_value small_val{"key", value_types::int_value, 42};

    // Shows memory efficiency
    std::cout << "Size of optimized_value: " << sizeof(optimized_value) << " bytes\n";
    std::cout << "vs Legacy shared_ptr<value>: 88 bytes\n";
}
```

### Thread-Safe Usage Example
```cpp
// thread_safe_demo.cpp
#include "container/internal/thread_safe_container.h"
#include <thread>

int main() {
    thread_safe_container container;

    // Multiple threads writing
    std::vector<std::thread> writers;
    for (int i = 0; i < 10; ++i) {
        writers.emplace_back([&, i]() {
            container.set_value("key_" + std::to_string(i), i);
        });
    }

    for (auto& t : writers) t.join();

    // Safe to read
    std::cout << "Container has " << container.size() << " values\n";
}
```

---

## Documentation

### README Update Items Required

1. **Quick Start** section update
2. **API Usage Examples** update
3. **Migration Guide** link addition

---

## Checklist

- [x] Migrate all files in samples/ directory
- [x] Migrate all files in examples/ directory
- [x] Add SOO example (included in basic_usage.cpp)
- [x] Add Thread-safe example (thread_safe_example.cpp)
- [x] Add Type-safe pattern example (included in basic_usage.cpp)
- [x] Enable samples in CMakeLists.txt
- [x] Enable examples in CMakeLists.txt
- [x] Update README (variant-based API documentation complete)
- [x] Verify all samples compile
- [x] Verify all samples run
- [x] Verify all examples compile
- [x] Verify all examples run

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-25
**Completed**: 2025-11-25
