/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

/**
 * @file internal/value_view.h
 * @brief Zero-copy value view for efficient data access
 *
 * This header provides value_view class for zero-copy access to serialized
 * data. Instead of copying values during deserialization, value_view points
 * directly into the original buffer, providing significant performance
 * improvements for large messages.
 *
 * Usage:
 * @code
 * auto container = std::make_shared<value_container>(large_data, true);
 *
 * // Zero-copy access (no allocation)
 * if (auto view = container->get_view("large_text")) {
 *     std::string_view text = view->as_string_view();
 *     // Use text without copying...
 *
 *     // Copy only when needed
 *     std::string owned = view->as_string();
 * }
 * @endcode
 */

#pragma once

#include "container/core/value_types.h"

#include <string>
#include <string_view>
#include <cstdlib>
#include <optional>
#include <type_traits>

namespace container_module
{

/**
 * @brief Zero-copy value view for efficient read access
 *
 * value_view provides a non-owning view into serialized data, enabling
 * zero-copy access to string and numeric values. The underlying buffer
 * must remain valid for the lifetime of the view.
 *
 * Key features:
 * - Zero-copy string access via string_view
 * - Type-safe numeric extraction
 * - Minimal memory overhead (just pointers and metadata)
 * - No heap allocations for primitive types
 */
class value_view
{
public:
    /**
     * @brief Construct a value view
     * @param name_data Pointer to the name string data
     * @param name_length Length of the name string
     * @param value_data Pointer to the value data
     * @param value_length Length of the value data
     * @param type The value type
     */
    value_view(const char* name_data, size_t name_length,
               const char* value_data, size_t value_length,
               value_types type) noexcept
        : name_data_(name_data)
        , name_length_(name_length)
        , value_data_(value_data)
        , value_length_(value_length)
        , type_(type)
    {}

    /**
     * @brief Get the value name as string_view (zero-copy)
     */
    [[nodiscard]] std::string_view name() const noexcept
    {
        return {name_data_, name_length_};
    }

    /**
     * @brief Get the value type
     */
    [[nodiscard]] value_types type() const noexcept
    {
        return type_;
    }

    /**
     * @brief Get string value as string_view (zero-copy)
     * @return string_view pointing into original buffer
     * @note Only valid for string_value and bytes_value types
     */
    [[nodiscard]] std::string_view as_string_view() const noexcept
    {
        return {value_data_, value_length_};
    }

    /**
     * @brief Get string value as owned copy
     * @return Copied string
     */
    [[nodiscard]] std::string as_string() const
    {
        return {value_data_, value_length_};
    }

    /**
     * @brief Type-safe value extraction
     * @tparam T The expected value type
     * @return std::optional containing the value if type matches
     */
    template<typename T>
    [[nodiscard]] std::optional<T> as() const noexcept
    {
        if constexpr (std::is_same_v<T, std::string_view>) {
            if (type_ == value_types::string_value ||
                type_ == value_types::bytes_value) {
                return as_string_view();
            }
            return std::nullopt;
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            if (type_ == value_types::string_value ||
                type_ == value_types::bytes_value) {
                return as_string();
            }
            return std::nullopt;
        }
        else if constexpr (std::is_same_v<T, bool>) {
            if (type_ == value_types::bool_value && value_length_ > 0) {
                return (value_data_[0] == 't' || value_data_[0] == 'T' ||
                        value_data_[0] == '1');
            }
            return std::nullopt;
        }
        else if constexpr (std::is_integral_v<T>) {
            return parse_integral<T>();
        }
        else if constexpr (std::is_floating_point_v<T>) {
            return parse_floating<T>();
        }
        else {
            return std::nullopt;
        }
    }

    /**
     * @brief Check if this is a null value
     */
    [[nodiscard]] bool is_null() const noexcept
    {
        return type_ == value_types::null_value;
    }

    /**
     * @brief Get raw value data pointer
     */
    [[nodiscard]] const char* data() const noexcept
    {
        return value_data_;
    }

    /**
     * @brief Get value data length
     */
    [[nodiscard]] size_t size() const noexcept
    {
        return value_length_;
    }

private:
    /**
     * @brief Parse integral value from string data
     */
    template<typename T>
    [[nodiscard]] std::optional<T> parse_integral() const noexcept
    {
        if (!is_integral_type(type_)) {
            return std::nullopt;
        }

        if (value_length_ == 0) {
            return std::nullopt;
        }

        // Use strtoll/strtoull for parsing
        char* end = nullptr;
        std::string temp(value_data_, value_length_);

        if constexpr (std::is_signed_v<T>) {
            long long val = std::strtoll(temp.c_str(), &end, 10);
            if (end != temp.c_str() + temp.size()) {
                return std::nullopt;
            }
            return static_cast<T>(val);
        } else {
            unsigned long long val = std::strtoull(temp.c_str(), &end, 10);
            if (end != temp.c_str() + temp.size()) {
                return std::nullopt;
            }
            return static_cast<T>(val);
        }
    }

    /**
     * @brief Parse floating-point value from string data
     */
    template<typename T>
    [[nodiscard]] std::optional<T> parse_floating() const noexcept
    {
        if (type_ != value_types::float_value &&
            type_ != value_types::double_value) {
            return std::nullopt;
        }

        if (value_length_ == 0) {
            return std::nullopt;
        }

        char* end = nullptr;
        std::string temp(value_data_, value_length_);

        if constexpr (std::is_same_v<T, float>) {
            float val = std::strtof(temp.c_str(), &end);
            if (end != temp.c_str() + temp.size()) {
                return std::nullopt;
            }
            return val;
        } else {
            double val = std::strtod(temp.c_str(), &end);
            if (end != temp.c_str() + temp.size()) {
                return std::nullopt;
            }
            return static_cast<T>(val);
        }
    }

    /**
     * @brief Check if type is an integral type
     */
    [[nodiscard]] static bool is_integral_type(value_types t) noexcept
    {
        switch (t) {
            case value_types::short_value:
            case value_types::ushort_value:
            case value_types::int_value:
            case value_types::uint_value:
            case value_types::long_value:
            case value_types::ulong_value:
            case value_types::llong_value:
            case value_types::ullong_value:
                return true;
            default:
                return false;
        }
    }

    const char* name_data_;
    size_t name_length_;
    const char* value_data_;
    size_t value_length_;
    value_types type_;
};

/**
 * @brief Value index entry for lazy parsing
 *
 * Stores metadata about a value's location within the serialized buffer,
 * enabling on-demand parsing without upfront deserialization of all values.
 */
struct value_index_entry
{
    std::string_view name;      ///< Key name (points into raw_data_ptr_)
    size_t value_offset;        ///< Offset to value data in buffer
    size_t value_length;        ///< Length of value data
    value_types type;           ///< Value type

    value_index_entry() noexcept
        : value_offset(0)
        , value_length(0)
        , type(value_types::null_value)
    {}

    value_index_entry(std::string_view n, size_t offset, size_t length, value_types t) noexcept
        : name(n)
        , value_offset(offset)
        , value_length(length)
        , type(t)
    {}
};

} // namespace container_module
