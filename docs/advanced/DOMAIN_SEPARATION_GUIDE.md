# Domain Separation Guide

**Date**: 2025-11-09
**Status**: Phase 3 Implementation Complete

## Overview

The container_system has been refactored to separate domain-agnostic value storage from messaging-specific functionality. This separation improves code reusability, maintainability, and allows the library to be used in non-messaging contexts.

## Architecture

### Before (Monolithic)

```
value_container
  ├─ Messaging metadata (source_id, target_id, message_type)
  └─ Value storage (mixed implementation)
```

**Problems:**
- Messaging domain coupled with storage layer
- Cannot use container for general-purpose serialization
- All users bear messaging overhead

### After (Separated)

```
message_container (Messaging Layer)
  ├─ Messaging metadata (source_id, target_id, message_type, version)
  └─ payload: value_store (Storage Layer)

value_store (Domain-Agnostic Layer)
  └─ Pure key-value storage with serialization
```

**Benefits:**
- Clear separation of concerns
- value_store can be used independently
- Easier to test and maintain
- Better code reusability

## Components

### value_store (Domain-Agnostic)

**Location**: `core/value_store.h`, `core/value_store.cpp`

**Purpose**: Pure value storage without any domain-specific assumptions.

**Features:**
- Key-value storage interface
- JSON/Binary serialization (TODO: complete implementation)
- Optional thread-safety
- Statistics tracking

**Usage:**
```cpp
#include <container/core/value_store.h>
#include <container/internal/variant_value_v2.h>

using namespace container_module;

// Create a value store
value_store store;

// Add values (thread-safe if enabled)
store.add("user_id", variant_value_v2(12345));
store.add("username", variant_value_v2(std::string("john_doe")));

// Retrieve values
auto user_id = store.get("user_id");
if (user_id) {
    std::cout << "User ID: " << user_id->get<int>() << std::endl;
}

// Serialize (when implemented)
// std::string json = store.serialize();
```

**Limitations:**
- Copy/move semantics deleted (due to std::shared_mutex member)
- Use unique_ptr or references to pass value_store instances

### message_container (Messaging-Specific)

**Location**: `messaging/message_container.h`, `messaging/message_container.cpp`

**Purpose**: Adds messaging metadata on top of value_store.

**Features:**
- Source/target identification
- Message type classification
- Version tracking
- Header swapping for request/response patterns
- Payload storage via value_store composition

**Usage:**
```cpp
#include <container/messaging/message_container.h>

using namespace container_module;

// Create a message container
message_container msg("client1", "", "server", "", "request");

// Set header information
msg.set_message_type("user_query");
msg.set_version("2.0.0.0");

// Add payload data
msg.payload().add("query", variant_value_v2(std::string("SELECT * FROM users")));
msg.payload().add("limit", variant_value_v2(100));

// Access metadata
std::cout << "Source: " << msg.source_id() << std::endl;
std::cout << "Target: " << msg.target_id() << std::endl;
std::cout << "Type: " << msg.message_type() << std::endl;

// Swap header for response
msg.swap_header();

// Serialize (partial implementation)
std::string json = msg.serialize();
```

**Limitations:**
- Copy/move semantics deleted (inherits from value_store)
- Deserialization returns unique_ptr
- Payload deserialization not yet implemented (TODO)

## Migration Guide

### For General-Purpose Use

If you only need value storage and serialization **without** messaging metadata:

```cpp
// OLD: Using value_container (overkill)
value_container container;
container.add(std::make_shared<int_value>("count", 42));

// NEW: Using value_store (cleaner)
value_store store;
store.add("count", variant_value_v2(42));
```

### For Messaging Use Cases

If you need messaging metadata (source, target, message type):

```cpp
// OLD: Using value_container
auto container = std::make_shared<value_container>(
    "source", "", "target", "", "msg_type", std::vector<std::shared_ptr<value>>{}
);

// NEW: Using message_container
auto msg = std::make_unique<message_container>(
    "source", "", "target", "", "msg_type"
);
// Add payload using msg.payload().add(...)
```

### Backward Compatibility

**value_container is NOT deprecated yet**. It will continue to be supported until the next major version. The migration timeline is:

1. **Current (Phase 3)**: message_container available, value_container maintained
2. **Phase 4-5**: Encourage migration, provide tooling
3. **Next Major Version**: Deprecate value_container
4. **Future Major Version**: Remove value_container

## Design Decisions

### Why Delete Copy/Move Semantics?

Both `value_store` and `message_container` contain `std::shared_mutex` for thread-safety, which makes them non-copyable and non-movable. Attempting to default these operations results in compiler errors.

**Options considered:**
1. **Delete copy/move** (chosen) - Prevents accidental copies, forces intentional design
2. **Use shared_ptr everywhere** - Adds overhead, complicates ownership
3. **Custom copy/move** - Complex, error-prone (mutex cannot be copied)

**Recommendation**: Use `unique_ptr` for ownership, references for passing.

### Why unique_ptr Return from Deserialize?

Since copy/move are deleted, factory methods cannot return by value. `unique_ptr` provides:
- Clear ownership semantics
- No copy overhead
- Compatible with move semantics when needed

```cpp
// Deserialization example
auto msg = message_container::deserialize(json_data);
// msg is unique_ptr<message_container>
msg->set_source("client");
```

## Implementation Status

### Completed
- ✅ value_store interface design
- ✅ message_container implementation
- ✅ Basic serialization structure
- ✅ CMake integration
- ✅ Deleted copy/move semantics
- ✅ All existing tests passing

### TODO (Future Work)
- ⏳ Complete payload deserialization in message_container
- ⏳ Implement value_store serialization/deserialization
- ⏳ Add comprehensive unit tests for message_container
- ⏳ Add integration tests showing migration path
- ⏳ Provide automated migration scripts
- ⏳ Performance benchmarks (value_store vs value_container)

## Future Plans

### Phase 4: Library Separation

Split into two independent libraries:

```
container_system_core/           # General-purpose
  ├── value_store
  ├── variant_value_v2
  └── serialization

container_system_messaging/      # Messaging extension
  ├── message_container
  └── messaging_integration
```

### Phase 5: Complete Migration

- Remove value_container
- Rename variant_value_v2 → value
- Remove legacy systems (polymorphic value, variant_value v1)

## References

- [IMPROVEMENT_PLAN.md](../IMPROVEMENT_PLAN.md) - Sprint 3: Domain Separation
- [value_store.h](../core/value_store.h) - Value store interface
- [message_container.h](../messaging/message_container.h) - Message container interface
- [SYSTEM_ANALYSIS_SUMMARY.md](../../SYSTEM_ANALYSIS_SUMMARY.md) - Overall system analysis

## Questions?

For questions or issues with the domain separation:
1. Review this guide
2. Check the IMPROVEMENT_PLAN.md for timeline
3. Review code examples in messaging/message_container.cpp

---

**Last Updated**: 2025-11-09
**Phase**: 3 (Domain Separation)
**Status**: Implementation Complete, Documentation In Progress
