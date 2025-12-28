# Changelog

All notable changes to the Container System project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

> **Language:** **English** | [한국어](CHANGELOG_KO.md)

---

## [Unreleased]

### Security
- **TOCTOU Thread Safety Fix** (#190): Eliminate Time-Of-Check-Time-Of-Use vulnerability
  - Always acquire locks in `read_lock_guard` and `write_lock_guard`
  - Remove conditional locking based on runtime flag
  - Deprecate `enable_thread_safety()` and `disable_thread_safety()` methods (will be removed in v0.3.0)
  - All operations are now thread-safe by default

### Fixed
- **XML Entity Encoding** (#187): Add proper XML 1.0 entity encoding in `to_xml()`
  - Encode XML special characters: `&` -> `&amp;`, `<` -> `&lt;`, `>` -> `&gt;`, `"` -> `&quot;`, `'` -> `&apos;`
  - Encode control characters (0x00-0x1F except tab, newline, carriage return) as `&#xNN;`
  - Apply encoding to string values and header fields
  - Prevents invalid XML output and potential XML injection attacks
- **JSON String Escaping** (#186): Add proper RFC 8259 compliant escaping in `to_json()`
  - Escape special characters: `"`, `\`, newline, carriage return, tab, backspace, form feed
  - Escape control characters (U+0000-U+001F) as `\uXXXX` format
  - Apply escaping to both string values and field names
  - Apply escaping to header fields (source_id, target_id, message_type, version)
  - Prevents invalid JSON output and potential injection attacks

### Changed
- Remove fmt library dependency and use C++20 std::format exclusively (#168)
- Simplify formatter.h by removing conditional compilation branches
- Update CMake configuration to require C++20 std::format support

### Added
- **C++20 Concepts Integration** (#172): Apply C++20 concepts from common_system for improved type safety and clearer error messages
  - New `core/concepts.h` header with container-specific concepts
  - `ValueVariantType` concept for type-safe value storage
  - `Arithmetic`, `TriviallyCopyable` concepts for template constraints
  - `KeyValueCallback`, `MutableKeyValueCallback` concepts for iteration functions
  - Replaced `static_assert` with concept constraints for better compile-time errors
- Comprehensive documentation unification across ecosystem
- CHANGELOG.md and MIGRATION.md for better version tracking

---

## [1.0.0] - 2025-10-21

### Added
- **15 Value Types**: int, uint, long, ulong, double, bool, string, blob, array, map, and more
- **SIMD Optimization**: ARM NEON and x86 AVX support for high-performance operations
- **Multiple Serialization Formats**: Binary, JSON, XML support
- **Type-Safe Container**: Compile-time type safety with runtime flexibility
- **Zero-Copy Operations**: Efficient memory management for large data
- **Builder Pattern API**: Fluent interface for container construction
- **Memory Pool**: Optimized memory allocation for hot paths
- **Thread-Safe Operations**: Lock-free reads with mutex-protected writes
- **Integration Tests**: 49 comprehensive test cases (Phase 5)

### Performance
- Container creation: 2M containers/sec
- Serialization (binary): 1.8M containers/sec
- SIMD operations: 25M ops/sec
- Memory overhead: ~32 bytes per container

### Integration
- messaging_system: Seamless message container mapping
- network_system: Network transport integration
- database_system: Data persistence support

---

## [0.9.0] - 2025-09-15

### Added
- Initial separation from messaging_system
- Core value types implementation
- Basic serialization support

### Changed
- Modular architecture design
- Standalone build system

---

## [0.8.0] - 2025-08-01

### Added
- Binary serialization format
- Value type extensibility framework

### Fixed
- Memory leaks in array/map types
- Thread safety issues in concurrent access

---

## Version Numbering

This project uses Semantic Versioning:
- **MAJOR** version: Incompatible API changes
- **MINOR** version: Backwards-compatible functionality additions
- **PATCH** version: Backwards-compatible bug fixes

---

## Migration Guides

### Migrating to 1.0.0

See [MIGRATION.md](MIGRATION.md) for detailed migration instructions.

#### Quick Migration Example

**Before** (messaging_system 0.x):
```cpp
#include <messaging/container.h>
auto container = messaging::create_container();
```

**After** (container_system 1.0):
```cpp
#include <container/container.h>
auto container = container_system::create_container();
```

---

## Breaking Changes

### Version 1.0.0
- Namespace change: `messaging::container` → `container_system::container`
- Header paths changed to independent structure
- CMake target renamed: `messaging_container` → `kcenon::container_system`

---

## Performance Improvements

### Version 1.0.0
- SIMD optimization: +150% performance improvement
- Memory pool: -40% allocation overhead
- Lock-free reads: +200% concurrent read performance

---

## Known Issues

### Version 1.0.0
- Large types (>64 bytes) may impact cache efficiency
- JSON serialization slower than binary format (~4x)
- Windows platform requires C++20 for full SIMD support

---

## Support

- **Issues**: [GitHub Issues](https://github.com/kcenon/container_system/issues)
- **Discussions**: [GitHub Discussions](https://github.com/kcenon/container_system/discussions)
- **Email**: kcenon@naver.com

---

## License

This project is licensed under the BSD 3-Clause License - see the [LICENSE](LICENSE) file for details.

---

[Unreleased]: https://github.com/kcenon/container_system/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/kcenon/container_system/releases/tag/v1.0.0
[0.9.0]: https://github.com/kcenon/container_system/releases/tag/v0.9.0
[0.8.0]: https://github.com/kcenon/container_system/releases/tag/v0.8.0
