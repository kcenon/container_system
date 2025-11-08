/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#pragma once

/**
 * @file value_migrator.h
 * @brief Migration tool for deprecated value classes → variant_value_v2
 *
 * ⚠️ DEPRECATION NOTICE:
 * - Polymorphic `value` class (core/value.h) is DEPRECATED as of v2.0.0
 * - `variant_value` v1 (internal/variant_value.h) is DEPRECATED as of v2.0.0
 * - Both will be REMOVED in v3.0.0 (approximately 6 months)
 *
 * This migration tool helps you convert existing code to use `variant_value_v2`.
 *
 * QUICK START:
 * ```cpp
 * #include <container/tools/value_migrator.h>
 *
 * // Convert deprecated polymorphic value to modern variant_value_v2
 * auto legacy_val = std::make_shared<string_value>("name", "John");
 * auto modern_val = value_migrator::to_modern(legacy_val);
 *
 * // Convert back if needed (for incremental migration)
 * auto legacy_again = value_migrator::to_legacy(modern_val);
 *
 * // Test conversion integrity
 * bool safe = value_migrator::verify_conversion(legacy_val);
 * ```
 *
 * RECOMMENDED MIGRATION STRATEGY:
 *
 * Phase 1 (Month 0-1): Add migration tool
 * - Include this header where you use deprecated classes
 * - Wrap existing values with to_modern() at API boundaries
 * - Test with verify_conversion()
 *
 * Phase 2 (Month 1-3): Incremental migration
 * - Rewrite internal code to use variant_value_v2 directly
 * - Use to_legacy() only for external interfaces
 * - Monitor statistics with get_stats()
 *
 * Phase 3 (Month 3-6): Complete migration
 * - Remove all uses of deprecated classes
 * - Remove value_bridge/value_migrator calls
 * - Pure variant_value_v2 codebase
 *
 * Phase 4 (Month 6+): Cleanup
 * - Upgrade to v3.0.0 which removes deprecated classes
 * - No longer need migration tool
 *
 * For detailed migration guide, see:
 * - docs/VARIANT_VALUE_V2_MIGRATION_GUIDE.md
 * - docs/MIGRATION.md
 */

#include "container/integration/value_bridge.h"

namespace container_module
{
    /**
     * @brief Migration helper for deprecated value classes
     *
     * This is a convenience wrapper around `value_bridge` with clearer naming
     * for migration purposes. All functionality is provided by value_bridge.
     *
     * Use this class to:
     * 1. Convert deprecated polymorphic value → variant_value_v2
     * 2. Convert deprecated variant_value v1 → variant_value_v2
     * 3. Test conversion integrity
     * 4. Monitor migration progress
     */
    class value_migrator
    {
    public:
        /**
         * @brief Convert deprecated polymorphic value to modern variant_value_v2
         *
         * Handles all 16 value types including:
         * - Primitives: null, bool, numeric types
         * - Complex: bytes, string, container, array
         *
         * Example:
         * ```cpp
         * // Old (deprecated)
         * auto old_val = std::make_shared<int_value>("age", 25);
         *
         * // New (recommended)
         * auto new_val = value_migrator::to_modern(old_val);
         * ```
         *
         * @param deprecated_value The legacy polymorphic value
         * @return Modern variant_value_v2
         * @throws std::runtime_error if conversion fails
         */
        static variant_value_v2 to_modern(std::shared_ptr<value> deprecated_value) {
            return value_bridge::to_modern(deprecated_value);
        }

        /**
         * @brief Convert deprecated value (by reference)
         */
        static variant_value_v2 to_modern(const value& deprecated_value) {
            return value_bridge::to_modern(deprecated_value);
        }

        /**
         * @brief Convert modern variant_value_v2 back to legacy (for incremental migration)
         *
         * Use this during transition period when you need to interface with
         * legacy code that expects polymorphic values.
         *
         * Example:
         * ```cpp
         * variant_value_v2 modern("name", std::string("John"));
         * auto legacy = value_migrator::to_legacy(modern);  // For legacy API
         * legacy_api->process(legacy);  // Still uses old interface
         * ```
         *
         * @param modern The modern variant value
         * @return Legacy polymorphic value
         * @throws std::runtime_error if conversion fails
         */
        static std::shared_ptr<value> to_legacy(const variant_value_v2& modern) {
            return value_bridge::to_legacy(modern);
        }

        /**
         * @brief Convert vector of deprecated values to modern
         */
        static std::vector<variant_value_v2> to_modern_vector(
            const std::vector<std::shared_ptr<value>>& deprecated_values) {
            return value_bridge::to_modern_vector(deprecated_values);
        }

        /**
         * @brief Convert vector of modern values back to legacy
         */
        static std::vector<std::shared_ptr<value>> to_legacy_vector(
            const std::vector<variant_value_v2>& modern_values) {
            return value_bridge::to_legacy_vector(modern_values);
        }

        /**
         * @brief Verify conversion integrity (recommended for testing)
         *
         * Tests round-trip conversion: deprecated → modern → deprecated
         * Returns true if data is preserved, false if data loss detected.
         *
         * Use this to test your migration before deploying:
         * ```cpp
         * auto my_value = get_legacy_value();
         * if (!value_migrator::verify_conversion(my_value)) {
         *     // Handle conversion issue
         *     log_error("Conversion failed for: " + my_value->name());
         * }
         * ```
         *
         * @param original Original deprecated value
         * @return true if conversion preserves data
         */
        static bool verify_conversion(std::shared_ptr<value> original) {
            return value_bridge::verify_round_trip(original);
        }

        /**
         * @brief Verify conversion (modern → legacy → modern)
         */
        static bool verify_conversion(const variant_value_v2& original) {
            return value_bridge::verify_round_trip(original);
        }

        /**
         * @brief Migration statistics for monitoring progress
         *
         * Track how many conversions have been performed:
         * ```cpp
         * auto stats = value_migrator::get_stats();
         * std::cout << "Successful: " << stats.successful_conversions << "\n";
         * std::cout << "Failed: " << stats.failed_conversions << "\n";
         * ```
         */
        struct migration_stats {
            size_t successful_conversions{0};  ///< Number of successful conversions
            size_t failed_conversions{0};      ///< Number of failed conversions
            size_t round_trip_errors{0};       ///< Number of round-trip integrity failures
            std::vector<std::string> error_messages;  ///< Detailed error logs
        };

        /**
         * @brief Get migration statistics
         *
         * Use this to monitor your migration progress and detect issues.
         */
        static migration_stats get_stats() {
            auto bridge_stats = value_bridge::get_stats();
            return migration_stats{
                bridge_stats.successful_conversions,
                bridge_stats.failed_conversions,
                bridge_stats.round_trip_errors,
                bridge_stats.error_messages
            };
        }

        /**
         * @brief Reset statistics (useful for testing)
         */
        static void reset_stats() {
            value_bridge::reset_stats();
        }
    };

    /**
     * @brief RAII helper for automatic migration
     *
     * Automatically converts deprecated values to modern on construction.
     * Optionally converts back on destruction (for in-place modification).
     *
     * Example:
     * ```cpp
     * auto legacy_val = get_legacy_value();
     * {
     *     auto_migrator migrator(legacy_val, true);  // Auto sync back
     *     auto& modern = migrator.get();
     *     modern.set_value(new_data);  // Modify modern value
     * }  // Automatically syncs back to legacy_val
     * ```
     */
    using auto_migrator = auto_value_bridge;

} // namespace container_module
