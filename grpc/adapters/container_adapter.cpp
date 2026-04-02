// BSD 3-Clause License
// Copyright (c) 2024, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

#include "container_adapter.h"
#include "value_mapper.h"
#include "container_service.pb.h"

#include <stdexcept>
#include <algorithm>

namespace kcenon::container_grpc {

// =============================================================================
// Container Conversion (Main API)
// =============================================================================

GrpcContainer container_adapter::to_grpc(
    const kcenon::container::value_container& container) {
    return to_grpc_recursive(container, 0);
}

std::shared_ptr<kcenon::container::value_container>
container_adapter::from_grpc(const GrpcContainer& grpc_container) {
    return from_grpc_recursive(grpc_container, 0);
}

// =============================================================================
// Value Conversion
// =============================================================================

GrpcValue container_adapter::to_grpc_value(
    const kcenon::container::optimized_value& value) {
    GrpcValue grpc_value;
    grpc_value.set_name(value.name);
    grpc_value.set_type(to_grpc_type(value.type));
    set_grpc_value_data(grpc_value, value.data, value.type);
    return grpc_value;
}

kcenon::container::optimized_value
container_adapter::from_grpc_value(const GrpcValue& grpc_value) {
    kcenon::container::optimized_value value;
    value.name = grpc_value.name();
    value.type = from_grpc_type(grpc_value.type());
    value.data = get_variant_from_grpc(grpc_value, value.type);
    return value;
}

// =============================================================================
// Type Mapping
// =============================================================================

ValueType container_adapter::to_grpc_type(kcenon::container::value_types type) {
    return static_cast<ValueType>(static_cast<int>(type));
}

kcenon::container::value_types
container_adapter::from_grpc_type(ValueType type) {
    return static_cast<kcenon::container::value_types>(static_cast<int>(type));
}

// =============================================================================
// Validation
// =============================================================================

bool container_adapter::can_convert(
    const kcenon::container::value_container& container) noexcept {
    try {
        for (const auto& val : container) {
            if (!value_mapper::is_supported(val.type)) {
                return false;
            }
            // Check for nested containers recursively
            if (val.type == kcenon::container::value_types::container_value) {
                if (std::holds_alternative<
                        std::shared_ptr<kcenon::container::value_container>>(
                        val.data)) {
                    auto nested = std::get<
                        std::shared_ptr<kcenon::container::value_container>>(
                        val.data);
                    if (nested && !can_convert(*nested)) {
                        return false;
                    }
                }
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool container_adapter::can_convert(
    const GrpcContainer& grpc_container) noexcept {
    try {
        for (const auto& grpc_val : grpc_container.values()) {
            auto type = from_grpc_type(grpc_val.type());
            if (!value_mapper::is_supported(type)) {
                return false;
            }
            // Check for nested containers
            if (grpc_val.has_container_val()) {
                if (!can_convert(grpc_val.container_val())) {
                    return false;
                }
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

// =============================================================================
// Internal Helpers
// =============================================================================

void container_adapter::set_grpc_value_data(
    GrpcValue& grpc_value,
    const kcenon::container::value_variant& data,
    kcenon::container::value_types type) {

    switch (type) {
        case kcenon::container::value_types::null_value:
            grpc_value.set_null_flag(true);
            break;

        case kcenon::container::value_types::bool_value:
            grpc_value.set_bool_val(std::get<bool>(data));
            break;

        case kcenon::container::value_types::short_value:
            grpc_value.set_short_val(std::get<short>(data));
            break;

        case kcenon::container::value_types::ushort_value:
            grpc_value.set_ushort_val(std::get<unsigned short>(data));
            break;

        case kcenon::container::value_types::int_value:
            grpc_value.set_int_val(std::get<int>(data));
            break;

        case kcenon::container::value_types::uint_value:
            grpc_value.set_uint_val(std::get<unsigned int>(data));
            break;

        case kcenon::container::value_types::long_value:
            grpc_value.set_long_val(std::get<long>(data));
            break;

        case kcenon::container::value_types::ulong_value:
            grpc_value.set_ulong_val(std::get<unsigned long>(data));
            break;

        case kcenon::container::value_types::llong_value:
            grpc_value.set_llong_val(std::get<long long>(data));
            break;

        case kcenon::container::value_types::ullong_value:
            grpc_value.set_ullong_val(std::get<unsigned long long>(data));
            break;

        case kcenon::container::value_types::float_value:
            grpc_value.set_float_val(std::get<float>(data));
            break;

        case kcenon::container::value_types::double_value:
            grpc_value.set_double_val(std::get<double>(data));
            break;

        case kcenon::container::value_types::string_value:
            grpc_value.set_string_val(std::get<std::string>(data));
            break;

        case kcenon::container::value_types::bytes_value: {
            const auto& bytes = std::get<std::vector<uint8_t>>(data);
            grpc_value.set_bytes_val(bytes.data(), bytes.size());
            break;
        }

        case kcenon::container::value_types::container_value: {
            auto nested = std::get<
                std::shared_ptr<kcenon::container::value_container>>(data);
            if (nested) {
                *grpc_value.mutable_container_val() = to_grpc(*nested);
            }
            break;
        }

        case kcenon::container::value_types::array_value:
            // Array values would need special handling
            // For now, we handle arrays in future iterations
            break;
    }
}

kcenon::container::value_variant container_adapter::get_variant_from_grpc(
    const GrpcValue& grpc_value,
    kcenon::container::value_types type) {

    switch (type) {
        case kcenon::container::value_types::null_value:
            return std::monostate{};

        case kcenon::container::value_types::bool_value:
            return grpc_value.bool_val();

        case kcenon::container::value_types::short_value:
            return static_cast<short>(grpc_value.short_val());

        case kcenon::container::value_types::ushort_value:
            return static_cast<unsigned short>(grpc_value.ushort_val());

        case kcenon::container::value_types::int_value:
            return grpc_value.int_val();

        case kcenon::container::value_types::uint_value:
            return grpc_value.uint_val();

        case kcenon::container::value_types::long_value:
            return static_cast<long>(grpc_value.long_val());

        case kcenon::container::value_types::ulong_value:
            return static_cast<unsigned long>(grpc_value.ulong_val());

        case kcenon::container::value_types::llong_value:
            return static_cast<long long>(grpc_value.llong_val());

        case kcenon::container::value_types::ullong_value:
            return static_cast<unsigned long long>(grpc_value.ullong_val());

        case kcenon::container::value_types::float_value:
            return grpc_value.float_val();

        case kcenon::container::value_types::double_value:
            return grpc_value.double_val();

        case kcenon::container::value_types::string_value:
            return grpc_value.string_val();

        case kcenon::container::value_types::bytes_value: {
            const auto& bytes_str = grpc_value.bytes_val();
            return std::vector<uint8_t>(bytes_str.begin(), bytes_str.end());
        }

        case kcenon::container::value_types::container_value: {
            if (grpc_value.has_container_val()) {
                return from_grpc(grpc_value.container_val());
            }
            return std::shared_ptr<kcenon::container::value_container>{};
        }

        case kcenon::container::value_types::array_value:
            // Array values would need special handling
            return std::monostate{};

        default:
            return std::monostate{};
    }
}

GrpcContainer container_adapter::to_grpc_recursive(
    const kcenon::container::value_container& container,
    int depth) {
    if (depth > MAX_NESTING_DEPTH) {
        throw std::runtime_error(
            "Maximum nesting depth exceeded during conversion to gRPC");
    }

    GrpcContainer grpc_container;

    // Set header fields
    grpc_container.set_source_id(container.source_id());
    grpc_container.set_source_sub_id(container.source_sub_id());
    grpc_container.set_target_id(container.target_id());
    grpc_container.set_target_sub_id(container.target_sub_id());
    grpc_container.set_message_type(container.message_type());
    grpc_container.set_version("1.0.0.0");

    // Convert values
    for (const auto& val : container) {
        auto* grpc_val = grpc_container.add_values();
        grpc_val->set_name(val.name);
        grpc_val->set_type(to_grpc_type(val.type));

        if (val.type == kcenon::container::value_types::container_value) {
            // Handle nested containers with depth tracking
            auto nested = std::get<
                std::shared_ptr<kcenon::container::value_container>>(val.data);
            if (nested) {
                *grpc_val->mutable_container_val() =
                    to_grpc_recursive(*nested, depth + 1);
            }
        } else {
            set_grpc_value_data(*grpc_val, val.data, val.type);
        }
    }

    return grpc_container;
}

std::shared_ptr<kcenon::container::value_container>
container_adapter::from_grpc_recursive(
    const GrpcContainer& grpc_container,
    int depth) {
    if (depth > MAX_NESTING_DEPTH) {
        throw std::runtime_error(
            "Maximum nesting depth exceeded during conversion from gRPC");
    }

    auto container = std::make_shared<kcenon::container::value_container>();

    // Set header fields
    container->set_source(grpc_container.source_id(),
                          grpc_container.source_sub_id());
    container->set_target(grpc_container.target_id(),
                          grpc_container.target_sub_id());
    container->set_message_type(grpc_container.message_type());

    // Convert values
    for (const auto& grpc_val : grpc_container.values()) {
        auto type = from_grpc_type(grpc_val.type());

        if (type == kcenon::container::value_types::container_value) {
            // Handle nested containers with depth tracking
            if (grpc_val.has_container_val()) {
                auto nested =
                    from_grpc_recursive(grpc_val.container_val(), depth + 1);
                container->set(grpc_val.name(),
                    std::shared_ptr<kcenon::container::value_container>(nested));
            }
        } else {
            auto data = get_variant_from_grpc(grpc_val, type);
            kcenon::container::optimized_value val;
            val.name = grpc_val.name();
            val.type = type;
            val.data = data;
            container->set(val);
        }
    }

    return container;
}

// =============================================================================
// value_mapper Implementation
// =============================================================================

const char* value_mapper::type_name(
    kcenon::container::value_types type) noexcept {
    switch (type) {
        case kcenon::container::value_types::null_value:
            return "null";
        case kcenon::container::value_types::bool_value:
            return "bool";
        case kcenon::container::value_types::short_value:
            return "short";
        case kcenon::container::value_types::ushort_value:
            return "ushort";
        case kcenon::container::value_types::int_value:
            return "int";
        case kcenon::container::value_types::uint_value:
            return "uint";
        case kcenon::container::value_types::long_value:
            return "long";
        case kcenon::container::value_types::ulong_value:
            return "ulong";
        case kcenon::container::value_types::llong_value:
            return "llong";
        case kcenon::container::value_types::ullong_value:
            return "ullong";
        case kcenon::container::value_types::float_value:
            return "float";
        case kcenon::container::value_types::double_value:
            return "double";
        case kcenon::container::value_types::string_value:
            return "string";
        case kcenon::container::value_types::bytes_value:
            return "bytes";
        case kcenon::container::value_types::container_value:
            return "container";
        case kcenon::container::value_types::array_value:
            return "array";
        default:
            return "unknown";
    }
}

const char* value_mapper::proto_type_name(ValueType type) noexcept {
    switch (type) {
        case NULL_VALUE:
            return "NULL_VALUE";
        case BOOL_VALUE:
            return "BOOL_VALUE";
        case SHORT_VALUE:
            return "SHORT_VALUE";
        case USHORT_VALUE:
            return "USHORT_VALUE";
        case INT_VALUE:
            return "INT_VALUE";
        case UINT_VALUE:
            return "UINT_VALUE";
        case LONG_VALUE:
            return "LONG_VALUE";
        case ULONG_VALUE:
            return "ULONG_VALUE";
        case LLONG_VALUE:
            return "LLONG_VALUE";
        case ULLONG_VALUE:
            return "ULLONG_VALUE";
        case FLOAT_VALUE:
            return "FLOAT_VALUE";
        case DOUBLE_VALUE:
            return "DOUBLE_VALUE";
        case STRING_VALUE:
            return "STRING_VALUE";
        case BYTES_VALUE:
            return "BYTES_VALUE";
        case CONTAINER_VALUE:
            return "CONTAINER_VALUE";
        case ARRAY_VALUE:
            return "ARRAY_VALUE";
        default:
            return "UNKNOWN";
    }
}

// =============================================================================
// size_calculator Implementation
// =============================================================================

size_t size_calculator::estimate_proto_size(
    kcenon::container::value_types type,
    size_t data_size) noexcept {
    // Base overhead for field tag and type
    constexpr size_t base_overhead = 3;

    switch (type) {
        case kcenon::container::value_types::null_value:
            return base_overhead + 1;
        case kcenon::container::value_types::bool_value:
            return base_overhead + 1;
        case kcenon::container::value_types::short_value:
        case kcenon::container::value_types::ushort_value:
        case kcenon::container::value_types::int_value:
        case kcenon::container::value_types::uint_value:
            return base_overhead + 5;  // varint
        case kcenon::container::value_types::long_value:
        case kcenon::container::value_types::ulong_value:
        case kcenon::container::value_types::llong_value:
        case kcenon::container::value_types::ullong_value:
            return base_overhead + 10;  // varint
        case kcenon::container::value_types::float_value:
            return base_overhead + 4;
        case kcenon::container::value_types::double_value:
            return base_overhead + 8;
        case kcenon::container::value_types::string_value:
        case kcenon::container::value_types::bytes_value:
            return base_overhead + data_size + 2;  // length prefix
        default:
            return base_overhead + data_size;
    }
}

size_t size_calculator::estimate_container_size(
    size_t value_count,
    size_t avg_name_length,
    size_t avg_data_size) noexcept {
    // Header fields estimate
    constexpr size_t header_overhead = 100;

    // Per-value estimate
    size_t per_value = avg_name_length + avg_data_size + 10;

    return header_overhead + (value_count * per_value);
}

} // namespace kcenon::container_grpc
