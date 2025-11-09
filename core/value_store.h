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

#pragma once

#include "container/core/value.h"
#include "container/core/optimized_value.h"
#include "container/internal/variant_value_v2.h"

#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <shared_mutex>
#include <atomic>
#include <unordered_map>

namespace container_module {

/**
 * @brief Domain-agnostic value storage
 *
 * Pure value storage layer without messaging-specific fields.
 * Can be used as a general-purpose serialization container.
 *
 * Features:
 * - Type-safe variant-based storage (variant_value_v2)
 * - Small Object Optimization (SOO) for performance
 * - JSON/Binary serialization support
 * - Thread-safe operations (optional)
 * - Key-value storage interface
 *
 * @note This class is part of Sprint 3: Domain Separation initiative
 * @see message_container for messaging-specific wrapper
 */
class value_store {
public:
    /**
     * @brief Default constructor
     */
    value_store() = default;

    /**
     * @brief Destructor
     */
    virtual ~value_store() = default;

    // Copy and move semantics (deleted due to mutex member)
    value_store(const value_store&) = delete;
    value_store(value_store&&) = delete;
    value_store& operator=(const value_store&) = delete;
    value_store& operator=(value_store&&) = delete;

    // =========================================================================
    // Value Management
    // =========================================================================

    /**
     * @brief Add a value with a key
     * @param key The key for the value
     * @param val The variant value to add
     * @note Thread-safe if enable_thread_safety() was called
     */
    virtual void add(const std::string& key, variant_value_v2 val);

    /**
     * @brief Get a value by key
     * @param key The key to look up
     * @return Optional variant value
     * @note Thread-safe if enable_thread_safety() was called
     */
    virtual std::optional<variant_value_v2> get(const std::string& key) const;

    /**
     * @brief Check if a key exists
     * @param key The key to check
     * @return true if key exists
     */
    virtual bool contains(const std::string& key) const;

    /**
     * @brief Remove a value by key
     * @param key The key to remove
     * @return true if removed, false if not found
     */
    virtual bool remove(const std::string& key);

    /**
     * @brief Clear all values
     */
    virtual void clear();

    /**
     * @brief Get number of stored values
     * @return Number of values
     */
    virtual size_t size() const;

    /**
     * @brief Check if store is empty
     * @return true if empty
     */
    virtual bool empty() const;

    // =========================================================================
    // Serialization
    // =========================================================================

    /**
     * @brief Serialize to JSON string
     * @return JSON representation
     * @throws std::runtime_error if serialization fails
     */
    virtual std::string serialize() const;

    /**
     * @brief Serialize to binary format
     * @return Binary data
     * @throws std::runtime_error if serialization fails
     */
    virtual std::vector<uint8_t> serialize_binary() const;

    /**
     * @brief Deserialize from JSON string
     * @param json_data JSON string
     * @return value_store instance
     * @throws std::runtime_error if deserialization fails
     */
    static value_store deserialize(std::string_view json_data);

    /**
     * @brief Deserialize from binary format
     * @param binary_data Binary data
     * @return value_store instance
     * @throws std::runtime_error if deserialization fails
     */
    static value_store deserialize_binary(const std::vector<uint8_t>& binary_data);

    // =========================================================================
    // Thread Safety
    // =========================================================================

    /**
     * @brief Enable thread-safe operations
     * @note All subsequent operations will be protected by mutex
     */
    void enable_thread_safety();

    /**
     * @brief Disable thread-safe operations
     * @note Use only if you guarantee single-threaded access
     */
    void disable_thread_safety();

    /**
     * @brief Check if thread safety is enabled
     * @return true if enabled
     */
    bool is_thread_safe() const;

    // =========================================================================
    // Statistics (Optional)
    // =========================================================================

    /**
     * @brief Get number of read operations
     * @return Read count
     */
    size_t get_read_count() const;

    /**
     * @brief Get number of write operations
     * @return Write count
     */
    size_t get_write_count() const;

    /**
     * @brief Reset statistics
     */
    void reset_statistics();

protected:
    // Key-value storage using variant_value_v2
    std::unordered_map<std::string, variant_value_v2> values_;

    // Thread safety
    mutable std::shared_mutex mutex_;
    std::atomic<bool> thread_safe_enabled_{false};

    // Statistics
    mutable std::atomic<size_t> read_count_{0};
    mutable std::atomic<size_t> write_count_{0};
};

} // namespace container_module
