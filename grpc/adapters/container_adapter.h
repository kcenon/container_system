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
 * @file container_adapter.h
 * @brief Adapter layer for converting between container_module types and gRPC proto messages
 *
 * This adapter provides a clean separation between the existing container system
 * and the gRPC protocol layer. It uses read-only access to existing container
 * types, requiring zero modifications to the core container system.
 *
 * Design Principles:
 * - Pure adapter pattern: No modifications to existing code
 * - Read-only access to container_module types
 * - Stateless conversion functions
 * - Zero runtime overhead when not using gRPC
 */

#include "core/container.h"
#include "core/value_types.h"

#include <memory>
#include <string>
#include <vector>
#include <optional>

// Forward declare proto types to avoid including generated headers in public API
namespace container_grpc {
class GrpcValue;
class GrpcArray;
class GrpcContainer;
enum ValueType : int;
}

namespace container_grpc {

/**
 * @class container_adapter
 * @brief Bidirectional adapter between container_module and gRPC proto messages
 *
 * This class provides static methods for converting between native container
 * types and Protocol Buffer messages. All methods are stateless and thread-safe.
 *
 * Usage:
 * @code
 *   // Convert native container to proto (for sending)
 *   auto native = std::make_shared<container_module::value_container>();
 *   native->set_message_type("request");
 *   native->set_value("count", 42);
 *
 *   GrpcContainer proto = container_adapter::to_grpc(*native);
 *
 *   // Convert proto to native container (after receiving)
 *   auto restored = container_adapter::from_grpc(proto);
 *   auto count = restored->get_value("count");
 * @endcode
 */
class container_adapter {
public:
    // =========================================================================
    // Container Conversion (Main API)
    // =========================================================================

    /**
     * @brief Convert native value_container to GrpcContainer proto message
     * @param container The native container to convert (read-only access)
     * @return GrpcContainer proto message containing all container data
     * @throws std::runtime_error if conversion fails for any value
     *
     * This method reads all data from the container and creates an equivalent
     * proto message. The original container is not modified.
     */
    static GrpcContainer to_grpc(const container_module::value_container& container);

    /**
     * @brief Convert GrpcContainer proto message to native value_container
     * @param grpc_container The proto message to convert
     * @return Shared pointer to newly created value_container
     * @throws std::runtime_error if conversion fails for any value
     *
     * This method creates a new value_container and populates it with data
     * from the proto message.
     */
    static std::shared_ptr<container_module::value_container>
    from_grpc(const GrpcContainer& grpc_container);

    // =========================================================================
    // Value Conversion (Internal/Advanced API)
    // =========================================================================

    /**
     * @brief Convert single optimized_value to GrpcValue proto message
     * @param value The native value to convert
     * @return GrpcValue proto message
     * @throws std::runtime_error if value type is not supported
     */
    static GrpcValue to_grpc_value(const container_module::optimized_value& value);

    /**
     * @brief Convert GrpcValue proto message to optimized_value
     * @param grpc_value The proto value to convert
     * @return Native optimized_value
     * @throws std::runtime_error if proto type is not supported
     */
    static container_module::optimized_value
    from_grpc_value(const GrpcValue& grpc_value);

    // =========================================================================
    // Type Mapping
    // =========================================================================

    /**
     * @brief Convert native value_types to proto ValueType
     * @param type Native type enumeration
     * @return Corresponding proto ValueType enumeration
     */
    static ValueType to_grpc_type(container_module::value_types type);

    /**
     * @brief Convert proto ValueType to native value_types
     * @param type Proto type enumeration
     * @return Corresponding native value_types enumeration
     */
    static container_module::value_types from_grpc_type(ValueType type);

    // =========================================================================
    // Validation
    // =========================================================================

    /**
     * @brief Validate that a container can be converted to proto
     * @param container The container to validate
     * @return true if container can be safely converted
     *
     * Checks for unsupported types, circular references, etc.
     */
    static bool can_convert(const container_module::value_container& container) noexcept;

    /**
     * @brief Validate that a proto message can be converted to native
     * @param grpc_container The proto message to validate
     * @return true if message can be safely converted
     */
    static bool can_convert(const GrpcContainer& grpc_container) noexcept;

private:
    // =========================================================================
    // Internal Helpers
    // =========================================================================

    /**
     * @brief Convert value_variant data to proto value
     */
    static void set_grpc_value_data(
        GrpcValue& grpc_value,
        const container_module::value_variant& data,
        container_module::value_types type);

    /**
     * @brief Extract data from proto value to value_variant
     */
    static container_module::value_variant get_variant_from_grpc(
        const GrpcValue& grpc_value,
        container_module::value_types type);

    /**
     * @brief Handle nested container conversion to avoid stack overflow
     */
    static GrpcContainer to_grpc_recursive(
        const container_module::value_container& container,
        int depth);

    /**
     * @brief Handle nested container conversion from proto
     */
    static std::shared_ptr<container_module::value_container>
    from_grpc_recursive(const GrpcContainer& grpc_container, int depth);

    // Maximum nesting depth to prevent stack overflow
    static constexpr int MAX_NESTING_DEPTH = 32;
};

/**
 * @class value_mapper
 * @brief Type mapping utilities between container_module and proto types
 *
 * This class provides compile-time and runtime type mapping between
 * the native container value types and Protocol Buffer types.
 */
class value_mapper {
public:
    /**
     * @brief Check if a native type is supported for gRPC conversion
     * @param type The native value type
     * @return true if type can be converted to proto
     */
    static constexpr bool is_supported(container_module::value_types type) noexcept {
        switch (type) {
            case container_module::value_types::null_value:
            case container_module::value_types::bool_value:
            case container_module::value_types::short_value:
            case container_module::value_types::ushort_value:
            case container_module::value_types::int_value:
            case container_module::value_types::uint_value:
            case container_module::value_types::long_value:
            case container_module::value_types::ulong_value:
            case container_module::value_types::llong_value:
            case container_module::value_types::ullong_value:
            case container_module::value_types::float_value:
            case container_module::value_types::double_value:
            case container_module::value_types::string_value:
            case container_module::value_types::bytes_value:
            case container_module::value_types::container_value:
            case container_module::value_types::array_value:
                return true;
            default:
                return false;
        }
    }

    /**
     * @brief Get human-readable type name for debugging
     * @param type The native value type
     * @return String representation of the type
     */
    static const char* type_name(container_module::value_types type) noexcept;

    /**
     * @brief Get proto type name for debugging
     * @param type The proto value type
     * @return String representation of the type
     */
    static const char* proto_type_name(ValueType type) noexcept;
};

} // namespace container_grpc
