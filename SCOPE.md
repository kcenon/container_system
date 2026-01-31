# container_system Scope

## Purpose

This repository provides container-specific optimizations for common_system, with a focus on high-performance data serialization and parsing.

## What This Repository Contains

### Container-Specific Optimizations
- **fast_parser.h**: High-performance parsing optimizations for serialization formats (Binary, JSON, XML)
- Performance-critical parsing utilities with SIMD acceleration
- Container manipulation and access patterns optimized for messaging systems

### Core Container Infrastructure
- Type-safe key-value storage with compile-time checks
- Multiple serialization format support (Binary, JSON, XML, MessagePack)
- Thread-safe container operations
- SIMD-accelerated container operations

## What This Repository Does NOT Contain

### Generic Utilities (see common_system)
- Logging infrastructure
- Error handling framework
- Configuration management
- Dependency injection containers
- Abstract interfaces and design patterns

### Foundation Infrastructure (see common_system)
- Cross-cutting concerns (adapters, bootstrap)
- General-purpose utilities
- Common abstractions

## Relationship with common_system

```
common_system (Foundation)
       │
       │ provides foundation to
       ▼
container_system (Data Serialization + Optimizations)
       │
       │ used by
       ▼
messaging_system, network_system, database_system
```

- **common_system**: Provides foundational infrastructure (DI, logging, error handling, interfaces)
- **container_system**: Specializes in high-performance data containers, serialization, and parsing optimizations
- **Other systems**: Consume container_system for data exchange and storage

## Dependency Management

Dependencies on common_system are managed through:
- CMake FetchContent for automatic dependency resolution
- No manual path configuration required
- Clean separation of concerns

## Key Differentiators

| Aspect | container_system | common_system |
|--------|------------------|---------------|
| **Focus** | Data serialization & parsing | General infrastructure |
| **Optimization** | SIMD-accelerated operations | Design pattern abstractions |
| **Scope** | Container-specific | Cross-cutting concerns |
| **Dependencies** | Only common_system | None (foundation) |

## For New Contributors

If you're looking to:
- **Add generic utilities**: Contribute to common_system instead
- **Optimize parsing/serialization**: This is the right repository
- **Improve container performance**: This is the right repository
- **Add infrastructure patterns**: Contribute to common_system instead
