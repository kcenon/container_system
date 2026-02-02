/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
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

/**
 * @file core/policy_container.h
 * @brief Policy-based container template for unified value storage (Issue #327)
 *
 * This header provides the `basic_value_container` template that uses storage
 * policies for customizable storage behavior. This unifies the previously
 * separate container variants (value_container, optimized_container).
 *
 * @code
 * // Default container (dynamic storage)
 * basic_value_container<> container;
 *
 * // Fast lookup container (indexed storage)
 * basic_value_container<indexed_storage_policy> fast_container;
 *
 * // Type-restricted container (static storage)
 * basic_value_container<static_storage_policy<int, double, std::string>> typed;
 * @endcode
 *
 * @see Issue #320: Consolidate container variants into unified template
 * @see Issue #327: Create unified value_container template with policy support
 */

#pragma once

#include "storage_policy.h"
#include "container/types.h"
#include "container/error_codes.h"
#include "value_types.h"

// Unified Result<T> integration (Issue #335)
#include "container/result_integration.h"

#include <memory>
#include <mutex>
#include <string_view>
#include <shared_mutex>
#include <optional>
#include <span>
#include <type_traits>

namespace container_module {

/**
 * @class basic_value_container
 * @brief Policy-based container template for unified value storage
 *
 * This template provides a container for key-value storage with customizable
 * storage behavior through the StoragePolicy template parameter.
 *
 * @tparam StoragePolicy The storage policy to use (must satisfy StoragePolicy concept)
 *
 * Storage Policies:
 * - dynamic_storage_policy: O(n) lookup, preserves insertion order (default)
 * - indexed_storage_policy: O(1) lookup, hash-indexed
 * - static_storage_policy<Types...>: Compile-time type restriction
 *
 * Thread Safety:
 * - All public methods are thread-safe (use shared_mutex internally)
 * - Read operations use shared locks (concurrent reads allowed)
 * - Write operations use exclusive locks
 */
template<typename StoragePolicy = policy::dynamic_storage_policy>
    requires policy::StoragePolicy<StoragePolicy>
class basic_value_container : public std::enable_shared_from_this<basic_value_container<StoragePolicy>> {
public:
    using storage_policy_type = StoragePolicy;
    using value_type = typename StoragePolicy::value_type;
    using iterator = typename StoragePolicy::iterator;
    using const_iterator = typename StoragePolicy::const_iterator;

    /**
     * @brief Default constructor
     */
    basic_value_container() = default;

    /**
     * @brief Copy constructor
     * @param other Container to copy from
     */
    basic_value_container(const basic_value_container& other) {
        std::shared_lock lock(other.mutex_);
        storage_ = other.storage_;
        source_id_ = other.source_id_;
        source_sub_id_ = other.source_sub_id_;
        target_id_ = other.target_id_;
        target_sub_id_ = other.target_sub_id_;
        message_type_ = other.message_type_;
        version_ = other.version_;
    }

    /**
     * @brief Copy assignment operator
     * @param other Container to copy from
     * @return Reference to this container
     */
    basic_value_container& operator=(const basic_value_container& other) {
        if (this != &other) {
            std::scoped_lock lock(mutex_, other.mutex_);
            storage_ = other.storage_;
            source_id_ = other.source_id_;
            source_sub_id_ = other.source_sub_id_;
            target_id_ = other.target_id_;
            target_sub_id_ = other.target_sub_id_;
            message_type_ = other.message_type_;
            version_ = other.version_;
        }
        return *this;
    }

    /**
     * @brief Move constructor
     * @param other Container to move from
     */
    basic_value_container(basic_value_container&& other) noexcept {
        std::unique_lock lock(other.mutex_);
        storage_ = std::move(other.storage_);
        source_id_ = std::move(other.source_id_);
        source_sub_id_ = std::move(other.source_sub_id_);
        target_id_ = std::move(other.target_id_);
        target_sub_id_ = std::move(other.target_sub_id_);
        message_type_ = std::move(other.message_type_);
        version_ = std::move(other.version_);
    }

    /**
     * @brief Move assignment operator
     * @param other Container to move from
     * @return Reference to this container
     */
    basic_value_container& operator=(basic_value_container&& other) noexcept {
        if (this != &other) {
            std::scoped_lock lock(mutex_, other.mutex_);
            storage_ = std::move(other.storage_);
            source_id_ = std::move(other.source_id_);
            source_sub_id_ = std::move(other.source_sub_id_);
            target_id_ = std::move(other.target_id_);
            target_sub_id_ = std::move(other.target_sub_id_);
            message_type_ = std::move(other.message_type_);
            version_ = std::move(other.version_);
        }
        return *this;
    }

    virtual ~basic_value_container() = default;

    /**
     * @brief Get shared pointer to this container
     * @return shared_ptr to this container
     */
    std::shared_ptr<basic_value_container> get_ptr() {
        return this->shared_from_this();
    }

    // =========================================================================
    // Header Management
    // =========================================================================

    /**
     * @brief Set source IDs
     * @param source_id Source identifier
     * @param source_sub_id Source sub-identifier
     */
    void set_source(std::string_view source_id, std::string_view source_sub_id) {
        std::unique_lock lock(mutex_);
        source_id_ = std::string(source_id);
        source_sub_id_ = std::string(source_sub_id);
    }

    /**
     * @brief Set target IDs
     * @param target_id Target identifier
     * @param target_sub_id Target sub-identifier (optional)
     */
    void set_target(std::string_view target_id, std::string_view target_sub_id = "") {
        std::unique_lock lock(mutex_);
        target_id_ = std::string(target_id);
        target_sub_id_ = std::string(target_sub_id);
    }

    /**
     * @brief Set message type
     * @param message_type Message type identifier
     */
    void set_message_type(std::string_view message_type) {
        std::unique_lock lock(mutex_);
        message_type_ = std::string(message_type);
    }

    /**
     * @brief Swap source and target IDs
     */
    void swap_header() {
        std::unique_lock lock(mutex_);
        std::swap(source_id_, target_id_);
        std::swap(source_sub_id_, target_sub_id_);
    }

    // Header accessors (thread-safe)
    [[nodiscard]] std::string source_id() const {
        std::shared_lock lock(mutex_);
        return source_id_;
    }

    [[nodiscard]] std::string source_sub_id() const {
        std::shared_lock lock(mutex_);
        return source_sub_id_;
    }

    [[nodiscard]] std::string target_id() const {
        std::shared_lock lock(mutex_);
        return target_id_;
    }

    [[nodiscard]] std::string target_sub_id() const {
        std::shared_lock lock(mutex_);
        return target_sub_id_;
    }

    [[nodiscard]] std::string message_type() const {
        std::shared_lock lock(mutex_);
        return message_type_;
    }

    [[nodiscard]] std::string version() const {
        std::shared_lock lock(mutex_);
        return version_;
    }

    // =========================================================================
    // Value Management (Unified API)
    // =========================================================================

    /**
     * @brief Set a typed value by key
     * @tparam T The value type
     * @param key Value identifier
     * @param data_val Value to store
     * @return Reference to this container for chaining
     */
    template<typename T>
    basic_value_container& set(std::string_view key, T&& data_val) {
        optimized_value val;
        val.name = std::string(key);
        val.data = std::forward<T>(data_val);
        val.type = static_cast<value_types>(val.data.index());

        std::unique_lock lock(mutex_);
        storage_.set(key, std::move(val));
        return *this;
    }

    /**
     * @brief Set a single optimized_value
     * @param val The value to set
     * @return Reference to this container for chaining
     */
    basic_value_container& set(const optimized_value& val) {
        std::unique_lock lock(mutex_);
        storage_.set(val.name, val);
        return *this;
    }

    /**
     * @brief Set multiple values at once
     * @param vals Span of values to set
     * @return Reference to this container for chaining
     */
    basic_value_container& set_all(std::span<const optimized_value> vals) {
        std::unique_lock lock(mutex_);
        for (const auto& val : vals) {
            storage_.set(val.name, val);
        }
        return *this;
    }

    /**
     * @brief Get a value by key
     * @param key Value identifier
     * @return Optional containing the value if found
     */
    [[nodiscard]] std::optional<optimized_value> get(std::string_view key) const noexcept {
        std::shared_lock lock(mutex_);
        return storage_.get(key);
    }

    /**
     * @brief Check if a key exists
     * @param key Value identifier
     * @return true if key exists
     */
    [[nodiscard]] bool contains(std::string_view key) const noexcept {
        std::shared_lock lock(mutex_);
        return storage_.contains(key);
    }

    /**
     * @brief Remove a value by key
     * @param key Value identifier
     * @return true if a value was removed
     */
    bool remove(std::string_view key) {
        std::unique_lock lock(mutex_);
        return storage_.remove(key);
    }

    /**
     * @brief Clear all values
     */
    void clear_value() {
        std::unique_lock lock(mutex_);
        storage_.clear();
    }

    // =========================================================================
    // Result-based API (when common_system is available)
    // =========================================================================

#if CONTAINER_HAS_RESULT
    /**
     * @brief Get a typed value with Result return type
     * @tparam T Expected value type
     * @param key Value identifier
     * @return Result containing value or error
     */
    template<typename T>
    [[nodiscard]] kcenon::common::Result<T> get(std::string_view key) const noexcept {
        std::shared_lock lock(mutex_);
        auto result = storage_.get(key);
        if (!result) {
            return kcenon::common::Result<T>(
                kcenon::common::error_info{
                    error_codes::key_not_found,
                    error_codes::make_message(error_codes::key_not_found, key),
                    "container_system"});
        }
        if (auto* ptr = std::get_if<T>(&result->data)) {
            return kcenon::common::ok(*ptr);
        }
        return kcenon::common::Result<T>(
            kcenon::common::error_info{
                error_codes::type_mismatch,
                error_codes::make_message(error_codes::type_mismatch, key),
                "container_system"});
    }

    /**
     * @brief Set a value with Result return type
     * @tparam T Value type
     * @param key Value identifier
     * @param data_val Value to store
     * @return VoidResult indicating success or error
     */
    template<typename T>
    [[nodiscard]] kcenon::common::VoidResult set_result(std::string_view key, T&& data_val) noexcept {
        try {
            if (key.empty()) {
                return kcenon::common::VoidResult(
                    kcenon::common::error_info{
                        error_codes::empty_key,
                        error_codes::make_message(error_codes::empty_key),
                        "container_system"});
            }

            optimized_value val;
            val.name = std::string(key);
            val.data = std::forward<T>(data_val);
            val.type = static_cast<value_types>(val.data.index());

            std::unique_lock lock(mutex_);
            storage_.set(key, std::move(val));
            return kcenon::common::ok();
        } catch (const std::bad_alloc&) {
            return kcenon::common::VoidResult(
                kcenon::common::error_info{
                    error_codes::memory_allocation_failed,
                    error_codes::make_message(error_codes::memory_allocation_failed, key),
                    "container_system"});
        } catch (const std::exception& e) {
            return kcenon::common::VoidResult(
                kcenon::common::error_info{
                    error_codes::invalid_value,
                    std::string("Failed to set value: ") + e.what(),
                    "container_system"});
        }
    }
#endif

    // =========================================================================
    // Batch Operations
    // =========================================================================

    /**
     * @brief Bulk insert values (move semantics)
     * @param values Vector of values to insert
     * @return Reference to this container for chaining
     */
    basic_value_container& bulk_insert(std::vector<optimized_value>&& values) {
        std::unique_lock lock(mutex_);
        for (auto& val : values) {
            storage_.set(val.name, std::move(val));
        }
        return *this;
    }

    /**
     * @brief Bulk insert values
     * @param values Span of values to insert
     * @param reserve_hint Hint for pre-allocating storage
     * @return Reference to this container for chaining
     */
    basic_value_container& bulk_insert(std::span<const optimized_value> values,
                                        size_t reserve_hint = 0) {
        std::unique_lock lock(mutex_);
        if (reserve_hint > 0) {
            storage_.reserve(reserve_hint);
        }
        for (const auto& val : values) {
            storage_.set(val.name, val);
        }
        return *this;
    }

    /**
     * @brief Get multiple values at once
     * @param keys Span of keys to retrieve
     * @return Vector of optional values in same order as keys
     */
    [[nodiscard]] std::vector<std::optional<optimized_value>> get_batch(
        std::span<const std::string_view> keys) const noexcept {
        std::shared_lock lock(mutex_);
        std::vector<std::optional<optimized_value>> results;
        results.reserve(keys.size());
        for (const auto& key : keys) {
            results.push_back(storage_.get(key));
        }
        return results;
    }

    /**
     * @brief Check multiple keys existence
     * @param keys Span of keys to check
     * @return Vector of booleans indicating existence
     */
    [[nodiscard]] std::vector<bool> contains_batch(
        std::span<const std::string_view> keys) const noexcept {
        std::shared_lock lock(mutex_);
        std::vector<bool> results;
        results.reserve(keys.size());
        for (const auto& key : keys) {
            results.push_back(storage_.contains(key));
        }
        return results;
    }

    /**
     * @brief Remove multiple keys
     * @param keys Span of keys to remove
     * @return Number of keys actually removed
     */
    size_t remove_batch(std::span<const std::string_view> keys) {
        std::unique_lock lock(mutex_);
        size_t count = 0;
        for (const auto& key : keys) {
            if (storage_.remove(key)) {
                ++count;
            }
        }
        return count;
    }

    // =========================================================================
    // Iterator Support (STL-compatible)
    // =========================================================================

    /**
     * @brief Get iterator to beginning
     * @warning Not thread-safe for concurrent modification
     */
    iterator begin() { return storage_.begin(); }
    iterator end() { return storage_.end(); }

    /**
     * @brief Get const iterator to beginning
     * @warning Not thread-safe for concurrent modification
     */
    [[nodiscard]] const_iterator begin() const { return storage_.begin(); }
    [[nodiscard]] const_iterator end() const { return storage_.end(); }
    [[nodiscard]] const_iterator cbegin() const { return storage_.begin(); }
    [[nodiscard]] const_iterator cend() const { return storage_.end(); }

    /**
     * @brief Get number of values
     */
    [[nodiscard]] size_t size() const noexcept {
        std::shared_lock lock(mutex_);
        return storage_.size();
    }

    /**
     * @brief Check if container is empty
     */
    [[nodiscard]] bool empty() const noexcept {
        std::shared_lock lock(mutex_);
        return storage_.empty();
    }

    // =========================================================================
    // Storage Policy Access
    // =========================================================================

    /**
     * @brief Get direct access to storage policy
     * @return Reference to storage policy
     * @warning Not thread-safe - use with caution
     */
    StoragePolicy& storage() { return storage_; }

    /**
     * @brief Get const access to storage policy
     * @return Const reference to storage policy
     */
    [[nodiscard]] const StoragePolicy& storage() const { return storage_; }

    /**
     * @brief Get direct access to underlying data
     * @return Reference to data container
     * @warning Not thread-safe - use with caution
     */
    auto& data() { return storage_.data(); }

    /**
     * @brief Get const access to underlying data
     * @return Const reference to data container
     */
    [[nodiscard]] const auto& data() const { return storage_.data(); }

    /**
     * @brief Reserve storage capacity
     * @param capacity Number of values to reserve space for
     */
    void reserve(size_t capacity) {
        std::unique_lock lock(mutex_);
        storage_.reserve(capacity);
    }

protected:
    StoragePolicy storage_;
    mutable std::shared_mutex mutex_;

    // Header fields
    std::string source_id_;
    std::string source_sub_id_;
    std::string target_id_;
    std::string target_sub_id_;
    std::string message_type_;
    std::string version_{"1.0.0.0"};
};

// =============================================================================
// Type Aliases for Common Use Cases
// =============================================================================

/**
 * @brief Default container using dynamic storage (backward compatible)
 *
 * This is the recommended container for most use cases.
 * Provides O(n) lookup but preserves insertion order.
 */
using policy_container = basic_value_container<policy::dynamic_storage_policy>;

/**
 * @brief Fast lookup container using indexed storage
 *
 * Provides O(1) average lookup time using hash map index.
 * Best for containers with many values (100+) and frequent lookups.
 */
using fast_policy_container = basic_value_container<policy::indexed_storage_policy>;

/**
 * @brief Type-restricted container template
 *
 * Provides compile-time type safety by restricting allowed value types.
 *
 * @code
 * typed_policy_container<int, double, std::string> container;
 * container.set("count", 42);      // OK
 * container.set("rate", 3.14);     // OK
 * container.set("flag", true);     // Silently ignored (bool not allowed)
 * @endcode
 */
template<typename... AllowedTypes>
using typed_policy_container = basic_value_container<policy::static_storage_policy<AllowedTypes...>>;

// =============================================================================
// Concept verification
// =============================================================================

static_assert(policy::StoragePolicy<policy::dynamic_storage_policy>,
    "dynamic_storage_policy must satisfy StoragePolicy concept");
static_assert(policy::StoragePolicy<policy::indexed_storage_policy>,
    "indexed_storage_policy must satisfy StoragePolicy concept");

} // namespace container_module
