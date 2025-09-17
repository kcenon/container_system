#pragma once

#include <string>
#include <sstream>
#include <fmt/format.h>

namespace utility_module {

/**
 * @brief Simple formatter wrapper around fmt library
 */
class formatter {
public:
    template<typename... Args>
    static std::string format(const std::string& format_str, Args&&... args) {
        try {
            return fmt::format(format_str, std::forward<Args>(args)...);
        } catch (const std::exception&) {
            return format_str; // Return original string if formatting fails
        }
    }

    template<typename OutputIt, typename... Args>
    static void format_to(OutputIt out, const std::string& format_str, Args&&... args) {
        try {
            fmt::format_to(out, format_str, std::forward<Args>(args)...);
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