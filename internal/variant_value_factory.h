/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#pragma once

#include "container/internal/variant_value_v2.h"
#include "container/core/value_types.h"
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <type_traits>

namespace container_module
{
    /**
     * @brief Factory functions for creating variant_value_v2 instances
     *
     * These functions provide a convenient, type-safe way to create variant values
     * without needing to specify template parameters or handle type conversions manually.
     *
     * Usage examples:
     * ```cpp
     * auto v1 = make_bool_value("enabled", true);
     * auto v2 = make_int_value("count", 42);
     * auto v3 = make_string_value("name", "John");
     * auto v4 = make_array_value("items", {v1, v2, v3});
     * ```
     */

    // ============================================================================
    // Null value
    // ============================================================================

    /**
     * @brief Create a null value
     */
    inline variant_value_v2 make_null_value(std::string_view name = "") {
        return variant_value_v2(name);
    }

    // ============================================================================
    // Boolean value
    // ============================================================================

    /**
     * @brief Create a boolean value
     */
    inline variant_value_v2 make_bool_value(std::string_view name, bool value) {
        return variant_value_v2(name, value);
    }

    // ============================================================================
    // Numeric values
    // ============================================================================

    /**
     * @brief Create a short (int16_t) value
     */
    inline variant_value_v2 make_short_value(std::string_view name, int16_t value) {
        return variant_value_v2(name, value);
    }

    /**
     * @brief Create an unsigned short (uint16_t) value
     */
    inline variant_value_v2 make_ushort_value(std::string_view name, uint16_t value) {
        return variant_value_v2(name, value);
    }

    /**
     * @brief Create an int (int32_t) value
     */
    inline variant_value_v2 make_int_value(std::string_view name, int32_t value) {
        return variant_value_v2(name, value);
    }

    /**
     * @brief Create an unsigned int (uint32_t) value
     */
    inline variant_value_v2 make_uint_value(std::string_view name, uint32_t value) {
        return variant_value_v2(name, value);
    }

    /**
     * @brief Create a long (int64_t) value
     */
    inline variant_value_v2 make_long_value(std::string_view name, int64_t value) {
        return variant_value_v2(name, value);
    }

    /**
     * @brief Create an unsigned long (uint64_t) value
     */
    inline variant_value_v2 make_ulong_value(std::string_view name, uint64_t value) {
        return variant_value_v2(name, value);
    }

    /**
     * @brief Create a float value
     */
    inline variant_value_v2 make_float_value(std::string_view name, float value) {
        return variant_value_v2(name, value);
    }

    /**
     * @brief Create a double value
     */
    inline variant_value_v2 make_double_value(std::string_view name, double value) {
        return variant_value_v2(name, value);
    }

    /**
     * @brief Generic numeric value factory with automatic type deduction
     *
     * Automatically selects the appropriate numeric type based on T:
     * - bool → bool_value
     * - int16_t, short → short_value
     * - uint16_t, unsigned short → ushort_value
     * - int32_t, int → int_value
     * - uint32_t, unsigned int → uint_value
     * - int64_t, long, long long → long_value
     * - uint64_t, unsigned long, unsigned long long → ulong_value
     * - float → float_value
     * - double → double_value
     *
     * @tparam T Numeric type (must be arithmetic)
     * @param name Value name
     * @param value Numeric value
     * @return variant_value_v2 with appropriate type
     */
    template<typename T>
    inline variant_value_v2 make_numeric_value(std::string_view name, T value) {
        static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");

        if constexpr (std::is_same_v<T, bool>) {
            return make_bool_value(name, value);
        } else if constexpr (std::is_same_v<T, int16_t> || std::is_same_v<T, short>) {
            return make_short_value(name, static_cast<int16_t>(value));
        } else if constexpr (std::is_same_v<T, uint16_t> || std::is_same_v<T, unsigned short>) {
            return make_ushort_value(name, static_cast<uint16_t>(value));
        } else if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, int>) {
            return make_int_value(name, static_cast<int32_t>(value));
        } else if constexpr (std::is_same_v<T, uint32_t> || std::is_same_v<T, unsigned int>) {
            return make_uint_value(name, static_cast<uint32_t>(value));
        } else if constexpr (std::is_same_v<T, int64_t> ||
                           std::is_same_v<T, long> ||
                           std::is_same_v<T, long long>) {
            return make_long_value(name, static_cast<int64_t>(value));
        } else if constexpr (std::is_same_v<T, uint64_t> ||
                           std::is_same_v<T, unsigned long> ||
                           std::is_same_v<T, unsigned long long>) {
            return make_ulong_value(name, static_cast<uint64_t>(value));
        } else if constexpr (std::is_same_v<T, float>) {
            return make_float_value(name, value);
        } else if constexpr (std::is_same_v<T, double>) {
            return make_double_value(name, value);
        } else {
            // Fallback: use the closest integer type
            if constexpr (sizeof(T) <= sizeof(int32_t)) {
                return make_int_value(name, static_cast<int32_t>(value));
            } else {
                return make_long_value(name, static_cast<int64_t>(value));
            }
        }
    }

    // ============================================================================
    // String values
    // ============================================================================

    /**
     * @brief Create a string value
     */
    inline variant_value_v2 make_string_value(std::string_view name, std::string value) {
        return variant_value_v2(name, std::move(value));
    }

    /**
     * @brief Create a string value from string_view
     */
    inline variant_value_v2 make_string_value(std::string_view name, std::string_view value) {
        return variant_value_v2(name, std::string(value));
    }

    /**
     * @brief Create a string value from C-string
     */
    inline variant_value_v2 make_string_value(std::string_view name, const char* value) {
        return variant_value_v2(name, std::string(value));
    }

    // ============================================================================
    // Bytes values
    // ============================================================================

    /**
     * @brief Create a bytes value
     */
    inline variant_value_v2 make_bytes_value(std::string_view name, std::vector<uint8_t> data) {
        return variant_value_v2(name, std::move(data));
    }

    /**
     * @brief Create a bytes value from raw data
     */
    inline variant_value_v2 make_bytes_value(std::string_view name,
                                             const uint8_t* data,
                                             size_t size) {
        return variant_value_v2(name, std::vector<uint8_t>(data, data + size));
    }

    /**
     * @brief Create a bytes value from string (copy bytes)
     * Note: This treats the string as binary data, not as a null-terminated C string
     */
    inline variant_value_v2 make_bytes_from_string(std::string_view name, std::string_view data) {
        return variant_value_v2(name, std::vector<uint8_t>(
            reinterpret_cast<const uint8_t*>(data.data()),
            reinterpret_cast<const uint8_t*>(data.data() + data.size())
        ));
    }

    // ============================================================================
    // Container values
    // ============================================================================

    /**
     * @brief Create a container value
     */
    inline variant_value_v2 make_container_value(std::string_view name,
                                                 std::shared_ptr<thread_safe_container> container) {
        return variant_value_v2(name, std::move(container));
    }

    // ============================================================================
    // Array values
    // ============================================================================

    /**
     * @brief Create an array value
     */
    inline variant_value_v2 make_array_value(std::string_view name,
                                             std::vector<std::shared_ptr<variant_value_v2>> values) {
        array_variant arr;
        arr.values = std::move(values);
        return variant_value_v2(name, std::move(arr));
    }

    /**
     * @brief Create an array value from initializer list
     */
    inline variant_value_v2 make_array_value(
        std::string_view name,
        std::initializer_list<variant_value_v2> values)
    {
        array_variant arr;
        arr.values.reserve(values.size());
        for (const auto& v : values) {
            arr.values.push_back(std::make_shared<variant_value_v2>(v));
        }
        return variant_value_v2(name, std::move(arr));
    }

    /**
     * @brief Create an empty array value
     */
    inline variant_value_v2 make_empty_array_value(std::string_view name) {
        return variant_value_v2(name, array_variant{});
    }

    // ============================================================================
    // Type-based factory (runtime type selection)
    // ============================================================================

    /**
     * @brief Create a value from value_types enum and raw data
     *
     * This is the low-level factory that mirrors the legacy system's constructor.
     * Prefer type-specific factories (make_int_value, etc.) when the type is known at compile time.
     *
     * @param name Value name
     * @param type The value_types enum
     * @param raw_data Binary data for the value
     * @return variant_value_v2 containing the parsed value
     * @throws std::runtime_error if type is unsupported or data is invalid
     */
    inline variant_value_v2 make_value_from_raw(std::string_view name,
                                                value_types type,
                                                const std::vector<uint8_t>& raw_data) {
        return variant_value_v2(name, type, raw_data);
    }

    // ============================================================================
    // Utility functions
    // ============================================================================

    /**
     * @brief Create a copy of a variant_value_v2 with a new name
     */
    inline variant_value_v2 clone_with_name(const variant_value_v2& original,
                                           std::string_view new_name) {
        // Serialize and deserialize to create a deep copy, then change name
        auto serialized = original.serialize();
        auto cloned = variant_value_v2::deserialize(serialized);
        if (!cloned) {
            throw std::runtime_error("Failed to clone variant_value_v2");
        }

        // Create new value with same data but different name
        return variant_value_v2(new_name, cloned->type(), serialized);
    }

    /**
     * @brief Check if two values have the same type
     */
    inline bool same_type(const variant_value_v2& a, const variant_value_v2& b) {
        return a.type() == b.type();
    }

    /**
     * @brief Get human-readable type name
     */
    inline std::string_view type_name(const variant_value_v2& value) {
        switch (value.type()) {
            case value_types::null_value: return "null";
            case value_types::bool_value: return "bool";
            case value_types::short_value: return "short";
            case value_types::ushort_value: return "ushort";
            case value_types::int_value: return "int";
            case value_types::uint_value: return "uint";
            case value_types::long_value: return "long";
            case value_types::ulong_value: return "ulong";
            case value_types::llong_value: return "llong";
            case value_types::ullong_value: return "ullong";
            case value_types::float_value: return "float";
            case value_types::double_value: return "double";
            case value_types::bytes_value: return "bytes";
            case value_types::string_value: return "string";
            case value_types::container_value: return "container";
            case value_types::array_value: return "array";
            default: return "unknown";
        }
    }

} // namespace container_module
