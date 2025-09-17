#pragma once

#include <string>
#include <sstream>
#include <fmt/format.h>

namespace utilities {
namespace core {

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

    static std::string format(const std::string& format_str) {
        return format_str;
    }
};

} // namespace core
} // namespace utilities