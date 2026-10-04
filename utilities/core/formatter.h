// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

#pragma once

#include <algorithm>
#include <string>
#include <utility>

#if __has_include(<format>)
#include <format>
#endif

#ifndef UTILITY_MODULE_HAS_STD_FORMAT
#if defined(__cpp_lib_format) && __cpp_lib_format >= 202110L
#define UTILITY_MODULE_HAS_STD_FORMAT 1
#else
#define UTILITY_MODULE_HAS_STD_FORMAT 0
#endif
#endif

#if !UTILITY_MODULE_HAS_STD_FORMAT
#include <array>
#include <sstream>
#endif

namespace utility_module {

/**
 * @brief Simple formatter wrapper around std::format
 *
 * Uses std::vformat when full C++20 <format> support is available
 * (GCC 13+, MSVC 19.29+). Falls back to ostringstream-based placeholder
 * substitution on compilers with incomplete <format> (e.g. Apple Clang).
 */
class formatter {
public:
#if UTILITY_MODULE_HAS_STD_FORMAT

    template<typename... Args>
    static std::string format(const std::string& format_str, Args&&... args) {
        try {
            return std::vformat(format_str, std::make_format_args(args...));
        } catch (const std::exception&) {
            return format_str; // Return original string if formatting fails
        }
    }

    template<typename OutputIt, typename... Args>
    static void format_to(OutputIt out, const std::string& format_str, Args&&... args) {
        try {
#if defined(_LIBCPP_VERSION)
            // libc++ 220106's vformat_to can overrun its fixed iterator buffer
            // for strings at a 256-byte boundary. Use vformat's growing buffer,
            // then copy only after formatting succeeds (no partial fallback).
            const auto formatted = std::vformat(format_str, std::make_format_args(args...));
            std::copy(formatted.begin(), formatted.end(), out);
#else
            // Other standard libraries retain their direct output path without
            // allocating an intermediate string for every serialization field.
            std::vformat_to(out, format_str, std::make_format_args(args...));
#endif
        } catch (const std::exception&) {
            // Fallback: just copy the format string
            std::copy(format_str.begin(), format_str.end(), out);
        }
    }

#else // fallback: ostringstream-based {} substitution

    template<typename... Args>
    static std::string format(const std::string& format_str, Args&&... args) {
        const std::array<std::string, sizeof...(Args)> values{stringify(std::forward<Args>(args))...};
        std::string result;
        size_t argument = 0;
        for (size_t i = 0; i < format_str.size(); ++i) {
            const char c = format_str[i];
            if (c != '{' && c != '}') {
                result += c;
            } else if (i + 1 < format_str.size() && format_str[i + 1] == c) {
                result += c; // Escaped {{ or }}.
                ++i;
            } else if (c == '{' && i + 1 < format_str.size() && format_str[i + 1] == '}' &&
                       argument < values.size()) {
                result += values[argument++]; // Never reparse braces inside an argument.
                ++i;
            } else {
                return format_str; // Match the standard-format error fallback.
            }
        }
        return result;
    }

    template<typename OutputIt, typename... Args>
    static void format_to(OutputIt out, const std::string& format_str, Args&&... args) {
        std::string result = format(format_str, std::forward<Args>(args)...);
        std::copy(result.begin(), result.end(), out);
    }

#endif

private:
#if !UTILITY_MODULE_HAS_STD_FORMAT
    template<typename T>
    static std::string stringify(T&& value) {
        std::ostringstream oss;
        oss << std::forward<T>(value);
        return oss.str();
    }
#endif
};

} // namespace utility_module
