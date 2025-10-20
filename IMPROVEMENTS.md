# Container System - Improvement Plan

> **Language:** **English** | [한국어](IMPROVEMENTS_KO.md)

## Current Status

**Version:** 1.0.0
**Last Review:** 2025-01-20
**Overall Score:** 3.5/5

### Critical Issues

## 1. Inefficient Lock Acquisition in Thread-Safe Mode

**Location:** `core/container.h:354-386`

**Current Issue:**
```cpp
class read_lock_guard {
    std::shared_lock<std::shared_mutex> lock_;
    bool is_active_;
public:
    read_lock_guard(const value_container* c)
        : lock_(c->mutex_)  // ❌ Always acquires lock, even if not needed!
        , is_active_(c->thread_safe_enabled_.load(std::memory_order_acquire)) {
        if (is_active_) {
            c->read_count_.fetch_add(1, std::memory_order_relaxed);
        }
    }
};
```

**Problem:**
- Acquires lock even when `thread_safe_enabled_` is false
- Unnecessary performance overhead in single-threaded mode
- Comment claims it's "FIXED" but actually introduces different issue

**Solution:**
```cpp
// Option 1: Conditional lock
class read_lock_guard {
public:
    read_lock_guard(const value_container* c)
        : container_(c) {
        if (c->thread_safe_enabled_.load(std::memory_order_acquire)) {
            lock_.emplace(c->mutex_);
            c->read_count_.fetch_add(1, std::memory_order_relaxed);
            is_locked_ = true;
        }
    }

    ~read_lock_guard() {
        if (is_locked_) {
            lock_.reset();
        }
    }

private:
    const value_container* container_;
    std::optional<std::shared_lock<std::shared_mutex>> lock_;
    bool is_locked_ = false;
};

// Option 2: Template-based (zero overhead when disabled)
template<bool ThreadSafe>
class container_impl;

template<>
class container_impl<true> {
    mutable std::shared_mutex mutex_;
    using read_guard = std::shared_lock<std::shared_mutex>;
    using write_guard = std::unique_lock<std::shared_mutex>;
};

template<>
class container_impl<false> {
    struct null_guard {
        null_guard(auto&&...) {}  // No-op
    };
    using read_guard = null_guard;
    using write_guard = null_guard;
};
```

**Priority:** P1
**Effort:** 2-3 days
**Impact:** High (performance in single-threaded mode)

---

## 2. Inefficient String Operations

**Current Issue:**
Many string copies in serialization/deserialization.

**Solution:**
```cpp
// Use string_view for parsing
class value_container {
public:
    bool deserialize(std::string_view data, bool parse_only_header = true);

private:
    void parsing(std::string_view source_name,
                std::string_view target_name,
                std::string_view target_value,
                std::string& target_variable);  // Only assign when needed
};

// Use move semantics
void set_message_type(std::string&& message_type) {  // Rvalue ref
    message_type_ = std::move(message_type);
}
```

**Priority:** P2
**Effort:** 2-3 days

---

## High Priority Improvements

### 3. Add Zero-Copy Deserialization

```cpp
class zero_copy_container {
public:
    // Parse without copying
    bool parse_view(std::string_view data) {
        // Store pointer to original data
        data_view_ = data;
        parse_header_only();
        parsed_ = false;  // Lazy parse on access
    }

    // Lazy value access
    std::shared_ptr<value> get_value(std::string_view name) {
        if (!parsed_) {
            parse_values_lazy();
        }
        return find_value(name);
    }

private:
    std::string_view data_view_;  // No copy
    bool parsed_ = false;
};
```

**Priority:** P2
**Effort:** 5-7 days

---

### 4. Add Schema Validation

```cpp
class container_schema {
public:
    container_schema& require_field(std::string_view name, value_types type) {
        required_fields_[std::string(name)] = type;
        return *this;
    }

    container_schema& optional_field(std::string_view name, value_types type) {
        optional_fields_[std::string(name)] = type;
        return *this;
    }

    result_void validate(const value_container& container) const {
        for (const auto& [name, type] : required_fields_) {
            auto value = container.get_value(name);
            if (!value || value->value_type() == value_types::null_value) {
                return error_info{-1, "Missing required field: " + name, "schema"};
            }
            if (value->value_type() != type) {
                return error_info{-1, "Type mismatch for field: " + name, "schema"};
            }
        }
        return result_void{};
    }

private:
    std::unordered_map<std::string, value_types> required_fields_;
    std::unordered_map<std::string, value_types> optional_fields_;
};

// Usage:
container_schema user_schema;
user_schema.require_field("id", value_types::int_value)
          .require_field("name", value_types::string_value)
          .optional_field("email", value_types::string_value);

auto result = user_schema.validate(container);
if (!result) {
    std::cerr << "Validation failed: " << result.error().message << "\n";
}
```

**Priority:** P3
**Effort:** 4-5 days

---

## Testing Requirements

```cpp
TEST(Container, ThreadSafeOverhead) {
    value_container container;
    container.enable_thread_safe(false);  // Disable

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000; ++i) {
        container.set_message_type("test");
        auto msg = container.message_type();
    }
    auto duration = std::chrono::high_resolution_clock::now() - start;

    // Should be fast without locks
    EXPECT_LT(duration, std::chrono::milliseconds(10));
}

TEST(Container, ZeroCopyPerformance) {
    std::string large_data = generate_large_container(10000);

    // Traditional copy
    auto start1 = std::chrono::high_resolution_clock::now();
    value_container c1(large_data);
    auto t1 = std::chrono::high_resolution_clock::now() - start1;

    // Zero-copy
    auto start2 = std::chrono::high_resolution_clock::now();
    zero_copy_container c2;
    c2.parse_view(large_data);
    auto t2 = std::chrono::high_resolution_clock::now() - start2;

    EXPECT_LT(t2, t1 / 10);  // 10x faster
}
```

**Total Effort:** 13-18 days
