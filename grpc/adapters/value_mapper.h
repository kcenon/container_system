/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, kcenon
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

/**
 * @file value_mapper.h
 * @brief Type mapping utilities for container_module <-> gRPC proto conversion
 *
 * This header provides compile-time and runtime type mapping between native
 * container value types and Protocol Buffer types. It ensures type-safe
 * conversions and provides utilities for type introspection.
 */

#include "container/core/value_types.h"

#include <cstdint>
#include <type_traits>
#include <variant>
#include <string>
#include <vector>
#include <memory>

// Forward declarations
namespace container_grpc {
enum ValueType : int;
}

namespace container_module {
class value_container;
}

namespace container_grpc {

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
    static constexpr container_module::value_types native_type =
        container_module::value_types::null_value;
};

template<>
struct native_to_proto_type<bool> {
    static constexpr bool supported = true;
    static constexpr container_module::value_types native_type =
        container_module::value_types::bool_value;
};

template<>
struct native_to_proto_type<short> {
    static constexpr bool supported = true;
    static constexpr container_module::value_types native_type =
        container_module::value_types::short_value;
};

template<>
struct native_to_proto_type<unsigned short> {
    static constexpr bool supported = true;
    static constexpr container_module::value_types native_type =
        container_module::value_types::ushort_value;
};

template<>
struct native_to_proto_type<int> {
    static constexpr bool supported = true;
    static constexpr container_module::value_types native_type =
        container_module::value_types::int_value;
};

template<>
struct native_to_proto_type<unsigned int> {
    static constexpr bool supported = true;
    static constexpr container_module::value_types native_type =
        container_module::value_types::uint_value;
};

template<>
struct native_to_proto_type<long> {
    static constexpr bool supported = true;
    static constexpr container_module::value_types native_type =
        container_module::value_types::long_value;
};

template<>
struct native_to_proto_type<unsigned long> {
    static constexpr bool supported = true;
    static constexpr container_module::value_types native_type =
        container_module::value_types::ulong_value;
};

template<>
struct native_to_proto_type<long long> {
    static constexpr bool supported = true;
    static constexpr container_module::value_types native_type =
        container_module::value_types::llong_value;
};

template<>
struct native_to_proto_type<unsigned long long> {
    static constexpr bool supported = true;
    static constexpr container_module::value_types native_type =
        container_module::value_types::ullong_value;
};

template<>
struct native_to_proto_type<float> {
    static constexpr bool supported = true;
    static constexpr container_module::value_types native_type =
        container_module::value_types::float_value;
};

template<>
struct native_to_proto_type<double> {
    static constexpr bool supported = true;
    static constexpr container_module::value_types native_type =
        container_module::value_types::double_value;
};

template<>
struct native_to_proto_type<std::string> {
    static constexpr bool supported = true;
    static constexpr container_module::value_types native_type =
        container_module::value_types::string_value;
};

template<>
struct native_to_proto_type<std::vector<uint8_t>> {
    static constexpr bool supported = true;
    static constexpr container_module::value_types native_type =
        container_module::value_types::bytes_value;
};

template<>
struct native_to_proto_type<std::shared_ptr<container_module::value_container>> {
    static constexpr bool supported = true;
    static constexpr container_module::value_types native_type =
        container_module::value_types::container_value;
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
inline constexpr container_module::value_types native_type_v =
    native_to_proto_type<T>::native_type;

} // namespace type_traits

/**
 * @brief Runtime type mapping utilities
 */
class type_mapping {
public:
    /**
     * @brief Map container_module::value_types to proto ValueType enum value
     * @param type Native type enumeration
     * @return Integer value of corresponding proto ValueType
     *
     * The mapping is 1:1 since proto ValueType mirrors value_types exactly.
     */
    static int to_proto_enum(container_module::value_types type) noexcept {
        return static_cast<int>(type);
    }

    /**
     * @brief Map proto ValueType to container_module::value_types
     * @param proto_type Proto type enumeration value
     * @return Native value_types enumeration
     */
    static container_module::value_types from_proto_enum(int proto_type) noexcept {
        if (proto_type < 0 || proto_type > 15) {
            return container_module::value_types::null_value;
        }
        return static_cast<container_module::value_types>(proto_type);
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
    static int get_proto_field_number(container_module::value_types type) noexcept {
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
        container_module::value_types type,
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

} // namespace container_grpc
