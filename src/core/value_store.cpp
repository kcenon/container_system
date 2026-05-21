// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

#include <kcenon/container/value_store.h>
#include <stdexcept>
#include <cstring>
#include <string>
#include <unordered_map>

#ifdef __has_include
#if __has_include(<nlohmann/json.hpp>)
#include <nlohmann/json.hpp>
#define HAS_NLOHMANN_JSON 1
#else
#define HAS_NLOHMANN_JSON -1
#endif
#else
#define HAS_NLOHMANN_JSON -1
#endif

namespace kcenon::container {

#if HAS_NLOHMANN_JSON == 1
namespace {

// Reconstruct a single value from its JSON object representation, mirroring
// the format produced by value::to_json():
//   {"name":"<name>","type":<int 0-15>,"value":<typed-json>}
value value_from_json(const std::string& key, const nlohmann::json& node) {
    if (!node.is_object() || !node.contains("type") || !node.contains("value")) {
        throw std::runtime_error(
            "value_store::deserialize() - malformed value object for key: " + key);
    }

    const std::string name = node.contains("name") && node["name"].is_string()
                                 ? node["name"].get<std::string>()
                                 : key;
    const int type_int = node["type"].get<int>();
    const nlohmann::json& v = node["value"];

    switch (static_cast<value_types>(type_int)) {
        case value_types::null_value:
            return value(name);
        case value_types::bool_value:
            return value(name, v.get<bool>());
        case value_types::short_value:
            return value(name, static_cast<int16_t>(v.get<int>()));
        case value_types::ushort_value:
            return value(name, static_cast<uint16_t>(v.get<unsigned int>()));
        case value_types::int_value:
            return value(name, v.get<int32_t>());
        case value_types::uint_value:
            return value(name, v.get<uint32_t>());
        case value_types::long_value:
        case value_types::llong_value:
            return value(name, v.get<int64_t>());
        case value_types::ulong_value:
        case value_types::ullong_value:
            return value(name, v.get<uint64_t>());
        case value_types::float_value:
            return value(name, v.get<float>());
        case value_types::double_value:
            return value(name, v.get<double>());
        case value_types::string_value:
            return value(name, v.get<std::string>());
        case value_types::bytes_value: {
            // to_json() renders bytes as a lowercase hex string
            const std::string hex = v.get<std::string>();
            if (hex.size() % 2 != 0) {
                throw std::runtime_error(
                    "value_store::deserialize() - odd-length hex for key: " + key);
            }
            std::vector<uint8_t> bytes;
            bytes.reserve(hex.size() / 2);
            for (size_t i = 0; i < hex.size(); i += 2) {
                bytes.push_back(static_cast<uint8_t>(
                    std::stoul(hex.substr(i, 2), nullptr, 16)));
            }
            return value(name, std::move(bytes));
        }
        case value_types::container_value:
        case value_types::array_value:
        default:
            throw std::runtime_error(
                "value_store::deserialize() - unsupported value type for key '" + key
                + "': " + std::to_string(type_int));
    }
}

} // namespace
#endif

void value_store::add(const std::string& key, value val) {
    // Always acquire lock to eliminate TOCTOU vulnerability (see #190)
    std::unique_lock lock(mutex_);
    values_[key] = std::move(val);
    write_count_.fetch_add(1, std::memory_order_relaxed);
}

std::optional<value> value_store::get(const std::string& key) const {
    // Always acquire lock to eliminate TOCTOU vulnerability (see #190)
    std::shared_lock lock(mutex_);
    auto it = values_.find(key);
    if (it != values_.end()) {
        read_count_.fetch_add(1, std::memory_order_relaxed);
        return it->second;
    }
    return std::nullopt;
}

bool value_store::contains(const std::string& key) const {
    // Always acquire lock to eliminate TOCTOU vulnerability (see #190)
    std::shared_lock lock(mutex_);
    return values_.find(key) != values_.end();
}

bool value_store::remove(const std::string& key) {
    // Always acquire lock to eliminate TOCTOU vulnerability (see #190)
    std::unique_lock lock(mutex_);
    auto it = values_.find(key);
    if (it != values_.end()) {
        values_.erase(it);
        return true;
    }
    return false;
}

void value_store::clear() {
    // Always acquire lock to eliminate TOCTOU vulnerability (see #190)
    std::unique_lock lock(mutex_);
    values_.clear();
}

size_t value_store::size() const {
    // Always acquire lock to eliminate TOCTOU vulnerability (see #190)
    std::shared_lock lock(mutex_);
    return values_.size();
}

bool value_store::empty() const {
    return size() == 0;
}

std::string value_store::serialize() const {
    // Always acquire lock to eliminate TOCTOU vulnerability (see #190)
    std::shared_lock lock(mutex_);
    return serialize_impl();
}

std::string value_store::serialize_impl() const {
    read_count_.fetch_add(1, std::memory_order_relaxed);

    std::string result = "{";
    bool first = true;

    for (const auto& [key, val] : values_) {
        if (!first) {
            result += ",";
        }
        first = false;

        // Escape key for JSON
        result += "\"";
        for (char c : key) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c;
            }
        }
        result += "\":";
        result += val.to_json();
    }

    result += "}";
    return result;
}

std::vector<uint8_t> value_store::serialize_binary() const {
    // Always acquire lock to eliminate TOCTOU vulnerability (see #190)
    std::shared_lock lock(mutex_);
    return serialize_binary_impl();
}

std::vector<uint8_t> value_store::serialize_binary_impl() const {
    read_count_.fetch_add(1, std::memory_order_relaxed);

    std::vector<uint8_t> result;

    // Version byte for future compatibility
    constexpr uint8_t version = 1;
    result.push_back(version);

    // Header: number of entries (4 bytes)
    uint32_t count = static_cast<uint32_t>(values_.size());
    result.insert(result.end(),
                 reinterpret_cast<const uint8_t*>(&count),
                 reinterpret_cast<const uint8_t*>(&count) + sizeof(count));

    // Serialize each key-value pair
    for (const auto& [key, val] : values_) {
        // Key length and key
        uint32_t key_len = static_cast<uint32_t>(key.size());
        result.insert(result.end(),
                     reinterpret_cast<const uint8_t*>(&key_len),
                     reinterpret_cast<const uint8_t*>(&key_len) + sizeof(key_len));
        result.insert(result.end(), key.begin(), key.end());

        // Value serialization
        auto value_data = val.serialize();
        uint32_t value_len = static_cast<uint32_t>(value_data.size());
        result.insert(result.end(),
                     reinterpret_cast<const uint8_t*>(&value_len),
                     reinterpret_cast<const uint8_t*>(&value_len) + sizeof(value_len));
        result.insert(result.end(), value_data.begin(), value_data.end());
    }

    return result;
}

std::unique_ptr<value_store> value_store::deserialize(std::string_view json_data) {
#if HAS_NLOHMANN_JSON == 1
    auto store = std::make_unique<value_store>();
    deserialize_into(*store, json_data);
    return store;
#else
    (void)json_data;
    throw std::runtime_error(
        "value_store::deserialize() requires JSON parser - use deserialize_binary() instead");
#endif
}

void value_store::deserialize_into(value_store& store, std::string_view json_data) {
#if HAS_NLOHMANN_JSON == 1
    auto json_obj = nlohmann::json::parse(json_data);
    if (!json_obj.is_object()) {
        throw std::runtime_error(
            "value_store::deserialize() - top-level JSON value must be an object");
    }

    // Parse into a temporary map so a malformed input leaves the target unmodified
    std::unordered_map<std::string, value> parsed;
    for (auto it = json_obj.begin(); it != json_obj.end(); ++it) {
        parsed.emplace(it.key(), value_from_json(it.key(), it.value()));
    }

    // Commit parsed entries atomically once the whole document is validated
    std::unique_lock lock(store.mutex_);
    store.values_ = std::move(parsed);
#else
    (void)store;
    (void)json_data;
    throw std::runtime_error(
        "value_store::deserialize() requires JSON parser - use deserialize_binary() instead");
#endif
}

std::unique_ptr<value_store> value_store::deserialize_binary(const std::vector<uint8_t>& binary_data) {
    auto store = std::make_unique<value_store>();
    deserialize_binary_into(*store, binary_data);
    return store;
}

void value_store::deserialize_binary_into(value_store& store,
                                          const std::vector<uint8_t>& binary_data) {
    if (binary_data.size() < 1 + sizeof(uint32_t)) {
        throw std::runtime_error("value_store::deserialize_binary() - invalid data: too small");
    }

    size_t offset = 0;

    // Read version byte
    uint8_t version = binary_data[offset++];
    if (version != 1) {
        throw std::runtime_error("value_store::deserialize_binary() - unsupported version: "
                                + std::to_string(version));
    }

    // Read number of entries
    uint32_t count;
    std::memcpy(&count, binary_data.data() + offset, sizeof(count));
    offset += sizeof(count);

    // Parse into a temporary map so a malformed input leaves the target unmodified
    std::unordered_map<std::string, value> parsed;

    // Read each key-value pair
    for (uint32_t i = 0; i < count; ++i) {
        if (offset + sizeof(uint32_t) > binary_data.size()) {
            throw std::runtime_error("value_store::deserialize_binary() - truncated data at entry "
                                    + std::to_string(i));
        }

        // Read key length
        uint32_t key_len;
        std::memcpy(&key_len, binary_data.data() + offset, sizeof(key_len));
        offset += sizeof(key_len);

        if (offset + key_len + sizeof(uint32_t) > binary_data.size()) {
            throw std::runtime_error("value_store::deserialize_binary() - truncated key data");
        }

        // Read key
        std::string key(binary_data.begin() + offset,
                       binary_data.begin() + offset + key_len);
        offset += key_len;

        // Read value length
        uint32_t value_len;
        std::memcpy(&value_len, binary_data.data() + offset, sizeof(value_len));
        offset += sizeof(value_len);

        if (offset + value_len > binary_data.size()) {
            throw std::runtime_error("value_store::deserialize_binary() - truncated value data");
        }

        // Deserialize value
        std::vector<uint8_t> value_data(binary_data.begin() + offset,
                                        binary_data.begin() + offset + value_len);
        offset += value_len;

        auto value_opt = value::deserialize(value_data);
        if (value_opt) {
            parsed[key] = std::move(*value_opt);
        } else {
            throw std::runtime_error("value_store::deserialize_binary() - failed to deserialize value for key: "
                                    + key);
        }
    }

    // Commit parsed entries atomically once the whole buffer is validated
    std::unique_lock lock(store.mutex_);
    store.values_ = std::move(parsed);
}

size_t value_store::get_read_count() const {
    return read_count_.load(std::memory_order_relaxed);
}

size_t value_store::get_write_count() const {
    return write_count_.load(std::memory_order_relaxed);
}

void value_store::reset_statistics() {
    read_count_.store(0, std::memory_order_relaxed);
    write_count_.store(0, std::memory_order_relaxed);
}

#if CONTAINER_HAS_COMMON_RESULT
kcenon::common::Result<std::unique_ptr<value_store>>
value_store::deserialize_result(std::string_view json_data) noexcept {
#if HAS_NLOHMANN_JSON == 1
    try {
        auto store = std::make_unique<value_store>();
        deserialize_into(*store, json_data);
        return kcenon::common::ok(std::move(store));
    } catch (const std::bad_alloc&) {
        return kcenon::common::Result<std::unique_ptr<value_store>>(
            kcenon::common::error_info{
                error_codes::memory_allocation_failed,
                "value_store::deserialize_result() - memory allocation failed",
                "container_system"});
    } catch (const std::exception& e) {
        return kcenon::common::Result<std::unique_ptr<value_store>>(
            kcenon::common::error_info{
                error_codes::deserialization_failed,
                std::string("value_store::deserialize_result() - ") + e.what(),
                "container_system"});
    }
#else
    (void)json_data;
    return kcenon::common::Result<std::unique_ptr<value_store>>(
        kcenon::common::error_info{
            error_codes::deserialization_failed,
            "value_store::deserialize_result() requires JSON parser - use deserialize_binary_result() instead",
            "container_system"});
#endif
}

kcenon::common::Result<std::unique_ptr<value_store>>
value_store::deserialize_binary_result(const std::vector<uint8_t>& binary_data) noexcept {
    try {
        auto store = std::make_unique<value_store>();

        if (binary_data.size() < 1 + sizeof(uint32_t)) {
            return kcenon::common::Result<std::unique_ptr<value_store>>(
                kcenon::common::error_info{
                    error_codes::corrupted_data,
                    "value_store::deserialize_binary_result() - invalid data: too small",
                    "container_system"});
        }

        size_t offset = 0;

        uint8_t version = binary_data[offset++];
        if (version != 1) {
            return kcenon::common::Result<std::unique_ptr<value_store>>(
                kcenon::common::error_info{
                    error_codes::version_mismatch,
                    "value_store::deserialize_binary_result() - unsupported version: "
                        + std::to_string(version),
                    "container_system"});
        }

        uint32_t count;
        std::memcpy(&count, binary_data.data() + offset, sizeof(count));
        offset += sizeof(count);

        for (uint32_t i = 0; i < count; ++i) {
            if (offset + sizeof(uint32_t) > binary_data.size()) {
                return kcenon::common::Result<std::unique_ptr<value_store>>(
                    kcenon::common::error_info{
                        error_codes::corrupted_data,
                        "value_store::deserialize_binary_result() - truncated data at entry "
                            + std::to_string(i),
                        "container_system"});
            }

            uint32_t key_len;
            std::memcpy(&key_len, binary_data.data() + offset, sizeof(key_len));
            offset += sizeof(key_len);

            if (offset + key_len + sizeof(uint32_t) > binary_data.size()) {
                return kcenon::common::Result<std::unique_ptr<value_store>>(
                    kcenon::common::error_info{
                        error_codes::corrupted_data,
                        "value_store::deserialize_binary_result() - truncated key data",
                        "container_system"});
            }

            std::string key(binary_data.begin() + offset,
                           binary_data.begin() + offset + key_len);
            offset += key_len;

            uint32_t value_len;
            std::memcpy(&value_len, binary_data.data() + offset, sizeof(value_len));
            offset += sizeof(value_len);

            if (offset + value_len > binary_data.size()) {
                return kcenon::common::Result<std::unique_ptr<value_store>>(
                    kcenon::common::error_info{
                        error_codes::corrupted_data,
                        "value_store::deserialize_binary_result() - truncated value data",
                        "container_system"});
            }

            std::vector<uint8_t> value_data(binary_data.begin() + offset,
                                            binary_data.begin() + offset + value_len);
            offset += value_len;

            auto value_opt = value::deserialize(value_data);
            if (value_opt) {
                store->values_[key] = std::move(*value_opt);
            } else {
                return kcenon::common::Result<std::unique_ptr<value_store>>(
                    kcenon::common::error_info{
                        error_codes::value_parse_failed,
                        "value_store::deserialize_binary_result() - failed to deserialize value for key: "
                            + key,
                        "container_system"});
            }
        }

        return kcenon::common::ok(std::move(store));
    } catch (const std::bad_alloc&) {
        return kcenon::common::Result<std::unique_ptr<value_store>>(
            kcenon::common::error_info{
                error_codes::memory_allocation_failed,
                "value_store::deserialize_binary_result() - memory allocation failed",
                "container_system"});
    } catch (const std::exception& e) {
        return kcenon::common::Result<std::unique_ptr<value_store>>(
            kcenon::common::error_info{
                error_codes::deserialization_failed,
                std::string("value_store::deserialize_binary_result() - unexpected error: ") + e.what(),
                "container_system"});
    }
}
#endif

} // namespace kcenon::container
