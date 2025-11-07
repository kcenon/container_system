/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#pragma once

#include "container/core/value.h"
#include "container/internal/variant_value_v2.h"
#include <memory>
#include <optional>
#include <vector>

namespace container_module
{
    /**
     * @brief Bridge for migrating between legacy value system and variant_value_v2
     *
     * This class provides bidirectional conversion between:
     * - Legacy polymorphic value classes (value, bool_value, string_value, etc.)
     * - Modern variant_value_v2 (type-safe, compile-time optimized)
     *
     * Use cases:
     * 1. Incremental migration: New code uses variant_value_v2, interoperates with legacy
     * 2. Deserialization: Read legacy binary data into modern system
     * 3. API compatibility: Maintain existing interfaces while upgrading internals
     *
     * Migration strategy:
     * - Phase 1 (0-3 months): Use bridge for all conversions, both systems coexist
     * - Phase 2 (3-6 months): New features use only variant_value_v2
     * - Phase 3 (6-12 months): Deprecate and remove legacy system
     */
    class value_bridge
    {
    public:
        /**
         * @brief Convert legacy value to modern variant_value_v2
         *
         * Supports all 16 value types:
         * - Primitives: null, bool, numeric types (short through double)
         * - Complex: bytes, string, container, array
         *
         * @param legacy The legacy value object
         * @return variant_value_v2 containing equivalent data
         * @throws std::runtime_error if conversion fails (unsupported type)
         */
        static variant_value_v2 to_modern(const value& legacy);

        /**
         * @brief Convert legacy value (shared_ptr) to modern
         */
        static variant_value_v2 to_modern(std::shared_ptr<value> legacy);

        /**
         * @brief Convert modern variant_value_v2 to legacy value
         *
         * Returns polymorphic value subclass matching the variant type:
         * - bool_value, int_value, string_value, etc.
         *
         * @param modern The modern variant value
         * @return Shared pointer to appropriate legacy value subclass
         * @throws std::runtime_error if conversion fails
         */
        static std::shared_ptr<value> to_legacy(const variant_value_v2& modern);

        /**
         * @brief Convert vector of legacy values to modern
         */
        static std::vector<variant_value_v2> to_modern_vector(
            const std::vector<std::shared_ptr<value>>& legacy_values);

        /**
         * @brief Convert vector of modern values to legacy
         */
        static std::vector<std::shared_ptr<value>> to_legacy_vector(
            const std::vector<variant_value_v2>& modern_values);

        /**
         * @brief Test round-trip conversion (legacy → modern → legacy)
         *
         * Verifies that converting to modern and back produces equivalent value.
         * Used for validation during migration.
         *
         * @param original Original legacy value
         * @return true if round-trip preserves data, false if data loss detected
         */
        static bool verify_round_trip(std::shared_ptr<value> original);

        /**
         * @brief Test round-trip conversion (modern → legacy → modern)
         */
        static bool verify_round_trip(const variant_value_v2& original);

        /**
         * @brief Migration statistics for monitoring
         */
        struct migration_stats {
            size_t successful_conversions{0};
            size_t failed_conversions{0};
            size_t round_trip_errors{0};
            std::vector<std::string> error_messages;
        };

        /**
         * @brief Get conversion statistics (for monitoring migration progress)
         */
        static migration_stats get_stats();

        /**
         * @brief Reset statistics (for testing)
         */
        static void reset_stats();

    private:
        // Statistics tracking
        static migration_stats stats_;
        static std::mutex stats_mutex_;

        // Helper: Convert specific legacy types
        static variant_value_v2 convert_bool_value(const value& legacy);
        static variant_value_v2 convert_numeric_value(const value& legacy);
        static variant_value_v2 convert_bytes_value(const value& legacy);
        static variant_value_v2 convert_string_value(const value& legacy);
        static variant_value_v2 convert_container_value(const value& legacy);
        static variant_value_v2 convert_array_value(const value& legacy);

        // Helper: Create specific legacy types
        static std::shared_ptr<value> create_legacy_bool(const variant_value_v2& modern);
        static std::shared_ptr<value> create_legacy_numeric(const variant_value_v2& modern);
        static std::shared_ptr<value> create_legacy_bytes(const variant_value_v2& modern);
        static std::shared_ptr<value> create_legacy_string(const variant_value_v2& modern);
        static std::shared_ptr<value> create_legacy_container(const variant_value_v2& modern);
        static std::shared_ptr<value> create_legacy_array(const variant_value_v2& modern);

        // Error handling
        static void record_error(const std::string& message);
    };

    /**
     * @brief RAII wrapper for automatic legacy value conversion
     *
     * Automatically converts legacy values to modern on construction,
     * and optionally converts back on destruction (for in-place modification).
     *
     * Usage:
     * ```cpp
     * auto legacy_val = get_legacy_value();
     * {
     *     auto_bridge bridge(legacy_val, true);  // Auto convert back on scope exit
     *     auto& modern = bridge.get();
     *     modern.set("new_value");  // Modify modern value
     * }  // Automatically syncs back to legacy_val
     * ```
     */
    class auto_value_bridge
    {
    public:
        /**
         * @brief Construct with legacy value
         * @param legacy The legacy value to wrap
         * @param sync_back If true, converts back to legacy on destruction
         */
        explicit auto_value_bridge(std::shared_ptr<value> legacy, bool sync_back = false)
            : legacy_(legacy)
            , modern_(value_bridge::to_modern(legacy))
            , sync_back_(sync_back)
        {}

        /**
         * @brief Get reference to modern value
         */
        variant_value_v2& get() { return modern_; }
        const variant_value_v2& get() const { return modern_; }

        /**
         * @brief Get legacy value (original)
         */
        std::shared_ptr<value> get_legacy() const { return legacy_; }

        /**
         * @brief Manually sync changes back to legacy
         */
        void sync() {
            if (legacy_) {
                // Replace legacy pointer with new object (legacy value doesn't support copy assignment)
                legacy_ = value_bridge::to_legacy(modern_);
            }
        }

        /**
         * @brief Destructor: optionally syncs back
         */
        ~auto_value_bridge() {
            if (sync_back_) {
                try {
                    sync();
                } catch (...) {
                    // Swallow exceptions in destructor
                }
            }
        }

        // Disable copy (unique ownership)
        auto_value_bridge(const auto_value_bridge&) = delete;
        auto_value_bridge& operator=(const auto_value_bridge&) = delete;

        // Enable move
        auto_value_bridge(auto_value_bridge&&) noexcept = default;
        auto_value_bridge& operator=(auto_value_bridge&&) noexcept = default;

    private:
        std::shared_ptr<value> legacy_;
        variant_value_v2 modern_;
        bool sync_back_;
    };

} // namespace container_module
