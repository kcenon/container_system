// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

#pragma once

#include <string>

#if __has_include(<format>)
#include <format>
#endif

#if defined(__cpp_lib_format) && __cpp_lib_format >= 202110L
#define UTILITY_MODULE_HAS_STD_FORMAT 1
#else
#define UTILITY_MODULE_HAS_STD_FORMAT 0
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
            std::vformat_to(out, format_str, std::make_format_args(args...));
        } catch (const std::exception&) {
            // Fallback: just copy the format string
            std::copy(format_str.begin(), format_str.end(), out);
        }
    }

#else // fallback: ostringstream-based {} substitution

    template<typename... Args>
    static std::string format(const std::string& format_str, Args&&... args) {
        std::string result = format_str;
        (replace_next(result, std::forward<Args>(args)), ...);
        return result;
    }

    template<typename OutputIt, typename... Args>
    static void format_to(OutputIt out, const std::string& format_str, Args&&... args) {
        std::string result = format(format_str, std::forward<Args>(args)...);
        std::copy(result.begin(), result.end(), out);
    }

#endif

    static std::string format(const std::string& format_str) {
        return format_str;
    }

private:
#if !UTILITY_MODULE_HAS_STD_FORMAT
    template<typename T>
    static void replace_next(std::string& str, T&& value) {
        auto pos = str.find("{}");
        if (pos == std::string::npos) return;

        std::ostringstream oss;
        oss << std::forward<T>(value);
        str.replace(pos, 2, oss.str());
    }
#endif
};

} // namespace utility_module
