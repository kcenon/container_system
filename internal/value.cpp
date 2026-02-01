/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "internal/value.h"
#include "internal/thread_safe_container.h"
#include <sstream>
#include <iomanip>
#include <cstring>
#include <stdexcept>
#include <set>

namespace container_module
{
    namespace detail {
        // Thread-local set for detecting circular references during serialization
        thread_local std::set<const void*> serializing_containers;

        struct circular_ref_guard {
            const void* ptr;
            bool inserted;

            explicit circular_ref_guard(const thread_safe_container* container)
                : ptr(container), inserted(false) {
                if (ptr) {
                    auto result = serializing_containers.insert(ptr);
                    inserted = result.second;
                }
            }

            ~circular_ref_guard() {
                if (inserted && ptr) {
                    serializing_containers.erase(ptr);
                }
            }

            bool is_circular() const { return ptr && !inserted; }
        };
    }
    // ============================================================================
    // array_variant implementation
    // ============================================================================

    array_variant::array_variant(const array_variant& other) {
        values.reserve(other.values.size());
        for (const auto& val : other.values) {
            if (val) {
                values.push_back(std::make_shared<value>(*val));
            }
        }
    }

    array_variant& array_variant::operator=(const array_variant& other) {
        if (this != &other) {
            values.clear();
            values.reserve(other.values.size());
            for (const auto& val : other.values) {
                if (val) {
                    values.push_back(std::make_shared<value>(*val));
                }
            }
        }
        return *this;
    }

    bool array_variant::operator==(const array_variant& other) const {
        if (values.size() != other.values.size()) return false;
        for (size_t i = 0; i < values.size(); ++i) {
            if (!values[i] && !other.values[i]) continue;
            if (!values[i] || !other.values[i]) return false;
            if (*values[i] != *other.values[i]) return false;
        }
        return true;
    }

    bool array_variant::operator<(const array_variant& other) const {
        return values.size() < other.values.size();
    }

    // ============================================================================
    // value implementation
    // ============================================================================

    value::value(std::string_view name,
                                       value_types type,
                                       const std::vector<uint8_t>& raw_data)
        : name_(name), data_(std::in_place_index<0>)
    {
        // Construct variant from legacy type and raw data
        size_t offset = 0;
        if (!deserialize_data(*this, type, raw_data, offset)) {
            // Failed to deserialize, reset to null
            data_.emplace<0>();
        }
    }

    value::value(const value& other)
        : name_(other.name_)
    {
        std::shared_lock lock(other.mutex_);
        data_ = other.data_;
    }

    value::value(value&& other) noexcept
        : name_(other.name_)
    {
        std::unique_lock lock(other.mutex_);
        data_ = std::move(other.data_);
        read_count_ = other.read_count_.load();
        write_count_ = other.write_count_.load();
        other.read_count_ = 0;
        other.write_count_ = 0;
    }

    value& value::operator=(const value& other) {
        if (this != &other) {
            std::unique_lock lock(mutex_);
            std::shared_lock other_lock(other.mutex_);
            data_ = other.data_;
            write_count_.fetch_add(1, std::memory_order_relaxed);
        }
        return *this;
    }

    value& value::operator=(value&& other) noexcept {
        if (this != &other) {
            std::unique_lock lock(mutex_);
            std::unique_lock other_lock(other.mutex_);
            data_ = std::move(other.data_);
            write_count_.fetch_add(1, std::memory_order_relaxed);
        }
        return *this;
    }

    value_types value::type() const {
        std::shared_lock lock(mutex_);
        size_t idx = data_.index();

        // Direct mapping: variant index == value_types enum value
        // Handle platform-specific llong/ullong
        if constexpr (!has_separate_llong) {
            // On macOS/Linux, llong_value (8) and ullong_value (9) are monostate
            // These should never be created directly, map to long_value instead
            if (idx == 8) return value_types::long_value;   // llong → long
            if (idx == 9) return value_types::ulong_value;  // ullong → ulong
        }

        return static_cast<value_types>(idx);
    }

    std::string value::to_string() const {
        return visit([](auto&& value) -> std::string {
            using T = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<T, std::monostate>) {
                return "null";
            }
            else if constexpr (std::is_same_v<T, bool>) {
                return value ? "true" : "false";
            }
            else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                std::ostringstream oss;
                oss << std::hex << std::setfill('0');
                for (auto byte : value) {
                    oss << std::setw(2) << static_cast<int>(byte);
                }
                return oss.str();
            }
            else if constexpr (std::is_arithmetic_v<T>) {
                return std::to_string(value);
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                return value;
            }
            else if constexpr (std::is_same_v<T, std::shared_ptr<thread_safe_container>>) {
                return value ? value->to_json() : "null";
            }
            else if constexpr (std::is_same_v<T, array_variant>) {
                std::string result = "[";
                for (size_t i = 0; i < value.values.size(); ++i) {
                    if (i > 0) result += ",";
                    result += value.values[i] ? value.values[i]->to_string() : "null";
                }
                result += "]";
                return result;
            }
            return "";
        });
    }

    std::string value::to_json() const {
        auto var_name = name();
        auto var_type = type();

        return visit([var_name, var_type](auto&& value) -> std::string {
            using T = std::decay_t<decltype(value)>;

            // Manual JSON header formatting (no std::format dependency)
            std::string result = "{\"name\":\"" + std::string(var_name) +
                                "\",\"type\":" + std::to_string(static_cast<int>(var_type)) +
                                ",\"value\":";

            if constexpr (std::is_same_v<T, std::monostate>) {
                result += "null";
            }
            else if constexpr (std::is_same_v<T, bool>) {
                result += value ? "true" : "false";
            }
            else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                result += "\"";
                std::ostringstream oss;
                oss << std::hex << std::setfill('0');
                for (auto byte : value) {
                    oss << std::setw(2) << static_cast<int>(byte);
                }
                result += oss.str();
                result += "\"";
            }
            else if constexpr (std::is_arithmetic_v<T>) {
                result += std::to_string(value);
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                // Escape string for JSON
                result += "\"";
                for (char c : value) {
                    switch (c) {
                        case '"': result += "\\\""; break;
                        case '\\': result += "\\\\"; break;
                        case '\b': result += "\\b"; break;
                        case '\f': result += "\\f"; break;
                        case '\n': result += "\\n"; break;
                        case '\r': result += "\\r"; break;
                        case '\t': result += "\\t"; break;
                        default:
                            if (c >= 0x20 && c <= 0x7E) {
                                result += c;
                            } else {
                                // Manual Unicode escape formatting (no std::format dependency)
                                char buf[7];  // "\uXXXX" + null terminator
                                std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                                result += buf;
                            }
                    }
                }
                result += "\"";
            }
            else if constexpr (std::is_same_v<T, std::shared_ptr<thread_safe_container>>) {
                result += value ? value->to_json() : "null";
            }
            else if constexpr (std::is_same_v<T, array_variant>) {
                result += "[";
                for (size_t i = 0; i < value.values.size(); ++i) {
                    if (i > 0) result += ",";
                    result += value.values[i] ? value.values[i]->to_json() : "null";
                }
                result += "]";
            }

            result += "}";
            return result;
        });
    }

    void value::serialize_data(std::vector<uint8_t>& result) const {
        visit([&result](auto&& val) {
            using T = std::decay_t<decltype(val)>;

            if constexpr (std::is_same_v<T, std::monostate>) {
                // Null val: no data
            }
            else if constexpr (std::is_same_v<T, bool>) {
                result.push_back(val ? 1 : 0);
            }
            else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                // Bytes: [length:4][data:length]
                uint32_t size = static_cast<uint32_t>(val.size());
                result.insert(result.end(),
                             reinterpret_cast<const uint8_t*>(&size),
                             reinterpret_cast<const uint8_t*>(&size) + sizeof(size));
                result.insert(result.end(), val.begin(), val.end());
            }
            else if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>) {
                // Numeric: raw bytes
                result.insert(result.end(),
                             reinterpret_cast<const uint8_t*>(&val),
                             reinterpret_cast<const uint8_t*>(&val) + sizeof(T));
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                // String: [length:4][UTF-8 data:length]
                uint32_t size = static_cast<uint32_t>(val.size());
                result.insert(result.end(),
                             reinterpret_cast<const uint8_t*>(&size),
                             reinterpret_cast<const uint8_t*>(&size) + sizeof(size));
                result.insert(result.end(), val.begin(), val.end());
            }
            else if constexpr (std::is_same_v<T, std::shared_ptr<thread_safe_container>>) {
                // Container: [serialized_size:4][serialized_data:size]
                if (val) {
                    // Check for circular reference
                    detail::circular_ref_guard guard(val.get());
                    if (guard.is_circular()) {
                        // Circular reference detected - serialize as null container
                        uint32_t size = 0;
                        result.insert(result.end(),
                                     reinterpret_cast<const uint8_t*>(&size),
                                     reinterpret_cast<const uint8_t*>(&size) + sizeof(size));
                    } else {
                        auto container_data = val->serialize();
                        uint32_t size = static_cast<uint32_t>(container_data.size());
                        result.insert(result.end(),
                                     reinterpret_cast<const uint8_t*>(&size),
                                     reinterpret_cast<const uint8_t*>(&size) + sizeof(size));
                        result.insert(result.end(), container_data.begin(), container_data.end());
                    }
                } else {
                    uint32_t size = 0;
                    result.insert(result.end(),
                                 reinterpret_cast<const uint8_t*>(&size),
                                 reinterpret_cast<const uint8_t*>(&size) + sizeof(size));
                }
            }
            else if constexpr (std::is_same_v<T, array_variant>) {
                // Array: [count:4][value1_serialized][value2_serialized]...
                uint32_t count = static_cast<uint32_t>(val.values.size());
                result.insert(result.end(),
                             reinterpret_cast<const uint8_t*>(&count),
                             reinterpret_cast<const uint8_t*>(&count) + sizeof(count));

                for (const auto& elem : val.values) {
                    if (elem) {
                        auto elem_data = elem->serialize();
                        result.insert(result.end(), elem_data.begin(), elem_data.end());
                    } else {
                        // Null element: serialize as null_value with empty name
                        container_module::value null_elem("");
                        auto null_data = null_elem.serialize();
                        result.insert(result.end(), null_data.begin(), null_data.end());
                    }
                }
            }
        });
    }

    std::vector<uint8_t> value::serialize() const {
        std::vector<uint8_t> result;

        // Header: [name_length:4][name:UTF-8][type:1]
        uint32_t name_len = static_cast<uint32_t>(name_.size());
        result.insert(result.end(),
                     reinterpret_cast<const uint8_t*>(&name_len),
                     reinterpret_cast<const uint8_t*>(&name_len) + sizeof(name_len));
        result.insert(result.end(), name_.begin(), name_.end());

        // Type byte: use value_types enum value (0-15)
        uint8_t type_byte = static_cast<uint8_t>(type());
        result.push_back(type_byte);

        // Data: type-specific serialization
        serialize_data(result);

        return result;
    }

    bool value::deserialize_data(value& result,
                                            value_types type,
                                            const std::vector<uint8_t>& data,
                                            size_t& offset) {
        switch (type) {
            case value_types::null_value:
                result.data_.emplace<0>();  // null is at index 0
                return true;

            case value_types::bool_value:
                if (offset >= data.size()) return false;
                result.data_ = (data[offset++] != 0);
                return true;

            case value_types::short_value: {
                if (offset + sizeof(int16_t) > data.size()) return false;
                int16_t value;
                std::memcpy(&value, data.data() + offset, sizeof(value));
                offset += sizeof(value);
                result.data_ = value;
                return true;
            }

            case value_types::ushort_value: {
                if (offset + sizeof(uint16_t) > data.size()) return false;
                uint16_t value;
                std::memcpy(&value, data.data() + offset, sizeof(value));
                offset += sizeof(value);
                result.data_ = value;
                return true;
            }

            case value_types::int_value: {
                if (offset + sizeof(int32_t) > data.size()) return false;
                int32_t value;
                std::memcpy(&value, data.data() + offset, sizeof(value));
                offset += sizeof(value);
                result.data_ = value;
                return true;
            }

            case value_types::uint_value: {
                if (offset + sizeof(uint32_t) > data.size()) return false;
                uint32_t value;
                std::memcpy(&value, data.data() + offset, sizeof(value));
                offset += sizeof(value);
                result.data_ = value;
                return true;
            }

            case value_types::long_value: {
                if (offset + sizeof(int64_t) > data.size()) return false;
                int64_t value;
                std::memcpy(&value, data.data() + offset, sizeof(value));
                offset += sizeof(value);
                result.data_ = value;
                return true;
            }

            case value_types::llong_value: {
                if (offset + sizeof(int64_t) > data.size()) return false;
#if defined(_MSC_VER) && _MSC_VER < 1900
                // Old MSVC (2013-): int64_t is __int64, long long is separate
                long long value;
                std::memcpy(&value, data.data() + offset, sizeof(value));
                offset += sizeof(value);
                result.data_ = value;  // Stored at index 8 (llong_value slot)
#else
                // Modern platforms (macOS, Linux, MSVC 2015+): int64_t == long long
                // llong_value uses same storage as long_value (int64_t)
                int64_t value;
                std::memcpy(&value, data.data() + offset, sizeof(value));
                offset += sizeof(value);
                result.data_ = value;  // Stored at index 6 (long_value slot)
#endif
                return true;
            }

            case value_types::ulong_value: {
                if (offset + sizeof(uint64_t) > data.size()) return false;
                uint64_t value;
                std::memcpy(&value, data.data() + offset, sizeof(value));
                offset += sizeof(value);
                result.data_ = value;
                return true;
            }

            case value_types::ullong_value: {
                if (offset + sizeof(uint64_t) > data.size()) return false;
#if defined(_MSC_VER) && _MSC_VER < 1900
                // Old MSVC (2013-): uint64_t is unsigned __int64, unsigned long long is separate
                unsigned long long value;
                std::memcpy(&value, data.data() + offset, sizeof(value));
                offset += sizeof(value);
                result.data_ = value;  // Stored at index 9 (ullong_value slot)
#else
                // Modern platforms (macOS, Linux, MSVC 2015+): uint64_t == unsigned long long
                // ullong_value uses same storage as ulong_value (uint64_t)
                uint64_t value;
                std::memcpy(&value, data.data() + offset, sizeof(value));
                offset += sizeof(value);
                result.data_ = value;  // Stored at index 7 (ulong_value slot)
#endif
                return true;
            }

            case value_types::float_value: {
                if (offset + sizeof(float) > data.size()) return false;
                float value;
                std::memcpy(&value, data.data() + offset, sizeof(value));
                offset += sizeof(value);
                result.data_ = value;
                return true;
            }

            case value_types::double_value: {
                if (offset + sizeof(double) > data.size()) return false;
                double value;
                std::memcpy(&value, data.data() + offset, sizeof(value));
                offset += sizeof(value);
                result.data_ = value;
                return true;
            }

            case value_types::bytes_value: {
                if (offset + sizeof(uint32_t) > data.size()) return false;
                uint32_t size;
                std::memcpy(&size, data.data() + offset, sizeof(size));
                offset += sizeof(size);
                if (offset + size > data.size()) return false;
                std::vector<uint8_t> bytes(data.begin() + offset,
                                          data.begin() + offset + size);
                offset += size;
                result.data_ = std::move(bytes);
                return true;
            }

            case value_types::string_value: {
                if (offset + sizeof(uint32_t) > data.size()) return false;
                uint32_t size;
                std::memcpy(&size, data.data() + offset, sizeof(size));
                offset += sizeof(size);
                if (offset + size > data.size()) return false;
                std::string str(data.begin() + offset, data.begin() + offset + size);
                offset += size;
                result.data_ = std::move(str);
                return true;
            }

            case value_types::container_value: {
                if (offset + sizeof(uint32_t) > data.size()) return false;
                uint32_t size;
                std::memcpy(&size, data.data() + offset, sizeof(size));
                offset += sizeof(size);
                if (size == 0) {
                    result.data_ = std::shared_ptr<thread_safe_container>(nullptr);
                    return true;
                }
                if (offset + size > data.size()) return false;
                std::vector<uint8_t> container_data(data.begin() + offset,
                                                    data.begin() + offset + size);
                offset += size;
                auto container = thread_safe_container::deserialize(container_data);
                result.data_ = container;
                return true;
            }

            case value_types::array_value: {
                if (offset + sizeof(uint32_t) > data.size()) return false;
                uint32_t count;
                std::memcpy(&count, data.data() + offset, sizeof(count));
                offset += sizeof(count);

                array_variant arr;
                arr.values.reserve(count);

                for (uint32_t i = 0; i < count; ++i) {
                    auto elem = deserialize(std::vector<uint8_t>(data.begin() + offset, data.end()));
                    if (!elem) return false;

                    // Update offset based on elem's serialized size
                    auto elem_size = elem->serialize().size();
                    offset += elem_size;

                    arr.values.push_back(std::make_shared<value>(std::move(*elem)));
                }

                result.data_ = std::move(arr);
                return true;
            }

            default:
                return false;
        }
    }

    std::optional<value> value::deserialize(const std::vector<uint8_t>& data) {
        if (data.size() < sizeof(uint32_t) + 1) {
            return std::nullopt;  // Too small: need at least name_len + type
        }

        size_t offset = 0;

        // Read name length
        uint32_t name_len;
        std::memcpy(&name_len, data.data() + offset, sizeof(name_len));
        offset += sizeof(name_len);

        if (offset + name_len + 1 > data.size()) {
            return std::nullopt;  // Invalid name length
        }

        // Read name
        std::string name(data.begin() + offset, data.begin() + offset + name_len);
        offset += name_len;

        // Read type byte
        uint8_t type_byte = data[offset++];
        value_types type = static_cast<value_types>(type_byte);

        // Validate type range
        if (type_byte > 15) {
            return std::nullopt;  // Invalid type
        }

        // Create result with name
        container_module::value result(name);

        // Deserialize data based on type
        if (!deserialize_data(result, type, data, offset)) {
            return std::nullopt;
        }

        return result;
    }

    bool value::operator==(const value& other) const {
        if (this == &other) return true;
        std::scoped_lock lock(mutex_, other.mutex_);
        return name_ == other.name_ && data_ == other.data_;
    }

    bool value::operator<(const value& other) const {
        if (this == &other) return false;
        std::scoped_lock lock(mutex_, other.mutex_);
        if (name_ != other.name_) return name_ < other.name_;
        return data_ < other.data_;
    }

} // namespace container_module
