/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#pragma once

#include "value.h"
#include "core/value_types.h"
#include "core/concepts.h"
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <type_traits>
#include <concepts>

namespace container_module
{
    // ============================================================================
    // Modern Factory API (Recommended)
    // ============================================================================

    /**
     * @brief Modern factory namespace for creating value instances
     *
     * Provides a unified, minimal API for value creation:
     * - factory::make<T>() for typed values
     * - factory::make_null() for null values
     *
     * Usage examples:
     * ```cpp
     * auto v1 = factory::make("enabled", true);       // bool
     * auto v2 = factory::make("count", 42);           // int
     * auto v3 = factory::make("name", std::string("John")); // string
     * auto v4 = factory::make_null("empty");          // null
     *
     * // Or use constructors directly (preferred):
     * auto v5 = value("flag", true);
     * auto v6 = value("num", 42);
     * ```
     */
    namespace factory
    {
        /**
         * @brief Generic factory template for creating typed values
         *
         * Forwards arguments to the value constructor with perfect forwarding.
         * This is a thin wrapper that provides a consistent factory interface.
         *
         * @tparam T The value type (automatically deduced)
         * @param name Value name
         * @param val The value to store
         * @return value containing the specified data
         */
        template<typename T>
        inline value make(std::string_view name, T&& val) {
            return value(name, std::forward<T>(val));
        }

        /**
         * @brief Create a null value
         *
         * Explicit factory for null value creation when a named null is needed.
         *
         * @param name Value name (default: empty string)
         * @return value containing null (std::monostate)
         */
        inline value make_null(std::string_view name = "") {
            return value(name);
        }

        /**
         * @brief Create an array value from vector of shared_ptr<value>
         */
        inline value make_array(std::string_view name,
                               std::vector<std::shared_ptr<value>> values) {
            array_variant arr;
            arr.values = std::move(values);
            return value(name, std::move(arr));
        }

        /**
         * @brief Create an array value from initializer list
         */
        inline value make_array(
            std::string_view name,
            std::initializer_list<value> values)
        {
            array_variant arr;
            arr.values.reserve(values.size());
            for (const auto& v : values) {
                arr.values.push_back(std::make_shared<value>(v));
            }
            return value(name, std::move(arr));
        }

        /**
         * @brief Create an empty array value
         */
        inline value make_empty_array(std::string_view name) {
            return value(name, array_variant{});
        }

        /**
         * @brief Create a container value
         */
        inline value make_container(std::string_view name,
                                   std::shared_ptr<thread_safe_container> container) {
            return value(name, std::move(container));
        }

        /**
         * @brief Create a bytes value from raw data pointer
         */
        inline value make_bytes(std::string_view name,
                               const uint8_t* data,
                               size_t size) {
            return value(name, std::vector<uint8_t>(data, data + size));
        }

        /**
         * @brief Create a bytes value from string (copy bytes)
         * Note: This treats the string as binary data
         */
        inline value make_bytes_from_string(std::string_view name, std::string_view data) {
            return value(name, std::vector<uint8_t>(
                reinterpret_cast<const uint8_t*>(data.data()),
                reinterpret_cast<const uint8_t*>(data.data() + data.size())
            ));
        }

    } // namespace factory

    // ============================================================================
    // Deprecated Factory Functions
    // ============================================================================
    // These functions are deprecated. Use value constructors directly or
    // factory::make<T>() for new code.
    // ============================================================================

    /**
     * @brief Create a null value
     * @deprecated Use value(name) constructor or factory::make_null() instead
     */
    [[deprecated("Use value(name) constructor or factory::make_null() instead")]]
    inline value make_null_value(std::string_view name = "") {
        return value(name);
    }

    /**
     * @brief Create a boolean value
     * @deprecated Use value(name, val) constructor or factory::make() instead
     */
    [[deprecated("Use value(name, val) constructor or factory::make() instead")]]
    inline value make_bool_value(std::string_view name, bool val) {
        return value(name, val);
    }

    /**
     * @brief Create a short (int16_t) value
     * @deprecated Use value(name, val) constructor or factory::make() instead
     */
    [[deprecated("Use value(name, val) constructor or factory::make() instead")]]
    inline value make_short_value(std::string_view name, int16_t val) {
        return value(name, val);
    }

    /**
     * @brief Create an unsigned short (uint16_t) value
     * @deprecated Use value(name, val) constructor or factory::make() instead
     */
    [[deprecated("Use value(name, val) constructor or factory::make() instead")]]
    inline value make_ushort_value(std::string_view name, uint16_t val) {
        return value(name, val);
    }

    /**
     * @brief Create an int (int32_t) value
     * @deprecated Use value(name, val) constructor or factory::make() instead
     */
    [[deprecated("Use value(name, val) constructor or factory::make() instead")]]
    inline value make_int_value(std::string_view name, int32_t val) {
        return value(name, val);
    }

    /**
     * @brief Create an unsigned int (uint32_t) value
     * @deprecated Use value(name, val) constructor or factory::make() instead
     */
    [[deprecated("Use value(name, val) constructor or factory::make() instead")]]
    inline value make_uint_value(std::string_view name, uint32_t val) {
        return value(name, val);
    }

    /**
     * @brief Create a long (int64_t) value
     * @deprecated Use value(name, val) constructor or factory::make() instead
     */
    [[deprecated("Use value(name, val) constructor or factory::make() instead")]]
    inline value make_long_value(std::string_view name, int64_t val) {
        return value(name, val);
    }

    /**
     * @brief Create an unsigned long (uint64_t) value
     * @deprecated Use value(name, val) constructor or factory::make() instead
     */
    [[deprecated("Use value(name, val) constructor or factory::make() instead")]]
    inline value make_ulong_value(std::string_view name, uint64_t val) {
        return value(name, val);
    }

    /**
     * @brief Create a float value
     * @deprecated Use value(name, val) constructor or factory::make() instead
     */
    [[deprecated("Use value(name, val) constructor or factory::make() instead")]]
    inline value make_float_value(std::string_view name, float val) {
        return value(name, val);
    }

    /**
     * @brief Create a double value
     * @deprecated Use value(name, val) constructor or factory::make() instead
     */
    [[deprecated("Use value(name, val) constructor or factory::make() instead")]]
    inline value make_double_value(std::string_view name, double val) {
        return value(name, val);
    }

    /**
     * @brief Generic numeric value factory with automatic type deduction
     * @deprecated Use value(name, val) constructor or factory::make() instead
     */
    template<concepts::Arithmetic T>
    [[deprecated("Use value(name, val) constructor or factory::make() instead")]]
    inline value make_numeric_value(std::string_view name, T val) {
        return value(name, val);
    }

    /**
     * @brief Create a string value
     * @deprecated Use value(name, val) constructor or factory::make() instead
     */
    [[deprecated("Use value(name, val) constructor or factory::make() instead")]]
    inline value make_string_value(std::string_view name, std::string str) {
        return value(name, std::move(str));
    }

    /**
     * @brief Create a string value from string_view
     * @deprecated Use value(name, std::string(str)) constructor instead
     */
    [[deprecated("Use value(name, std::string(str)) constructor instead")]]
    inline value make_string_value(std::string_view name, std::string_view str) {
        return value(name, std::string(str));
    }

    /**
     * @brief Create a string value from C-string
     * @deprecated Use value(name, std::string(str)) constructor instead
     */
    [[deprecated("Use value(name, std::string(str)) constructor instead")]]
    inline value make_string_value(std::string_view name, const char* str) {
        return value(name, std::string(str));
    }

    /**
     * @brief Create a bytes value
     * @deprecated Use value(name, data) constructor or factory::make() instead
     */
    [[deprecated("Use value(name, data) constructor or factory::make() instead")]]
    inline value make_bytes_value(std::string_view name, std::vector<uint8_t> data) {
        return value(name, std::move(data));
    }

    /**
     * @brief Create a bytes value from raw data
     * @deprecated Use factory::make_bytes() instead
     */
    [[deprecated("Use factory::make_bytes() instead")]]
    inline value make_bytes_value(std::string_view name,
                                             const uint8_t* data,
                                             size_t size) {
        return value(name, std::vector<uint8_t>(data, data + size));
    }

    /**
     * @brief Create a bytes value from string (copy bytes)
     * @deprecated Use factory::make_bytes_from_string() instead
     */
    [[deprecated("Use factory::make_bytes_from_string() instead")]]
    inline value make_bytes_from_string(std::string_view name, std::string_view data) {
        return value(name, std::vector<uint8_t>(
            reinterpret_cast<const uint8_t*>(data.data()),
            reinterpret_cast<const uint8_t*>(data.data() + data.size())
        ));
    }

    /**
     * @brief Create a container value
     * @deprecated Use value(name, container) constructor or factory::make_container() instead
     */
    [[deprecated("Use value(name, container) constructor or factory::make_container() instead")]]
    inline value make_container_value(std::string_view name,
                                                 std::shared_ptr<thread_safe_container> container) {
        return value(name, std::move(container));
    }

    /**
     * @brief Create an array value
     * @deprecated Use factory::make_array() instead
     */
    [[deprecated("Use factory::make_array() instead")]]
    inline value make_array_value(std::string_view name,
                                             std::vector<std::shared_ptr<value>> values) {
        array_variant arr;
        arr.values = std::move(values);
        return value(name, std::move(arr));
    }

    /**
     * @brief Create an array value from initializer list
     * @deprecated Use factory::make_array() instead
     */
    [[deprecated("Use factory::make_array() instead")]]
    inline value make_array_value(
        std::string_view name,
        std::initializer_list<value> values)
    {
        array_variant arr;
        arr.values.reserve(values.size());
        for (const auto& v : values) {
            arr.values.push_back(std::make_shared<value>(v));
        }
        return value(name, std::move(arr));
    }

    /**
     * @brief Create an empty array value
     * @deprecated Use factory::make_empty_array() instead
     */
    [[deprecated("Use factory::make_empty_array() instead")]]
    inline value make_empty_array_value(std::string_view name) {
        return value(name, array_variant{});
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
     * @return value containing the parsed value
     * @throws std::runtime_error if type is unsupported or data is invalid
     */
    inline value make_value_from_raw(std::string_view name,
                                                value_types type,
                                                const std::vector<uint8_t>& raw_data) {
        return value(name, type, raw_data);
    }

    // ============================================================================
    // Utility functions
    // ============================================================================

    /**
     * @brief Create a copy of a value with a new name
     */
    inline value clone_with_name(const value& original,
                                           std::string_view new_name) {
        // Serialize and deserialize to create a deep copy, then change name
        auto serialized = original.serialize();
        auto cloned = value::deserialize(serialized);
        if (!cloned) {
            throw std::runtime_error("Failed to clone value");
        }

        // Create new value with same data but different name
        return value(new_name, cloned->type(), serialized);
    }

    /**
     * @brief Check if two values have the same type
     */
    inline bool same_type(const value& a, const value& b) {
        return a.type() == b.type();
    }

    /**
     * @brief Get human-readable type name
     */
    inline std::string_view type_name(const value& value) {
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
