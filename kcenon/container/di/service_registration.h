// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file service_registration.h
 * @brief Service container registration for container_system services.
 *
 * This header provides functions to register container_system services
 * with the unified service container from common_system.
 *
 * @see TICKET-103 for integration requirements.
 */

#pragma once

#include <memory>
#include <functional>

// Include feature flags for unified macro detection
#if __has_include(<kcenon/common/config/feature_flags.h>)
    #include <kcenon/common/config/feature_flags.h>
#endif

#if KCENON_HAS_COMMON_SYSTEM

#include <kcenon/common/di/service_container.h>

#include "../core/container.h"

namespace kcenon::container::di {

/**
 * @brief Interface for serialization operations
 *
 * This interface provides a common abstraction for serialization,
 * using container_system's value_container as the underlying implementation.
 */
class ISerializer {
public:
    virtual ~ISerializer() = default;

    /**
     * @brief Serialize data to string format
     * @param container The value_container to serialize
     * @return Serialized string representation
     */
    virtual std::string serialize(const container_module::value_container& container) const = 0;

    /**
     * @brief Serialize data to byte array format
     * @param container The value_container to serialize
     * @return Serialized byte array
     */
    virtual std::vector<uint8_t> serialize_bytes(const container_module::value_container& container) const = 0;

    /**
     * @brief Deserialize string to value_container
     * @param data String data to deserialize
     * @return Shared pointer to deserialized value_container
     */
    virtual std::shared_ptr<container_module::value_container> deserialize(const std::string& data) const = 0;

    /**
     * @brief Deserialize byte array to value_container
     * @param data Byte array to deserialize
     * @return Shared pointer to deserialized value_container
     */
    virtual std::shared_ptr<container_module::value_container> deserialize(const std::vector<uint8_t>& data) const = 0;

    /**
     * @brief Create a new empty value_container
     * @return Shared pointer to new value_container
     */
    virtual std::shared_ptr<container_module::value_container> create_container() const = 0;
};

/**
 * @brief Default implementation of ISerializer using value_container
 */
class value_container_serializer : public ISerializer {
public:
    value_container_serializer() = default;
    ~value_container_serializer() override = default;

    std::string serialize(const container_module::value_container& container) const override {
        // Create a copy to call non-const serialize
        container_module::value_container copy(container, false);
        return copy.serialize();
    }

    std::vector<uint8_t> serialize_bytes(const container_module::value_container& container) const override {
        container_module::value_container copy(container, false);
        return copy.serialize_array();
    }

    std::shared_ptr<container_module::value_container> deserialize(const std::string& data) const override {
        auto container = std::make_shared<container_module::value_container>(data, false);
        return container;
    }

    std::shared_ptr<container_module::value_container> deserialize(const std::vector<uint8_t>& data) const override {
        auto container = std::make_shared<container_module::value_container>(data, false);
        return container;
    }

    std::shared_ptr<container_module::value_container> create_container() const override {
        return std::make_shared<container_module::value_container>();
    }
};

/**
 * @brief Configuration for serializer service registration
 */
struct serializer_registration_config {
    /// Enable Small Object Optimization for containers
    bool enable_soo = true;

    /// Service lifetime (singleton or transient for serializers)
    common::di::service_lifetime lifetime = common::di::service_lifetime::singleton;
};

/**
 * @brief Register serializer services with the service container.
 *
 * Registers ISerializer implementation using container_system's value_container.
 * By default, registers as a singleton.
 *
 * @param container The service container to register with
 * @param config Optional configuration for the serializer
 * @return VoidResult indicating success or registration error
 *
 * @code
 * auto& container = common::di::service_container::global();
 *
 * // Register with default configuration
 * auto result = register_serializer_services(container);
 *
 * // Then resolve serializer anywhere in the application
 * auto serializer = container.resolve<ISerializer>().value();
 * auto data_container = serializer->create_container();
 * data_container->set_value("name", std::string("John"));
 * data_container->set_value("age", 30);
 * std::string serialized = serializer->serialize(*data_container);
 * @endcode
 */
inline common::VoidResult register_serializer_services(
    common::di::IServiceContainer& container,
    const serializer_registration_config& config = {}) {

    // Check if already registered
    if (container.is_registered<ISerializer>()) {
        return common::make_error<std::monostate>(
            common::di::di_error_codes::already_registered,
            "ISerializer is already registered",
            "container_system::di"
        );
    }

    // Register serializer factory
    return container.register_factory<ISerializer>(
        [config](common::di::IServiceContainer&) -> std::shared_ptr<ISerializer> {
            return std::make_shared<value_container_serializer>();
        },
        config.lifetime
    );
}

/**
 * @brief Configuration for value_container factory registration
 */
struct container_factory_config {
    /// Enable Small Object Optimization for created containers
    bool enable_soo = true;

    /// Service lifetime (transient for factories)
    common::di::service_lifetime lifetime = common::di::service_lifetime::transient;
};

/**
 * @brief Factory type for creating value_container instances
 */
using ValueContainerFactory = std::function<std::shared_ptr<container_module::value_container>()>;

/**
 * @brief Register value_container factory with the service container.
 *
 * Registers a factory for creating new value_container instances.
 * By default, registers as transient to create new instances each time.
 *
 * @param container The service container to register with
 * @param config Optional configuration for the factory
 * @return VoidResult indicating success or registration error
 *
 * @code
 * auto& container = common::di::service_container::global();
 *
 * // Register factory
 * auto result = register_container_factory(container);
 *
 * // Resolve factory and create containers
 * auto factory = container.resolve<ValueContainerFactory>().value();
 * auto data = factory();
 * data->set_value("key", std::string("value"));
 * @endcode
 */
inline common::VoidResult register_container_factory(
    common::di::IServiceContainer& container,
    const container_factory_config& config = {}) {

    // Check if already registered
    if (container.is_registered<ValueContainerFactory>()) {
        return common::make_error<std::monostate>(
            common::di::di_error_codes::already_registered,
            "ValueContainerFactory is already registered",
            "container_system::di"
        );
    }

    // Register container factory
    return container.register_factory<ValueContainerFactory>(
        [config](common::di::IServiceContainer&) -> std::shared_ptr<ValueContainerFactory> {
            auto factory = std::make_shared<ValueContainerFactory>(
                [soo_enabled = config.enable_soo]() -> std::shared_ptr<container_module::value_container> {
                    auto container = std::make_shared<container_module::value_container>();
                    container->set_soo_enabled(soo_enabled);
                    return container;
                }
            );
            return factory;
        },
        config.lifetime
    );
}

/**
 * @brief Unregister serializer services from the container.
 *
 * @param container The service container to unregister from
 * @return VoidResult indicating success or error
 */
inline common::VoidResult unregister_serializer_services(
    common::di::IServiceContainer& container) {

    return container.unregister<ISerializer>();
}

/**
 * @brief Unregister container factory from the container.
 *
 * @param container The service container to unregister from
 * @return VoidResult indicating success or error
 */
inline common::VoidResult unregister_container_factory(
    common::di::IServiceContainer& container) {

    return container.unregister<ValueContainerFactory>();
}

/**
 * @brief Register all container_system services with the container.
 *
 * Convenience function to register all available container_system services.
 *
 * @param container The service container to register with
 * @param serializer_config Optional configuration for serializer
 * @param factory_config Optional configuration for container factory
 * @return VoidResult indicating success or registration error
 */
inline common::VoidResult register_all_container_services(
    common::di::IServiceContainer& container,
    const serializer_registration_config& serializer_config = {},
    const container_factory_config& factory_config = {}) {

    // Register ISerializer
    auto result = register_serializer_services(container, serializer_config);
    if (result.is_err()) {
        return result;
    }

    // Register ValueContainerFactory
    result = register_container_factory(container, factory_config);
    if (result.is_err()) {
        // Rollback serializer registration
        unregister_serializer_services(container);
        return result;
    }

    return common::VoidResult::ok({});
}

} // namespace kcenon::container::di

#endif // KCENON_HAS_COMMON_SYSTEM
