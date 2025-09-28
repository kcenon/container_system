#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, container_system contributors
All rights reserved.
*****************************************************************************/

#include <memory>
#include <string>
#include <variant>
#include <optional>
#include <type_traits>

// Check if common_system is available
#ifdef BUILD_WITH_COMMON_SYSTEM
#include <kcenon/common/patterns/result.h>
#endif

#include "../container.h"
#include "../serialization.h"
#include "../deserialization.h"

namespace container {
namespace adapters {

#ifdef BUILD_WITH_COMMON_SYSTEM

/**
 * @brief Result type conversions between container_system and common_system
 */
template<typename T>
using container_result = std::variant<T, std::string>; // Simple error type for container

/**
 * @brief Convert container_result to common::Result
 */
template<typename T>
inline ::common::Result<T> to_common_result(const container_result<T>& result) {
    if (std::holds_alternative<T>(result)) {
        return ::common::Result<T>(std::get<T>(result));
    } else {
        const auto& error = std::get<std::string>(result);
        return ::common::Result<T>(::common::error_info(1, error, "container_system"));
    }
}

/**
 * @brief Convert common::Result to container_result
 */
template<typename T>
inline container_result<T> from_common_result(const ::common::Result<T>& result) {
    if (!::common::is_error(result)) {
        return container_result<T>(::common::get_value(result));
    } else {
        const auto& error = ::common::get_error(result);
        return container_result<T>(error.message);
    }
}

/**
 * @brief Adapter for serialization operations with Result<T> error handling
 */
class serialization_result_adapter {
public:
    /**
     * @brief Serialize a container with Result<T> error handling
     */
    template<typename T>
    static ::common::Result<std::vector<uint8_t>> serialize(const T& value) {
        try {
            serializer ser;
            ser.write(value);
            return ::common::Result<std::vector<uint8_t>>(ser.get_data());
        } catch (const std::exception& e) {
            return ::common::error_info(1, e.what(), "container_system");
        }
    }

    /**
     * @brief Serialize to string with Result<T> error handling
     */
    template<typename T>
    static ::common::Result<std::string> serialize_to_string(const T& value) {
        try {
            serializer ser;
            ser.write(value);
            auto data = ser.get_data();
            return ::common::Result<std::string>(
                std::string(data.begin(), data.end()));
        } catch (const std::exception& e) {
            return ::common::error_info(1, e.what(), "container_system");
        }
    }

    /**
     * @brief Serialize container object with Result<T> error handling
     */
    static ::common::Result<std::vector<uint8_t>> serialize_container(
        const container::container& cont) {
        try {
            return ::common::Result<std::vector<uint8_t>>(cont.serialize());
        } catch (const std::exception& e) {
            return ::common::error_info(1, e.what(), "container_system");
        }
    }
};

/**
 * @brief Adapter for deserialization operations with Result<T> error handling
 */
class deserialization_result_adapter {
public:
    /**
     * @brief Deserialize with Result<T> error handling
     */
    template<typename T>
    static ::common::Result<T> deserialize(const std::vector<uint8_t>& data) {
        try {
            deserializer deser(data);
            T value;
            deser.read(value);
            return ::common::Result<T>(value);
        } catch (const std::exception& e) {
            return ::common::error_info(1, e.what(), "container_system");
        }
    }

    /**
     * @brief Deserialize from string with Result<T> error handling
     */
    template<typename T>
    static ::common::Result<T> deserialize_from_string(const std::string& str) {
        try {
            std::vector<uint8_t> data(str.begin(), str.end());
            deserializer deser(data);
            T value;
            deser.read(value);
            return ::common::Result<T>(value);
        } catch (const std::exception& e) {
            return ::common::error_info(1, e.what(), "container_system");
        }
    }

    /**
     * @brief Deserialize container object with Result<T> error handling
     */
    static ::common::Result<container::container> deserialize_container(
        const std::vector<uint8_t>& data) {
        try {
            container::container cont;
            cont.deserialize(data);
            return ::common::Result<container::container>(cont);
        } catch (const std::exception& e) {
            return ::common::error_info(1, e.what(), "container_system");
        }
    }
};

/**
 * @brief Adapter for container operations with Result<T> error handling
 */
class container_result_adapter {
public:
    /**
     * @brief Get value from container with Result<T> error handling
     */
    template<typename T>
    static ::common::Result<T> get_value(
        const container::container& cont,
        const std::string& key) {
        try {
            if (!cont.has_key(key)) {
                return ::common::error_info(2, "Key not found: " + key, "container_system");
            }
            return ::common::Result<T>(cont.get<T>(key));
        } catch (const std::exception& e) {
            return ::common::error_info(3, e.what(), "container_system");
        }
    }

    /**
     * @brief Set value in container with Result<T> error handling
     */
    template<typename T>
    static ::common::VoidResult set_value(
        container::container& cont,
        const std::string& key,
        const T& value) {
        try {
            cont.set(key, value);
            return ::common::VoidResult(std::monostate{});
        } catch (const std::exception& e) {
            return ::common::error_info(3, e.what(), "container_system");
        }
    }

    /**
     * @brief Remove value from container with Result<T> error handling
     */
    static ::common::VoidResult remove_value(
        container::container& cont,
        const std::string& key) {
        try {
            if (!cont.has_key(key)) {
                return ::common::error_info(2, "Key not found: " + key, "container_system");
            }
            cont.remove(key);
            return ::common::VoidResult(std::monostate{});
        } catch (const std::exception& e) {
            return ::common::error_info(3, e.what(), "container_system");
        }
    }

    /**
     * @brief Merge containers with Result<T> error handling
     */
    static ::common::Result<container::container> merge_containers(
        const container::container& cont1,
        const container::container& cont2) {
        try {
            container::container merged = cont1;
            merged.merge(cont2);
            return ::common::Result<container::container>(merged);
        } catch (const std::exception& e) {
            return ::common::error_info(4, e.what(), "container_system");
        }
    }
};

/**
 * @brief Utility functions for Result<T> chaining
 */
template<typename T, typename Func>
auto map_result(const ::common::Result<T>& result, Func&& func)
    -> ::common::Result<decltype(func(std::declval<T>()))> {
    using ReturnType = decltype(func(std::declval<T>()));

    if (::common::is_error(result)) {
        return ::common::Result<ReturnType>(::common::get_error(result));
    }

    try {
        return ::common::Result<ReturnType>(func(::common::get_value(result)));
    } catch (const std::exception& e) {
        return ::common::error_info(5, e.what(), "container_system");
    }
}

/**
 * @brief Flat map for Result<T> (monadic bind)
 */
template<typename T, typename Func>
auto flat_map_result(const ::common::Result<T>& result, Func&& func)
    -> decltype(func(std::declval<T>())) {
    if (::common::is_error(result)) {
        using ReturnType = decltype(func(std::declval<T>()));
        return ReturnType(::common::get_error(result));
    }

    return func(::common::get_value(result));
}

/**
 * @brief Factory for creating Result-based operations
 */
class common_result_factory {
public:
    /**
     * @brief Create a serialization adapter with Result<T> support
     */
    static std::unique_ptr<serialization_result_adapter> create_serialization_adapter() {
        return std::make_unique<serialization_result_adapter>();
    }

    /**
     * @brief Create a deserialization adapter with Result<T> support
     */
    static std::unique_ptr<deserialization_result_adapter> create_deserialization_adapter() {
        return std::make_unique<deserialization_result_adapter>();
    }

    /**
     * @brief Create a container adapter with Result<T> support
     */
    static std::unique_ptr<container_result_adapter> create_container_adapter() {
        return std::make_unique<container_result_adapter>();
    }

    /**
     * @brief Create a success Result
     */
    template<typename T>
    static ::common::Result<T> success(T&& value) {
        return ::common::Result<T>(std::forward<T>(value));
    }

    /**
     * @brief Create an error Result
     */
    template<typename T>
    static ::common::Result<T> error(int code, const std::string& message) {
        return ::common::Result<T>(
            ::common::error_info(code, message, "container_system"));
    }
};

#endif // BUILD_WITH_COMMON_SYSTEM

} // namespace adapters
} // namespace container