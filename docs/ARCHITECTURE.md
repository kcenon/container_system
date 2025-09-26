Architecture Overview
=====================

Purpose
- container_system is a type‑safe, high‑performance value/container framework with SIMD‑accelerated operations and efficient serialization (binary/JSON/XML).
- It serves as a shared data model for messaging/network/database subsystems.

Layers
- Core: value, container, value_types, variant storage
- Internal: thread_safe_container, SIMD processors
- Utilities: string conversion, format helpers (local stubs to avoid hard deps)
- Integration: messaging_integration for seamless message assembly/serialization

Integration Topology
```
container_system ──► messaging_system ──► network_system
        │                                   │
        └──────────────► database_system ◄──┘
```

Data Model
- Primitive values (bool/int/float), bytes, string, nested container.
- Zero‑copy where possible; compact binary encoding; optional JSON/XML export.

Concurrency & Performance
- Lock‑free/mutex hybrid in thread_safe_container for predictable behavior.
- SIMD (NEON/AVX) accelerated numeric operations (opt‑in by target).

Design Principles
- Single responsibility per type; builder pattern for assembly; no global state.
- Avoid heavy external dependencies; provide local utility stubs with clear seams.

Build
- C++20, CMake. Optional sanitizer/benchmark targets. Works standalone.

