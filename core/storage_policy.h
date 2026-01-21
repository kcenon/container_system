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

/**
 * @file core/storage_policy.h
 * @brief Storage policy interfaces for unified container architecture
 *
 * This header defines the StoragePolicy concept and concrete policy
 * implementations for the policy-based container design (Issue #320).
 *
 * Storage policies control how values are stored and retrieved:
 * - dynamic_storage_policy: Variant-based storage (current container behavior)
 * - indexed_storage_policy: Hash-indexed storage for O(1) lookup
 *
 * @see Issue #320: Consolidate container variants into unified template
 * @see Issue #322: Design storage policy interfaces
 */

#pragma once

#include "container/core/container/types.h"
#include "container/core/value_types.h"

#include <concepts>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstddef>

namespace container_module::policy {

/**
 * @concept StoragePolicy
 * @brief Defines the interface contract for container storage strategies.
 *
 * A storage policy must provide:
 * - value_type: The type of values stored
 * - iterator/const_iterator: Iterator types for traversal
 * - CRUD operations: set, get, contains, remove, clear
 * - Iteration support: begin, end, size, empty
 *
 * Example implementation:
 * @code
 * struct my_storage_policy {
 *     using value_type = optimized_value;
 *     using iterator = std::vector<value_type>::iterator;
 *     using const_iterator = std::vector<value_type>::const_iterator;
 *
 *     void set(std::string_view key, const value_type& value);
 *     std::optional<value_type> get(std::string_view key) const;
 *     // ... other required operations
 * };
 * static_assert(StoragePolicy<my_storage_policy>);
 * @endcode
 */
template<typename P>
concept StoragePolicy = requires(P p, const P cp, std::string_view key) {
    // Type requirements
    typename P::value_type;
    typename P::iterator;
    typename P::const_iterator;

    // CRUD operations
    { p.set(key, std::declval<const typename P::value_type&>()) } -> std::same_as<void>;
    { cp.get(key) } -> std::same_as<std::optional<typename P::value_type>>;
    { cp.contains(key) } -> std::convertible_to<bool>;
    { p.remove(key) } -> std::convertible_to<bool>;
    { p.clear() } -> std::same_as<void>;

    // Iteration support
    { p.begin() } -> std::same_as<typename P::iterator>;
    { p.end() } -> std::same_as<typename P::iterator>;
    { cp.begin() } -> std::same_as<typename P::const_iterator>;
    { cp.end() } -> std::same_as<typename P::const_iterator>;
    { cp.size() } -> std::convertible_to<std::size_t>;
    { cp.empty() } -> std::convertible_to<bool>;
};

/**
 * @class dynamic_storage_policy
 * @brief Default storage policy using variant-based storage with vector backend.
 *
 * This policy provides:
 * - O(n) lookup by key (linear search)
 * - Stack allocation for primitive types via value_variant
 * - Insertion-order preservation
 * - Memory efficiency for small containers
 *
 * Best suited for:
 * - Containers with fewer than ~100 values
 * - Scenarios where iteration order matters
 * - General-purpose use
 *
 * @note This matches the current value_container storage behavior.
 */
class dynamic_storage_policy {
public:
    using value_type = optimized_value;
    using container_type = std::vector<value_type>;
    using iterator = container_type::iterator;
    using const_iterator = container_type::const_iterator;

    dynamic_storage_policy() = default;
    ~dynamic_storage_policy() = default;

    dynamic_storage_policy(const dynamic_storage_policy&) = default;
    dynamic_storage_policy& operator=(const dynamic_storage_policy&) = default;
    dynamic_storage_policy(dynamic_storage_policy&&) noexcept = default;
    dynamic_storage_policy& operator=(dynamic_storage_policy&&) noexcept = default;

    /**
     * @brief Set or update a value by key
     * @param key The value identifier
     * @param value The value to store
     *
     * If a value with the same key exists, it is replaced.
     * Otherwise, the value is appended.
     */
    void set(std::string_view key, const value_type& value) {
        auto it = find_by_key(key);
        if (it != values_.end()) {
            *it = value;
        } else {
            values_.push_back(value);
        }
    }

    /**
     * @brief Set or update a value by key (move semantics)
     * @param key The value identifier
     * @param value The value to store (moved)
     */
    void set(std::string_view key, value_type&& value) {
        auto it = find_by_key(key);
        if (it != values_.end()) {
            *it = std::move(value);
        } else {
            values_.push_back(std::move(value));
        }
    }

    /**
     * @brief Get a value by key
     * @param key The value identifier
     * @return Optional containing the value if found, nullopt otherwise
     */
    [[nodiscard]] std::optional<value_type> get(std::string_view key) const {
        auto it = find_by_key(key);
        if (it != values_.end()) {
            return *it;
        }
        return std::nullopt;
    }

    /**
     * @brief Check if a key exists
     * @param key The value identifier
     * @return true if the key exists, false otherwise
     */
    [[nodiscard]] bool contains(std::string_view key) const {
        return find_by_key(key) != values_.end();
    }

    /**
     * @brief Remove a value by key
     * @param key The value identifier
     * @return true if a value was removed, false if key not found
     */
    bool remove(std::string_view key) {
        auto it = find_by_key(key);
        if (it != values_.end()) {
            values_.erase(it);
            return true;
        }
        return false;
    }

    /**
     * @brief Clear all values
     */
    void clear() {
        values_.clear();
    }

    /**
     * @brief Get iterator to the beginning
     */
    iterator begin() { return values_.begin(); }

    /**
     * @brief Get iterator to the end
     */
    iterator end() { return values_.end(); }

    /**
     * @brief Get const iterator to the beginning
     */
    [[nodiscard]] const_iterator begin() const { return values_.begin(); }

    /**
     * @brief Get const iterator to the end
     */
    [[nodiscard]] const_iterator end() const { return values_.end(); }

    /**
     * @brief Get the number of values
     */
    [[nodiscard]] std::size_t size() const { return values_.size(); }

    /**
     * @brief Check if the storage is empty
     */
    [[nodiscard]] bool empty() const { return values_.empty(); }

    /**
     * @brief Reserve capacity for expected number of values
     * @param capacity Number of values to reserve space for
     */
    void reserve(std::size_t capacity) {
        values_.reserve(capacity);
    }

    /**
     * @brief Get direct access to underlying container (for compatibility)
     * @return Reference to the internal vector
     */
    container_type& data() { return values_; }

    /**
     * @brief Get const access to underlying container
     * @return Const reference to the internal vector
     */
    [[nodiscard]] const container_type& data() const { return values_; }

private:
    container_type values_;

    [[nodiscard]] iterator find_by_key(std::string_view key) {
        return std::find_if(values_.begin(), values_.end(),
            [key](const value_type& v) { return v.name == key; });
    }

    [[nodiscard]] const_iterator find_by_key(std::string_view key) const {
        return std::find_if(values_.begin(), values_.end(),
            [key](const value_type& v) { return v.name == key; });
    }
};

/**
 * @class indexed_storage_policy
 * @brief Hash-indexed storage policy for O(1) lookup performance.
 *
 * This policy provides:
 * - O(1) average lookup by key (hash map)
 * - O(n) iteration (maintains vector for ordering)
 * - Additional memory overhead for index
 *
 * Best suited for:
 * - Containers with many values (100+)
 * - Frequent key-based lookups
 * - Scenarios where lookup performance is critical
 *
 * @note This corresponds to optimized_container behavior.
 */
class indexed_storage_policy {
public:
    using value_type = optimized_value;
    using container_type = std::vector<value_type>;
    using index_type = std::unordered_map<std::string, std::size_t>;
    using iterator = container_type::iterator;
    using const_iterator = container_type::const_iterator;

    indexed_storage_policy() = default;
    ~indexed_storage_policy() = default;

    indexed_storage_policy(const indexed_storage_policy& other)
        : values_(other.values_), index_(other.index_) {}

    indexed_storage_policy& operator=(const indexed_storage_policy& other) {
        if (this != &other) {
            values_ = other.values_;
            index_ = other.index_;
        }
        return *this;
    }

    indexed_storage_policy(indexed_storage_policy&& other) noexcept
        : values_(std::move(other.values_)), index_(std::move(other.index_)) {}

    indexed_storage_policy& operator=(indexed_storage_policy&& other) noexcept {
        if (this != &other) {
            values_ = std::move(other.values_);
            index_ = std::move(other.index_);
        }
        return *this;
    }

    /**
     * @brief Set or update a value by key
     * @param key The value identifier
     * @param value The value to store
     */
    void set(std::string_view key, const value_type& value) {
        std::string key_str(key);
        auto it = index_.find(key_str);
        if (it != index_.end()) {
            values_[it->second] = value;
        } else {
            index_[key_str] = values_.size();
            values_.push_back(value);
        }
    }

    /**
     * @brief Set or update a value by key (move semantics)
     * @param key The value identifier
     * @param value The value to store (moved)
     */
    void set(std::string_view key, value_type&& value) {
        std::string key_str(key);
        auto it = index_.find(key_str);
        if (it != index_.end()) {
            values_[it->second] = std::move(value);
        } else {
            index_[key_str] = values_.size();
            values_.push_back(std::move(value));
        }
    }

    /**
     * @brief Get a value by key (O(1) average)
     * @param key The value identifier
     * @return Optional containing the value if found, nullopt otherwise
     */
    [[nodiscard]] std::optional<value_type> get(std::string_view key) const {
        auto it = index_.find(std::string(key));
        if (it != index_.end()) {
            return values_[it->second];
        }
        return std::nullopt;
    }

    /**
     * @brief Check if a key exists (O(1) average)
     * @param key The value identifier
     * @return true if the key exists, false otherwise
     */
    [[nodiscard]] bool contains(std::string_view key) const {
        return index_.find(std::string(key)) != index_.end();
    }

    /**
     * @brief Remove a value by key
     * @param key The value identifier
     * @return true if a value was removed, false if key not found
     *
     * @note This operation is O(n) as it requires rebuilding the index.
     */
    bool remove(std::string_view key) {
        std::string key_str(key);
        auto it = index_.find(key_str);
        if (it == index_.end()) {
            return false;
        }

        std::size_t idx = it->second;
        values_.erase(values_.begin() + static_cast<std::ptrdiff_t>(idx));
        index_.erase(it);

        // Rebuild index for elements after the removed one
        for (auto& [k, v] : index_) {
            if (v > idx) {
                --v;
            }
        }
        return true;
    }

    /**
     * @brief Clear all values and index
     */
    void clear() {
        values_.clear();
        index_.clear();
    }

    iterator begin() { return values_.begin(); }
    iterator end() { return values_.end(); }
    [[nodiscard]] const_iterator begin() const { return values_.begin(); }
    [[nodiscard]] const_iterator end() const { return values_.end(); }
    [[nodiscard]] std::size_t size() const { return values_.size(); }
    [[nodiscard]] bool empty() const { return values_.empty(); }

    /**
     * @brief Reserve capacity for expected number of values
     * @param capacity Number of values to reserve space for
     */
    void reserve(std::size_t capacity) {
        values_.reserve(capacity);
        index_.reserve(capacity);
    }

    /**
     * @brief Rebuild the index from current values
     *
     * Call this after bulk modifications to ensure index consistency.
     */
    void rebuild_index() {
        index_.clear();
        index_.reserve(values_.size());
        for (std::size_t i = 0; i < values_.size(); ++i) {
            index_[values_[i].name] = i;
        }
    }

    container_type& data() { return values_; }
    [[nodiscard]] const container_type& data() const { return values_; }

private:
    container_type values_;
    index_type index_;
};

// Static assertions to verify concepts are satisfied
static_assert(StoragePolicy<dynamic_storage_policy>,
    "dynamic_storage_policy must satisfy StoragePolicy concept");
static_assert(StoragePolicy<indexed_storage_policy>,
    "indexed_storage_policy must satisfy StoragePolicy concept");

} // namespace container_module::policy
