// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

#pragma once

#include "internal/value.h"

#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <shared_mutex>
#include <atomic>
#include <unordered_map>

namespace kcenon::container {

/**
 * @brief Domain-agnostic value storage
 *
 * Pure value storage layer without messaging-specific fields.
 * Can be used as a general-purpose serialization container.
 *
 * Features:
 * - Type-safe variant-based storage (value)
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
     * @note All operations are thread-safe (always enabled since v0.2.0)
     */
    virtual void add(const std::string& key, value val);

    /**
     * @brief Get a value by key
     * @param key The key to look up
     * @return Optional variant value
     * @note All operations are thread-safe (always enabled since v0.2.0)
     */
    virtual std::optional<value> get(const std::string& key) const;

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
     * @return unique_ptr to value_store instance (nullptr on failure)
     * @throws std::runtime_error if deserialization fails
     */
    static std::unique_ptr<value_store> deserialize(std::string_view json_data);

    /**
     * @brief Deserialize from binary format
     * @param binary_data Binary data
     * @return unique_ptr to value_store instance
     * @throws std::runtime_error if deserialization fails
     */
    static std::unique_ptr<value_store> deserialize_binary(const std::vector<uint8_t>& binary_data);

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
    // Key-value storage using value
    std::unordered_map<std::string, value> values_;

private:
    // Internal serialization helpers (lock-free, called with lock held)
    std::string serialize_impl() const;
    std::vector<uint8_t> serialize_binary_impl() const;

    // Thread safety
    // Note: Lock is always acquired to prevent TOCTOU vulnerability (see #190)
    mutable std::shared_mutex mutex_;

    // Statistics
    mutable std::atomic<size_t> read_count_{0};
    mutable std::atomic<size_t> write_count_{0};
};

} // namespace kcenon::container
