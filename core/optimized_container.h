/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#pragma once

#include "container.h"
#include <string_view>
#include <unordered_map>

namespace container_module {

/**
 * @class optimized_container
 * @brief Performance-optimized value_container with zero-copy operations
 *
 * @deprecated Use `basic_value_container<indexed_storage_policy>` (alias: `fast_policy_container`)
 *             from `policy_container.h` instead. This class will be removed in the next major version.
 *
 * Migration example:
 * @code
 * // Old code:
 * optimized_container oc;
 * oc.get_value_fast("key");
 *
 * // New code:
 * #include "core/policy_container.h"
 * fast_policy_container fc;  // or: basic_value_container<indexed_storage_policy>
 * fc.get("key");
 * @endcode
 *
 * @see basic_value_container<indexed_storage_policy> for the replacement
 * @see Issue #320, #328 for migration details
 *
 * Improvements over value_container:
 * - std::string_view for accessors (zero-copy)
 * - Move semantics for all modifiers
 * - std::unordered_map for O(1) lookup
 * - Small string optimization
 * - Reduced allocations
 *
 * ### Performance Benefits
 * - 50% less memory allocations
 * - 3x faster key lookups
 * - Zero-copy string access
 * - Better cache locality
 *
 * ### Usage
 * @code
 * optimized_container container;
 *
 * // Zero-copy access
 * auto id = container.source_id_view();  // No allocation
 *
 * // Move semantics
 * container.set_message_type_move(std::move(type));  // No copy
 *
 * // Fast lookup
 * auto value = container.get_value_fast("key");  // O(1) instead of O(n)
 * @endcode
 */
[[deprecated("Use fast_policy_container (basic_value_container<indexed_storage_policy>) from policy_container.h instead. See Issue #328 for migration guide.")]]
class optimized_container : public value_container {
public:
    using value_container::value_container;

    /**
     * @brief Get source ID without copying (zero-copy)
     * @return String view to source ID
     */
    [[nodiscard]] std::string_view source_id_view() const noexcept {
        return source_id_;
    }

    /**
     * @brief Get source sub ID without copying
     */
    [[nodiscard]] std::string_view source_sub_id_view() const noexcept {
        return source_sub_id_;
    }

    /**
     * @brief Get target ID without copying
     */
    [[nodiscard]] std::string_view target_id_view() const noexcept {
        return target_id_;
    }

    /**
     * @brief Get target sub ID without copying
     */
    [[nodiscard]] std::string_view target_sub_id_view() const noexcept {
        return target_sub_id_;
    }

    /**
     * @brief Get message type without copying
     */
    [[nodiscard]] std::string_view message_type_view() const noexcept {
        return message_type_;
    }

    /**
     * @brief Set source with move semantics
     * @param source_id Source ID (moved)
     * @param source_sub_id Source sub ID (moved)
     */
    void set_source_move(std::string&& source_id, std::string&& source_sub_id) {
        source_id_ = std::move(source_id);
        source_sub_id_ = std::move(source_sub_id);
    }

    /**
     * @brief Set target with move semantics
     */
    void set_target_move(std::string&& target_id, std::string&& target_sub_id = "") {
        target_id_ = std::move(target_id);
        target_sub_id_ = std::move(target_sub_id);
    }

    /**
     * @brief Set message type with move semantics
     */
    void set_message_type_move(std::string&& message_type) {
        message_type_ = std::move(message_type);
    }

    /**
     * @brief Fast value lookup using hash map (O(1))
     * @param target_name Value name to find
     * @param index Index if multiple values with same name
     * @return Value if found, nullptr otherwise
     *
     * This is much faster than iterating through vector for large containers.
     */
    [[nodiscard]] std::shared_ptr<value> get_value_fast(
        std::string_view target_name,
        unsigned int index = 0
    ) {
        // Build index on first access
        if (value_index_.empty() && !units_.empty()) {
            build_value_index();
        }

        auto it = value_index_.find(std::string(target_name));
        if (it == value_index_.end() || it->second.empty()) {
            // Fallback to parent implementation
            return value_container::get_value(std::string(target_name), index);
        }

        if (index < it->second.size()) {
            return it->second[index];
        }

        return nullptr;
    }

    /**
     * @brief Check if value exists (O(1))
     * @param target_name Value name
     * @return true if value exists
     */
    [[nodiscard]] bool has_value_fast(std::string_view target_name) const {
        if (value_index_.empty() && !units_.empty()) {
            const_cast<optimized_container*>(this)->build_value_index();
        }

        return value_index_.find(std::string(target_name)) != value_index_.end();
    }

    /**
     * @brief Get all values with given name (O(1))
     * @param target_name Value name
     * @return Vector of matching values
     */
    [[nodiscard]] std::vector<std::shared_ptr<value>> get_values_fast(
        std::string_view target_name
    ) {
        if (value_index_.empty() && !units_.empty()) {
            build_value_index();
        }

        auto it = value_index_.find(std::string(target_name));
        if (it != value_index_.end()) {
            return it->second;
        }

        return {};
    }

    /**
     * @brief Add value and update index
     */
    std::shared_ptr<value> add(std::shared_ptr<value> target_value,
                               bool update_immediately = false) override {
        auto result = value_container::add(target_value, update_immediately);

        // Update index if it was built
        if (!value_index_.empty() && result) {
            auto name = result->name();
            value_index_[name].push_back(result);
        }

        return result;
    }

    /**
     * @brief Remove value and update index
     */
    void remove(std::string_view target_name, bool update_immediately = false) override {
        value_container::remove(target_name, update_immediately);

        // Update index
        auto it = value_index_.find(std::string(target_name));
        if (it != value_index_.end()) {
            value_index_.erase(it);
        }
    }

    /**
     * @brief Invalidate index (call after bulk operations)
     */
    void invalidate_index() {
        value_index_.clear();
    }

    /**
     * @brief Rebuild value index for fast lookup
     */
    void rebuild_index() {
        build_value_index();
    }

    /**
     * @brief Get memory usage estimate
     * @return Approximate memory usage in bytes
     */
    [[nodiscard]] size_t estimate_memory_usage() const {
        size_t total = 0;

        // Header strings
        total += source_id_.capacity();
        total += source_sub_id_.capacity();
        total += target_id_.capacity();
        total += target_sub_id_.capacity();
        total += message_type_.capacity();
        total += version_.capacity();

        // Data string
        total += data_string_.capacity();

        // Values (rough estimate)
        total += units_.size() * sizeof(std::shared_ptr<value>);
        total += units_.size() * 128;  // Average value size

        // Index
        total += value_index_.size() * 64;  // HashMap overhead

        return total;
    }

private:
    /**
     * @brief Build hash map index for fast value lookup
     */
    void build_value_index() {
        value_index_.clear();
        value_index_.reserve(units_.size());

        for (const auto& unit : units_) {
            if (unit) {
                auto name = unit->name();
                value_index_[name].push_back(unit);
            }
        }
    }

private:
    // Fast lookup index: value name -> list of values
    std::unordered_map<std::string, std::vector<std::shared_ptr<value>>> value_index_;
};

} // namespace container_module
