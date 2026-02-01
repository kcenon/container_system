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

#include "core/value_store.h"
#include <stdexcept>
#include <cstring>

namespace container_module {

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

std::unique_ptr<value_store> value_store::deserialize(std::string_view /*json_data*/) {
    // JSON deserialization requires a JSON parser library
    // For now, use serialize_binary/deserialize_binary for round-trip serialization
    throw std::runtime_error(
        "value_store::deserialize() requires JSON parser - use deserialize_binary() instead");
}

std::unique_ptr<value_store> value_store::deserialize_binary(const std::vector<uint8_t>& binary_data) {
    auto store = std::make_unique<value_store>();

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
            store->values_[key] = std::move(*value_opt);
        } else {
            throw std::runtime_error("value_store::deserialize_binary() - failed to deserialize value for key: "
                                    + key);
        }
    }

    return store;
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

} // namespace container_module
