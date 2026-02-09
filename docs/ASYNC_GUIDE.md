# C++20 Coroutine and Async Framework

**Date**: 2025-02-09
**Status**: Active

## Overview

container_system includes a full C++20 coroutine framework at `internal/async/`
that enables asynchronous and lazy container operations. The framework provides:

1. **`task<T>`** - Coroutine task type for async computations
2. **`generator<T>`** - Lazy sequence generator for on-demand values
3. **`async_container`** - Async wrapper for `value_container` operations
4. **`thread_pool_executor`** - Executor integration for scheduling coroutines
5. **`async.h`** - Umbrella header with compile-time feature detection

**Namespace**: `container_module::async`

**Headers**:
- `internal/async/task.h` - `task<T>` coroutine type
- `internal/async/generator.h` - `generator<T>` lazy sequence
- `internal/async/async_container.h` - Async container wrapper
- `internal/async/thread_pool_executor.h` - Executor integration
- `internal/async/async.h` - Umbrella header (include this)

---

## 1. C++20 Coroutine Primer

C++20 coroutines are functions that can suspend and resume execution. They use
three new keywords:

| Keyword | Purpose | Used In |
|---------|---------|---------|
| `co_await` | Suspend until an async operation completes | `task<T>` |
| `co_yield` | Produce a value and suspend | `generator<T>` |
| `co_return` | Return a final value and complete | Both |

A coroutine's behavior is defined by its **promise type**, which controls:
- What happens at start (`initial_suspend`)
- What happens at end (`final_suspend`)
- How values are returned (`return_value` / `return_void`)
- How exceptions are handled (`unhandled_exception`)

The coroutine framework in container_system handles all of this internally.
You only need to use `task<T>` and `generator<T>` as return types.

### Quick Example

```cpp
#include <internal/async/async.h>
using namespace container_module::async;

// Async function returning a value
task<int> compute_async() {
    co_return 42;
}

// Consuming the result
task<void> process() {
    int result = co_await compute_async();
    // result == 42
    co_return;
}
```

---

## 2. `task<T>` — Async Computation

### Architecture

`task<T>` is a coroutine type for async computations that produce a single
value. It supports both value-returning (`task<T>`) and void-returning
(`task<void>`) variants.

```
task<T> Lifecycle:

  Creation ──► Immediate Execution ──► co_return/exception ──► Final Suspend
                 (eager start)              │                       │
                                     Store result in          Resume continuation
                                     promise_type              (symmetric transfer)
                                            │                       │
                                     completed_ = true         Awaiter resumes
                                     (memory_order_release)    (await_resume)
```

Key properties:
- **Eager execution**: Starts running immediately upon creation (`initial_suspend` = `suspend_never`)
- **Move-only**: Non-copyable, transferable via `std::move`
- **Symmetric transfer**: Uses `final_awaiter` for efficient coroutine chaining
- **Thread-safe completion**: `completed_` flag uses `std::atomic<bool>` with acquire/release semantics
- **Exception propagation**: Exceptions are captured and rethrown on `get()` or `co_await`

### Promise Type

The promise type is split into a base and specializations:

| Component | Role |
|-----------|------|
| `detail::promise_base` | Shared logic: exception handling, continuation, atomic completion |
| `detail::promise_type<T>` | Value storage via `std::optional<T>`, `return_value()` |
| `detail::promise_type<void>` | Void specialization, `return_void()` |

The `final_awaiter` in `promise_base` implements symmetric transfer:

```cpp
// Inside final_awaiter::await_suspend:
auto continuation = promise.continuation_;
// Release barrier: all writes visible to threads checking done()
promise.completed_.store(true, std::memory_order_release);
if (continuation) {
    return continuation;  // Symmetric transfer to awaiting coroutine
}
return std::noop_coroutine();
```

### API Reference

| Method | Description |
|--------|-------------|
| `task()` | Default constructor (empty task) |
| `explicit task(handle_type)` | Construct from coroutine handle |
| `task(task&&)` | Move constructor |
| `task& operator=(task&&)` | Move assignment (destroys previous coroutine) |
| `~task()` | Destructor (destroys owned coroutine) |
| `valid()` | Check if task has a coroutine |
| `operator bool()` | Same as `valid()` (explicit conversion) |
| `done()` | Thread-safe completion check (atomic acquire) |
| `get()` | Get result (assumes completion; rethrows exceptions) |
| `operator co_await()` | Await interface for `co_await` |
| `resume()` | Manual resume (for testing) |

### Usage Examples

**Basic value task:**

```cpp
task<int> compute() {
    co_return 42;
}

auto t = compute();
// Task starts immediately (eager)
assert(t.done());
assert(t.get() == 42);
```

**Task chaining with `co_await`:**

```cpp
task<int> inner() {
    co_return 21;
}

task<int> outer() {
    int value = co_await inner();
    co_return value * 2;  // 42
}
```

**Exception propagation:**

```cpp
task<int> may_fail() {
    throw std::runtime_error("something went wrong");
    co_return 0;  // Never reached
}

auto t = may_fail();
try {
    t.get();  // Rethrows the runtime_error
} catch (const std::runtime_error& e) {
    // Handle error
}
```

**Nested await (3 levels):**

```cpp
task<int> level3() { co_return 10; }
task<int> level2() { co_return co_await level3() + 20; }
task<int> level1() { co_return co_await level2() + 30; }

auto t = level1();
assert(t.get() == 60);
```

### Factory Functions

| Function | Description |
|----------|-------------|
| `make_ready_task(T&& value)` | Create a task that immediately returns `value` |
| `make_ready_task()` | Create a void task that immediately completes |
| `make_exceptional_task<T>(exception_ptr)` | Create a task that throws on await |

---

## 3. `generator<T>` — Lazy Sequences

### Architecture

`generator<T>` produces a sequence of values lazily using `co_yield`. Values
are computed on-demand when the iterator advances.

```
generator<T> Lifecycle:

  Creation ──► Suspended              (lazy start: initial_suspend = suspend_always)
                  │
            begin() called
                  │
            ┌─► resume() ──► co_yield value ──► Suspended
            │       │                                │
            │   iterator++                      *iterator
            │       │                                │
            └───────┴────────────────────────────────┘
                  │
            co_return (or end of function)
                  │
            handle.done() == true ──► end sentinel matched
```

Key properties:
- **Lazy evaluation**: Does not start until `begin()` is called
- **On-demand**: Each value computed only when iterator advances
- **STL-compatible**: Works with range-based `for` loops
- **Sentinel-based**: Uses `std::default_sentinel_t` for end comparison
- **Move-only**: Non-copyable

### Iterator Interface

The generator provides an `input_iterator` compatible interface:

| Type | Value |
|------|-------|
| `iterator_category` | `std::input_iterator_tag` |
| `difference_type` | `std::ptrdiff_t` |
| `value_type` | `std::remove_reference_t<T>` |

| Method | Description |
|--------|-------------|
| `operator*()` | Dereference: get current value |
| `operator->()` | Arrow: get pointer to current value |
| `operator++()` | Pre-increment: advance to next value |
| `operator++(int)` | Post-increment: advance to next value |
| `operator==(default_sentinel_t)` | End check: `handle_.done()` |

### API Reference

| Method | Description |
|--------|-------------|
| `generator()` | Default constructor (empty) |
| `explicit generator(handle_type)` | Construct from coroutine handle |
| `generator(generator&&)` | Move constructor |
| `generator& operator=(generator&&)` | Move assignment |
| `~generator()` | Destructor (destroys coroutine) |
| `valid()` | Check if generator has a coroutine |
| `operator bool()` | Same as `valid()` |
| `begin()` | Get iterator (resumes to first value) |
| `end()` | Get end sentinel (`std::default_sentinel_t`) |

### Usage Examples

**Simple range generator:**

```cpp
generator<int> range(int start, int end) {
    for (int i = start; i < end; ++i) {
        co_yield i;
    }
}

for (int value : range(0, 5)) {
    std::cout << value << " ";  // 0 1 2 3 4
}
```

**Infinite generator with `take()`:**

```cpp
generator<int> naturals() {
    int i = 0;
    while (true) {
        co_yield i++;
    }
}

// take() limits the number of yielded values
for (int value : take(naturals(), 5)) {
    std::cout << value << " ";  // 0 1 2 3 4
}
```

**Exception propagation in generators:**

```cpp
generator<int> failing_gen() {
    co_yield 1;
    co_yield 2;
    throw std::runtime_error("generator error");
    co_yield 3;  // Never reached
}

std::vector<int> results;
try {
    for (int v : failing_gen()) {
        results.push_back(v);
    }
} catch (const std::runtime_error&) {
    // results == {1, 2}
}
```

### Utility Functions

| Function | Description |
|----------|-------------|
| `from_range(Range&&)` | Create a generator from any range |
| `take(generator<T>, size_t)` | Limit generator to N elements |

---

## 4. `async_container` — Async Container Wrapper

### Architecture

`async_container` wraps a `value_container` and provides async versions of
serialization, deserialization, file I/O, and streaming operations.

```
async_container
├── Async Serialization
│   ├── serialize_async()       → task<Result<vector<uint8_t>>>
│   └── serialize_string_async() → task<Result<string>>
├── Async Deserialization
│   ├── deserialize_async()     → task<Result<shared_ptr<value_container>>>
│   └── deserialize_string_async() → task<Result<shared_ptr<value_container>>>
├── Async File I/O
│   ├── load_async(path)        → task<VoidResult>
│   └── save_async(path)        → task<VoidResult>
├── Streaming
│   ├── serialize_chunked()     → generator<vector<uint8_t>>
│   └── deserialize_streaming() → task<Result<shared_ptr<value_container>>>
└── Convenience
    ├── set<T>(key, value)      → async_container& (chaining)
    ├── get<T>(key)             → optional<T>
    └── contains(key)           → bool
```

### Thread Model

Async operations use `detail::async_awaitable` which offloads work to a
detached `std::thread`:

```cpp
void await_suspend(std::coroutine_handle<> handle) {
    auto state = state_;  // Copy shared_ptr for thread lifetime
    std::thread([state, handle]() mutable {
        try {
            state->result_.emplace(state->work_());
        } catch (...) {
            state->exception_ = std::current_exception();
        }
        // Release barrier: all writes visible to resuming coroutine
        state->ready_.store(true, std::memory_order_release);
        handle.resume();
    }).detach();
}
```

The shared state (`async_state<T>`) uses `std::shared_ptr` to ensure the worker
thread can safely access it even if the awaitable is moved or destroyed.

### Conditional Compilation

Return types vary based on `CONTAINER_HAS_COMMON_RESULT`:

| API | With Result | Without Result |
|-----|-------------|----------------|
| `serialize_async()` | `task<Result<vector<uint8_t>>>` | `task<vector<uint8_t>>` |
| `deserialize_async()` | `task<Result<shared_ptr<value_container>>>` | `task<shared_ptr<value_container>>` |
| `load_async()` | `task<VoidResult>` | `task<bool>` |
| `save_async()` | `task<VoidResult>` | `task<bool>` |

### Usage Examples

**Async serialization:**

```cpp
auto container = std::make_shared<value_container>();
container->set("key", std::string("value"));

async_container async_cont(container);
auto serialize_task = async_cont.serialize_async();

// Poll for completion
while (!serialize_task.done()) {
    std::this_thread::sleep_for(1ms);
}

auto result = serialize_task.get();
#if CONTAINER_HAS_COMMON_RESULT
if (result.is_ok()) {
    auto& bytes = result.value();
    // Use serialized bytes
}
#endif
```

**Async file save and load:**

```cpp
async_container async_cont;
async_cont.set("name", std::string("test"))
          .set("count", static_cast<int64_t>(100));

// Save with progress callback
auto save_task = async_cont.save_async("data.bin",
    [](size_t bytes, size_t total) {
        std::cout << bytes << "/" << total << " bytes\n";
    });

while (!save_task.done()) {
    std::this_thread::sleep_for(1ms);
}

// Load into new container
async_container loaded;
auto load_task = loaded.load_async("data.bin");
while (!load_task.done()) {
    std::this_thread::sleep_for(1ms);
}
```

**Chunked streaming serialization:**

```cpp
async_container async_cont(large_container);

// Serialize in 32KB chunks using generator
for (auto& chunk : async_cont.serialize_chunked(32 * 1024)) {
    network_send(chunk);
}
```

**Streaming deserialization:**

```cpp
// Accumulate chunks, then deserialize when complete
std::vector<uint8_t> accumulated;
while (has_more_data()) {
    auto chunk = receive_chunk();
    accumulated.insert(accumulated.end(), chunk.begin(), chunk.end());
}

auto task = async_container::deserialize_streaming(accumulated, true);
while (!task.done()) {
    std::this_thread::sleep_for(1ms);
}
auto result = task.get();
```

### Free Functions

| Function | Description |
|----------|-------------|
| `read_file_async(path, callback)` | Read file bytes in worker thread |
| `write_file_async(path, data, callback)` | Write bytes to file in worker thread |

Both support optional `progress_callback`:
```cpp
using progress_callback = std::function<void(size_t bytes_processed, size_t total_bytes)>;
```

### API Reference

| Method | Description |
|--------|-------------|
| `async_container()` | Default: creates empty container |
| `explicit async_container(shared_ptr<value_container>)` | Wrap existing container |
| `get_container()` | Get underlying `shared_ptr<value_container>` |
| `set_container(shared_ptr<value_container>)` | Replace underlying container |
| `serialize_async()` | Async serialize to bytes |
| `serialize_string_async()` | Async serialize to string |
| `deserialize_async(span<const uint8_t>)` | Static: async deserialize from bytes |
| `deserialize_string_async(string_view)` | Static: async deserialize from string |
| `load_async(path, callback)` | Load from file with optional progress |
| `save_async(path, callback)` | Save to file with optional progress |
| `serialize_chunked(chunk_size)` | Stream serialize via generator (default: 64KB chunks) |
| `deserialize_streaming(data, is_final)` | Static: streaming deserialize |
| `set<T>(key, value)` | Set value (returns `*this` for chaining) |
| `get<T>(key)` | Get value as `optional<T>` |
| `contains(key)` | Check if key exists |

---

## 5. Thread Pool Executor

### Architecture

`thread_pool_executor.h` provides integration between container_system's async
operations and external thread pool executors.

```
Executor Resolution:

  executor_awaitable::await_suspend()
        │
        ├── KCENON_HAS_COMMON_SYSTEM defined?
        │     YES → executor && executor->is_running()?
        │              YES → Submit work_job to IExecutor
        │              NO  → Fallback to std::thread
        │     NO  → Fallback to std::thread
        │
        └── On completion:
              ready_.store(true, release)
              handle.resume()
```

### Executor Types

| Condition | `executor_ptr` Type |
|-----------|---------------------|
| `KCENON_HAS_COMMON_SYSTEM` defined | `shared_ptr<IExecutor>` |
| Otherwise | `std::nullptr_t` (fallback mode) |

### `async_executor_context` — Global Executor

A singleton that holds the global executor for all async operations:

```cpp
// Set a global executor (e.g., during application startup)
auto& ctx = async_executor_context::instance();
ctx.set_executor(my_thread_pool);

// All subsequent async operations use this executor
auto task = async_cont.serialize_async();
```

| Method | Description |
|--------|-------------|
| `instance()` | Get singleton reference |
| `set_executor(executor_ptr)` | Set global executor |
| `get_executor()` | Get current executor |
| `has_executor()` | Check if executor is configured |
| `clear_executor()` | Remove global executor |

### `executor_context_guard` — RAII Executor Scope

Sets an executor for the current scope and restores the previous one on
destruction:

```cpp
{
    executor_context_guard guard(my_executor);
    // All async operations in this scope use my_executor

    auto result = co_await container.serialize_async();
}
// Previous executor automatically restored
```

### Fallback Behavior

When no executor is configured (or `KCENON_HAS_COMMON_SYSTEM` is not defined),
all async operations fall back to `std::thread`:

```cpp
static void run_in_thread(
    std::shared_ptr<executor_state<T>> state,
    std::coroutine_handle<> handle)
{
    std::thread([state, handle]() mutable {
        try {
            state->result_.emplace(state->work_());
        } catch (...) {
            state->exception_ = std::current_exception();
        }
        state->ready_.store(true, std::memory_order_release);
        handle.resume();
    }).detach();
}
```

---

## 6. Complete Examples

### Example 1: Async Container Round-Trip

```cpp
#include <internal/async/async.h>
using namespace container_module;
using namespace container_module::async;

task<void> round_trip_example() {
    // Create and populate container
    auto container = std::make_shared<value_container>();
    container->set("name", std::string("test_data"));
    container->set("version", static_cast<int64_t>(1));

    async_container async_cont(container);

    // Serialize asynchronously
    auto serialized = co_await async_cont.serialize_async();
#if CONTAINER_HAS_COMMON_RESULT
    if (!serialized.is_ok()) {
        co_return;  // Handle error
    }
    auto& bytes = serialized.value();
#else
    auto& bytes = serialized;
#endif

    // Deserialize asynchronously
    auto restored_result = co_await async_container::deserialize_async(bytes);
#if CONTAINER_HAS_COMMON_RESULT
    if (restored_result.is_ok()) {
        auto restored = restored_result.value();
        // restored now contains the original data
    }
#endif
    co_return;
}
```

### Example 2: Lazy Container Value Iteration

```cpp
generator<std::string> get_all_string_values(
    std::shared_ptr<value_container> container,
    const std::vector<std::string>& keys)
{
    for (const auto& key : keys) {
        auto val = container->get_value(key);
        if (val.has_value()) {
            if (auto* str = std::get_if<std::string>(&val->data)) {
                co_yield *str;
            }
        }
    }
}

// Usage: only processes values as needed
auto gen = get_all_string_values(container, keys);
for (auto& value : take(gen, 10)) {
    process(value);  // Only first 10 string values
}
```

### Example 3: Async File Pipeline

```cpp
task<void> async_pipeline(const std::string& input_path,
                          const std::string& output_path) {
    // Load container from file
    async_container cont;
    auto load_result = co_await cont.load_async(input_path);
#if CONTAINER_HAS_COMMON_RESULT
    if (!load_result.is_ok()) {
        co_return;
    }
#endif

    // Modify the container
    cont.set("processed", true)
        .set("timestamp", static_cast<int64_t>(
            std::chrono::system_clock::now().time_since_epoch().count()));

    // Save to output file with progress reporting
    auto save_result = co_await cont.save_async(output_path,
        [](size_t bytes, size_t total) {
            // Report progress
        });

    co_return;
}
```

### Example 4: Producer-Consumer with Chunked Streaming

```cpp
task<void> stream_container_over_network(
    std::shared_ptr<value_container> container,
    NetworkConnection& conn)
{
    async_container async_cont(container);

    // Producer: serialize in chunks
    for (auto& chunk : async_cont.serialize_chunked(16 * 1024)) {
        conn.send(chunk);
    }
    conn.send_eof();

    co_return;
}

task<std::shared_ptr<value_container>> receive_container(
    NetworkConnection& conn)
{
    // Consumer: accumulate chunks
    std::vector<uint8_t> buffer;
    while (auto chunk = conn.receive()) {
        buffer.insert(buffer.end(), chunk->begin(), chunk->end());
    }

    // Deserialize the complete data
    auto result = co_await async_container::deserialize_streaming(buffer, true);
#if CONTAINER_HAS_COMMON_RESULT
    co_return result.is_ok() ? result.value() : nullptr;
#else
    co_return result;
#endif
}
```

---

## 7. Compiler Requirements

### Feature Detection

The framework uses compile-time feature detection in `async.h`:

```cpp
#if defined(__cpp_impl_coroutine) && __cpp_impl_coroutine >= 201902L
    #define CONTAINER_HAS_COROUTINES 1
#elif defined(_MSC_VER) && _MSC_VER >= 1928
    #define CONTAINER_HAS_COROUTINES 1
#else
    #define CONTAINER_HAS_COROUTINES 0
#endif
```

Runtime check:
```cpp
if constexpr (container_module::async::has_coroutine_support) {
    // Use coroutine APIs
}
```

### CMake Configuration

```cmake
option(CONTAINER_ENABLE_COROUTINES "Enable C++20 coroutine-based async API" ON)

# When enabled:
target_compile_definitions(container_system PUBLIC CONTAINER_HAS_COROUTINES=1)

# GCC requires explicit flag:
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    check_cxx_compiler_flag("-fcoroutines" COMPILER_HAS_FCOROUTINES)
    if(COMPILER_HAS_FCOROUTINES)
        target_compile_options(container_system PUBLIC -fcoroutines)
    endif()
endif()
```

```bash
# Enable (default)
cmake -DCONTAINER_ENABLE_COROUTINES=ON ..

# Disable
cmake -DCONTAINER_ENABLE_COROUTINES=OFF ..
```

### Minimum Compiler Versions

| Compiler | Minimum Version | Flag Required |
|----------|----------------|---------------|
| GCC | 11.0+ | `-fcoroutines` |
| Clang | 14.0+ | None (built-in) |
| MSVC | 19.28+ (VS 2019 16.8) | `/std:c++20` |
| Apple Clang | 14.0+ | None (built-in) |

### Known Compiler-Specific Issues

| Compiler | Issue | Workaround |
|----------|-------|------------|
| GCC < 11 | No coroutine support | Use Clang or upgrade GCC |
| GCC 11 | `-fcoroutines` required | CMake auto-detects and adds flag |
| MSVC | Some edge cases with symmetric transfer | Use latest VS updates |
| Clang with libstdc++ | May need `-stdlib=libc++` for full support | Use libc++ or recent libstdc++ |

### Conditional Compilation Macros

| Macro | Defined By | Purpose |
|-------|-----------|---------|
| `CONTAINER_HAS_COROUTINES` | `async.h` auto-detect or CMake | Feature gate for all async code |
| `CONTAINER_ENABLE_COROUTINES` | CMake option | Build system control |
| `CONTAINER_HAS_COMMON_RESULT` | Build configuration | Enables Result-based return types |
| `KCENON_HAS_COMMON_SYSTEM` | Build configuration | Enables IExecutor thread pool integration |

---

## References

- **Source files**:
  - `internal/async/task.h` - `task<T>` coroutine type (453 lines)
  - `internal/async/generator.h` - `generator<T>` lazy sequence (406 lines)
  - `internal/async/async_container.h` - Async container wrapper (1155 lines)
  - `internal/async/thread_pool_executor.h` - Executor integration (334 lines)
  - `internal/async/async.h` - Umbrella header with feature detection (93 lines)
- **Tests**: `tests/async_tests.cpp` - Comprehensive tests (953 lines)
- **Related docs**:
  - [Lock-Free Data Structures](advanced/LOCK_FREE.md) - RCU and epoch-based reclamation
  - [Memory Management](advanced/MEMORY_MANAGEMENT.md) - Memory pool architecture
