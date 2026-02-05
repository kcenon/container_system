# Namespace Migration Guide

> **Language:** **English** | [한국어](NAMESPACE_MIGRATION.kr.md)

## Overview

container_system will transition from `container_module` to `kcenon::container` namespace in a future major version to align with ecosystem conventions.

**Migration Path**: container_module (v1.x) → kcenon::container (v2.0+)
**Breaking Changes**: Yes (namespace, include paths)
**Compatibility**: Alias provided during transition period

---

## Why This Change?

### Current Ecosystem Namespaces

| System | Current Namespace | Pattern |
|--------|-------------------|---------|
| common_system | `kcenon::common` | ✅ Consistent |
| thread_system | `kcenon::thread` | ✅ Consistent |
| logger_system | `kcenon::logger` | ✅ Consistent |
| network_system | `kcenon::network` | ✅ Consistent |
| **container_system** | `container_module` | ❌ Inconsistent |

The `container_module` namespace breaks the `kcenon::<system>` naming convention, making ecosystem integration less intuitive and requiring special handling in user code.

---

## Current vs Future

| Version | Namespace | Include Path |
|---------|-----------|--------------|
| v1.x (current) | `container_module` | `<container/...>` |
| v2.0 (planned) | `kcenon::container` | `<kcenon/container/...>` |

---

## Timeline

| Version | Status | Description |
|---------|--------|-------------|
| **v1.x** | Current | Current namespace (`container_module`), fully supported |
| **v1.x+1** | Planned | Deprecation warnings introduced for old namespace |
| **v2.0** | Planned | New namespace becomes default, old namespace aliased |
| **v3.0** | Future | Old namespace removed |

---

## Migration Steps

### Before (v1.x)

```cpp
#include <container/container.h>
#include <container/values/int_value.h>
#include <container/values/string_value.h>

using namespace container_module;

auto container = value_container();
container.add("name", "John");
container.add("age", 30);

auto name = container.get_value<std::string>("name");
```

### After (v2.0)

```cpp
#include <kcenon/container/container.h>
#include <kcenon/container/values/int_value.h>
#include <kcenon/container/values/string_value.h>

using namespace kcenon::container;

auto container = value_container();
container.add("name", "John");
container.add("age", 30);

auto name = container.get_value<std::string>("name");
```

---

## Compatibility Layer (v2.0)

During the transition period, a compatibility alias will be provided to allow gradual migration without breaking existing code:

```cpp
// In kcenon/container/compat.h
namespace container_module = kcenon::container;
```

### Using the Compatibility Layer

If you cannot migrate immediately, include the compatibility header:

```cpp
#include <kcenon/container/compat.h>

// Your existing code continues to work
using namespace container_module;  // Aliased to kcenon::container
```

> **Note**: The compatibility layer is intended as a transitional aid. Plan to update to the new namespace before v3.0.

---

## CMake Target Changes

### Before (v1.x)

```cmake
find_package(container_system CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE kcenon::container_system)
```

### After (v2.0)

```cmake
find_package(container_system CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE kcenon::container)
```

> **Note**: The CMake target name change is planned to align with the namespace change.

---

## Migration Checklist

Use this checklist when migrating your codebase:

- [ ] Update include paths from `<container/...>` to `<kcenon/container/...>`
- [ ] Update `using namespace container_module` to `using namespace kcenon::container`
- [ ] Update explicit namespace references (e.g., `container_module::value_container` → `kcenon::container::value_container`)
- [ ] Update CMake targets if applicable
- [ ] Run tests to verify functionality
- [ ] Remove compatibility header includes after full migration

---

## Automated Migration (Future)

A migration script will be provided with v2.0 to assist with automatic code updates:

```bash
# Future command (not yet available)
./scripts/migrate-namespace.sh /path/to/your/project
```

---

## FAQ

### Q: When will v2.0 be released?

A: The release date for v2.0 has not been announced yet. Follow the repository for updates.

### Q: Will my v1.x code break immediately when v2.0 is released?

A: No. The compatibility layer will ensure your existing code continues to work. However, you will see deprecation warnings encouraging migration.

### Q: How long will the compatibility layer be supported?

A: The compatibility layer will be available throughout v2.x. It will be removed in v3.0.

### Q: Can I use the new namespace before v2.0?

A: No. The new namespace and include paths will only be available starting with v2.0.

---

## Related Documentation

- [General Migration Guide](MIGRATION.md) - Migrating from messaging_system
- [API Reference](../API_REFERENCE.md) - Complete API documentation
- [Changelog](../CHANGELOG.md) - Version history and changes

---

## Support

If you encounter issues during migration:

1. Check the [FAQ](FAQ.md) for common issues
2. Search [existing issues](https://github.com/kcenon/container_system/issues)
3. Create a new issue with the `migration` label

---

*Last updated: 2026-02*
