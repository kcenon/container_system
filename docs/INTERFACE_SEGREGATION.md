# Interface Segregation in container_system

**Date**: 2025-11-09
**Status**: Active
**Related**: VISITOR_PATTERN_GUIDE.md, EXCEPTION_SAFETY.md

## Overview

This document explains how container_system adheres to the Interface Segregation Principle (ISP) and provides guidelines for maintaining clean, focused interfaces.

## Interface Segregation Principle (ISP)

> "Clients should not be forced to depend on interfaces they do not use."
> — Robert C. Martin (SOLID Principles)

### Benefits
- Smaller, more focused interfaces
- Reduced coupling between components
- Easier testing and mocking
- Better code organization

---

## Historical Problem: Fat Interface in Legacy value Class

### The Anti-Pattern

The legacy polymorphic `value` class violated ISP by forcing all subclasses to implement a large number of methods:

```cpp
// ❌ DEPRECATED: Fat Interface (violates ISP)
class [[deprecated]] value {
public:
    // Conversion methods (20+ methods)
    virtual bool to_bool() const { throw std::logic_error("not bool"); }
    virtual short to_short() const { throw std::logic_error("not short"); }
    virtual unsigned short to_ushort() const { throw std::logic_error("not ushort"); }
    virtual int to_int() const { throw std::logic_error("not int"); }
    virtual unsigned int to_uint() const { throw std::logic_error("not uint"); }
    virtual long to_long() const { throw std::logic_error("not long"); }
    virtual unsigned long to_ulong() const { throw std::logic_error("not ulong"); }
    virtual long long to_llong() const { throw std::logic_error("not llong"); }
    virtual unsigned long long to_ullong() const { throw std::logic_error("not ullong"); }
    virtual float to_float() const { throw std::logic_error("not float"); }
    virtual double to_double() const { throw std::logic_error("not double"); }
    virtual std::string to_string() const { throw std::logic_error("not string"); }
    virtual std::vector<uint8_t> to_bytes() const { throw std::logic_error("not bytes"); }
    // ... and more

    // Arithmetic operations
    virtual value& add(const value& other) { throw std::logic_error("not supported"); }
    virtual value& subtract(const value& other) { throw std::logic_error("not supported"); }
    virtual value& multiply(const value& other) { throw std::logic_error("not supported"); }
    virtual value& divide(const value& other) { throw std::logic_error("not supported"); }
};
```

### Problems with This Approach

1. **Interface Bloat**: 20+ methods, most of which throw exceptions
2. **Forced Implementation**: Every subclass must override all methods (even to throw)
3. **Runtime Errors**: Type mismatches discovered at runtime, not compile-time
4. **Poor Discoverability**: Hard to tell which methods are valid for which types
5. **Testing Burden**: Must test all methods for all subclasses

### Example: string_value Implementation

```cpp
// ❌ DEPRECATED: string_value forced to implement all methods
class string_value : public value {
public:
    std::string to_string() const override {
        return data_;  // Only this method is meaningful
    }

    // All these just throw (wasted code)
    int to_int() const override {
        throw std::logic_error("Cannot convert string to int");
    }
    long to_long() const override {
        throw std::logic_error("Cannot convert string to long");
    }
    double to_double() const override {
        throw std::logic_error("Cannot convert string to double");
    }
    // ... 15+ more throwing methods
};
```

---

## Solution: Interface Segregation with variant_value_v2

The modern `variant_value_v2` class uses `std::variant` and the Visitor pattern to achieve proper interface segregation.

### Design Principles

1. **No Fat Interface**: No type-specific methods on the base type
2. **Type-Safe Access**: Use `std::visit` for type-specific operations
3. **Compile-Time Checking**: Type mismatches caught at compile time
4. **Single Responsibility**: Each visitor handles one specific concern

### The Solution

```cpp
// ✅ RECOMMENDED: Clean interface with variant_value_v2
class variant_value_v2 {
public:
    // Core interface (small, focused)
    value_types type() const noexcept;
    const std::string& name() const noexcept;

    // Generic access to underlying variant
    const ValueVariant& get_data() const noexcept;

    // Serialization (applies to all types)
    std::vector<uint8_t> serialize() const;
    static std::optional<variant_value_v2> deserialize(const std::vector<uint8_t>& data);

    // NO type-specific conversion methods!
    // Use std::visit instead
};
```

### Type-Specific Operations via Visitors

Instead of having `to_int()`, `to_double()`, etc., we use focused visitor functions:

```cpp
// Focused conversion functions (not class methods)
namespace converters {
    // Each function has a single, clear purpose

    int to_int(const variant_value_v2& value) {
        return std::visit([](const auto& val) -> int {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_integral_v<T>) {
                return static_cast<int>(val);
            } else {
                throw std::invalid_argument("Cannot convert to int");
            }
        }, value.get_data());
    }

    double to_double(const variant_value_v2& value) {
        return std::visit([](const auto& val) -> double {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_arithmetic_v<T>) {
                return static_cast<double>(val);
            } else {
                throw std::invalid_argument("Cannot convert to double");
            }
        }, value.get_data());
    }

    std::string to_string(const variant_value_v2& value) {
        return std::visit([](const auto& val) -> std::string {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, std::string>) {
                return val;
            } else if constexpr (std::is_arithmetic_v<T>) {
                return std::to_string(val);
            } else {
                throw std::invalid_argument("Cannot convert to string");
            }
        }, value.get_data());
    }
}
```

---

## Interface Segregation in Practice

### Guideline 1: Keep Interfaces Minimal

**Bad** (Fat Interface):
```cpp
// ❌ Interface does too much
class IValueOperations {
public:
    virtual int to_int() const = 0;
    virtual double to_double() const = 0;
    virtual std::string to_string() const = 0;
    virtual void serialize(std::ostream& out) const = 0;
    virtual void validate() const = 0;
    virtual void log() const = 0;
    virtual void notify_observers() const = 0;
    // ... 20 more methods
};
```

**Good** (Segregated Interfaces):
```cpp
// ✅ Split into focused interfaces
class ISerializable {
public:
    virtual void serialize(std::ostream& out) const = 0;
};

class IValidatable {
public:
    virtual bool validate(std::string& error_message) const = 0;
};

class ILoggable {
public:
    virtual std::string to_log_string() const = 0;
};

// Use only what you need
class SimpleValue : public ISerializable {
    // Implements only serialize, not forced to implement logging/validation
};
```

### Guideline 2: Role-Based Interfaces

Separate interfaces by role/concern:

```cpp
// Separate concerns
class IReadable {
public:
    virtual variant_value_v2 read_value(const std::string& key) const = 0;
};

class IWritable {
public:
    virtual void write_value(const std::string& key, variant_value_v2 value) = 0;
};

class IQueryable {
public:
    virtual std::vector<std::string> list_keys() const = 0;
    virtual bool contains(const std::string& key) const = 0;
};

// Combine as needed
class value_container : public IReadable, public IWritable, public IQueryable {
    // Implementation
};

// Or use only one
class ReadOnlyContainer : public IReadable {
    // Read-only implementation
};
```

### Guideline 3: Free Functions Over Member Functions

When operations don't need access to private state, use free functions:

```cpp
// ✅ Free functions (better for ISP)
namespace value_operations {
    bool is_numeric(const variant_value_v2& value);
    bool is_string(const variant_value_v2& value);
    double to_double(const variant_value_v2& value);
    std::string to_json(const variant_value_v2& value);
}

// Usage - clear and focused
if (value_operations::is_numeric(val)) {
    double d = value_operations::to_double(val);
}
```

This is better than:
```cpp
// ❌ Member functions bloat the interface
class variant_value_v2 {
public:
    bool is_numeric() const;
    bool is_string() const;
    double to_double() const;
    std::string to_json() const;
    // ... interface keeps growing
};
```

---

## Migration Guide

### Migrating from Fat Interface

**Step 1**: Identify which methods you actually use
```cpp
// Old code
auto val = container->get_value("count");
int count = val->to_int();  // Only using to_int()
```

**Step 2**: Use appropriate visitor or helper function
```cpp
// New code
auto val = container->get_variant_value("count");
int count = std::visit([](const auto& v) -> int {
    if constexpr (std::is_integral_v<std::decay_t<decltype(v)>>) {
        return static_cast<int>(v);
    }
    throw std::invalid_argument("Not an integer");
}, val.get_data());
```

**Step 3**: Or use helper functions
```cpp
// Even simpler with helpers
auto val = container->get_variant_value("count");
int count = value_operations::to_int(val);
```

---

## Benefits Achieved

| Aspect | Fat Interface (Old) | Segregated (New) |
|--------|---------------------|------------------|
| **Methods per class** | 20+ (most throwing) | 5-7 (all functional) |
| **Compile-time safety** | ❌ No | ✅ Yes |
| **Code bloat** | High (every subclass implements all) | Low (visitors only as needed) |
| **Testing** | Test all methods for all types | Test only used combinations |
| **Extensibility** | Add method to base → change all | Add new visitor function |
| **Client coupling** | High (depends on full interface) | Low (depends on specific visitor) |

---

## Best Practices

### DO:
✅ Keep interfaces focused on a single responsibility
✅ Use free functions for operations that don't need private access
✅ Separate read and write interfaces
✅ Use visitors for type-specific operations
✅ Provide helper functions for common patterns

### DON'T:
❌ Add methods to interfaces "just in case"
❌ Force clients to implement methods they don't use
❌ Mix unrelated concerns in one interface
❌ Use runtime type checking when compile-time is possible
❌ Let interfaces grow unbounded

---

## Examples in container_system

### Good: value_store Interface

```cpp
class value_store {
public:
    // Focused interface for storage operations only
    virtual void add(const std::string& key, variant_value_v2 value);
    virtual std::optional<variant_value_v2> get(const std::string& key) const;
    virtual bool contains(const std::string& key) const;
    virtual bool remove(const std::string& key);
    virtual void clear();

    // Serialization (separate concern, but related)
    virtual std::string serialize() const;
    static value_store deserialize(std::string_view json_data);
};
```

### Good: message_container Composition

```cpp
class message_container {
    // Uses composition instead of inheriting fat interface
    value_store payload_;  // Focused storage interface

    // Message-specific concerns
    std::string source_id_;
    std::string target_id_;
    std::string message_type_;

public:
    // Exposes only message-related operations
    // Delegates storage to value_store
    value_store& payload() { return payload_; }
};
```

---

## Testing Interface Segregation

```cpp
// Easy to test focused interfaces
TEST(InterfaceSegregation, OnlyTestWhatMatters) {
    // Mock only what you need
    class MockReadable : public IReadable {
    public:
        MOCK_METHOD(variant_value_v2, read_value, (const std::string&), (const, override));
    };

    MockReadable mock;
    EXPECT_CALL(mock, read_value("key"))
        .WillOnce(Return(variant_value_v2("key", value_types::int_value, 42)));

    // No need to mock 20+ unrelated methods!
}
```

---

## Related Documentation

- [VISITOR_PATTERN_GUIDE.md](VISITOR_PATTERN_GUIDE.md) - How to use visitors with variant_value_v2
- [EXCEPTION_SAFETY.md](EXCEPTION_SAFETY.md) - Exception safety guarantees
- [VARIANT_VALUE_V2_MIGRATION_GUIDE.md](VARIANT_VALUE_V2_MIGRATION_GUIDE.md) - Migration from legacy types

---

## References

- [SOLID Principles](https://en.wikipedia.org/wiki/SOLID)
- [Interface Segregation Principle](https://en.wikipedia.org/wiki/Interface_segregation_principle)
- [C++ Core Guidelines: I.4](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i4-make-interfaces-precisely-and-strongly-typed)
- [C++ Core Guidelines: C.5](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c5-place-helper-functions-in-the-same-namespace-as-the-class-they-support)

---

**Last Updated**: 2025-11-09
**Review Cycle**: Quarterly
**Status**: Active - All new code should follow these guidelines
