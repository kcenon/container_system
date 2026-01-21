/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, kcenon
All rights reserved.
*****************************************************************************/

/**
 * @file concepts.h
 * @brief C++20 concepts for container_system type validation.
 *
 * This header provides concepts for validating types used throughout
 * the container_system library. These concepts enable compile-time
 * validation with clear error messages, replacing SFINAE-based constraints.
 *
 * Requirements:
 * - C++20 compiler with concepts support
 * - GCC 10+, Clang 10+, MSVC 2022+
 *
 * Integration with common_system:
 * - Imports core concepts from kcenon::common::concepts
 * - Extends with container_system-specific concepts
 *
 * @see kcenon/common/concepts/concepts.h for common_system concepts
 */

#pragma once

#include <concepts>
#include <type_traits>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <variant>
#include <cstdint>

namespace container_module {

// Forward declarations
class thread_safe_container;
class value;
struct array_variant;

namespace concepts {

/**
 * @concept Arithmetic
 * @brief A type that is arithmetic (integral or floating-point).
 *
 * Example usage:
 * @code
 * template<Arithmetic T>
 * value make_numeric_value(std::string_view name, T val);
 * @endcode
 */
template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

/**
 * @concept IntegralType
 * @brief A type that is an integral type.
 */
template<typename T>
concept IntegralType = std::integral<T>;

/**
 * @concept FloatingPointType
 * @brief A type that is a floating-point type.
 */
template<typename T>
concept FloatingPointType = std::floating_point<T>;

/**
 * @concept SignedIntegral
 * @brief A signed integral type.
 */
template<typename T>
concept SignedIntegral = std::signed_integral<T>;

/**
 * @concept UnsignedIntegral
 * @brief An unsigned integral type.
 */
template<typename T>
concept UnsignedIntegral = std::unsigned_integral<T>;

/**
 * @concept TriviallyCopyable
 * @brief A type that can be safely copied with memcpy.
 *
 * Use this concept for types that require SIMD-optimized operations
 * or deterministic memory layout.
 *
 * Example usage:
 * @code
 * template<TriviallyCopyable T>
 * class simd_batch { ... };  // Renamed from typed_container (Issue #328)
 * @endcode
 */
template<typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

/**
 * @concept ValueVariantType
 * @brief A type that is valid for storage in ValueVariant.
 *
 * Valid types include:
 * - std::monostate (null)
 * - bool
 * - int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t
 * - float, double
 * - std::string
 * - std::vector<uint8_t> (bytes)
 * - std::shared_ptr<thread_safe_container>
 * - array_variant
 *
 * Example usage:
 * @code
 * template<ValueVariantType T>
 * void set_typed(std::string_view key, T&& val);
 * @endcode
 */
template<typename T>
concept ValueVariantType =
    std::same_as<std::decay_t<T>, std::monostate> ||
    std::same_as<std::decay_t<T>, bool> ||
    std::same_as<std::decay_t<T>, int16_t> ||
    std::same_as<std::decay_t<T>, uint16_t> ||
    std::same_as<std::decay_t<T>, int32_t> ||
    std::same_as<std::decay_t<T>, uint32_t> ||
    std::same_as<std::decay_t<T>, int64_t> ||
    std::same_as<std::decay_t<T>, uint64_t> ||
    std::same_as<std::decay_t<T>, float> ||
    std::same_as<std::decay_t<T>, double> ||
    std::same_as<std::decay_t<T>, std::string> ||
    std::same_as<std::decay_t<T>, std::vector<uint8_t>> ||
    std::same_as<std::decay_t<T>, std::shared_ptr<thread_safe_container>> ||
    std::same_as<std::decay_t<T>, array_variant>;

/**
 * @concept NumericValueType
 * @brief A numeric type suitable for value storage.
 *
 * Excludes bool (which has its own handling) and includes
 * all signed/unsigned integers and floating-point types.
 */
template<typename T>
concept NumericValueType =
    Arithmetic<T> && !std::same_as<std::decay_t<T>, bool>;

/**
 * @concept StringLike
 * @brief A type that can be converted to or used as a string.
 */
template<typename T>
concept StringLike =
    std::same_as<std::decay_t<T>, std::string> ||
    std::same_as<std::decay_t<T>, std::string_view> ||
    std::same_as<std::decay_t<T>, const char*> ||
    std::convertible_to<T, std::string_view>;

/**
 * @concept ByteContainer
 * @brief A type that represents byte data.
 */
template<typename T>
concept ByteContainer =
    std::same_as<std::decay_t<T>, std::vector<uint8_t>>;

/**
 * @concept ValueVisitor
 * @brief A callable type that can visit value variants.
 *
 * Example usage:
 * @code
 * template<ValueVisitor V>
 * auto visit(V&& visitor) const;
 * @endcode
 */
template<typename V>
concept ValueVisitor = std::move_constructible<V>;

/**
 * @concept KeyValueCallback
 * @brief A callable for key-value pair iteration.
 *
 * Example usage:
 * @code
 * template<KeyValueCallback Func>
 * void for_each(Func&& func) const;
 * @endcode
 */
template<typename F>
concept KeyValueCallback = std::invocable<F, const std::string&, const value&>;

/**
 * @concept MutableKeyValueCallback
 * @brief A callable for mutable key-value pair iteration.
 */
template<typename F>
concept MutableKeyValueCallback = std::invocable<F, const std::string&, value&>;

/**
 * @concept ValueMapCallback
 * @brief A callable that operates on the entire value map.
 */
template<typename F, typename Map>
concept ValueMapCallback = std::invocable<F, Map&>;

/**
 * @concept ConstValueMapCallback
 * @brief A callable that operates on a const value map.
 */
template<typename F, typename Map>
concept ConstValueMapCallback = std::invocable<F, const Map&>;

/**
 * @concept Serializable
 * @brief A type that provides serialization methods.
 */
template<typename T>
concept Serializable = requires(const T& t) {
    { t.serialize() } -> std::convertible_to<std::vector<uint8_t>>;
};

/**
 * @concept JsonSerializable
 * @brief A type that can be serialized to JSON.
 */
template<typename T>
concept JsonSerializable = requires(const T& t) {
    { t.to_json() } -> std::convertible_to<std::string>;
};

/**
 * @concept ContainerValue
 * @brief A type representing a nested container value.
 */
template<typename T>
concept ContainerValue =
    std::same_as<std::decay_t<T>, std::shared_ptr<thread_safe_container>>;

} // namespace concepts
} // namespace container_module
