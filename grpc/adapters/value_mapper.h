// BSD 3-Clause License
// Copyright (c) 2024, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

#pragma once

/**
 * @file value_mapper.h
 * @brief Type mapping utilities for kcenon::container <-> gRPC proto conversion
 *
 * This header provides compile-time and runtime type mapping between native
 * container value types and Protocol Buffer types. It ensures type-safe
 * conversions and provides utilities for type introspection.
 */

#include "core/value_types.h"

#include <cstdint>
#include <type_traits>
#include <variant>
#include <string>
#include <vector>
#include <memory>

// Forward declarations
namespace kcenon::container_grpc {
enum ValueType : int;
}

namespace kcenon::container {
class value_container;
}

namespace kcenon::container_grpc {

/**
 * @brief Compile-time type traits for value mapping
 *
 * These traits enable compile-time type checking and conversion
 * selection based on the native C++ type.
 */
namespace type_traits {

/**
 * @brief Primary template for native type to proto type mapping
 */
template<typename T>
struct native_to_proto_type {
    static constexpr bool supported = false;
};

// Specializations for supported types
template<>
struct native_to_proto_type<std::monostate> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::null_value;
};

template<>
struct native_to_proto_type<bool> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::bool_value;
};

template<>
struct native_to_proto_type<short> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::short_value;
};

template<>
struct native_to_proto_type<unsigned short> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::ushort_value;
};

template<>
struct native_to_proto_type<int> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::int_value;
};

template<>
struct native_to_proto_type<unsigned int> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::uint_value;
};

template<>
struct native_to_proto_type<long> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::long_value;
};

template<>
struct native_to_proto_type<unsigned long> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::ulong_value;
};

template<>
struct native_to_proto_type<long long> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::llong_value;
};

template<>
struct native_to_proto_type<unsigned long long> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::ullong_value;
};

template<>
struct native_to_proto_type<float> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::float_value;
};

template<>
struct native_to_proto_type<double> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::double_value;
};

template<>
struct native_to_proto_type<std::string> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::string_value;
};

template<>
struct native_to_proto_type<std::vector<uint8_t>> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::bytes_value;
};

template<>
struct native_to_proto_type<std::shared_ptr<kcenon::container::value_container>> {
    static constexpr bool supported = true;
    static constexpr kcenon::container::value_types native_type =
        kcenon::container::value_types::container_value;
};

/**
 * @brief Helper variable template for checking if type is supported
 */
template<typename T>
inline constexpr bool is_supported_v = native_to_proto_type<T>::supported;

/**
 * @brief Helper variable template for getting native type enum
 */
template<typename T>
inline constexpr kcenon::container::value_types native_type_v =
    native_to_proto_type<T>::native_type;

} // namespace type_traits

/**
 * @brief Runtime type mapping utilities
 */
class type_mapping {
public:
    /**
     * @brief Map kcenon::container::value_types to proto ValueType enum value
     * @param type Native type enumeration
     * @return Integer value of corresponding proto ValueType
     *
     * The mapping is 1:1 since proto ValueType mirrors value_types exactly.
     */
    static int to_proto_enum(kcenon::container::value_types type) noexcept {
        return static_cast<int>(type);
    }

    /**
     * @brief Map proto ValueType to kcenon::container::value_types
     * @param proto_type Proto type enumeration value
     * @return Native value_types enumeration
     */
    static kcenon::container::value_types from_proto_enum(int proto_type) noexcept {
        if (proto_type < 0 || proto_type > 15) {
            return kcenon::container::value_types::null_value;
        }
        return static_cast<kcenon::container::value_types>(proto_type);
    }

    /**
     * @brief Get the proto field number for a given type
     * @param type Native type enumeration
     * @return Proto oneof field number (3-18 based on container_service.proto)
     *
     * Mapping:
     *   null_value (0) -> null_flag (3)
     *   bool_value (1) -> bool_val (4)
     *   short_value (2) -> short_val (5)
     *   ... etc
     */
    static int get_proto_field_number(kcenon::container::value_types type) noexcept {
        // Field numbers are offset by 3 from type enum values
        // (name=1, type=2, then value fields start at 3)
        return static_cast<int>(type) + 3;
    }
};

/**
 * @brief Size calculation utilities for proto messages
 */
class size_calculator {
public:
    /**
     * @brief Estimate serialized proto size for a value
     * @param type Value type
     * @param data_size Size of the actual data in bytes
     * @return Estimated proto serialized size in bytes
     *
     * This is useful for pre-allocating buffers.
     */
    static size_t estimate_proto_size(
        kcenon::container::value_types type,
        size_t data_size) noexcept;

    /**
     * @brief Estimate total container proto size
     * @param value_count Number of values
     * @param avg_name_length Average value name length
     * @param avg_data_size Average value data size
     * @return Estimated total proto size in bytes
     */
    static size_t estimate_container_size(
        size_t value_count,
        size_t avg_name_length,
        size_t avg_data_size) noexcept;
};

} // namespace kcenon::container_grpc
