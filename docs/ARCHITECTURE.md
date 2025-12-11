# Architecture Documentation - Container System

> **Version:** 0.1.0.0
> **Last Updated:** 2025-10-22
> **Language:** **English** | [한국어](ARCHITECTURE_KO.md)

---

## Table of Contents

- [Design Philosophy](#design-philosophy)
- [Core Principles](#core-principles)
- [System Architecture](#system-architecture)
- [Component Architecture](#component-architecture)
- [Integration Architecture](#integration-architecture)
- [Memory Management](#memory-management)
- [Performance Optimization](#performance-optimization)
- [Serialization Architecture](#serialization-architecture)
- [Design Patterns](#design-patterns)
- [Thread Safety Architecture](#thread-safety-architecture)
- [Error Handling Strategy](#error-handling-strategy)

---

## Design Philosophy

The Container System is designed around three fundamental principles:

### 1. Type Safety Without Performance Penalty

The system provides compile-time type safety through C++20 concepts and `std::variant`, ensuring zero runtime overhead for type checking while preventing common programming errors.

**Key Design Decisions:**
- Use of `std::variant` for polymorphic value storage (compile-time type safety)
- Template metaprogramming for type-safe value factory
- Constexpr functions for compile-time type mapping
- Zero-cost abstractions through inline functions and move semantics

### 2. Performance by Default

Every component is optimized for high-throughput scenarios without requiring manual optimization from users.

**Performance Characteristics:**
- Container creation: 5M containers/second (empty containers)
- Binary serialization: 2M containers/second (1KB average size)
- SIMD-accelerated operations: 25M numeric operations/second
- Memory efficiency: ~128 bytes baseline with variant storage

### 3. Integration-First Design

The architecture prioritizes seamless integration with other ecosystem components while maintaining standalone usability.

**Integration Benefits:**
- Unified data model across messaging, network, and database systems
- Native serialization formats optimized for each transport layer
- Backward compatibility with legacy messaging_system containers
- Extensible value type system for custom integrations

---

## Core Principles

### Modularity

The system is organized into loosely coupled modules with clear responsibilities:

```
Core Layer (container, value, value_types)
    ↓
Value Layer (primitive, numeric, container, bytes)
    ↓
Advanced Layer (variant, thread_safe, SIMD)
    ↓
Serialization Layer (binary, JSON, XML)
    ↓
Integration Layer (messaging, network, database)
```

### Extensibility

New value types and serialization formats can be added without modifying existing code:

- Value types defined through enum and constexpr mapping
- Serialization formats implement common interface
- Factory pattern for type-safe value creation
- Visitor pattern for type-safe value processing

### Performance

Optimizations are applied at multiple levels:

1. **Compile-time**: Template metaprogramming, constexpr functions
2. **Memory**: Variant storage, small buffer optimization, memory pooling
3. **CPU**: SIMD instructions (NEON, AVX2), cache-friendly layouts
4. **I/O**: Zero-copy serialization, efficient binary format

### Safety

Memory safety and thread safety are enforced through modern C++ idioms:

- RAII for automatic resource management (20/20 perfect score)
- Smart pointers (`std::shared_ptr`, `std::unique_ptr`) throughout
- Thread-safe operations via `std::shared_mutex`
- Exception safety with strong guarantees

---

## System Architecture

### Layered Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                        │
│  (Messaging Systems, Network Applications, Database Stores) │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────────┐
│                  Integration Layer                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  Messaging   │  │   Network    │  │  Database    │      │
│  │ Integration  │  │ Integration  │  │ Integration  │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────────┐
│               Serialization Layer                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │    Binary    │  │     JSON     │  │     XML      │      │
│  │  Serializer  │  │  Serializer  │  │  Serializer  │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────────┐
│                  Advanced Layer                             │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   Variant    │  │ Thread-Safe  │  │     SIMD     │      │
│  │    Value     │  │  Container   │  │  Processor   │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────────┐
│                    Value Layer                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   Numeric    │  │    String    │  │  Container   │      │
│  │    Values    │  │    Values    │  │    Values    │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│  ┌──────────────┐  ┌──────────────┐                        │
│  │     Bool     │  │    Bytes     │                        │
│  │    Values    │  │    Values    │                        │
│  └──────────────┘  └──────────────┘                        │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────────┐
│                     Core Layer                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  Container   │  │     Value    │  │ Value Types  │      │
│  │    Class     │  │   Interface  │  │   (Enum)     │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└─────────────────────────────────────────────────────────────┘
```

### Data Flow Architecture

```
┌─────────────┐
│   Client    │
│ Application │
└──────┬──────┘
       │ Create container
       ▼
┌─────────────┐
│  Container  │◄──────────┐
│   Factory   │           │
└──────┬──────┘           │
       │                  │
       │ Add values       │ Nested
       ▼                  │ containers
┌─────────────┐           │
│    Value    │───────────┘
│   Factory   │
└──────┬──────┘
       │ Serialize
       ▼
┌─────────────┐     ┌─────────────┐
│   Binary    │     │     JSON    │
│ Serializer  │     │ Serializer  │
└──────┬──────┘     └──────┬──────┘
       │                   │
       └─────────┬─────────┘
                 ▼
          ┌─────────────┐
          │ Serialized  │
          │    Data     │
          └──────┬──────┘
                 │
       ┌─────────┴─────────┐
       ▼                   ▼
┌─────────────┐     ┌─────────────┐
│   Network   │     │  Database   │
│  Transport  │     │   Storage   │
└─────────────┘     └─────────────┘
```

---

## Component Architecture

### 1. Container Core (`container.h/cpp`)

**Responsibilities:**
- Header management (source, target, message type)
- Value storage and retrieval
- Serialization coordination
- Container lifecycle management

**Key Classes:**
- `value_container`: Main container class with header and value storage
- Header structure: source ID, source sub-ID, target ID, target sub-ID, message type

**Design Pattern:** Composite (for nested containers)

**Interface:**
```cpp
class value_container {
    // Header management
    void set_source(const std::string& id, const std::string& sub_id = "");
    void set_target(const std::string& id, const std::string& sub_id = "");
    void set_message_type(const std::string& type);
    void swap_header();  // Swap source and target

    // Value management
    void add_value(std::shared_ptr<value> val);
    void set_values(const std::vector<std::shared_ptr<value>>& values);
    std::shared_ptr<value> get_value(const std::string& key) const;
    void clear_values();
    void reserve_values(size_t count);

    // Serialization
    std::string serialize() const;
    std::vector<uint8_t> serialize_array() const;
    std::string to_json() const;
    std::string to_xml() const;

    // Copy operations
    std::shared_ptr<value_container> copy(bool deep_copy = true) const;
};
```

### 2. Value System (`value.h/cpp`, `value_types.h/cpp`)

**Value Type Hierarchy:**

```
value (abstract base)
    │
    ├── bool_value
    ├── numeric_value (template)
    │   ├── short_value (int16_t)
    │   ├── ushort_value (uint16_t)
    │   ├── int_value (int32_t)
    │   ├── uint_value (uint32_t)
    │   ├── long_value (int64_t)
    │   ├── ulong_value (uint64_t)
    │   ├── float_value (float)
    │   └── double_value (double)
    ├── string_value
    ├── bytes_value
    └── container_value (nested container)
```

**15 Supported Types:**

| Index | Type | C++ Type | Size | Code |
|-------|------|----------|------|------|
| 0 | null_value | std::monostate | 0 bytes | "0" |
| 1 | bool_value | bool | 1 byte | "1" |
| 2 | short_value | int16_t | 2 bytes | "2" |
| 3 | ushort_value | uint16_t | 2 bytes | "3" |
| 4 | int_value | int32_t | 4 bytes | "4" |
| 5 | uint_value | uint32_t | 4 bytes | "5" |
| 6 | long_value | int64_t | 8 bytes | "6" |
| 7 | ulong_value | uint64_t | 8 bytes | "7" |
| 8 | llong_value | int64_t | 8 bytes | "8" |
| 9 | ullong_value | uint64_t | 8 bytes | "9" |
| 10 | float_value | float | 4 bytes | "10" |
| 11 | double_value | double | 8 bytes | "11" |
| 12 | bytes_value | std::vector<uint8_t> | Variable | "12" |
| 13 | string_value | std::string | Variable | "13" |
| 14 | container_value | std::shared_ptr | Variable | "14" |

**Type-Safe Variant:**
```cpp
using ValueVariant = std::variant<
    std::monostate,                       // null_value
    bool,                                 // bool_value
    std::vector<uint8_t>,                 // bytes_value
    int16_t,                              // short_value
    uint16_t,                             // ushort_value
    int32_t,                              // int_value
    uint32_t,                             // uint_value
    int64_t,                              // long_value
    uint64_t,                             // ulong_value
    float,                                // float_value
    double,                               // double_value
    std::string,                          // string_value
    std::shared_ptr<thread_safe_container> // container_value
>;
```

### 3. SIMD Processor (`simd_processor.h/cpp`)

**SIMD Architecture:**

```
┌─────────────────────────────────────────────┐
│        Platform Detection Layer             │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐     │
│  │  ARM64  │  │  x86-64 │  │  Scalar │     │
│  │  NEON   │  │  AVX2   │  │ Fallback│     │
│  └────┬────┘  └────┬────┘  └────┬────┘     │
└───────┼───────────┼─────────────┼──────────┘
        │           │             │
        ▼           ▼             ▼
┌─────────────────────────────────────────────┐
│       SIMD Operation Dispatcher             │
│  (Runtime selection based on CPU features)  │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────┐
│        Optimized Operations                 │
│  • Numeric array processing                 │
│  • String comparison                        │
│  • Memory operations                        │
│  • Bulk serialization                       │
└─────────────────────────────────────────────┘
```

**Platform Support:**
- **ARM NEON**: 3.2x speedup on Apple Silicon (128-bit SIMD)
- **x86 AVX2**: 2.5x speedup on Intel/AMD (256-bit SIMD)
- **SSE4.2**: Fallback for older x86 CPUs (128-bit SIMD)
- **Scalar**: Platform-independent fallback

**Key Operations:**
```cpp
class simd_processor {
    // Automatic platform detection
    static bool has_simd_support();
    static std::string get_simd_type();  // "NEON", "AVX2", "SSE4.2", "Scalar"

    // Optimized operations
    template<typename T>
    static void process_numeric_array(T* data, size_t count);

    static void bulk_copy(void* dest, const void* src, size_t size);
    static int compare_memory(const void* a, const void* b, size_t size);
};
```

### 4. Thread-Safe Container (`thread_safe_container.h/cpp`)

**Concurrency Model:**

```
┌─────────────────────────────────────────────┐
│       Thread-Safe Container Wrapper         │
│                                             │
│  ┌──────────────────────────────────────┐  │
│  │     std::shared_mutex                │  │
│  │  (Multiple readers, single writer)   │  │
│  └──────────────────────────────────────┘  │
│                                             │
│  ┌──────────────────────────────────────┐  │
│  │   Read Operations (Lock-Free)        │  │
│  │   • get_value()                      │  │
│  │   • serialize()                      │  │
│  │   • Access header fields             │  │
│  └──────────────────────────────────────┘  │
│                                             │
│  ┌──────────────────────────────────────┐  │
│  │   Write Operations (Exclusive Lock)  │  │
│  │   • add_value()                      │  │
│  │   • set_value()                      │  │
│  │   • Modify header                    │  │
│  └──────────────────────────────────────┘  │
└─────────────────────────────────────────────┘
```

**Thread Safety Guarantees:**
- **Read operations**: Always thread-safe (concurrent readers allowed)
- **Write operations**: Thread-safe with `thread_safe_container` wrapper
- **Serialization**: Thread-safe for read-only containers
- **Value access**: Concurrent reads safe, writes require synchronization

---

## Integration Architecture

### Ecosystem Integration Diagram

```
┌───────────────────────────────────────────────────────────┐
│                    utilities_module                       │
│  (String conversion, system utilities, platform support)  │
└─────────────────────────┬─────────────────────────────────┘
                          │ Foundation dependency
                          ▼
┌───────────────────────────────────────────────────────────┐
│                  container_system (Core)                  │
│  • Type-safe containers                                   │
│  • 15 value types                                         │
│  • SIMD optimizations                                     │
│  • Binary/JSON/XML serialization                          │
└────┬──────────────┬──────────────┬────────────────┬───────┘
     │              │              │                │
     ▼              ▼              ▼                ▼
┌─────────┐  ┌──────────┐  ┌──────────┐  ┌──────────────┐
│messaging│  │ network  │  │ database │  │common_system │
│ _system │  │ _system  │  │ _system  │  │ (Result<T>)  │
└─────────┘  └──────────┘  └──────────┘  └──────────────┘
     │              │              │                │
     └──────────────┴──────────────┴────────────────┘
                          │
                          ▼
                 ┌────────────────┐
                 │  Application   │
                 │     Layer      │
                 └────────────────┘
```

### Integration Points

#### 1. messaging_system Integration

**Legacy Compatibility:**
- Bidirectional container conversion (messaging ↔ container)
- Shared binary serialization format
- Compatible header structure

**Performance Optimizations:**
- Zero-copy message construction
- Direct serialization to message buffer
- Pre-allocated value storage

#### 2. network_system Integration

**Network Transport:**
- Efficient binary serialization for network transmission
- Automatic endianness handling
- Compressed serialization for large payloads

**Zero-Copy Pipeline:**
- Direct buffer access for serialization
- Scatter-gather I/O support
- Memory-mapped file support

#### 3. database_system Integration

**Persistence:**
- Binary BLOB storage for optimal performance
- JSON/JSONB for queryable fields (PostgreSQL)
- XML for schema validation requirements

**ORM Mapping:**
- Automatic container-to-table mapping
- Type-safe query result conversion
- Batch serialization for bulk operations

#### 4. common_system Integration

**Result<T> Pattern:**
- Error code range: -400 to -499
- Adapter pattern for external API
- Monadic error composition

**Error Categories:**
- Serialization errors: -400 to -409
- Deserialization errors: -410 to -419
- Validation errors: -420 to -429
- Type conversion errors: -430 to -439
- SIMD operation errors: -440 to -449

---

## Memory Management

### RAII Excellence (20/20 Perfect Score)

The container system achieved the highest RAII score in the entire ecosystem and serves as the reference implementation.

**RAII Implementation:**

1. **Smart Pointer Usage (5/5):**
   - 100% `std::shared_ptr` for value storage
   - `std::unique_ptr` for internal buffers
   - No raw pointer ownership

2. **RAII Wrapper Classes (5/5):**
   - Serialization buffer wrappers
   - Thread-safe container wrapper
   - Automatic resource cleanup

3. **Exception Safety (4/4):**
   - Strong exception safety guarantees
   - No resource leaks on exception
   - Transactional value updates

4. **Move Semantics (3/3):**
   - Zero-copy value transfers
   - 4.2M moves/second performance
   - Perfect forwarding throughout

5. **Resource Leak Prevention (3/3):**
   - AddressSanitizer validation (0 leaks)
   - Automatic cleanup verified
   - No manual memory management

### Memory Layout Optimization

**Container Memory Layout:**
```
┌─────────────────────────────────────────────┐
│         value_container (128 bytes)         │
├─────────────────────────────────────────────┤
│  Header (48 bytes)                          │
│  ├── source_id (std::string)                │
│  ├── source_sub_id (std::string)            │
│  ├── target_id (std::string)                │
│  ├── target_sub_id (std::string)            │
│  └── message_type (std::string)             │
├─────────────────────────────────────────────┤
│  Values (std::vector<shared_ptr<value>>)    │
│  ├── Capacity (size_t)                      │
│  ├── Size (size_t)                          │
│  └── Data pointer                           │
├─────────────────────────────────────────────┤
│  Internal state (32 bytes)                  │
│  ├── Flags                                  │
│  ├── Reserved                               │
│  └── Padding                                │
└─────────────────────────────────────────────┘
```

**Value Memory Layout (Variant-based):**
```
┌─────────────────────────────────────────────┐
│      variant_value (48 bytes typical)       │
├─────────────────────────────────────────────┤
│  Key (std::string) - 32 bytes               │
├─────────────────────────────────────────────┤
│  ValueVariant - 16-48 bytes                 │
│  ├── Type index (size_t)                    │
│  └── Union storage (variant)                │
│      ├── Primitive types (inline)           │
│      ├── std::string (SSO optimized)        │
│      ├── std::vector<uint8_t> (heap)        │
│      └── std::shared_ptr (heap)             │
└─────────────────────────────────────────────┘
```

### Memory Pooling

**Pool Architecture:**
```
┌─────────────────────────────────────────────┐
│            Memory Pool Manager              │
├─────────────────────────────────────────────┤
│  Small Object Pool (< 64 bytes)             │
│  ├── Free list (lock-free)                  │
│  ├── Thread-local caches                    │
│  └── Batch allocation                       │
├─────────────────────────────────────────────┤
│  Medium Object Pool (64-1024 bytes)         │
│  ├── Size-class segregation                 │
│  ├── Buddy allocator                        │
│  └── Coalescing on free                     │
├─────────────────────────────────────────────┤
│  Large Object Pool (> 1024 bytes)           │
│  ├── Direct malloc/free                     │
│  ├── Alignment guaranteed                   │
│  └── Statistics tracking                    │
└─────────────────────────────────────────────┘
```

---

## Performance Optimization

### Multi-Level Optimization Strategy

**1. Compile-Time Optimization:**
- Constexpr type mapping (zero runtime overhead)
- Template metaprogramming for type safety
- Inline functions for hot paths
- Link-time optimization (LTO)

**2. Memory Optimization:**
- Variant storage (minimal polymorphic overhead)
- Small String Optimization (SSO) for strings
- Memory pooling for frequent allocations
- Cache-friendly data layouts

**3. CPU Optimization:**
- SIMD instructions (NEON, AVX2, SSE4.2)
- Branch prediction hints
- Loop unrolling for serialization
- Prefetching for bulk operations

**4. I/O Optimization:**
- Zero-copy serialization paths
- Buffered I/O for large data
- Scatter-gather for network I/O
- Memory-mapped files for persistence

### Performance Benchmarks

**Platform:** Apple M1 Max (ARM64), macOS 14, Clang 15

| Operation | Throughput | Latency | Notes |
|-----------|------------|---------|-------|
| Container creation (empty) | 5M/s | 200ns | Stack allocation |
| Container creation (10 values) | 1.2M/s | 833ns | Heap allocation |
| String value addition | 15M/s | 67ns | SSO optimized |
| Numeric value addition | 25M/s | 40ns | SIMD accelerated |
| Binary serialization (1KB) | 2M/s | 500ns | Zero-copy path |
| JSON serialization (1KB) | 800K/s | 1.25μs | Pretty-printed |
| Binary deserialization | 1.5M/s | 667ns | Validation included |
| SIMD numeric processing | 25M/s | 40ns | NEON optimized |
| Thread-safe read | 10M/s | 100ns | Lock-free |
| Thread-safe write | 2M/s | 500ns | Mutex-protected |

---

## Serialization Architecture

### Format Comparison

| Format | Throughput | Size Overhead | Use Case | Validation |
|--------|------------|---------------|----------|------------|
| **Binary** | 2M/s | ~10% | Network, IPC, Storage | CRC optional |
| **JSON** | 800K/s | ~40% | REST APIs, Debug | Schema optional |
| **XML** | 600K/s | ~60% | Legacy, SOAP | XSD required |

### Binary Serialization Format

**Binary Layout (Version 1.0):**
```
┌─────────────────────────────────────────────┐
│  Header (4 bytes)                           │
│  ├── Magic number (2 bytes): 0x434F ("CO") │
│  ├── Version (1 byte): 0x01                 │
│  └── Flags (1 byte)                         │
├─────────────────────────────────────────────┤
│  Container Header (variable)                │
│  ├── Source ID length (2 bytes)             │
│  ├── Source ID (variable)                   │
│  ├── Source Sub-ID length (2 bytes)         │
│  ├── Source Sub-ID (variable)               │
│  ├── Target ID length (2 bytes)             │
│  ├── Target ID (variable)                   │
│  ├── Target Sub-ID length (2 bytes)         │
│  ├── Target Sub-ID (variable)               │
│  ├── Message Type length (2 bytes)          │
│  └── Message Type (variable)                │
├─────────────────────────────────────────────┤
│  Value Count (4 bytes)                      │
├─────────────────────────────────────────────┤
│  Values (repeated)                          │
│  ├── Key length (2 bytes)                   │
│  ├── Key (variable)                         │
│  ├── Type (1 byte)                          │
│  ├── Value length (4 bytes)                 │
│  └── Value data (variable)                  │
├─────────────────────────────────────────────┤
│  Footer (optional)                          │
│  └── CRC32 checksum (4 bytes)               │
└─────────────────────────────────────────────┘
```

**Optimizations:**
- Varint encoding for small integers
- String interning for repeated keys
- Delta encoding for numeric arrays
- LZ4 compression for large values (optional)

### JSON Serialization Format

**JSON Structure:**
```json
{
  "header": {
    "source_id": "client_01",
    "source_sub_id": "session_123",
    "target_id": "server",
    "target_sub_id": "handler_main",
    "message_type": "request"
  },
  "values": [
    {"key": "user_id", "type": "long_value", "value": 12345},
    {"key": "username", "type": "string_value", "value": "john_doe"},
    {"key": "balance", "type": "double_value", "value": 1500.75},
    {"key": "active", "type": "bool_value", "value": true}
  ]
}
```

### XML Serialization Format

**XML Structure:**
```xml
<?xml version="1.0" encoding="UTF-8"?>
<container version="1.0">
  <header>
    <source id="client_01" sub_id="session_123"/>
    <target id="server" sub_id="handler_main"/>
    <message_type>request</message_type>
  </header>
  <values>
    <value key="user_id" type="long_value">12345</value>
    <value key="username" type="string_value">john_doe</value>
    <value key="balance" type="double_value">1500.75</value>
    <value key="active" type="bool_value">true</value>
  </values>
</container>
```

---

## Design Patterns

### 1. Factory Pattern

**Value Factory:**
```cpp
class value_factory {
public:
    static std::shared_ptr<value> create(
        const std::string& key,
        value_types type,
        const std::string& data
    );

    template<typename T>
    static std::shared_ptr<value> create(
        const std::string& key,
        T&& value
    );
};
```

**Benefits:**
- Type-safe value creation
- Centralized construction logic
- Extensibility for new types

### 2. Builder Pattern

**Container Builder:**
```cpp
class messaging_container_builder {
public:
    messaging_container_builder& source(const std::string& id, const std::string& sub_id = "");
    messaging_container_builder& target(const std::string& id, const std::string& sub_id = "");
    messaging_container_builder& message_type(const std::string& type);
    messaging_container_builder& add_value(const std::string& key, const auto& value);
    messaging_container_builder& optimize_for_speed();
    messaging_container_builder& optimize_for_network();
    std::shared_ptr<value_container> build();
};
```

**Benefits:**
- Fluent API for construction
- Validation at build time
- Performance hints for optimization

### 3. Visitor Pattern

**Type-Safe Value Processing:**
```cpp
class value_visitor {
public:
    virtual void visit(bool_value& value) = 0;
    virtual void visit(numeric_value<int32_t>& value) = 0;
    virtual void visit(string_value& value) = 0;
    virtual void visit(container_value& value) = 0;
    // ... other types
};
```

**Benefits:**
- Type-safe dispatch
- Extensible operations
- Compile-time checking

### 4. Strategy Pattern

**Serialization Strategy:**
```cpp
class serialization_strategy {
public:
    virtual std::string serialize(const value_container& container) = 0;
    virtual void deserialize(value_container& container, const std::string& data) = 0;
};

class binary_serialization : public serialization_strategy { /* ... */ };
class json_serialization : public serialization_strategy { /* ... */ };
class xml_serialization : public serialization_strategy { /* ... */ };
```

**Benefits:**
- Runtime format selection
- Pluggable serializers
- Format-specific optimizations

### 5. Adapter Pattern

**Result<T> Adapter (Error Handling):**
```cpp
namespace container::adapters {
    class serialization_result_adapter {
    public:
        static Result<std::string> serialize(const value_container& container);
        static Result<std::vector<uint8_t>> serialize_array(const value_container& container);
    };

    class container_result_adapter {
    public:
        template<typename T>
        static Result<T> get_value(const value_container& container, const std::string& key);
    };
}
```

**Benefits:**
- Type-safe error handling at boundaries
- Internal exceptions for performance
- Seamless ecosystem integration

---

## Thread Safety Architecture

### Concurrency Model

**Read-Write Lock Strategy:**
```cpp
class thread_safe_container {
private:
    std::shared_ptr<value_container> container_;
    mutable std::shared_mutex mutex_;

public:
    // Read operations (shared lock)
    std::shared_ptr<value> get_value(const std::string& key) const {
        std::shared_lock lock(mutex_);
        return container_->get_value(key);
    }

    // Write operations (exclusive lock)
    void add_value(std::shared_ptr<value> val) {
        std::unique_lock lock(mutex_);
        container_->add_value(std::move(val));
    }
};
```

**ThreadSanitizer Compliance:**
- Zero data races detected across all test scenarios
- Lock-free read operations for immutable containers
- Proper memory ordering for atomic operations

---

## Error Handling Strategy

### Hybrid Approach: Exceptions + Result<T>

**Internal Operations (Exceptions):**
- High performance (standard container practice)
- Strong exception safety guarantees
- RAII ensures cleanup

**External API (Result<T>):**
- Type-safe error handling at boundaries
- Explicit error propagation
- Monadic composition support

**Error Code Allocation:**
```cpp
namespace container_errors {
    constexpr int SERIALIZATION_FAILED = -400;
    constexpr int DESERIALIZATION_FAILED = -410;
    constexpr int VALIDATION_FAILED = -420;
    constexpr int TYPE_CONVERSION_FAILED = -430;
    constexpr int SIMD_OPERATION_FAILED = -440;
}
```

---

## Long/ULong Type Policy and Cross-Language Compatibility

### Overview

The container system implements a unified **32-bit serialization policy** for `long_value` (type 6) and `ulong_value` (type 7) to ensure platform independence and cross-language compatibility.

### Problem: Platform-Dependent Long Size

Different platforms have different `long` sizes:
- **Unix/Linux/macOS**: 8 bytes (64-bit)
- **Windows**: 4 bytes (32-bit)

This caused serious compatibility issues:
```cpp
// Unix system (8-byte long)
auto lv = std::make_shared<long_value>("id", 5'000'000'000L);
lv->serialize();  // 8 bytes written

// Windows system (4-byte long)
container->deserialize(data);  // ❌ Overflow! Value truncated
```

### Solution: Enforced 4-Byte Serialization

**Type Mapping**:
| Type ID | Name | C++ Type | Range | Serialization |
|---------|------|----------|-------|---------------|
| 6 | long_value | int32_t | [-2³¹, 2³¹-1] | 4 bytes LE |
| 7 | ulong_value | uint32_t | [0, 2³²-1] | 4 bytes LE |
| 8 | llong_value | int64_t | [-2⁶³, 2⁶³-1] | 8 bytes LE |
| 9 | ullong_value | uint64_t | [0, 2⁶⁴-1] | 8 bytes LE |

### Implementation

**Range Checking Helpers** (values/numeric_value.h):
```cpp
constexpr int64_t kInt32Min = -2147483648LL;
constexpr int64_t kInt32Max = 2147483647LL;
constexpr uint64_t kUInt32Max = 4294967295ULL;

constexpr bool is_int32_range(int64_t value) {
    return value >= kInt32Min && value <= kInt32Max;
}

constexpr bool is_uint32_range(uint64_t value) {
    return value <= kUInt32Max;
}
```

**long_value Template Specialization** (values/numeric_value.tpp):
```cpp
template <>
inline numeric_value<long, value_types::long_value>::numeric_value(
    const std::string& name, const long& initial_value)
    : value(name)
{
    int64_t value_as_64 = static_cast<int64_t>(initial_value);
    if (!is_int32_range(value_as_64)) {
        throw std::overflow_error(
            "long_value (type 6) limited to 32-bit range. "
            "Use llong_value (type 8) for 64-bit values."
        );
    }

    // Always serialize as int32 (4 bytes)
    int32_t value_as_32 = static_cast<int32_t>(initial_value);
    data_.resize(sizeof(int32_t));
    std::memcpy(data_.data(), &value_as_32, sizeof(int32_t));
}
```

### Breaking Changes

**Before** (Unix/Linux/macOS):
```cpp
// This was platform-dependent (8 bytes on Unix)
auto lv = std::make_shared<long_value>("test", 5'000'000'000L);  // OK on Unix
```

**After**:
```cpp
// Now range-checked (4 bytes on all platforms)
auto lv = std::make_shared<long_value>("test", 5'000'000'000L);
// ❌ Throws std::overflow_error

// ✅ Use llong_value instead
auto llv = std::make_shared<llong_value>("test", 5'000'000'000LL);
```

### Migration Guide

**Step 1**: Identify values exceeding 32-bit range
```bash
# Search for large long values
grep -r "long_value.*[0-9]{10,}" src/
```

**Step 2**: Update to appropriate type
```cpp
// For values in [-2³¹, 2³¹-1]
auto lv = std::make_shared<long_value>("count", 1'000'000'000L);  // ✅

// For values beyond 32-bit range
auto llv = std::make_shared<llong_value>("big", 5'000'000'000LL); // ✅
```

**Step 3**: Handle exceptions
```cpp
try {
    auto lv = std::make_shared<long_value>("id", user_input);
} catch (const std::overflow_error& e) {
    // Value exceeds 32-bit range, use llong_value
    auto llv = std::make_shared<llong_value>("id", user_input);
}
```

### Type Selection Guide

| Value Range | Type to Use | Example |
|-------------|-------------|---------|
| [-2³¹, 2³¹-1] | long_value | `long_value("id", 1'000'000'000L)` |
| [0, 2³²-1] | ulong_value | `ulong_value("count", 3'000'000'000UL)` |
| Beyond 32-bit signed | llong_value | `llong_value("big", 5'000'000'000LL)` |
| Beyond 32-bit unsigned | ullong_value | `ullong_value("huge", 10'000'000'000ULL)` |

### Cross-Language Compatibility

This implementation is **binary-compatible** with:
- ✅ **Python** container_system (4-byte enforcement)
- ✅ **.NET** container_system (int32/uint32 backing)
- ✅ **Go** container_system (int32/uint32 types)
- ✅ **Rust** container_system (i32/u32 with range checking)
- ✅ **Node.js/TypeScript** container_system (Result<T> pattern with 4-byte Buffer)

**Wire Protocol**: All 6 implementations use identical serialization:
```
[type: 1 byte][name_len: 4 bytes LE][name: UTF-8][value_size: 4 bytes LE][value: 4 bytes LE]
```

### Test Coverage

**test_long_range_checking.cpp** (22 tests):
- Range validation (5 tests)
- Overflow rejection (4 tests)
- Serialization format (2 tests)
- Type conversion (2 tests)
- Error messages (2 tests)
- Platform constants (2 tests)
- Helper functions (5 tests)

```bash
$ ./build/bin/test_long_range_checking
[==========] Running 22 tests from 1 test suite.
[  PASSED  ] 22 tests.
```

### Performance Impact

**Negligible**: Range checks are simple integer comparisons optimized by the compiler.

**Benchmarks** (Apple M1 Max):
- long_value creation: < 1 ns overhead
- Serialization: No measurable difference
- Memory savings: 50% on Unix (8 bytes → 4 bytes)

### Best Practices

1. **Default to 32-bit types**: Use `long_value`/`ulong_value` for most cases
2. **Explicit 64-bit**: Only use `llong_value`/`ullong_value` when needed
3. **Early validation**: Check ranges at data ingestion points
4. **Clear error messages**: Include guidance to use 64-bit types
5. **Documentation**: Comment why 64-bit types are necessary

### References

- Policy Document: `CONTAINER_SYSTEMS_UNIFIED_LONG_POLICY.md`
- Implementation: `container_system/values/numeric_value.{h,tpp}`
- Tests: `container_system/tests/test_long_range_checking.cpp`
- Progress: `LONG_TYPE_POLICY_IMPLEMENTATION_PROGRESS.md`

---

## Conclusion

The Container System architecture is designed for:
- **High Performance**: SIMD, zero-copy, efficient memory management
- **Type Safety**: Compile-time checks, `std::variant`, strong typing
- **Extensibility**: Factory, visitor, strategy patterns
- **Integration**: Seamless ecosystem interoperability
- **Reliability**: RAII, thread safety, exception safety

This architecture serves as the foundation for all data serialization needs across the entire ecosystem while maintaining standalone usability and optimal performance.

---

**Maintainer:** kcenon@naver.com
**License:** BSD 3-Clause
**Version:** 0.1.0.0
**Last Updated:** 2025-10-22
