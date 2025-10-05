#pragma once

#include <string>
#include <sstream>

// Check for std::format support
#if defined(USE_STD_FORMAT) && __has_include(<format>) && defined(__cpp_lib_format)
    #include <format>
    #define HAS_STD_FORMAT 1
#elif __has_include(<fmt/format.h>)
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