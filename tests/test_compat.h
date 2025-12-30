// BSD 3-Clause License
// Copyright (c) 2021-2025

/**
 * @file test_compat.h
 * @brief Value API helper functions for tests
 *
 * This header provides factory functions and helper utilities for working
 * with the value class in tests. The legacy_value wrapper class has been
 * removed - tests now use the value class directly with free functions
 * for type checking and conversion (is_boolean(), to_int(), etc.).
 */

#pragma once

#include <container.h>
#include <container/internal/value.h>
#include <container/internal/thread_safe_container.h>
#include <memory>
#include <string>
#include <vector>
#include <limits>
#include <cstdint>

namespace container_module {
namespace test_compat {

// Factory functions that return shared_ptr for legacy API compatibility
inline std::shared_ptr<value> make_int_value(std::string_view name, int32_t val) {
    return std::make_shared<value>(name, val);
}

inline std::shared_ptr<value> make_bool_value(std::string_view name, bool val) {
    return std::make_shared<value>(name, val);
}

inline std::shared_ptr<value> make_string_value(std::string_view name, std::string val) {
    return std::make_shared<value>(name, std::move(val));
}

inline std::shared_ptr<value> make_llong_value(std::string_view name, int64_t val) {
    return std::make_shared<value>(name, val);
}

inline std::shared_ptr<value> make_long_value(std::string_view name, int64_t val) {
    return std::make_shared<value>(name, val);
}

inline std::shared_ptr<value> make_ulong_value(std::string_view name, uint64_t val) {
    return std::make_shared<value>(name, val);
}

inline std::shared_ptr<value> make_bytes_value(std::string_view name, std::vector<uint8_t> val) {
    return std::make_shared<value>(name, std::move(val));
}

inline std::shared_ptr<value> make_double_value(std::string_view name, double val) {
    return std::make_shared<value>(name, val);
}

inline std::shared_ptr<value> make_float_value(std::string_view name, float val) {
    return std::make_shared<value>(name, val);
}

inline std::shared_ptr<value> make_short_value(std::string_view name, int16_t val) {
    return std::make_shared<value>(name, val);
}

inline std::shared_ptr<value> make_ushort_value(std::string_view name, uint16_t val) {
    return std::make_shared<value>(name, val);
}

inline std::shared_ptr<value> make_uint_value(std::string_view name, uint32_t val) {
    return std::make_shared<value>(name, val);
}

inline std::shared_ptr<value> make_ullong_value(std::string_view name, uint64_t val) {
    return std::make_shared<value>(name, val);
}

// Type checking helper functions for value API compatibility
inline bool is_boolean(const value& v) {
    return v.type() == value_types::bool_value;
}

inline bool is_numeric(const value& v) {
    auto t = v.type();
    return t == value_types::short_value || t == value_types::ushort_value ||
           t == value_types::int_value || t == value_types::uint_value ||
           t == value_types::long_value || t == value_types::ulong_value ||
           t == value_types::llong_value || t == value_types::ullong_value ||
           t == value_types::float_value || t == value_types::double_value;
}

inline bool is_string(const value& v) {
    return v.type() == value_types::string_value;
}

inline bool is_container(const value& v) {
    return v.type() == value_types::container_value;
}

// Type conversion helper functions
inline bool to_boolean(const value& v) {
    return v.visit([](const auto& data) -> bool {
        using T = std::decay_t<decltype(data)>;
        if constexpr (std::is_same_v<T, bool>) {
            return data;
        } else if constexpr (std::is_arithmetic_v<T>) {
            return data != T{};
        } else if constexpr (std::is_same_v<T, std::string>) {
            return data == "true" || data == "1" || data == "yes";
        } else {
            return false;
        }
    });
}

inline int32_t to_int(const value& v) {
    return v.visit([](const auto& data) -> int32_t {
        using T = std::decay_t<decltype(data)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<int32_t>(data);
        } else if constexpr (std::is_same_v<T, std::string>) {
            try { return std::stoi(data); } catch (...) { return 0; }
        } else {
            return 0;
        }
    });
}

inline int64_t to_long(const value& v) {
    return v.visit([](const auto& data) -> int64_t {
        using T = std::decay_t<decltype(data)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<int64_t>(data);
        } else if constexpr (std::is_same_v<T, std::string>) {
            try { return std::stoll(data); } catch (...) { return 0; }
        } else {
            return 0;
        }
    });
}

inline int64_t to_llong(const value& v) {
    return to_long(v);
}

inline uint64_t to_ulong(const value& v) {
    return v.visit([](const auto& data) -> uint64_t {
        using T = std::decay_t<decltype(data)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<uint64_t>(data);
        } else if constexpr (std::is_same_v<T, std::string>) {
            try { return std::stoull(data); } catch (...) { return 0; }
        } else {
            return 0;
        }
    });
}

inline uint64_t to_ullong(const value& v) {
    return to_ulong(v);
}

inline double to_double(const value& v) {
    return v.visit([](const auto& data) -> double {
        using T = std::decay_t<decltype(data)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<double>(data);
        } else if constexpr (std::is_same_v<T, std::string>) {
            try { return std::stod(data); } catch (...) { return 0.0; }
        } else {
            return 0.0;
        }
    });
}

inline size_t value_size(const value& v) {
    return v.visit([](const auto& data) -> size_t {
        using T = std::decay_t<decltype(data)>;
        if constexpr (std::is_same_v<T, std::string>) {
            return data.size();
        } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
            return data.size();
        } else if constexpr (std::is_same_v<T, std::monostate>) {
            return 0;
        } else {
            return sizeof(T);
        }
    });
}

inline bool is_bytes(const value& v) {
    return v.type() == value_types::bytes_value;
}

inline std::vector<uint8_t> to_bytes(const value& v) {
    return v.visit([](const auto& data) -> std::vector<uint8_t> {
        using T = std::decay_t<decltype(data)>;
        if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
            return data;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return std::vector<uint8_t>(data.begin(), data.end());
        } else {
            return {};
        }
    });
}

// Helper functions for range checking
inline bool is_int32_range(int64_t val) {
    return val >= static_cast<int64_t>(std::numeric_limits<int32_t>::min()) &&
           val <= static_cast<int64_t>(std::numeric_limits<int32_t>::max());
}

inline bool is_uint32_range(uint64_t val) {
    return val <= static_cast<uint64_t>(std::numeric_limits<uint32_t>::max());
}

// Helper functions for optimized_value (from container->get_value())
inline std::string ov_to_string(const std::optional<optimized_value>& ov) {
    if (!ov) return "";
    if (std::holds_alternative<std::string>(ov->data)) {
        return std::get<std::string>(ov->data);
    }
    return variant_helpers::to_string(ov->data, ov->type);
}

inline int32_t ov_to_int(const std::optional<optimized_value>& ov) {
    if (!ov) return 0;
    return std::visit([](const auto& v) -> int32_t {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, std::monostate>) {
            return static_cast<int32_t>(v);
        } else if constexpr (std::is_same_v<T, std::string>) {
            try { return std::stoi(v); } catch (...) { return 0; }
        } else {
            return 0;
        }
    }, ov->data);
}

inline bool ov_to_boolean(const std::optional<optimized_value>& ov) {
    if (!ov) return false;
    return std::visit([](const auto& v) -> bool {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, bool>) {
            return v;
        } else if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, std::monostate>) {
            return v != T{};
        } else if constexpr (std::is_same_v<T, std::string>) {
            return v == "true" || v == "1" || v == "yes";
        } else {
            return false;
        }
    }, ov->data);
}

inline bool ov_is_null(const std::optional<optimized_value>& ov) {
    if (!ov) return true;
    return ov->type == value_types::null_value;
}

inline bool ov_is_container(const std::optional<optimized_value>& ov) {
    if (!ov) return false;
    return ov->type == value_types::container_value;
}

inline std::string ov_data(const std::optional<optimized_value>& ov) {
    if (!ov) return "";
    // Return string representation that can be used as container data
    return variant_helpers::to_string(ov->data, ov->type);
}

inline bool ov_is_bytes(const std::optional<optimized_value>& ov) {
    if (!ov) return false;
    return ov->type == value_types::bytes_value;
}

inline int64_t ov_to_llong(const std::optional<optimized_value>& ov) {
    if (!ov) return 0;
    return std::visit([](const auto& v) -> int64_t {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, std::monostate>) {
            return static_cast<int64_t>(v);
        } else if constexpr (std::is_same_v<T, std::string>) {
            try { return std::stoll(v); } catch (...) { return 0; }
        } else {
            return 0;
        }
    }, ov->data);
}

inline double ov_to_double(const std::optional<optimized_value>& ov) {
    if (!ov) return 0.0;
    return std::visit([](const auto& v) -> double {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, std::monostate>) {
            return static_cast<double>(v);
        } else if constexpr (std::is_same_v<T, std::string>) {
            try { return std::stod(v); } catch (...) { return 0.0; }
        } else {
            return 0.0;
        }
    }, ov->data);
}

inline std::string ov_name(const std::optional<optimized_value>& ov) {
    if (!ov) return "";
    return ov->name;
}

} // namespace test_compat
} // namespace container_module

// Pull test_compat functions into global scope for test convenience
using container_module::test_compat::make_int_value;
using container_module::test_compat::make_bool_value;
using container_module::test_compat::make_string_value;
using container_module::test_compat::make_llong_value;
using container_module::test_compat::make_long_value;
using container_module::test_compat::make_ulong_value;
using container_module::test_compat::make_bytes_value;
using container_module::test_compat::make_double_value;
using container_module::test_compat::make_float_value;
using container_module::test_compat::make_short_value;
using container_module::test_compat::make_ushort_value;
using container_module::test_compat::make_uint_value;
using container_module::test_compat::make_ullong_value;

// Pull helper functions into global scope
using container_module::test_compat::is_boolean;
using container_module::test_compat::is_numeric;
using container_module::test_compat::is_string;
using container_module::test_compat::is_container;
using container_module::test_compat::is_bytes;
using container_module::test_compat::to_boolean;
using container_module::test_compat::to_int;
using container_module::test_compat::to_long;
using container_module::test_compat::to_llong;
using container_module::test_compat::to_ulong;
using container_module::test_compat::to_ullong;
using container_module::test_compat::to_double;
using container_module::test_compat::to_bytes;
using container_module::test_compat::value_size;
using container_module::test_compat::ov_to_string;
using container_module::test_compat::ov_to_int;
using container_module::test_compat::ov_to_boolean;
using container_module::test_compat::ov_to_llong;
using container_module::test_compat::ov_to_double;
using container_module::test_compat::ov_is_null;
using container_module::test_compat::ov_is_bytes;
using container_module::test_compat::ov_is_container;
using container_module::test_compat::ov_data;
using container_module::test_compat::ov_name;
using container_module::test_compat::is_int32_range;
using container_module::test_compat::is_uint32_range;
