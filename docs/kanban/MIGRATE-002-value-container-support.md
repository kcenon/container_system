# MIGRATE-002: Add variant_value_v2 Support to value_container

## Metadata
- **ID**: MIGRATE-002
- **Priority**: HIGH
- **Estimated Duration**: 2 days
- **Dependencies**: None (MIGRATE-001 completed)
- **Status**: TODO
- **Assignee**: TBD
- **Related Documents**: [MIGRATION_PLAN.md](../advanced/MIGRATION_PLAN.md)

---

## Overview

### What are we changing?

Add API to the `value_container` class that directly supports `variant_value_v2` type, enabling legacy and modern systems to coexist.

**Current state**:
- `value_container` only supports `std::shared_ptr<value>`
- Using `variant_value_v2` requires conversion through `value_bridge`

**Goals**:
- Add API to directly store/retrieve `variant_value_v2`
- Maintain backward compatibility
- Support gradual migration

---

## Changes

### How will we implement this?

#### 1. Add New API (container.h)

```cpp
class value_container {
public:
    // Existing API (keep)
    void add_value(std::shared_ptr<value> val);
    std::shared_ptr<value> get_value(const std::string& key) const;

    // New API (add)

    // Set variant_value_v2 directly
    void set_unit(const variant_value_v2& val);
    void set_units(const std::vector<variant_value_v2>& vals);

    // Convenience methods - set values directly
    template<typename T>
    void set_value(const std::string& key, T value);

    // Retrieve as variant_value_v2
    std::optional<variant_value_v2> get_variant_value(const std::string& key) const;

    // Return all values as variant_value_v2
    std::vector<variant_value_v2> get_variant_values() const;

    // Dual mode support flag
    bool is_variant_mode() const;
    void enable_variant_mode(bool enable = true);

private:
    // Internal storage (dual mode)
    std::vector<std::shared_ptr<value>> legacy_values_;
    std::vector<variant_value_v2> variant_values_;
    bool variant_mode_ = false;
};
```

#### 2. Implementation Details (container.cpp)

```cpp
void value_container::set_unit(const variant_value_v2& val) {
    if (variant_mode_) {
        // Add directly to variant storage
        auto it = std::find_if(variant_values_.begin(), variant_values_.end(),
            [&](const auto& v) { return v.name() == val.name(); });

        if (it != variant_values_.end()) {
            *it = val;
        } else {
            variant_values_.push_back(val);
        }
    } else {
        // Legacy mode: convert through value_bridge and store
        auto legacy = value_bridge::to_legacy(val);
        add_value(legacy);
    }
}

template<typename T>
void value_container::set_value(const std::string& key, T value) {
    if constexpr (std::is_same_v<T, bool>) {
        set_unit(make_bool_value(key, value));
    } else if constexpr (std::is_integral_v<T>) {
        set_unit(make_numeric_value(key, value));
    } else if constexpr (std::is_floating_point_v<T>) {
        set_unit(make_numeric_value(key, value));
    } else if constexpr (std::is_same_v<T, std::string>) {
        set_unit(make_string_value(key, value));
    }
    // ... other types
}

std::optional<variant_value_v2>
value_container::get_variant_value(const std::string& key) const {
    if (variant_mode_) {
        auto it = std::find_if(variant_values_.begin(), variant_values_.end(),
            [&](const auto& v) { return v.name() == key; });
        if (it != variant_values_.end()) {
            return *it;
        }
    } else {
        // Legacy mode: convert through value_bridge
        auto legacy = get_value(key);
        if (legacy) {
            return value_bridge::to_modern(legacy);
        }
    }
    return std::nullopt;
}
```

#### 3. Serialization Support

```cpp
// Support variant_mode in serialize() method
std::string value_container::serialize() const {
    if (variant_mode_) {
        // Serialize variant_values_ directly
        return serialize_variant_values();
    } else {
        // Serialize existing legacy_values_
        return serialize_legacy_values();
    }
}

// Auto-detect format in deserialize()
static std::shared_ptr<value_container> deserialize(const std::string& data) {
    auto container = std::make_shared<value_container>();
    // Deserialize in appropriate mode based on data format
    // Wire format is identical, maintaining compatibility
    return container;
}
```

---

## Test Plan

### How will we test this?

#### 1. API Tests

```cpp
TEST(ValueContainer, SetVariantValue) {
    value_container container;
    container.enable_variant_mode();

    container.set_value("count", 42);
    container.set_value("name", std::string("test"));

    auto count = container.get_variant_value("count");
    ASSERT_TRUE(count.has_value());
    EXPECT_EQ(count->get<int32_t>().value(), 42);
}

TEST(ValueContainer, BackwardCompatibility) {
    value_container container;  // Default: variant_mode = false

    // Use legacy API
    container.add_value(std::make_shared<int_value>("count", 42));

    // Can retrieve with new API
    auto val = container.get_variant_value("count");
    ASSERT_TRUE(val.has_value());
}
```

#### 2. Serialization Compatibility Tests

```cpp
TEST(ValueContainer, SerializationCompatibility) {
    // Serialize in legacy mode
    value_container legacy;
    legacy.add_value(std::make_shared<int_value>("id", 123));
    auto legacy_bytes = legacy.serialize();

    // Deserialize in variant mode
    auto restored = value_container::deserialize(legacy_bytes);
    restored->enable_variant_mode();

    auto val = restored->get_variant_value("id");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val->get<int32_t>().value(), 123);
}
```

#### 3. Performance Tests

```cpp
TEST(ValueContainer, VariantModePerformance) {
    value_container container;
    container.enable_variant_mode();

    const int iterations = 100'000;

    // Measure variant mode performance
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        container.set_value("key_" + std::to_string(i), i);
    }
    auto end = std::chrono::high_resolution_clock::now();

    // Target: No performance degradation compared to legacy
}
```

#### Success Criteria
- [ ] All existing tests pass (backward compatibility)
- [ ] New API tests 100% pass
- [ ] No serialization format changes
- [ ] No performance regression

---

## Migration Guide

### Step-by-Step Migration

**Step 1**: Start using new API (backward compatible)
```cpp
// Existing code works as-is
container.add_value(std::make_shared<int_value>("count", 42));
```

**Step 2**: Gradual transition to new API
```cpp
// Use new API
container.set_value("count", 42);
```

**Step 3**: Enable variant mode
```cpp
container.enable_variant_mode();
container.set_value("count", 42);  // Uses optimized path
```

---

## Checklist

- [ ] Add `set_unit()` API
- [ ] Add `set_units()` API
- [ ] Add `set_value<T>()` template method
- [ ] Add `get_variant_value()` API
- [ ] Add `get_variant_values()` API
- [ ] Add variant_mode flag
- [ ] Maintain serialization compatibility
- [ ] Verify existing tests pass
- [ ] Write and pass new tests
- [ ] Performance benchmark

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-23
