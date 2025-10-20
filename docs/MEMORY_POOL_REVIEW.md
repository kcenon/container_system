Container System Memory Pool Review
===================================

> **Language:** **English** | [한국어](MEMORY_POOL_REVIEW_KO.md)

Scope
- Assess allocation patterns and propose optional memory pool strategies for container_system.

Findings
- Hot paths: value_container construction, nested container_value creation, string/bytes storage.
- Allocation patterns: many small, short‑lived allocations under bursty workloads.

Options
- A) std::pmr resources (monotonic_buffer_resource, unsynchronized_pool_resource)
  - Pros: Standard, low integration cost, good for batch lifetimes
  - Cons: Requires PMR plumbed into value containers
- B) Fixed‑block pool (custom)
  - Pros: Predictable latency; reuses blocks for frequent small values
  - Cons: Fragmentation risk; tuning required; code to maintain
- C) Slab allocator per container
  - Pros: Excellent locality; easy bulk free
  - Cons: Slab sizing trade‑offs; worst‑case overhead

Recommended Strategy
- Phase 1: Introduce PMR‑ready constructors and typedefs (non‑breaking)
- Phase 2: Provide optional fixed‑block pool for small objects (64/128/256B classes)
- Phase 3: Benchmarks + heuristics: switch by workload profile

Integration Points
- value_container internals (node/object allocations)
- container_value (children vector, metadata)
- string_value / bytes_value buffers

API Sketch
```cpp
// Phase 1 – PMR hooks
namespace container_module {
  struct container_resources {
    std::pmr::memory_resource* general{std::pmr::get_default_resource()};
    std::pmr::memory_resource* small_objects{std::pmr::get_default_resource()};
  };

  class value_container {
  public:
    explicit value_container(container_resources r = {});
    // …
  };
}
```

Benchmark Plan
- Construct/Serialize/Deserialize microbenchmarks with varying sizes
- Compare: default, PMR mono, PMR pool, fixed‑block pool
- Metrics: alloc count, bytes, latency percentiles, peak RSS

Risks
- Over‑tuning pools can regress general workloads
- PMR propagation requires careful API additions

Next Steps
- Add PMR hooks (no behavior change)
- Provide prototype fixed‑block pool header for experimentation
- Add benchmarks and report


---

*Last Updated: 2025-10-20*
