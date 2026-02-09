# Memory Pool and Allocator Architecture

**Date**: 2025-02-09
**Status**: Active

## Overview

container_system provides a custom memory pool and pool allocator to reduce
`malloc`/`new` overhead for small, frequently-allocated objects. This is a key
factor in achieving the benchmark performance (5M containers/sec creation,
25M ops/sec).

The memory management system consists of:

1. **`fixed_block_pool`** - Fixed-size block memory pool with free list management
2. **`pool_allocator`** - Thread-local allocator with size-class routing
3. **Helper functions** - `pool_allocate<T>()` / `pool_deallocate<T>()` for typed allocation

**Headers**:
- `internal/memory_pool.h` - `fixed_block_pool` implementation
- `internal/pool_allocator.h` - `pool_allocator` and helpers

**Namespace**: `container_module::internal`

---

## 1. Memory Pool (`fixed_block_pool`)

### Architecture

`fixed_block_pool` is a fixed-size block allocator that uses an intrusive free
list for O(1) allocation and deallocation:

```
fixed_block_pool (block_size=64, blocks_per_chunk=4)

Chunk 0 (256 bytes)                    Chunk 1 (256 bytes)
+--------+--------+--------+--------+ +--------+--------+--------+--------+
| Block  | Block  | Block  | Block  | | Block  | Block  | Block  | Block  |
| 64B    | 64B    | 64B    | 64B    | | 64B    | 64B    | 64B    | 64B    |
+--------+--------+--------+--------+ +--------+--------+--------+--------+

Free List (intrusive singly-linked list):
head -> [Block 7] -> [Block 5] -> [Block 2] -> nullptr
         ^chunk1      ^chunk1      ^chunk0
```

Key characteristics:
- All blocks in a pool have the **same fixed size**
- Free blocks store a `void*` pointer to the next free block in their first bytes
- Minimum block size is `sizeof(void*)` (8 bytes on 64-bit) to fit the free list pointer
- New chunks are allocated on demand when the free list is exhausted

### Block Sizing Strategy

The block size is set at construction and cannot change:

```cpp
// Minimum block size is enforced to sizeof(void*) for free list pointers
fixed_block_pool(std::size_t block_size, std::size_t blocks_per_chunk = 1024)
    : block_size_(block_size < sizeof(void*) ? sizeof(void*) : block_size)
    , blocks_per_chunk_(blocks_per_chunk) {}
```

If the requested block size is smaller than `sizeof(void*)`, it is automatically
rounded up. This is because each free block must be large enough to store the
intrusive free list pointer.

### Free List Management

**Allocation** - O(1): Pop from the head of the free list.

```cpp
void* allocate() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!free_list_) {
        allocate_chunk_unlocked();  // Grow pool
    }
    void* p = free_list_;
    free_list_ = *reinterpret_cast<void**>(free_list_);  // Follow next pointer
    ++allocated_count_;
    return p;
}
```

**Deallocation** - O(1): Push to the head of the free list.

```cpp
void deallocate(void* p) {
    if (!p) return;
    std::lock_guard<std::mutex> lock(mutex_);
    *reinterpret_cast<void**>(p) = free_list_;  // Point to current head
    free_list_ = p;                              // New head
    --allocated_count_;
}
```

In debug builds (`NDEBUG` not defined), `deallocate` validates that the pointer
belongs to one of the pool's chunks before adding it to the free list.

### Growth / Expansion Policy

When `allocate()` is called and the free list is empty, a new chunk is allocated:

```cpp
void allocate_chunk_unlocked() {
    auto chunk = std::make_unique<uint8_t[]>(block_size_ * blocks_per_chunk_);
    uint8_t* base = chunk.get();
    chunks_.push_back(std::move(chunk));

    // Build free list from new chunk
    for (std::size_t i = 0; i < blocks_per_chunk_; ++i) {
        void* p = base + i * block_size_;
        *reinterpret_cast<void**>(p) = free_list_;
        free_list_ = p;
    }
}
```

Properties:
- Chunks are never freed until the pool is destroyed
- Each chunk allocates `block_size * blocks_per_chunk` bytes contiguously
- New blocks are prepended to the free list (LIFO ordering)
- Growth is unbounded - the pool grows as needed

### Thread Safety

`fixed_block_pool` uses a `std::mutex` for all operations:

| Operation | Thread Safety | Lock Scope |
|-----------|---------------|------------|
| `allocate()` | Mutex-protected | Entire operation |
| `deallocate()` | Mutex-protected | Entire operation |
| `get_statistics()` | Mutex-protected | Snapshot read |
| `block_size()` | Lock-free | Immutable after construction |

### Statistics API

```cpp
struct statistics {
    std::size_t total_chunks;      // Number of chunks allocated
    std::size_t allocated_blocks;  // Blocks currently in use
    std::size_t total_capacity;    // Total blocks across all chunks
    std::size_t free_blocks;       // Blocks in free list

    double utilization_ratio() const;  // allocated / capacity
};

auto stats = pool.get_statistics();
```

### API Reference

| Method | Thread Safety | Description |
|--------|---------------|-------------|
| `fixed_block_pool(block_size, blocks_per_chunk)` | N/A | Construct pool |
| `allocate()` | Mutex-protected | Allocate one block |
| `deallocate(ptr)` | Mutex-protected | Return block to pool |
| `block_size()` | Lock-free | Get block size |
| `get_statistics()` | Mutex-protected | Get usage statistics |

---

## 2. Pool Allocator (`pool_allocator`)

### Size-Class Routing

`pool_allocator` manages two fixed-block pools organized by size class:

```
                     pool_allocator
                    (thread-local singleton)
                           |
             +-------------+-------------+
             |                           |
        Small Pool                  Medium Pool
     (block_size=64)             (block_size=256)
     (1024 blocks/chunk)         (1024 blocks/chunk)
             |                           |
    Allocations <= 64B          Allocations 65-256B
             |                           |
             +-------------+-------------+
                           |
                   Allocations > 256B
                      go to heap
                   (::operator new)
```

Size class thresholds:

```cpp
struct pool_size_class {
    static constexpr std::size_t small_threshold = 64;    // <= 64 bytes
    static constexpr std::size_t medium_threshold = 256;  // <= 256 bytes
    static constexpr std::size_t blocks_per_chunk = 1024;
};
```

| Size Class | Range | Pool | Allocation |
|------------|-------|------|------------|
| Small | 0-64 bytes | `small_pool_` (64B blocks) | Pool allocation |
| Medium | 65-256 bytes | `medium_pool_` (256B blocks) | Pool allocation |
| Large | >256 bytes | N/A | `::operator new` (heap) |

### Thread-Local Design

`pool_allocator` uses a **thread-local singleton** pattern:

```cpp
static pool_allocator& instance() noexcept {
    thread_local pool_allocator allocator;
    return allocator;
}
```

Each thread gets its own `pool_allocator` instance with its own two pools. This
means the **allocation fast path is completely lock-free** from the caller's
perspective - no cross-thread contention on the allocator itself.

However, each `fixed_block_pool` inside the allocator still uses a mutex
internally. Since the pools are thread-local, this mutex is uncontended in
practice (only the owning thread accesses it).

### Compile-Time Configuration

Pool allocation is controlled by the `CONTAINER_USE_MEMORY_POOL` CMake option:

```cmake
option(CONTAINER_USE_MEMORY_POOL "Enable memory pool for small allocations" ON)

if(CONTAINER_USE_MEMORY_POOL)
    target_compile_definitions(container_system PUBLIC CONTAINER_USE_MEMORY_POOL=1)
endif()
```

When disabled, all allocations fall through to `::operator new`:

```cpp
void* allocate(std::size_t size) noexcept {
#if CONTAINER_USE_MEMORY_POOL
    // ... pool allocation logic ...
#endif
    // Fallback: heap allocation
    return ::operator new(size, std::nothrow);
}
```

### Statistics Tracking

```cpp
struct pool_allocator_stats {
    std::size_t pool_hits{0};           // Allocations from pool
    std::size_t pool_misses{0};         // Allocations from heap
    std::size_t small_pool_allocs{0};   // Small pool hits
    std::size_t medium_pool_allocs{0};  // Medium pool hits
    std::size_t deallocations{0};       // Total deallocations

    double hit_rate() const noexcept;   // hits / (hits + misses)
};
```

### Helper Functions

**`pool_allocate<T>(args...)`** - Allocate and construct an object:

```cpp
auto* obj = pool_allocate<MyStruct>(42, "hello");
// Equivalent to: new (pool_memory) MyStruct(42, "hello")
```

Requires `std::is_nothrow_destructible_v<T>`. Returns `nullptr` on failure.

**`pool_deallocate<T>(ptr)`** - Destroy and deallocate an object:

```cpp
pool_deallocate(obj);
// Equivalent to: obj->~MyStruct(); pool.deallocate(obj, sizeof(MyStruct))
```

**`is_pool_allocatable<T>()`** - Check if a type fits in a pool:

```cpp
is_pool_allocatable<SmallStruct>();  // true if sizeof(T) <= 256
```

**`get_size_class(size)`** - Get the size class for a given byte count:

```cpp
get_size_class(32);   // 0 (small)
get_size_class(128);  // 1 (medium)
get_size_class(512);  // 2 (large, heap)
```

### API Reference

| Method / Function | Description |
|-------------------|-------------|
| `pool_allocator::instance()` | Get thread-local singleton |
| `allocate(size)` | Allocate from appropriate pool or heap |
| `deallocate(ptr, size)` | Return to appropriate pool or heap |
| `get_stats()` | Get hit/miss statistics |
| `get_small_pool_stats()` | Get small pool `fixed_block_pool::statistics` |
| `get_medium_pool_stats()` | Get medium pool `fixed_block_pool::statistics` |
| `reset_stats()` | Reset statistics counters |
| `pool_allocate<T>(args...)` | Allocate + construct from pool |
| `pool_deallocate<T>(ptr)` | Destroy + deallocate to pool |
| `is_pool_allocatable<T>()` | Check if type fits in pool |
| `get_size_class(size)` | Get size class (0=small, 1=medium, 2=large) |

---

## 3. Internal Usage

### Integration with `value_container`

The pool allocator is used by `value_container` (the primary public container type)
through the `CONTAINER_USE_MEMORY_POOL` compile-time flag. When enabled, small
internal allocations during container operations use pool allocation instead of
the default heap.

`value_container` exposes pool statistics through its public API:

```cpp
// Get current pool statistics
pool_stats stats = value_container::get_pool_stats();
std::cout << "Pool hits: " << stats.hits
          << ", misses: " << stats.misses << std::endl;

// Reset statistics (useful for benchmarking)
value_container::clear_pool();
```

The `pool_stats` structure (defined in `core/container/types.h`):

```cpp
struct pool_stats {
    size_t hits{0};                // Pool allocations satisfied
    size_t misses{0};              // Heap allocations (pool bypassed)
    size_t small_pool_allocs{0};   // Small pool (<=64 bytes) allocations
    size_t medium_pool_allocs{0};  // Medium pool (<=256 bytes) allocations
    size_t deallocations{0};       // Total deallocations
    size_t available{0};           // Free blocks available
};
```

### Allocation Patterns

| Operation | Typical Size | Size Class | Notes |
|-----------|-------------|------------|-------|
| Small value storage | 8-64 bytes | Small | Numeric types, booleans |
| String value storage | Variable | Medium or heap | Depends on string length |
| Container metadata | 32-128 bytes | Small/Medium | Internal bookkeeping |
| Serialization buffers | Variable | Heap | Typically >256 bytes |

### Memory Layout

Pool blocks within a chunk are contiguous in memory, which provides excellent
cache locality for sequential access patterns:

```
Chunk memory layout (64B blocks, 4 blocks):
Address    0x1000   0x1040   0x1080   0x10C0
           +--------+--------+--------+--------+
           | Block0 | Block1 | Block2 | Block3 |
           | 64B    | 64B    | 64B    | 64B    |
           +--------+--------+--------+--------+
           Contiguous in virtual memory → cache-friendly
```

---

## 4. Performance Analysis

### Allocation / Deallocation Throughput

Pool allocation avoids the overhead of general-purpose `malloc`:
- No free list search across size classes (fixed-size blocks)
- No coalescing of adjacent free blocks
- No metadata headers per allocation
- LIFO free list means recently freed blocks (still in cache) are reused first

Expected performance improvement over `::operator new`:

| Block Size | Expected Speedup | Reason |
|------------|-----------------|--------|
| 64B | 10-50x faster | Hot-path LIFO reuse, no malloc overhead |
| 256B | 5-20x faster | Same benefits, larger copy |
| 1KB | 2-10x faster | Moderate benefit |
| 4KB+ | 1-5x faster | Diminishing returns, similar to malloc |

Comprehensive benchmarks are available in `tests/memory_pool_benchmark.cpp`
using Google Benchmark. Run with:

```bash
cmake --build build --target memory_pool_benchmark
./build/tests/memory_pool_benchmark
```

### Memory Fragmentation

The fixed-block design **eliminates external fragmentation** within a size class:
- Every block is the same size, so any free block can satisfy any allocation
- No fragmentation-induced allocation failures
- Trade-off: **internal fragmentation** exists when the allocated object is smaller
  than the block size (e.g., 20 bytes in a 64-byte block wastes 44 bytes)

### Cache Locality

Pool-allocated objects from the same chunk are contiguous in memory:
- Sequential access to recently-allocated objects benefits from spatial locality
- Free list LIFO ordering means the most recently freed block (likely still in
  L1/L2 cache) is allocated next, benefiting from temporal locality
- Benchmark comparison available: `BM_PoolCacheEfficiency_Sequential` vs
  `BM_PoolCacheEfficiency_Random` in `tests/memory_pool_benchmark.cpp`

---

## 5. Configuration

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `CONTAINER_USE_MEMORY_POOL` | `ON` | Enable pool allocation for small objects |

```bash
# Enable (default)
cmake -DCONTAINER_USE_MEMORY_POOL=ON ..

# Disable (all allocations use heap)
cmake -DCONTAINER_USE_MEMORY_POOL=OFF ..
```

### Pool Parameters

Pool parameters are set at compile time via `pool_size_class`:

| Parameter | Value | Description |
|-----------|-------|-------------|
| `small_threshold` | 64 bytes | Maximum size for small pool |
| `medium_threshold` | 256 bytes | Maximum size for medium pool |
| `blocks_per_chunk` | 1024 | Blocks allocated per growth step |

Chunk sizes:
- Small pool chunk: 64 * 1024 = **64 KB**
- Medium pool chunk: 256 * 1024 = **256 KB**

### Pre-Allocation

The pool starts empty and grows on demand. The first allocation in each pool
triggers a chunk allocation. For latency-sensitive applications, you can warm
the pool by performing allocations during initialization:

```cpp
// Warm the pool by allocating and immediately freeing
auto& allocator = internal::pool_allocator::instance();
std::vector<void*> warmup(1024);
for (auto& ptr : warmup) {
    ptr = allocator.allocate(64);  // Pre-allocate small pool
}
for (auto* ptr : warmup) {
    allocator.deallocate(ptr, 64);
}
// Pool now has 1024 warm blocks ready
```

### Memory Limit

The pool has **no built-in memory limit** - it grows unbounded as needed. Chunks
are never returned to the OS until the pool is destroyed (when the thread exits
for thread-local pools). Monitor memory usage via statistics:

```cpp
auto stats = pool_allocator::instance().get_small_pool_stats();
size_t memory_bytes = stats.total_capacity * 64;  // Small pool memory
```

---

## 6. Best Practices

### When Pool Allocation Helps

Pool allocation is most beneficial for:
- **Short-lived, fixed-size objects**: Allocate, use briefly, deallocate
- **High-frequency allocation**: Thousands of allocations per second
- **Small objects**: <= 256 bytes (the pool's sweet spot)
- **Read-heavy workloads**: Pool objects have good cache locality

### When Standard Allocation is Better

Prefer `new`/`malloc` when:
- Objects are **large** (> 256 bytes) - pool provides minimal benefit
- Objects have **variable sizes** - leads to high internal fragmentation
- Objects are **long-lived** - pool memory is not returned to the OS
- **Memory pressure** is a concern - pools hold reserved memory even when idle

### Sizing Recommendations

| Workload | Recommended Configuration |
|----------|---------------------------|
| General purpose | Default (`CONTAINER_USE_MEMORY_POOL=ON`) |
| Memory-constrained | Disable pool (`CONTAINER_USE_MEMORY_POOL=OFF`) |
| High-throughput | Default + warm pool at startup |
| Benchmarking | Call `reset_stats()` before measuring |

### Monitoring and Diagnostics

Use the statistics API to monitor pool health:

```cpp
auto stats = value_container::get_pool_stats();

// Check hit rate - should be >80% for effective pooling
double hit_rate = (stats.hits + stats.misses > 0)
    ? static_cast<double>(stats.hits) / (stats.hits + stats.misses)
    : 0.0;

// Check pool utilization
auto small = pool_allocator::instance().get_small_pool_stats();
// High utilization_ratio (>0.9) may indicate the pool should grow faster
// Low utilization_ratio (<0.1) may indicate over-provisioning
```

---

## References

- **Source files**:
  - `internal/memory_pool.h` - `fixed_block_pool` implementation
  - `internal/pool_allocator.h` - `pool_allocator` and helpers
  - `core/container.cpp` - Integration with `value_container`
  - `core/container/types.h` - `pool_stats` structure definition
- **Tests**: `tests/memory_pool_tests.cpp` - Unit tests and integration tests
- **Benchmarks**: `tests/memory_pool_benchmark.cpp` - Google Benchmark suite
- **Performance data**: [Memory Pool Performance](../performance/MEMORY_POOL_PERFORMANCE.md) - Auto-generated benchmark results
- **Related docs**:
  - [Lock-Free Data Structures](LOCK_FREE.md) - RCU and epoch-based reclamation
  - [Performance](../performance/PERFORMANCE.md) - Overall performance characteristics
