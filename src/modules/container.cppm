// BSD 3-Clause License
// Copyright (c) 2021-2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file container.cppm
 * @brief Primary C++20 module for container_system.
 *
 * This is the main module interface for the container_system library.
 * Container system is a small codebase, so it's implemented as a single
 * module without partitions.
 *
 * Usage:
 * @code
 * import kcenon.container;
 *
 * using namespace container_module;
 *
 * value_container container;
 * container.set("name", std::string("value"));
 * container.set("count", 42);
 *
 * auto result = container.serialize();
 * @endcode
 *
 * Dependencies:
 * - kcenon.common (Tier 0) - for Result<T> pattern
 */

module;

// =============================================================================
// Global Module Fragment - Standard Library Headers
// =============================================================================
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <format>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <ostream>
#include <shared_mutex>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

export module kcenon.container;

export import kcenon.common;

// =============================================================================
// Exported Container Module Types
// =============================================================================

export namespace container_module {

// =============================================================================
// Value Types Enumeration
// =============================================================================

/**
 * @brief Enumeration of available value types in the container system.
 */
enum class value_types {
    null_value,
    bool_value,
    short_value,
    ushort_value,
    int_value,
    uint_value,
    long_value,
    ulong_value,
    llong_value,
    ullong_value,
    float_value,
    double_value,
    string_value,
    bytes_value,
    container_value,
    array_value
};

// Compile-time type mapping
constexpr std::array<std::pair<std::string_view, value_types>, 16> type_map{{
    {"0", value_types::null_value},
    {"1", value_types::bool_value},
    {"2", value_types::short_value},
    {"3", value_types::ushort_value},
    {"4", value_types::int_value},
    {"5", value_types::uint_value},
    {"6", value_types::long_value},
    {"7", value_types::ulong_value},
    {"8", value_types::llong_value},
    {"9", value_types::ullong_value},
    {"10", value_types::float_value},
    {"11", value_types::double_value},
    {"12", value_types::string_value},
    {"13", value_types::bytes_value},
    {"14", value_types::container_value},
    {"15", value_types::array_value}
}};

/**
 * @brief Compile-time conversion from string to value_types
 */
constexpr value_types get_type_from_string(std::string_view str) noexcept {
    for (const auto& [key, type] : type_map) {
        if (key == str) return type;
    }
    return value_types::null_value;
}

/**
 * @brief Compile-time conversion from value_types to string
 */
constexpr std::string_view get_string_from_type(value_types type) noexcept {
    for (const auto& [key, val] : type_map) {
        if (val == type) return key;
    }
    return "0";
}

/**
 * @brief Convert a string-based type indicator to a value_types enum.
 */
inline value_types convert_value_type(const std::string& target) {
    for (const auto& [key, type] : type_map) {
        if (key == target) return type;
    }
    return value_types::null_value;
}

/**
 * @brief Convert a value_types enum to its associated string indicator.
 */
inline std::string convert_value_type(const value_types& target) {
    return std::string(get_string_from_type(target));
}

// =============================================================================
// Forward Declarations
// =============================================================================

class value_container;
struct optimized_value;
struct pool_stats;

using value_container_ptr = std::shared_ptr<value_container>;

// =============================================================================
// Value Variant Type (SOO - Small Object Optimization)
// =============================================================================

/**
 * @brief Small Object Optimization (SOO) for value storage
 *
 * This variant-based storage allows small primitive values to be stored
 * on the stack rather than heap-allocated, significantly reducing memory
 * overhead and improving cache locality.
 */
using value_variant = std::variant<
    std::monostate,
    bool,
    short,
    unsigned short,
    int,
    unsigned int,
    long,
    unsigned long,
    long long,
    unsigned long long,
    float,
    double,
    std::string,
    std::vector<uint8_t>,
    std::shared_ptr<value_container>
>;

// =============================================================================
// Optimized Value Structure
// =============================================================================

/**
 * @brief Optimized value storage with Small Object Optimization
 */
struct optimized_value {
    std::string name;
    value_types type;
    value_variant data;

    optimized_value()
        : name("")
        , type(value_types::null_value)
        , data(std::monostate{})
    {}

    optimized_value(const std::string& n, value_types t)
        : name(n)
        , type(t)
        , data(std::monostate{})
    {}

    size_t memory_footprint() const {
        size_t base = sizeof(optimized_value);
        base += name.capacity();
        if (std::holds_alternative<std::string>(data)) {
            base += std::get<std::string>(data).capacity();
        } else if (std::holds_alternative<std::vector<uint8_t>>(data)) {
            base += std::get<std::vector<uint8_t>>(data).capacity();
        }
        return base;
    }

    bool is_stack_allocated() const {
        return type != value_types::string_value &&
               type != value_types::bytes_value &&
               type != value_types::container_value;
    }
};

// =============================================================================
// Pool Statistics
// =============================================================================

/**
 * @brief Pool statistics structure
 */
struct pool_stats {
    size_t hits{0};
    size_t misses{0};
    size_t available{0};
    double hit_rate{0.0};

    pool_stats() = default;
    pool_stats(size_t h, size_t m, size_t a)
        : hits(h), misses(m), available(a)
        , hit_rate(h + m > 0 ? static_cast<double>(h) / (h + m) : 0.0)
    {}
};

// =============================================================================
// Variant Helpers
// =============================================================================

namespace variant_helpers {

/**
 * @brief Escape a string for JSON output per RFC 8259
 */
inline std::string json_escape(std::string_view input) {
    std::string result;
    result.reserve(input.size() + input.size() / 8);

    for (char c : input) {
        switch (c) {
        case '"':
            result += "\\\"";
            break;
        case '\\':
            result += "\\\\";
            break;
        case '\b':
            result += "\\b";
            break;
        case '\f':
            result += "\\f";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x",
                              static_cast<unsigned char>(c));
                result += buf;
            } else {
                result += c;
            }
            break;
        }
    }

    return result;
}

/**
 * @brief Encode a string for XML output per XML 1.0 specification
 */
inline std::string xml_encode(std::string_view input) {
    std::string result;
    result.reserve(input.size() + input.size() / 8);

    for (char c : input) {
        switch (c) {
        case '&':
            result += "&amp;";
            break;
        case '<':
            result += "&lt;";
            break;
        case '>':
            result += "&gt;";
            break;
        case '"':
            result += "&quot;";
            break;
        case '\'':
            result += "&apos;";
            break;
        default:
            if (static_cast<unsigned char>(c) < 0x20
                && c != '\t' && c != '\n' && c != '\r') {
                char buf[16];
                std::snprintf(buf, sizeof(buf), "&#x%02x;",
                              static_cast<unsigned char>(c));
                result += buf;
            } else {
                result += c;
            }
            break;
        }
    }

    return result;
}

/**
 * @brief Convert value_variant to string representation
 */
inline std::string to_string(const value_variant& var, value_types type) {
    switch (type) {
    case value_types::null_value:
        return "";
    case value_types::bool_value:
        return std::get<bool>(var) ? "true" : "false";
    case value_types::short_value:
        return std::to_string(std::get<short>(var));
    case value_types::ushort_value:
        return std::to_string(std::get<unsigned short>(var));
    case value_types::int_value:
        return std::to_string(std::get<int>(var));
    case value_types::uint_value:
        return std::to_string(std::get<unsigned int>(var));
    case value_types::long_value:
        return std::to_string(std::get<long>(var));
    case value_types::ulong_value:
        return std::to_string(std::get<unsigned long>(var));
    case value_types::llong_value:
        return std::to_string(std::get<long long>(var));
    case value_types::ullong_value:
        return std::to_string(std::get<unsigned long long>(var));
    case value_types::float_value:
        return std::to_string(std::get<float>(var));
    case value_types::double_value:
        return std::to_string(std::get<double>(var));
    case value_types::string_value:
        return std::get<std::string>(var);
    default:
        return "";
    }
}

/**
 * @brief Get size in bytes of variant data
 */
inline size_t data_size(const value_variant& var, value_types type) {
    switch (type) {
    case value_types::null_value:
        return 0;
    case value_types::bool_value:
        return sizeof(bool);
    case value_types::short_value:
        return sizeof(short);
    case value_types::ushort_value:
        return sizeof(unsigned short);
    case value_types::int_value:
        return sizeof(int);
    case value_types::uint_value:
        return sizeof(unsigned int);
    case value_types::long_value:
        return sizeof(long);
    case value_types::ulong_value:
        return sizeof(unsigned long);
    case value_types::llong_value:
        return sizeof(long long);
    case value_types::ullong_value:
        return sizeof(unsigned long long);
    case value_types::float_value:
        return sizeof(float);
    case value_types::double_value:
        return sizeof(double);
    case value_types::string_value:
        return std::get<std::string>(var).size();
    case value_types::bytes_value:
        return std::get<std::vector<uint8_t>>(var).size();
    default:
        return 0;
    }
}

} // namespace variant_helpers

// =============================================================================
// Value Container Class
// =============================================================================

/**
 * @class value_container
 * @brief A high-level container for messages, including source/target IDs,
 * message type, and a list of values.
 */
class value_container : public std::enable_shared_from_this<value_container> {
public:
    value_container();
    value_container(const std::string& data_string, bool parse_only_header = true);
    value_container(const std::vector<uint8_t>& data_array, bool parse_only_header = true);
    value_container(const value_container& data_container, bool parse_only_header = true);
    value_container(std::shared_ptr<value_container> data_container, bool parse_only_header = true);
    value_container(value_container&& other) noexcept;
    value_container& operator=(value_container&& other) noexcept;
    virtual ~value_container();

    std::shared_ptr<value_container> get_ptr();

    void set_source(std::string_view source_id, std::string_view source_sub_id);
    void set_target(std::string_view target_id, std::string_view target_sub_id = "");
    void set_message_type(std::string_view message_type);
    void swap_header();
    void clear_value();
    std::shared_ptr<value_container> copy(bool containing_values = true);

    std::string source_id() const noexcept;
    std::string source_sub_id() const noexcept;
    std::string target_id() const noexcept;
    std::string target_sub_id() const noexcept;
    std::string message_type() const noexcept;

    // Unified Value Setter API
    template<typename T>
    value_container& set(std::string_view key, T&& data_val) {
        optimized_value val;
        val.name = std::string(key);
        val.data = std::forward<T>(data_val);
        val.type = static_cast<value_types>(val.data.index());
        set_unit_impl(val);
        return *this;
    }

    value_container& set(const optimized_value& val);
    value_container& set_all(std::span<const optimized_value> vals);

    [[nodiscard]] bool contains(std::string_view key) const noexcept;

    template<typename T>
    [[nodiscard]] kcenon::common::Result<T> get(std::string_view key) const noexcept {
        read_lock_guard lock(this);

        for (const auto& val : optimized_units_) {
            if (val.name == key) {
                if (auto* ptr = std::get_if<T>(&val.data)) {
                    return kcenon::common::ok(*ptr);
                }
                return kcenon::common::Result<T>(
                    kcenon::common::error_info{-2, "Type mismatch for key: " + std::string(key), "container_system"});
            }
        }
        return kcenon::common::Result<T>(
            kcenon::common::error_info{-1, "Key not found: " + std::string(key), "container_system"});
    }

    std::optional<optimized_value> get_value(const std::string& name) const noexcept;
    std::optional<optimized_value> get_variant_value(const std::string& key) const noexcept;
    std::vector<optimized_value> get_variant_values() const;

    bool is_variant_mode() const noexcept { return true; }
    void enable_variant_mode(bool enable = true) { (void)enable; }

    void remove(std::string_view target_name, bool update_immediately = false);
    void initialize();

    std::string serialize() const;
    std::vector<uint8_t> serialize_array() const;
    bool deserialize(const std::string& data_string, bool parse_only_header = true);
    bool deserialize(const std::vector<uint8_t>& data_array, bool parse_only_header = true);

    kcenon::common::VoidResult deserialize_result(const std::string& data_string,
                                                   bool parse_only_header = true) noexcept;
    kcenon::common::VoidResult deserialize_result(const std::vector<uint8_t>& data_array,
                                                   bool parse_only_header = true) noexcept;

    const std::string to_xml();
    const std::string to_json();
    std::string datas() const;

    void load_packet(const std::string& file_path);
    void save_packet(const std::string& file_path);

    std::pair<size_t, size_t> memory_stats() const {
        return {
            heap_allocations_.load(std::memory_order_relaxed),
            stack_allocations_.load(std::memory_order_relaxed)
        };
    }

    size_t memory_footprint() const;
    void set_soo_enabled(bool enable) { use_soo_ = enable; }
    bool is_soo_enabled() const { return use_soo_; }

    static pool_stats get_pool_stats();
    static void clear_pool();

    // Iterator support
    using iterator = std::vector<optimized_value>::iterator;
    using const_iterator = std::vector<optimized_value>::const_iterator;

    iterator begin() { return optimized_units_.begin(); }
    iterator end() { return optimized_units_.end(); }
    const_iterator begin() const { return optimized_units_.begin(); }
    const_iterator end() const { return optimized_units_.end(); }
    const_iterator cbegin() const { return optimized_units_.cbegin(); }
    const_iterator cend() const { return optimized_units_.cend(); }
    size_t size() const { return optimized_units_.size(); }
    bool empty() const { return optimized_units_.empty(); }

private:
    bool deserialize_values(const std::string& data, bool parse_only_header);
    void parsing(std::string_view source_name, std::string_view target_name,
                 std::string_view target_value, std::string& target_variable);
    void set_unit_impl(const optimized_value& val);

    class read_lock_guard {
        std::shared_lock<std::shared_mutex> lock_;
    public:
        explicit read_lock_guard(const value_container* c) : lock_(c->mutex_) {}
        bool is_locked() const noexcept { return true; }
    };

    class write_lock_guard {
        std::unique_lock<std::shared_mutex> lock_;
    public:
        explicit write_lock_guard(value_container* c) : lock_(c->mutex_) {}
        bool is_locked() const noexcept { return true; }
    };

    bool parsed_data_;
    bool changed_data_;
    std::string data_string_;

    std::shared_ptr<const std::string> raw_data_ptr_;
    bool zero_copy_mode_{false};

    std::string source_id_;
    std::string source_sub_id_;
    std::string target_id_;
    std::string target_sub_id_;
    std::string message_type_;
    std::string version_;

    std::vector<optimized_value> optimized_units_;
    bool use_soo_{true};

    mutable std::shared_mutex mutex_;
    mutable std::atomic<size_t> heap_allocations_{0};
    mutable std::atomic<size_t> stack_allocations_{0};
};

// =============================================================================
// Module Version Information
// =============================================================================

/**
 * @brief Version information for container_system module.
 */
struct module_version {
    static constexpr int major = 0;
    static constexpr int minor = 1;
    static constexpr int patch = 0;
    static constexpr int tweak = 0;
    static constexpr const char* string = "0.1.0.0";
    static constexpr const char* module_name = "kcenon.container";
};

} // namespace container_module
