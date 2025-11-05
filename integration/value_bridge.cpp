/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "container/integration/value_bridge.h"
#include "container/internal/thread_safe_container.h"
#include "container/values/bool_value.h"
#include "container/values/numeric_value.h"
#include "container/values/bytes_value.h"
#include "container/values/string_value.h"
#include "container/values/container_value.h"
#include "container/values/array_value.h"
#include <stdexcept>

namespace container_module
{
    // Static members
    value_bridge::migration_stats value_bridge::stats_;
    std::mutex value_bridge::stats_mutex_;

    // ============================================================================
    // Legacy → Modern conversion
    // ============================================================================

    variant_value_v2 value_bridge::to_modern(const value& legacy) {
        try {
            value_types type = legacy.type();

            switch (type) {
                case value_types::null_value:
                    return variant_value_v2(legacy.name());

                case value_types::bool_value:
                    return variant_value_v2(legacy.name(), legacy.to_boolean());

                case value_types::short_value:
                    return variant_value_v2(legacy.name(), static_cast<int16_t>(legacy.to_short()));

                case value_types::ushort_value:
                    return variant_value_v2(legacy.name(), static_cast<uint16_t>(legacy.to_ushort()));

                case value_types::int_value:
                    return variant_value_v2(legacy.name(), static_cast<int32_t>(legacy.to_int()));

                case value_types::uint_value:
                    return variant_value_v2(legacy.name(), static_cast<uint32_t>(legacy.to_uint()));

                case value_types::long_value:
                case value_types::llong_value:
                    return variant_value_v2(legacy.name(), static_cast<int64_t>(legacy.to_llong()));

                case value_types::ulong_value:
                case value_types::ullong_value:
                    return variant_value_v2(legacy.name(), static_cast<uint64_t>(legacy.to_ullong()));

                case value_types::float_value:
                    return variant_value_v2(legacy.name(), legacy.to_float());

                case value_types::double_value:
                    return variant_value_v2(legacy.name(), legacy.to_double());

                case value_types::bytes_value:
                    return convert_bytes_value(legacy);

                case value_types::string_value:
                    return convert_string_value(legacy);

                case value_types::container_value:
                    return convert_container_value(legacy);

                case value_types::array_value:
                    return convert_array_value(legacy);

                default:
                    throw std::runtime_error("Unsupported legacy value type");
            }

            stats_.successful_conversions++;
        } catch (const std::exception& e) {
            stats_.failed_conversions++;
            record_error(std::string("to_modern failed: ") + e.what());
            throw;
        }
    }

    variant_value_v2 value_bridge::to_modern(std::shared_ptr<value> legacy) {
        if (!legacy) {
            throw std::invalid_argument("Cannot convert null legacy value pointer");
        }
        return to_modern(*legacy);
    }

    variant_value_v2 value_bridge::convert_bytes_value(const value& legacy) {
        auto bytes = legacy.to_bytes();
        return variant_value_v2(legacy.name(), std::move(bytes));
    }

    variant_value_v2 value_bridge::convert_string_value(const value& legacy) {
        auto str = legacy.to_string();
        return variant_value_v2(legacy.name(), std::move(str));
    }

    variant_value_v2 value_bridge::convert_container_value(const value& legacy) {
        // Container value contains nested values
        // For now, serialize and deserialize through binary format
        // Note: serialize() is not const, but we're only reading
        auto serialized = const_cast<value&>(legacy).serialize();
        auto bytes = std::vector<uint8_t>(serialized.begin(), serialized.end());

        // Deserialize into thread_safe_container
        auto container = thread_safe_container::deserialize(bytes);
        return variant_value_v2(legacy.name(), container);
    }

    variant_value_v2 value_bridge::convert_array_value(const value& legacy) {
        // Get array elements
        // Note: children() is not const, but we're only reading
        auto children = const_cast<value&>(legacy).children();

        array_variant arr;
        arr.values.reserve(children.size());

        for (const auto& child : children) {
            if (child) {
                auto modern_child = to_modern(child);
                arr.values.push_back(std::make_shared<variant_value_v2>(std::move(modern_child)));
            }
        }

        return variant_value_v2(legacy.name(), std::move(arr));
    }

    // ============================================================================
    // Modern → Legacy conversion
    // ============================================================================

    std::shared_ptr<value> value_bridge::to_legacy(const variant_value_v2& modern) {
        try {
            value_types type = modern.type();

            switch (type) {
                case value_types::null_value:
                    return std::make_shared<value>(std::string(modern.name()));

                case value_types::bool_value:
                    return create_legacy_bool(modern);

                case value_types::short_value:
                case value_types::ushort_value:
                case value_types::int_value:
                case value_types::uint_value:
                case value_types::long_value:
                case value_types::ulong_value:
                case value_types::llong_value:
                case value_types::ullong_value:
                case value_types::float_value:
                case value_types::double_value:
                    return create_legacy_numeric(modern);

                case value_types::bytes_value:
                    return create_legacy_bytes(modern);

                case value_types::string_value:
                    return create_legacy_string(modern);

                case value_types::container_value:
                    return create_legacy_container(modern);

                case value_types::array_value:
                    return create_legacy_array(modern);

                default:
                    throw std::runtime_error("Unsupported modern value type");
            }

            stats_.successful_conversions++;
        } catch (const std::exception& e) {
            stats_.failed_conversions++;
            record_error(std::string("to_legacy failed: ") + e.what());
            throw;
        }
    }

    std::shared_ptr<value> value_bridge::create_legacy_bool(const variant_value_v2& modern) {
        auto val = modern.get<bool>();
        if (!val) throw std::runtime_error("Type mismatch: expected bool");
        return std::make_shared<bool_value>(std::string(modern.name()), *val);
    }

    std::shared_ptr<value> value_bridge::create_legacy_numeric(const variant_value_v2& modern) {
        value_types type = modern.type();

        switch (type) {
            case value_types::short_value: {
                auto val = modern.get<int16_t>();
                return std::make_shared<short_value>(std::string(modern.name()), *val);
            }
            case value_types::ushort_value: {
                auto val = modern.get<uint16_t>();
                return std::make_shared<ushort_value>(std::string(modern.name()), *val);
            }
            case value_types::int_value: {
                auto val = modern.get<int32_t>();
                return std::make_shared<int_value>(std::string(modern.name()), *val);
            }
            case value_types::uint_value: {
                auto val = modern.get<uint32_t>();
                return std::make_shared<uint_value>(std::string(modern.name()), *val);
            }
            case value_types::long_value:
            case value_types::llong_value: {
                auto val = modern.get<int64_t>();
                return std::make_shared<llong_value>(std::string(modern.name()), *val);
            }
            case value_types::ulong_value:
            case value_types::ullong_value: {
                auto val = modern.get<uint64_t>();
                return std::make_shared<ullong_value>(std::string(modern.name()), *val);
            }
            case value_types::float_value: {
                auto val = modern.get<float>();
                return std::make_shared<float_value>(std::string(modern.name()), *val);
            }
            case value_types::double_value: {
                auto val = modern.get<double>();
                return std::make_shared<double_value>(std::string(modern.name()), *val);
            }
            default:
                throw std::runtime_error("Not a numeric type");
        }
    }

    std::shared_ptr<value> value_bridge::create_legacy_bytes(const variant_value_v2& modern) {
        auto val = modern.get<std::vector<uint8_t>>();
        if (!val) throw std::runtime_error("Type mismatch: expected bytes");
        return std::make_shared<bytes_value>(std::string(modern.name()), *val);
    }

    std::shared_ptr<value> value_bridge::create_legacy_string(const variant_value_v2& modern) {
        auto val = modern.get<std::string>();
        if (!val) throw std::runtime_error("Type mismatch: expected string");
        return std::make_shared<string_value>(std::string(modern.name()), *val);
    }

    std::shared_ptr<value> value_bridge::create_legacy_container(const variant_value_v2& modern) {
        auto container_ptr = modern.get<std::shared_ptr<thread_safe_container>>();
        if (!container_ptr || !*container_ptr) {
            return std::make_shared<container_value>(std::string(modern.name()));
        }

        // Serialize modern container and deserialize into legacy
        auto serialized = (*container_ptr)->serialize();
        auto legacy = std::make_shared<container_value>(std::string(modern.name()));
        // TODO: Implement proper container deserialization
        return legacy;
    }

    std::shared_ptr<value> value_bridge::create_legacy_array(const variant_value_v2& modern) {
        auto arr = modern.get<array_variant>();
        if (!arr) throw std::runtime_error("Type mismatch: expected array");

        std::vector<std::shared_ptr<value>> legacy_values;
        legacy_values.reserve(arr->values.size());

        for (const auto& modern_elem : arr->values) {
            if (modern_elem) {
                legacy_values.push_back(to_legacy(*modern_elem));
            }
        }

        return std::make_shared<array_value>(std::string(modern.name()), legacy_values);
    }

    // ============================================================================
    // Batch conversion
    // ============================================================================

    std::vector<variant_value_v2> value_bridge::to_modern_vector(
        const std::vector<std::shared_ptr<value>>& legacy_values)
    {
        std::vector<variant_value_v2> result;
        result.reserve(legacy_values.size());

        for (const auto& legacy_val : legacy_values) {
            if (legacy_val) {
                result.push_back(to_modern(legacy_val));
            }
        }

        return result;
    }

    std::vector<std::shared_ptr<value>> value_bridge::to_legacy_vector(
        const std::vector<variant_value_v2>& modern_values)
    {
        std::vector<std::shared_ptr<value>> result;
        result.reserve(modern_values.size());

        for (const auto& modern_val : modern_values) {
            result.push_back(to_legacy(modern_val));
        }

        return result;
    }

    // ============================================================================
    // Round-trip verification
    // ============================================================================

    bool value_bridge::verify_round_trip(std::shared_ptr<value> original) {
        try {
            // Legacy → Modern → Legacy
            auto modern = to_modern(original);
            auto back_to_legacy = to_legacy(modern);

            // Compare serialized forms (byte-for-byte equality)
            auto original_serialized = original->serialize();
            auto result_serialized = back_to_legacy->serialize();

            if (original_serialized != result_serialized) {
                stats_.round_trip_errors++;
                record_error("Round-trip mismatch: serialized data differs");
                return false;
            }

            return true;
        } catch (const std::exception& e) {
            stats_.round_trip_errors++;
            record_error(std::string("Round-trip failed: ") + e.what());
            return false;
        }
    }

    bool value_bridge::verify_round_trip(const variant_value_v2& original) {
        try {
            // Modern → Legacy → Modern
            auto legacy = to_legacy(original);
            auto back_to_modern = to_modern(legacy);

            // Compare serialized forms
            auto original_serialized = original.serialize();
            auto result_serialized = back_to_modern.serialize();

            if (original_serialized != result_serialized) {
                stats_.round_trip_errors++;
                record_error("Round-trip mismatch: serialized data differs");
                return false;
            }

            return true;
        } catch (const std::exception& e) {
            stats_.round_trip_errors++;
            record_error(std::string("Round-trip failed: ") + e.what());
            return false;
        }
    }

    // ============================================================================
    // Statistics
    // ============================================================================

    value_bridge::migration_stats value_bridge::get_stats() {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        return stats_;
    }

    void value_bridge::reset_stats() {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_ = migration_stats{};
    }

    void value_bridge::record_error(const std::string& message) {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.error_messages.push_back(message);
    }

} // namespace container_module
