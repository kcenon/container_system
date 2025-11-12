// BSD 3-Clause License
//
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <string>
#include <sstream>

// Check for std::format support
#if defined(USE_STD_FORMAT)
    // When explicitly requested via CMake, trust that std::format is available
    // This bypasses __cpp_lib_format check which may not be defined in older libstdc++
    #include <format>
    #define HAS_STD_FORMAT 1
#elif __has_include(<format>) && defined(__cpp_lib_format)
    // Auto-detect std::format with feature macro (GCC 13+, Clang 14+ with libc++)
    #include <format>
    #define HAS_STD_FORMAT 1
#elif __has_include(<fmt/format.h>)
    // Use fmt library if available
    #include <fmt/format.h>
    #define HAS_STD_FORMAT 0
#else
    // No formatting library available, use fallback
    #define HAS_STD_FORMAT -1
#endif

namespace utility_module {

/**
 * @brief Simple formatter wrapper around fmt library or std::format
 */
class formatter {
public:
    template<typename... Args>
    static std::string format(const std::string& format_str, Args&&... args) {
        try {
#if HAS_STD_FORMAT == 1
            return std::vformat(format_str, std::make_format_args(args...));
#elif HAS_STD_FORMAT == 0
            return fmt::vformat(format_str, fmt::make_format_args(args...));
#else
            // Fallback: simple string stream concatenation
            std::ostringstream oss;
            oss << format_str;
            return oss.str();
#endif
        } catch (const std::exception&) {
            return format_str; // Return original string if formatting fails
        }
    }

    template<typename OutputIt, typename... Args>
    static void format_to(OutputIt out, const std::string& format_str, Args&&... args) {
        try {
#if HAS_STD_FORMAT == 1
            std::vformat_to(out, format_str, std::make_format_args(args...));
#elif HAS_STD_FORMAT == 0
            fmt::vformat_to(out, format_str, fmt::make_format_args(args...));
#else
            // Fallback: just copy the format string
            std::copy(format_str.begin(), format_str.end(), out);
#endif
        } catch (const std::exception&) {
            // Fallback: just copy the format string
            std::copy(format_str.begin(), format_str.end(), out);
        }
    }

    static std::string format(const std::string& format_str) {
        return format_str;
    }
};

} // namespace utility_module