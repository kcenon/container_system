# Visitor Pattern with variant_value_v2

**Date**: 2025-11-09
**Status**: Active

## Overview

The `variant_value_v2` class uses `std::variant` internally, which provides excellent support for the Visitor pattern through `std::visit`. This document demonstrates how to use the Visitor pattern with variant_value_v2 to avoid the fat interface problem.

## Problem: Fat Interface (Legacy value class)

The legacy polymorphic `value` class suffers from Interface Segregation Principle (ISP) violations:

```cpp
// ❌ Anti-pattern: Fat Interface (deprecated)
class value {
public:
    // 20+ conversion methods, most throw std::logic_error
    virtual int to_int() const;
    virtual long to_long() const;
    virtual double to_double() const;
    virtual std::string to_string() const;
    virtual std::vector<uint8_t> to_bytes() const;
    // ... 15+ more methods

    // Most implementations just throw
    int to_int() const override {
        throw std::logic_error("Cannot convert string_value to int");
    }
};
```

**Problems**:
- Every subclass must implement or throw for all methods
- Interface bloat makes code hard to understand
- Runtime errors instead of compile-time safety
- Violates ISP (clients forced to depend on methods they don't use)

---

## Solution: Visitor Pattern with std::visit

`variant_value_v2` uses `std::variant` and provides compile-time type safety through the Visitor pattern.

### Basic Visitor Pattern

```cpp
#include <container/internal/variant_value_v2.h>
#include <variant>
#include <iostream>

using namespace container_module;

// Example 1: Simple type-based visitor
void print_value(const variant_value_v2& value) {
    std::visit([](const auto& val) {
        using T = std::decay_t<decltype(val)>;

        if constexpr (std::is_same_v<T, std::monostate>) {
            std::cout << "null" << std::endl;
        } else if constexpr (std::is_same_v<T, bool>) {
            std::cout << std::boolalpha << val << std::endl;
        } else if constexpr (std::is_arithmetic_v<T>) {
            std::cout << val << std::endl;
        } else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "\"" << val << "\"" << std::endl;
        } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
            std::cout << "[" << val.size() << " bytes]" << std::endl;
        } else {
            std::cout << "<unknown type>" << std::endl;
        }
    }, value.get_data());
}
```

### Structured Visitor Class

For complex operations, create a visitor functor:

```cpp
// Example 2: Visitor functor for JSON serialization
class JsonSerializer {
public:
    std::string operator()(std::monostate) const {
        return "null";
    }

    std::string operator()(bool val) const {
        return val ? "true" : "false";
    }

    std::string operator()(int16_t val) const { return std::to_string(val); }
    std::string operator()(uint16_t val) const { return std::to_string(val); }
    std::string operator()(int32_t val) const { return std::to_string(val); }
    std::string operator()(uint32_t val) const { return std::to_string(val); }
    std::string operator()(int64_t val) const { return std::to_string(val); }
    std::string operator()(uint64_t val) const { return std::to_string(val); }
    std::string operator()(float val) const { return std::to_string(val); }
    std::string operator()(double val) const { return std::to_string(val); }

    std::string operator()(const std::string& val) const {
        return "\"" + escape_json(val) + "\"";
    }

    std::string operator()(const std::vector<uint8_t>& val) const {
        // Base64 encode or hex string
        return "\"base64:" + base64_encode(val) + "\"";
    }

    std::string operator()(const std::shared_ptr<value_container>& container) const {
        // Recursively serialize container
        return container ? container->to_json() : "null";
    }

private:
    std::string escape_json(const std::string& str) const {
        // JSON escaping implementation
        std::string result;
        for (char c : str) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c;
            }
        }
        return result;
    }

    std::string base64_encode(const std::vector<uint8_t>& data) const {
        // Base64 encoding implementation (simplified)
        return "<base64>";
    }
};

// Usage
variant_value_v2 value("test", value_types::string_value, "Hello, World!");
std::string json = std::visit(JsonSerializer{}, value.get_data());
```

### Generic Visitor with Overload Pattern

C++17's overload pattern makes visitors cleaner:

```cpp
// Overload pattern helper
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

// Example 3: Type-safe arithmetic operations
double to_double(const variant_value_v2& value) {
    return std::visit(overload{
        [](std::monostate) -> double { return 0.0; },
        [](bool val) -> double { return val ? 1.0 : 0.0; },
        [](auto val) -> double {
            if constexpr (std::is_arithmetic_v<decltype(val)>) {
                return static_cast<double>(val);
            } else {
                throw std::invalid_argument("Cannot convert to double");
            }
        }
    }, value.get_data());
}

// Example 4: Conditional operations
bool is_numeric(const variant_value_v2& value) {
    return std::visit(overload{
        [](std::monostate) { return false; },
        [](bool) { return false; },
        [](const std::string&) { return false; },
        [](const std::vector<uint8_t>&) { return false; },
        [](const std::shared_ptr<value_container>&) { return false; },
        [](auto) { return true; }  // All numeric types
    }, value.get_data());
}
```

---

## Advanced Patterns

### Pattern 1: Type-Safe Comparisons

```cpp
bool variant_value_equal(const variant_value_v2& lhs, const variant_value_v2& rhs) {
    if (lhs.type() != rhs.type()) {
        return false;
    }

    return std::visit([&rhs](const auto& lhs_val) {
        return std::visit([&lhs_val](const auto& rhs_val) -> bool {
            using LhsT = std::decay_t<decltype(lhs_val)>;
            using RhsT = std::decay_t<decltype(rhs_val)>;

            if constexpr (std::is_same_v<LhsT, RhsT>) {
                if constexpr (std::is_same_v<LhsT, std::shared_ptr<value_container>>) {
                    // Deep container comparison
                    if (!lhs_val || !rhs_val) return lhs_val == rhs_val;
                    return lhs_val->serialize() == rhs_val->serialize();
                } else {
                    return lhs_val == rhs_val;
                }
            } else {
                return false;
            }
        }, rhs.get_data());
    }, lhs.get_data());
}
```

### Pattern 2: Validation and Transformation

```cpp
// Visitor for data validation
class ValueValidator {
public:
    struct ValidationResult {
        bool valid;
        std::string error_message;
    };

    ValidationResult operator()(std::monostate) const {
        return {true, ""};
    }

    ValidationResult operator()(const std::string& val) const {
        if (val.empty()) {
            return {false, "String cannot be empty"};
        }
        if (val.size() > max_string_length_) {
            return {false, "String exceeds maximum length"};
        }
        return {true, ""};
    }

    ValidationResult operator()(const std::vector<uint8_t>& val) const {
        if (val.size() > max_bytes_length_) {
            return {false, "Byte array exceeds maximum length"};
        }
        return {true, ""};
    }

    template<typename T>
    ValidationResult operator()(T val) const {
        if constexpr (std::is_arithmetic_v<T>) {
            if (std::isnan(static_cast<double>(val)) ||
                std::isinf(static_cast<double>(val))) {
                return {false, "Invalid numeric value (NaN or Inf)"};
            }
        }
        return {true, ""};
    }

private:
    size_t max_string_length_ = 10000;
    size_t max_bytes_length_ = 1000000;
};

// Usage
auto result = std::visit(ValueValidator{}, value.get_data());
if (!result.valid) {
    std::cerr << "Validation failed: " << result.error_message << std::endl;
}
```

### Pattern 3: Exhaustive Switch with Compile-Time Checking

```cpp
// Helper to ensure all types are handled (compile-time check)
template<typename Variant>
auto visit_variant(Variant&& var) {
    return std::visit(overload{
        [](std::monostate) -> std::string { return "null"; },
        [](bool val) -> std::string { return val ? "true" : "false"; },
        [](int16_t val) -> std::string { return "short:" + std::to_string(val); },
        [](uint16_t val) -> std::string { return "ushort:" + std::to_string(val); },
        [](int32_t val) -> std::string { return "int:" + std::to_string(val); },
        [](uint32_t val) -> std::string { return "uint:" + std::to_string(val); },
        [](int64_t val) -> std::string { return "long:" + std::to_string(val); },
        [](uint64_t val) -> std::string { return "ulong:" + std::to_string(val); },
        [](float val) -> std::string { return "float:" + std::to_string(val); },
        [](double val) -> std::string { return "double:" + std::to_string(val); },
        [](const std::string& val) -> std::string { return "string:" + val; },
        [](const std::vector<uint8_t>& val) -> std::string {
            return "bytes:" + std::to_string(val.size());
        },
        [](const std::shared_ptr<value_container>& val) -> std::string {
            return val ? "container" : "null_container";
        }
    }, std::forward<Variant>(var));
}

// If you forget a type, you get a compile error!
```

---

## Benefits of Visitor Pattern

| Feature | Fat Interface (Old) | Visitor Pattern (New) |
|---------|---------------------|----------------------|
| **Type Safety** | Runtime errors | Compile-time checking |
| **Performance** | Virtual dispatch + exceptions | Zero-cost abstraction |
| **Extensibility** | Add method to base class | Add new visitor |
| **Code Clarity** | Methods scattered across hierarchy | Visitor concentrates logic |
| **ISP Compliance** | ❌ Violates ISP | ✅ Adheres to ISP |
| **Error Handling** | Exceptions | Compile errors or Result<T> |

---

## Migration from Fat Interface

### Before (Legacy value class)
```cpp
// ❌ Runtime error, fat interface
auto value = container->get_value("count");
try {
    int count = value->to_int();
    process(count);
} catch (const std::logic_error& e) {
    std::cerr << "Not an int: " << e.what() << std::endl;
}
```

### After (variant_value_v2)
```cpp
// ✅ Compile-time safety, clean visitor
auto value_v2 = container->get_variant_value("count");

std::visit(overload{
    [](int32_t count) { process(count); },
    [](int64_t count) { process(static_cast<int>(count)); },
    [](auto) { std::cerr << "Expected numeric value" << std::endl; }
}, value_v2.get_data());
```

Or with helper function:
```cpp
// Helper for common pattern
template<typename T, typename Func>
void if_type(const variant_value_v2& value, Func&& func) {
    std::visit([&](const auto& val) {
        if constexpr (std::is_same_v<std::decay_t<decltype(val)>, T>) {
            func(val);
        }
    }, value.get_data());
}

// Usage
if_type<int32_t>(value_v2, [](int32_t count) {
    process(count);
});
```

---

## Best Practices

### 1. Use Structured Visitors for Complex Logic
Create visitor classes instead of lambdas when logic is non-trivial.

### 2. Leverage `if constexpr` for Type Dispatching
```cpp
std::visit([](const auto& val) {
    using T = std::decay_t<decltype(val)>;
    if constexpr (std::is_integral_v<T>) {
        // Handle all integer types together
    } else if constexpr (std::is_floating_point_v<T>) {
        // Handle all floating-point types
    } else {
        // Handle other types
    }
}, value.get_data());
```

### 3. Return Result<T> from Visitors
Instead of throwing exceptions:
```cpp
Result<int> safe_to_int(const variant_value_v2& value) {
    return std::visit(overload{
        [](int32_t val) -> Result<int> { return ok(val); },
        [](int64_t val) -> Result<int> {
            if (val > INT_MAX || val < INT_MIN) {
                return error("Integer overflow");
            }
            return ok(static_cast<int>(val));
        },
        [](auto) -> Result<int> {
            return error("Cannot convert to int");
        }
    }, value.get_data());
}
```

### 4. Avoid Nested std::visit When Possible
Instead of nested visits, extract helper functions.

### 5. Document Visitor Contracts
Clearly document what each visitor returns and any side effects.

---

## Testing Visitors

```cpp
#include <gtest/gtest.h>

TEST(VisitorPattern, ToDoubleConversion) {
    variant_value_v2 int_val("test", value_types::int_value, 42);
    variant_value_v2 float_val("test", value_types::float_value, 3.14f);
    variant_value_v2 string_val("test", value_types::string_value, "hello");

    EXPECT_DOUBLE_EQ(to_double(int_val), 42.0);
    EXPECT_DOUBLE_EQ(to_double(float_val), 3.14);
    EXPECT_THROW(to_double(string_val), std::invalid_argument);
}

TEST(VisitorPattern, TypeChecking) {
    variant_value_v2 numeric("num", value_types::int_value, 100);
    variant_value_v2 text("text", value_types::string_value, "test");

    EXPECT_TRUE(is_numeric(numeric));
    EXPECT_FALSE(is_numeric(text));
}
```

---

## References

- [C++ Core Guidelines: C.152](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c152-never-assign-a-pointer-to-an-array-of-derived-class-objects-to-a-pointer-to-its-base)
- [std::variant documentation](https://en.cppreference.com/w/cpp/utility/variant)
- [std::visit documentation](https://en.cppreference.com/w/cpp/utility/variant/visit)
- [Visitor Pattern (Gang of Four)](https://refactoring.guru/design-patterns/visitor)

---

**Last Updated**: 2025-11-09
**Review Cycle**: Quarterly
