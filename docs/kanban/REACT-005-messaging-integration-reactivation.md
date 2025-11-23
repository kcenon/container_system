# REACT-005: Messaging Integration Reactivation

## Metadata
- **ID**: REACT-005
- **Priority**: MEDIUM
- **Estimated Duration**: 1 day
- **Dependencies**: REACT-001
- **Status**: TODO
- **Assignee**: TBD
- **Related Documents**: [REACTIVATION_PLAN.md](../advanced/REACTIVATION_PLAN.md)

---

## Overview

### What are we changing?

Redesign the `messaging_integration` module that depended on the deleted `add()` method to use the new `set_value()` pattern to restore integration with the messaging system.

**Current State**:
- Disabled at `CMakeLists.txt:115-116`
- Using deleted `add()` method

**Affected Files**:
- `integration/messaging_integration.h`
- `integration/messaging_integration.cpp`

---

## Changes

### How are we changing it?

#### 1. API Redesign (0.5 days)

**Existing API (Removed)**:
```cpp
class messaging_integration {
public:
    void add_to_message(message& msg, std::shared_ptr<value> val);
    std::shared_ptr<value> extract_from_message(const message& msg);
};
```

**New API**:
```cpp
class messaging_integration {
public:
    // New pattern: set_message
    void set_message_value(message& msg, const std::string& key,
                           const optimized_value& val);

    // variant_value_v2 support
    void set_message_value(message& msg, const std::string& key,
                           const variant_value_v2& val);

    // Direct value setting (convenience method)
    template<typename T>
    void set_message_value(message& msg, const std::string& key, T value);

    // Value extraction
    std::optional<optimized_value> get_message_value(const message& msg,
                                                      const std::string& key);
};
```

#### 2. Bridge Layer Update (0.25 days)

**Message Container Bridge**:
```cpp
class message_container_bridge {
public:
    // thread_safe_container <-> message conversion
    static message to_message(const thread_safe_container& container);
    static thread_safe_container from_message(const message& msg);

    // Streaming support
    static void stream_to_message(const thread_safe_container& container,
                                  message_stream& stream);
};
```

#### 3. Existing Code Migration (0.25 days)

**Migration Pattern**:
```cpp
// Before
void send_data(message& msg) {
    auto val = std::make_shared<int_value>("count", 42);
    messaging.add_to_message(msg, val);
}

// After
void send_data(message& msg) {
    messaging.set_message_value(msg, "count", 42);
    // or
    optimized_value val{"count", value_types::int_value, 42};
    messaging.set_message_value(msg, "count", val);
}
```

---

## Test Plan

### How will we test it?

#### 1. Unit Tests
```cpp
TEST(MessagingIntegration, SetAndGetValue) {
    message msg;
    messaging_integration mi;

    mi.set_message_value(msg, "key", 42);

    auto val = mi.get_message_value(msg, "key");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(std::get<int32_t>(val->data), 42);
}

TEST(MessagingIntegration, RoundTrip) {
    thread_safe_container container;
    container.set_value("id", 123);
    container.set_value("name", std::string("test"));

    auto msg = message_container_bridge::to_message(container);
    auto restored = message_container_bridge::from_message(msg);

    EXPECT_EQ(container, restored);
}
```

#### 2. Integration Tests
- Integration tests with actual messaging system
- Protocol compatibility verification

#### 3. Performance Tests
- Measure message conversion performance
- Compare performance against legacy

#### Success Criteria
- [ ] messaging_integration functionality works correctly
- [ ] Messaging protocol compatibility maintained
- [ ] No performance regression

---

## Technical Details

### Message Format Compatibility

**Wire Format remains unchanged**:
```
[name_length:4][name:UTF-8][type:1][data:variable]
```

- Both variant_value_v2 and legacy systems use the same wire format
- Full compatibility with existing messages maintained

### Thread Safety

- `messaging_integration` is designed as stateless
- `message` object synchronization is caller's responsibility
- `thread_safe_container` is internally thread-safe

---

## Risks and Mitigations

| Risk | Probability | Impact | Mitigation |
|--------|------|------|-----------|
| Protocol incompatibility | Low | High | Maintain identical wire format |
| Performance degradation | Low | Medium | Benchmark verification |
| API confusion | Medium | Low | Clear documentation |

---

## Checklist

- [ ] Redesign messaging_integration.h API
- [ ] Implement messaging_integration.cpp
- [ ] Implement message_container_bridge
- [ ] Write unit tests
- [ ] Write integration tests
- [ ] Enable in CMakeLists.txt
- [ ] Pass local tests
- [ ] Confirm CI passes
- [ ] Update documentation

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-23
