# MIGRATE-003: thread_safe_container Integration

## Metadata
- **ID**: MIGRATE-003
- **Priority**: HIGH
- **Estimated Duration**: 1 day
- **Dependencies**: MIGRATE-001, MIGRATE-002
- **Status**: TODO
- **Assignee**: TBD
- **Related Documents**: [MIGRATION_PLAN.md](../advanced/MIGRATION_PLAN.md)

---

## Overview

### What are we changing?

Integrate `variant_value_v2` to store `thread_safe_container` and support recursive container serialization.

**Current state**:
- `std::shared_ptr<thread_safe_container>` is defined in variant_value_v2's ValueVariant
- Actual serialization/deserialization implementation needed

**Goals**:
- Full nested container support
- Recursive serialization/deserialization
- Thread-safe operation guarantee

---

## Changes

### How will we implement this?

#### 1. Store Container in variant_value_v2

```cpp
// Already defined in ValueVariant:
// std::shared_ptr<thread_safe_container> [14]

// Add/verify constructor
variant_value_v2::variant_value_v2(
    const std::string& name,
    std::shared_ptr<thread_safe_container> container)
    : name_(name)
    , data_(std::move(container))
{
}
```

#### 2. Implement Container Value Serialization

```cpp
// variant_value_v2.cpp - serialize()
std::vector<uint8_t> variant_value_v2::serialize() const {
    std::vector<uint8_t> result;

    // Serialize name
    serialize_name(result);

    // Serialize type
    result.push_back(static_cast<uint8_t>(type()));

    // Serialize data
    std::visit([&result](const auto& value) {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, std::shared_ptr<thread_safe_container>>) {
            // Recursive container serialization
            if (value) {
                auto container_bytes = value->serialize_array();
                uint32_t size = static_cast<uint32_t>(container_bytes.size());
                append_le(result, size);
                result.insert(result.end(),
                              container_bytes.begin(),
                              container_bytes.end());
            } else {
                append_le(result, uint32_t{0});  // null container
            }
        }
        // ... other types
    }, data_);

    return result;
}
```

#### 3. Implement Container Value Deserialization

```cpp
// variant_value_v2.cpp - deserialize()
std::optional<variant_value_v2> variant_value_v2::deserialize(
    const std::vector<uint8_t>& data,
    size_t& offset)
{
    // Deserialize name
    auto name = deserialize_name(data, offset);
    if (!name) return std::nullopt;

    // Deserialize type
    auto type = static_cast<value_types>(data[offset++]);

    // Deserialize data
    switch (type) {
        case value_types::container_value: {
            uint32_t size = read_le<uint32_t>(data, offset);
            if (size == 0) {
                return variant_value_v2(*name, std::shared_ptr<thread_safe_container>{});
            }

            std::vector<uint8_t> container_data(
                data.begin() + offset,
                data.begin() + offset + size);
            offset += size;

            // Recursive container deserialization
            auto container = thread_safe_container::deserialize(container_data);
            return variant_value_v2(*name, std::move(container));
        }
        // ... other types
    }
}
```

#### 4. Add Variant Storage to thread_safe_container

```cpp
// thread_safe_container.h
class thread_safe_container {
public:
    // Store variant_value_v2 directly
    void set_variant(const variant_value_v2& val);
    std::optional<variant_value_v2> get_variant(const std::string& key) const;

    // Nested container support
    void set_container(const std::string& key,
                       std::shared_ptr<thread_safe_container> nested);
    std::shared_ptr<thread_safe_container> get_container(const std::string& key) const;

private:
    std::vector<variant_value_v2> values_;
    mutable std::shared_mutex mutex_;
};
```

---

## Test Plan

### How will we test this?

#### 1. Nested Container Test

```cpp
TEST(ThreadSafeContainer, NestedContainerStorage) {
    auto inner = std::make_shared<thread_safe_container>();
    inner->set_value("inner_key", 42);

    thread_safe_container outer;
    outer.set_container("nested", inner);

    auto retrieved = outer.get_container("nested");
    ASSERT_NE(retrieved, nullptr);

    auto val = retrieved->get_variant("inner_key");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val->get<int32_t>().value(), 42);
}
```

#### 2. Recursive Serialization Test

```cpp
TEST(ThreadSafeContainer, RecursiveSerialization) {
    // 3-level nested container
    auto level3 = std::make_shared<thread_safe_container>();
    level3->set_value("data", std::string("deepest"));

    auto level2 = std::make_shared<thread_safe_container>();
    level2->set_container("child", level3);

    auto level1 = std::make_shared<thread_safe_container>();
    level1->set_container("child", level2);

    // Serialize
    auto bytes = level1->serialize_array();

    // Deserialize
    auto restored = thread_safe_container::deserialize(bytes);

    // Verify
    auto l2 = restored->get_container("child");
    ASSERT_NE(l2, nullptr);
    auto l3 = l2->get_container("child");
    ASSERT_NE(l3, nullptr);
    auto data = l3->get_variant("data");
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(data->get<std::string>().value(), "deepest");
}
```

#### 3. Thread Safety Test

```cpp
TEST(ThreadSafeContainer, ConcurrentNestedAccess) {
    auto container = std::make_shared<thread_safe_container>();

    // Access nested containers from multiple threads
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([container, i]() {
            auto nested = std::make_shared<thread_safe_container>();
            nested->set_value("id", i);
            container->set_container("nested_" + std::to_string(i), nested);
        });
    }

    for (auto& t : threads) t.join();

    // Verify all nested containers
    for (int i = 0; i < 10; ++i) {
        auto nested = container->get_container("nested_" + std::to_string(i));
        ASSERT_NE(nested, nullptr);
    }
}
```

#### 4. Round-Trip Test

```cpp
TEST(VariantValueV2, ContainerRoundTrip) {
    auto container = std::make_shared<thread_safe_container>();
    container->set_value("key1", 123);
    container->set_value("key2", std::string("value"));

    variant_value_v2 val("nested", container);

    auto bytes = val.serialize();
    auto restored = variant_value_v2::deserialize(bytes);

    ASSERT_TRUE(restored.has_value());
    EXPECT_EQ(restored->type(), value_types::container_value);

    auto restored_container = restored->get<std::shared_ptr<thread_safe_container>>();
    ASSERT_TRUE(restored_container.has_value());
    // ... verify values
}
```

#### Success Criteria
- [ ] Nested container store/retrieve
- [ ] Recursive serialization works perfectly
- [ ] Thread safety verified
- [ ] Round-trip test pass

---

## Technical Details

### Serialization Format

```
Container Value Serialization Format:
┌─────────────────────────────────────────┐
│ name_length (4 bytes, LE)               │
│ name (variable)                         │
│ type (1 byte) = 14 (container_value)    │
│ container_size (4 bytes, LE)            │
│ container_data (variable)               │
│   └── Recursively serialize entire      │
│       container                         │
└─────────────────────────────────────────┘
```

### Circular Reference Prevention

```cpp
// Detect circular references during serialize
std::set<void*> visited_containers;

bool check_circular_reference(
    const std::shared_ptr<thread_safe_container>& container,
    std::set<void*>& visited)
{
    if (visited.count(container.get())) {
        return true;  // Circular reference detected
    }
    visited.insert(container.get());
    // Recursively check nested containers
    return false;
}
```

---

## Checklist

- [ ] Verify variant_value_v2 container constructor
- [ ] Implement container serialization
- [ ] Implement container deserialization
- [ ] Add variant storage to thread_safe_container
- [ ] Nested container tests
- [ ] Recursive serialization tests
- [ ] Thread safety tests
- [ ] Round-trip tests
- [ ] Circular reference prevention

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-23
