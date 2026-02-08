# Lock-Free Data Structures

**Date**: 2025-02-09
**Status**: Active

## Overview

container_system uses lock-free data structures internally to provide high-performance
concurrent access. This document covers the two foundational primitives:

1. **`rcu_value<T>`** - Read-Copy-Update pattern for lock-free value access
2. **`epoch_manager`** - Epoch-based memory reclamation for safe deallocation

These primitives power the `lockfree_container_reader` and `auto_refresh_reader`,
which provide lock-free reads of `thread_safe_container` data.

---

## 1. RCU Pattern Implementation (`rcu_value<T>`)

**Header**: `internal/rcu_value.h`
**Namespace**: `container_module`

### What is RCU?

Read-Copy-Update (RCU) is a synchronization mechanism that allows readers to access
shared data without acquiring any locks. Writers create a new copy of the data and
atomically publish it, while old copies remain valid until all readers release them.

In container_system, `rcu_value<T>` implements RCU using C++ `std::shared_ptr` atomic
operations, providing:

- **Wait-free reads**: O(1), no blocking, no spinning
- **Lock-free updates**: May retry under contention (CAS)
- **Automatic memory reclamation**: Via `shared_ptr` reference counting

### Read Path

Readers access the current value through `read()`, which returns an immutable snapshot:

```cpp
rcu_value<int> counter{0};

// From any thread - wait-free, no locks
auto snapshot = counter.read();
int value = *snapshot;  // Safe even if another thread updates counter
```

Internally, `read()` performs a single atomic load:

```cpp
std::shared_ptr<const T> read() const noexcept {
    return std::atomic_load_explicit(&current_, std::memory_order_acquire);
}
```

Key properties:
- Returns `std::shared_ptr<const T>` - the snapshot is **immutable**
- The snapshot remains valid even after the value is updated by another thread
- Uses `memory_order_acquire` to ensure visibility of the pointed-to data

### Write Path

Writers create a new immutable version and publish it atomically:

```cpp
// Simple update - always succeeds
counter.update(42);

// Compare-and-swap update - succeeds only if current matches expected
auto expected = counter.read();
bool success = counter.compare_and_update(expected, 100);
```

**`update(T new_value)`**: Unconditionally replaces the current value.

```cpp
void update(T new_value) {
    auto new_ptr = std::make_shared<const T>(std::move(new_value));
    std::atomic_store_explicit(&current_, new_ptr, std::memory_order_release);
}
```

**`compare_and_update(expected, new_value)`**: Atomically replaces the value only if
the current pointer matches `expected`. This enables lock-free algorithms that need
to detect concurrent modifications (optimistic concurrency).

```cpp
bool compare_and_update(const std::shared_ptr<const T>& expected, T new_value) {
    auto new_ptr = std::make_shared<const T>(std::move(new_value));
    auto expected_copy = expected;
    return std::atomic_compare_exchange_strong_explicit(
        &current_, &expected_copy, new_ptr,
        std::memory_order_acq_rel, std::memory_order_acquire);
}
```

### Memory Reclamation

Unlike traditional RCU implementations that require explicit grace period tracking,
`rcu_value<T>` relies on `std::shared_ptr` reference counting:

1. When `update()` is called, the old `shared_ptr` is replaced atomically
2. Any existing `read()` snapshots still hold a reference to the old data
3. When the last snapshot is destroyed, the old data is automatically freed

This approach trades slightly higher per-operation overhead (atomic reference count
operations) for simplicity and safety.

### API Reference

| Method | Thread Safety | Complexity | Description |
|--------|---------------|------------|-------------|
| `rcu_value()` | N/A | O(1) | Default construct with `T()` |
| `rcu_value(T initial)` | N/A | O(1) | Construct with initial value |
| `read()` | Wait-free | O(1) | Get immutable snapshot |
| `update(T)` | Lock-free | O(1) | Replace current value |
| `compare_and_update(expected, T)` | Lock-free | O(1) | CAS update |
| `update_count()` | Lock-free | O(1) | Get update counter |
| `has_value()` | Lock-free | O(1) | Check if initialized |

### Type Requirements

`T` must satisfy `std::is_copy_constructible_v<T> || std::is_move_constructible_v<T>`.

---

## 2. Epoch-Based Reclamation (`epoch_manager`)

**Header**: `internal/epoch_manager.h`
**Namespace**: `container_module`

### The Problem

In lock-free data structures, a fundamental challenge is knowing when it's safe to
free memory. A thread may still be reading a node that another thread wants to delete.
Simply calling `delete` would cause use-after-free.

### The Solution: Epoch-Based Reclamation

`epoch_manager` implements a three-epoch rotation scheme:

```
Epoch 0          Epoch 1          Epoch 2
+-----------+    +-----------+    +-----------+
| Retire    |    | Retire    |    | Retire    |
| List      |    | List      |    | Retire    |
+-----------+    +-----------+    +-----------+
     ^                                  |
     |       2 epoch gap = safe         |
     +----------------------------------+
```

The algorithm:

1. A **global epoch counter** advances monotonically
2. Threads **pin** to the current epoch when entering a critical section
3. Writers **retire** pointers to the current epoch's retire list
4. After 2 epoch advances, retired memory from the oldest epoch is safely reclaimable

The 2-epoch gap guarantees that no thread can still be reading memory retired in
that epoch, because all threads must have exited and re-entered since then.

### Thread Lifecycle

```
Thread A                                 epoch_manager
--------                                 -------------
enter_critical()  ─────────────────>     thread_epoch = global_epoch (pin)
  ... read lock-free data ...
exit_critical()   ─────────────────>     thread_epoch = INACTIVE (unpin)
```

Each thread has a `thread_local` epoch variable:
- `INACTIVE` (= `UINT64_MAX`): Thread is not in a critical section
- Any other value: Thread is pinned to that epoch

### Deferred Deletion

When removing a node from a lock-free structure, instead of immediately deleting it:

```cpp
epoch_manager& em = epoch_manager::instance();

// Don't do: delete old_node;
// Instead, defer the deletion:
em.defer_delete(old_node, [](void* p) { delete static_cast<Node*>(p); });

// Or use the typed convenience overload:
em.defer_delete(old_node);  // Automatically creates appropriate deleter
```

The pointer is added to the current epoch's retire list and will be freed during
a future garbage collection pass.

### Garbage Collection

```cpp
// Advance epoch and collect safe-to-free objects
size_t freed = em.try_gc();

// Force collection of all epochs (only safe during shutdown)
size_t freed = em.force_gc();
```

`try_gc()` advances the global epoch and collects objects from 2 epochs ago.
`force_gc()` collects all epochs unconditionally - this is only safe when no
threads are accessing lock-free data structures.

### RAII Guard

The `epoch_guard` class provides automatic critical section management:

```cpp
{
    epoch_guard guard;  // enter_critical()
    // ... safely access lock-free data ...
}  // exit_critical() called automatically
```

This prevents forgetting to call `exit_critical()`, which would block garbage
collection for any epoch the thread was pinned to.

### API Reference

#### `epoch_manager` (Singleton)

| Method | Thread Safety | Description |
|--------|---------------|-------------|
| `instance()` | Thread-safe | Get singleton instance |
| `enter_critical()` | Lock-free | Pin thread to current epoch |
| `exit_critical()` | Lock-free | Unpin thread |
| `in_critical_section()` | Lock-free | Check if thread is pinned |
| `defer_delete(ptr, deleter)` | Mutex-protected | Schedule deferred deletion |
| `defer_delete<T>(ptr)` | Mutex-protected | Typed deferred deletion |
| `try_gc()` | Mutex-protected | Advance epoch and collect |
| `force_gc()` | Mutex-protected | Collect all epochs |
| `current_epoch()` | Lock-free | Get current global epoch |
| `gc_count()` | Lock-free | Get GC cycle count |
| `reclaimed_count()` | Lock-free | Get total reclaimed objects |
| `pending_count()` | Mutex-protected | Get pending deletion count |

#### `epoch_guard` (RAII)

| Method | Description |
|--------|-------------|
| Constructor | Calls `enter_critical()` |
| Destructor | Calls `exit_critical()` |

### Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `INACTIVE` | `UINT64_MAX` | Sentinel for "not in critical section" |
| `NUM_EPOCHS` | `3` | Number of epochs in rotation |

---

## 3. Correctness Guarantees

### Memory Ordering

The implementation uses the following memory orderings:

| Operation | Ordering | Rationale |
|-----------|----------|-----------|
| `rcu_value::read()` | `acquire` | Ensures data pointed to is visible |
| `rcu_value::update()` | `release` | Ensures new data is visible before pointer |
| `rcu_value::compare_and_update()` | `acq_rel` / `acquire` | Full barrier on success, acquire on failure |
| `epoch_manager::enter_critical()` | `acquire` | Sees current epoch before accessing data |
| `epoch_manager::exit_critical()` | N/A (plain store) | Thread-local variable, no ordering needed |
| `epoch_manager::try_gc()` | `acq_rel` | Epoch advance visible to all threads |
| Update/GC counters | `relaxed` | Statistics only, no ordering required |

### ABA Problem

The ABA problem occurs when a pointer value is reused after being freed, making
a CAS appear to succeed when the underlying data has changed.

`rcu_value<T>` avoids the ABA problem because:

1. **`shared_ptr` identity**: CAS compares pointer identity, not value. Two different
   `shared_ptr` instances pointing to equal values are still different pointers.
2. **Deferred reclamation**: Old values are kept alive by reader snapshots, preventing
   premature reuse of the same memory address.
3. **Immutable snapshots**: Values stored as `const T` cannot be modified in-place.

### Progress Guarantees

| Component | Read | Write |
|-----------|------|-------|
| `rcu_value<T>` | **Wait-free** | **Lock-free** |
| `epoch_manager` critical section | **Lock-free** | N/A |
| `epoch_manager` defer/gc | **Blocking** (mutex) | **Blocking** (mutex) |
| `lockfree_container_reader` | **Wait-free** | N/A (refresh is blocking) |

- **Wait-free**: Every operation completes in a bounded number of steps
- **Lock-free**: At least one thread makes progress in a bounded number of steps
- **Blocking**: May wait for a mutex

### Hazard Scenarios

| Scenario | How It's Handled |
|----------|------------------|
| Reader holds stale snapshot | Safe - `shared_ptr` keeps old data alive |
| Writer updates during read | Safe - readers see consistent pre- or post-update snapshot |
| GC during active readers | Safe - pinned threads prevent premature reclamation |
| Thread exits without `exit_critical()` | `epoch_guard` RAII prevents this; raw API requires discipline |
| Multiple concurrent updates | `update()`: last writer wins; `compare_and_update()`: only one succeeds |

---

## 4. Performance Characteristics

### Read-Side Overhead

| Operation | Cost |
|-----------|------|
| `rcu_value::read()` | ~1 atomic load + shared_ptr ref count increment |
| `lockfree_container_reader::get()` | ~1 atomic load + hash map lookup |
| `epoch_manager::enter_critical()` | ~1 atomic load + 1 thread-local store |
| `epoch_manager::exit_critical()` | ~1 thread-local store |

Read-side overhead is **near-zero** - no locks, no spinning, no kernel calls. The
primary cost is the `shared_ptr` atomic reference count manipulation, which is
typically a single `lock xadd` instruction on x86.

### Write-Side Overhead

| Operation | Cost |
|-----------|------|
| `rcu_value::update()` | `make_shared` allocation + atomic store |
| `rcu_value::compare_and_update()` | `make_shared` allocation + atomic CAS |
| `lockfree_container_reader::refresh()` | Full container copy (O(n)) + atomic store |

Write-side overhead is dominated by memory allocation (`make_shared`) and, for
`lockfree_container_reader`, the full container snapshot copy. This is the classic
RCU trade-off: fast reads at the cost of slower writes.

### Memory Overhead

| Component | Overhead |
|-----------|----------|
| `rcu_value<T>` | `sizeof(shared_ptr<T>)` + `sizeof(atomic<size_t>)` per instance |
| `epoch_manager` | 3 retire lists + 3 mutexes (singleton) |
| Old snapshots | Kept alive until last reader releases |

Memory overhead per epoch in `epoch_manager` is proportional to the number of
retired objects. Each retired object costs `sizeof(void*) + sizeof(std::function)`.

### Scalability

- **Readers scale linearly**: Adding more reader threads has minimal impact on
  per-thread read performance because there is no lock contention.
- **Writers have fixed cost**: Writer overhead does not increase with reader count.
- **GC throughput**: Proportional to the number of retired objects per epoch.

Benchmark results from `thread_safety_tests.cpp` show that `lockfree_container_reader`
achieves >2x throughput improvement over mutex-based reads under contention with
10+ concurrent reader threads. See the `DISABLED_CompareWithMutexBased` benchmark
for details.

---

## 5. Integration with Container Types

### Architecture

```
+-----------------------------+
| thread_safe_container       |
| (shared_mutex for R/W)      |
+-----------------------------+
        |
        | create_lockfree_reader()
        v
+-----------------------------+
| lockfree_container_reader   |
| (atomic shared_ptr snapshot)|  <--- rcu_value pattern
+-----------------------------+
        |
        | Wraps
        v
+-----------------------------+
| auto_refresh_reader         |
| (background refresh thread) |
+-----------------------------+
```

### How Container Operations Map to RCU

| Container Operation | RCU Equivalent | Description |
|---------------------|----------------|-------------|
| `reader.get<T>(key)` | `rcu_value::read()` | Atomic load of snapshot pointer, then hash map lookup |
| `reader.refresh()` | `rcu_value::update()` | Copy container data into new snapshot, atomic publish |
| `reader.contains(key)` | `rcu_value::read()` | Atomic load + existence check |
| `reader.for_each(fn)` | `rcu_value::read()` | Atomic load + iteration over snapshot |

### Lock-Free Reader (`lockfree_container_reader`)

The reader maintains an atomic `shared_ptr` to an immutable snapshot of the container:

```cpp
// Creation
auto container = std::make_shared<thread_safe_container>();
auto reader = container->create_lockfree_reader();

// Lock-free reads from any thread (wait-free)
auto val = reader->get<int>("counter");     // No locks!
bool exists = reader->contains("counter");   // No locks!
size_t n = reader->size();                   // No locks!

// Refresh snapshot (acquires container's read lock)
reader->refresh();
```

Reads access an immutable `shared_ptr<const snapshot_type>` via atomic load.
The snapshot is a `std::unordered_map<std::string, value>` copied from the container.

### Auto-Refresh Reader (`auto_refresh_reader`)

Wraps `lockfree_container_reader` with a background thread that calls `refresh()`
at configurable intervals:

```cpp
auto reader = container->create_auto_refresh_reader(
    std::chrono::milliseconds(100));

// Lock-free reads, automatically updated every 100ms
auto val = reader->get<int>("counter");

// Stop auto-refresh (reads still work with last snapshot)
reader->stop();
```

The background thread uses `std::condition_variable::wait_for()` to sleep between
refreshes, allowing clean shutdown via `stop()`.

### When Epoch-Based Reclamation Triggers

In the current implementation, `epoch_manager` is available as a general-purpose
reclamation facility for lock-free structures. The `rcu_value<T>` and
`lockfree_container_reader` use `shared_ptr` reference counting for reclamation
instead, which provides automatic lifetime management.

`epoch_manager` is intended for scenarios where:
- Custom lock-free data structures need explicit memory management
- Raw pointer-based lock-free algorithms require deferred deletion
- The overhead of `shared_ptr` atomic operations is unacceptable

---

## 6. Comparison with Alternatives

### RCU vs Mutex-Based Access

| Aspect | RCU (`lockfree_container_reader`) | Mutex (`thread_safe_container`) |
|--------|-----------------------------------|--------------------------------|
| Read latency | Near-zero (wait-free) | Variable (lock contention) |
| Read throughput | Scales linearly with cores | Limited by lock contention |
| Write latency | Higher (copy + publish) | Lower (in-place modification) |
| Data freshness | Snapshot (potentially stale) | Always current |
| Memory usage | Higher (multiple snapshots) | Lower (single copy) |
| Best for | Read-heavy workloads (>90% reads) | Write-heavy or balanced workloads |

**Choose RCU when**:
- Read operations vastly outnumber writes
- Read latency must be predictable (real-time systems, signal handlers)
- Many concurrent readers need access simultaneously

**Choose mutex-based when**:
- Writes are frequent and data must always be current
- Memory is constrained (cannot afford multiple copies)
- Workload is balanced between reads and writes

### Shared Pointer vs Epoch-Based Reclamation

| Aspect | `shared_ptr` (used by `rcu_value`) | Epoch-based (`epoch_manager`) |
|--------|-------------------------------------|-------------------------------|
| Ease of use | Automatic, no manual management | Requires enter/exit discipline |
| Read overhead | Atomic ref count increment | Thread-local store |
| Write overhead | Atomic ref count decrement | Retire list append |
| Reclamation | Immediate when ref count reaches 0 | Batched during GC passes |
| Scalability | Ref count contention under heavy load | Better under extreme contention |
| Memory efficiency | Prompt reclamation | Delayed (2 epoch lag) |

The current implementation favors `shared_ptr` for its simplicity and safety.
`epoch_manager` is available for advanced use cases requiring lower per-operation
overhead or integration with raw-pointer lock-free algorithms.

### Reader Type Comparison

| Reader Type | Lock-Free Reads | Auto-Refresh | Data Freshness | Use Case |
|-------------|-----------------|--------------|----------------|----------|
| `thread_safe_container` (direct) | No | N/A | Always current | General purpose |
| `snapshot_reader` | No (uses `shared_mutex`) | Manual | Stale until refresh | Legacy compatibility |
| `lockfree_container_reader` | Yes (wait-free) | Manual | Stale until refresh | High-performance reads |
| `auto_refresh_reader` | Yes (wait-free) | Automatic | Bounded staleness | Hands-off high-performance |

---

## References

- **Source files**:
  - `internal/rcu_value.h` - RCU value implementation
  - `internal/epoch_manager.h` - Epoch-based reclamation
  - `internal/thread_safe_container.h` - Container readers integration
- **Tests**: `tests/thread_safety_tests.cpp` - Comprehensive thread safety and stress tests
- **Related docs**:
  - [Exception Safety](EXCEPTION_SAFETY.md) - Exception guarantees for container APIs
  - [Performance](../performance/PERFORMANCE.md) - Overall performance characteristics
