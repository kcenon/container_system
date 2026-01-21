# Migration Guide - Container System

> **Language:** **English** | [한국어](MIGRATION.kr.md)

## Overview

This guide helps you migrate from messaging_system's integrated container to the standalone container_system module.

**Migration Path**: messaging_system (< 0.9) → container_system (1.0+)
**Estimated Time**: 2-4 hours for typical applications
**Breaking Changes**: Yes (namespace, headers, CMake targets)

---

## Quick Migration Checklist

- [ ] Update CMakeLists.txt dependencies
- [ ] Replace header includes
- [ ] Update namespace references
- [ ] Rebuild and test
- [ ] Update integration points

---

## Step-by-Step Migration

### Step 1: Update CMakeLists.txt

**Before**:
```cmake
find_package(messaging_system CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE kcenon::messaging_system)
```

**After**:
```cmake
find_package(container_system CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE kcenon::container_system)
```

### Step 2: Update Header Includes

**Before**:
```cpp
#include <messaging/container.h>
#include <messaging/values/int_value.h>
#include <messaging/values/string_value.h>
```

**After**:
```cpp
#include <container/container.h>
#include <container/values/int_value.h>
#include <container/values/string_value.h>
```

### Step 3: Update Namespace

**Before**:
```cpp
using namespace messaging;

auto container = messaging::create_container();
container->set_value("key", "value");
```

**After**:
```cpp
using namespace container_system;

auto container = container_system::create_container();
container->set_value("key", "value");
```

### Step 4: Update API Calls

Most APIs remain unchanged, but factory functions have been renamed:

| Old API | New API |
|---------|---------|
| `messaging::create_container()` | `container_system::create_container()` |
| `messaging::deserialize_from_binary()` | `container_system::deserialize_from_binary()` |
| `messaging::deserialize_from_json()` | `container_system::deserialize_from_json()` |

---

## Code Migration Examples

### Example 1: Basic Container Usage

**Before** (messaging_system 0.x):
```cpp
#include <messaging/container.h>

void process_data() {
    auto container = messaging::create_container();
    container->set_value("id", 123);
    container->set_value("name", "test");

    auto data = container->serialize_to_binary();
    send(data);
}
```

**After** (container_system 1.0):
```cpp
#include <container/container.h>

void process_data() {
    auto container = container_system::create_container();
    container->set_value("id", 123);
    container->set_value("name", "test");

    auto data = container->serialize_to_binary();
    send(data);
}
```

### Example 2: Container with Custom Types

**Before**:
```cpp
#include <messaging/container.h>
#include <messaging/values/array_value.h>

auto container = messaging::create_container();
auto arr = std::make_shared<messaging::array_value>();
arr->push_back(1);
arr->push_back(2);
container->set_value("numbers", arr);
```

**After**:
```cpp
#include <container/container.h>
#include <container/values/array_value.h>

auto container = container_system::create_container();
auto arr = std::make_shared<container_system::array_value>();
arr->push_back(1);
arr->push_back(2);
container->set_value("numbers", arr);
```

---

## Integration Migration

### Migrating messaging_system Integration

If you're still using messaging_system alongside container_system:

```cpp
// Both libraries can coexist
#include <container/container.h>
#include <messaging/message.h>

// Create container with new system
auto container = container_system::create_container();
container->set_value("data", "payload");

// Pass to messaging_system
auto message = messaging_system::create_message_from_container(container);
```

### Build Configuration for Coexistence

```cmake
find_package(container_system CONFIG REQUIRED)
find_package(messaging_system CONFIG REQUIRED)

target_link_libraries(your_target PRIVATE
    kcenon::container_system
    kcenon::messaging_system  # Still needed for messaging
)
```

---

## Automated Migration Script

We provide a migration script to automate common changes:

```bash
# Download migration script
curl -O https://raw.githubusercontent.com/kcenon/container_system/main/scripts/migrate.sh
chmod +x migrate.sh

# Run migration on your source directory
./migrate.sh /path/to/your/source

# Review changes
git diff

# Rebuild
mkdir build && cd build
cmake ..
make
```

The script performs:
- Header include updates
- Namespace replacements
- CMakeLists.txt modifications

---

## Testing Your Migration

### 1. Compile Test

```bash
mkdir build && cd build
cmake ..
make
```

Expected: Clean build with no errors

### 2. Unit Tests

```bash
# Run your existing tests
ctest

# Or manually
./build/tests/your_tests
```

Expected: All tests pass

### 3. Integration Tests

```bash
# Test with actual dependencies
./build/integration_tests/container_integration_test
```

Expected: All 49 integration tests pass

---

## Common Migration Issues

### Issue 1: Namespace Conflicts

**Problem**: Both `messaging::container` and `container_system::container` in same file
**Solution**: Use explicit namespaces
```cpp
// Avoid: using namespace messaging; using namespace container_system;

// Better: Explicit namespace
auto old_container = messaging::create_container();
auto new_container = container_system::create_container();
```

### Issue 2: CMake Cache Issues

**Problem**: CMake still finds old messaging_system package
**Solution**: Clear CMake cache
```bash
rm -rf build
mkdir build && cd build
cmake ..
```

### Issue 3: Binary Incompatibility

**Problem**: Old serialized data doesn't deserialize
**Solution**: Container format is backward-compatible
```cpp
// Old data still works
auto old_binary = load_old_data();
auto container = container_system::deserialize_from_binary(old_binary); // OK
```

---

## Performance Comparison

After migration, you should see similar or better performance:

| Metric | messaging_system | container_system | Change |
|--------|------------------|------------------|--------|
| Creation | 1.8M/s | 2.0M/s | +11% |
| Serialization | 1.7M/s | 1.8M/s | +6% |
| SIMD ops | Not available | 25M/s | New |
| Memory | 48 bytes | 32 bytes | -33% |

---

## Rollback Plan

If migration fails, you can rollback:

```bash
# Revert changes
git checkout -- .

# Or restore from backup
cp -r backup_before_migration/* .

# Rebuild with old version
mkdir build && cd build
cmake -DUSE_MESSAGING_CONTAINERS=ON ..
make
```

---

## value_container → message_buffer Migration (v2.0+)

Starting from v2.0.0, we recommend using `message_buffer` instead of `value_container`.
The `message_buffer` name better describes the class's purpose: a serializable message
buffer for network transmission, avoiding confusion with STL containers.

### Why the Name Change?

| Issue | Old Name | Problem |
|-------|----------|---------|
| Naming conflict | `container` | Confuses with `std::vector`, `std::map` |
| Purpose unclear | `value_container` | Doesn't reveal serialization purpose |
| Redundant namespace | `container_module::container` | `container::container` is redundant |

### Quick Migration

**Before**:
```cpp
#include <container/container.h>

container_module::value_container msg;
container_module::value_container_ptr ptr = std::make_shared<container_module::value_container>();
```

**After**:
```cpp
#include <container/container.h>

container_module::message_buffer msg;
container_module::message_buffer_ptr ptr = std::make_shared<container_module::message_buffer>();
```

### Migration Steps

1. **Replace type names**:
   - `value_container` → `message_buffer`
   - `value_container_ptr` → `message_buffer_ptr`

2. **Namespace remains the same**: `container_module` (for now)

3. **API is unchanged**: All methods work identically

### Deprecation Timeline

| Version | Status |
|---------|--------|
| v2.0.0 | `message_buffer` alias introduced |
| v2.1.0 | `value_container_ptr` deprecated warning |
| v3.0.0 | `value_container` name removed (planned) |

### Compatibility

Both names can be used interchangeably during the migration period:

```cpp
// These are equivalent
container_module::value_container c1;
container_module::message_buffer c2;

// Type alias - both refer to the same class
static_assert(std::is_same_v<container_module::value_container,
                             container_module::message_buffer>);
```

---

## Support

Need help with migration?

- **Migration Issues**: [GitHub Issues](https://github.com/kcenon/container_system/issues)
- **Migration Questions**: [GitHub Discussions](https://github.com/kcenon/container_system/discussions)
- **Email Support**: kcenon@naver.com

---

**Last Updated**: 2026-01-22
**Migration Script Version**: 1.1.0
