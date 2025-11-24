/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#pragma once

#include "container/core/value_types.h"
#include <variant>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <optional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <type_traits>

namespace container_module
{
    // Forward declarations
    class thread_safe_container;
    class value;

    /**
     * @brief Platform-specific type handling for long long
     *
     * Type equivalence across platforms:
     * - macOS/Linux: int64_t == long long (always)
     * - MSVC 2015+ (_MSC_VER >= 1900): int64_t == long long (typedef)
     * - MSVC 2013- (_MSC_VER < 1900): int64_t == __int64 (distinct from long long)
     *
     * We use std::monostate as placeholder when types are identical
     * to avoid variant duplicate type compilation errors.
     */
#if defined(_MSC_VER) && _MSC_VER < 1900
    // Old MSVC (2013 and earlier): int64_t is __int64, long long is separate
    using llong_placeholder_t = long long;
    using ullong_placeholder_t = unsigned long long;
    constexpr bool has_separate_llong = true;
#else
    // Modern platforms (macOS, Linux, MSVC 2015+): int64_t is long long
    using llong_placeholder_t = std::monostate;
    using ullong_placeholder_t = std::monostate;
    constexpr bool has_separate_llong = false;
#endif

    /**
     * @brief Recursive array type for variant
     *
     * array_value (type 15) stores heterogeneous collections.
     * Uses vector of value for type safety.
     */
    struct array_variant {
        std::vector<std::shared_ptr<value>> values;

        array_variant() = default;
        array_variant(const array_variant& other);
        array_variant(array_variant&& other) noexcept = default;
        array_variant& operator=(const array_variant& other);
        array_variant& operator=(array_variant&& other) noexcept = default;

        bool operator==(const array_variant& other) const;
        bool operator!=(const array_variant& other) const { return !(*this == other); }
        bool operator<(const array_variant& other) const;
    };

    /**
     * @brief Type-aligned variant matching value_types enum EXACTLY
     *
     * CRITICAL: The order of types MUST match value_types enum (0-15)
     * This ensures variant::index() == static_cast<size_t>(value_types)
     *
     * Index mapping verification:
     * - variant.index() 0 == value_types::null_value (0)
     * - variant.index() 1 == value_types::bool_value (1)
     * - variant.index() 2 == value_types::short_value (2)
     * ...
     * - variant.index() 15 == value_types::array_value (15)
     */
    using ValueVariant = std::variant<
        std::monostate,                         // 0: null_value
        bool,                                   // 1: bool_value
        int16_t,                                // 2: short_value
        uint16_t,                               // 3: ushort_value
        int32_t,                                // 4: int_value
        uint32_t,                               // 5: uint_value
        int64_t,                                // 6: long_value
        uint64_t,                               // 7: ulong_value
        llong_placeholder_t,                    // 8: llong_value (monostate on macOS/Linux)
        ullong_placeholder_t,                   // 9: ullong_value (monostate on macOS/Linux)
        float,                                  // 10: float_value
        double,                                 // 11: double_value
        std::string,                            // 12: string_value
        std::vector<uint8_t>,                   // 13: bytes_value
        std::shared_ptr<thread_safe_container>, // 14: container_value
        array_variant                           // 15: array_value
    >;

    /**
     * @brief Compile-time verification of type alignment
     *
     * These static assertions ensure that variant indices match value_types enum.
     * If compilation fails here, the variant type order is wrong.
     */
    namespace detail {
        template<typename T, typename Variant>
        constexpr size_t variant_index_of() {
            return std::variant<T, Variant>::index();
        }

        // Verify critical type alignments
        static_assert(std::variant_size_v<ValueVariant> == 16,
                     "ValueVariant must have exactly 16 types");
        static_assert(std::is_same_v<std::variant_alternative_t<0, ValueVariant>, std::monostate>,
                     "Index 0 must be std::monostate");
        static_assert(std::is_same_v<std::variant_alternative_t<1, ValueVariant>, bool>,
                     "Index 1 must be bool");
        static_assert(std::is_same_v<std::variant_alternative_t<12, ValueVariant>, std::string>,
                     "Index 12 must be string");
        static_assert(std::is_same_v<std::variant_alternative_t<13, ValueVariant>, std::vector<uint8_t>>,
                     "Index 13 must be bytes (vector<uint8_t>)");
        static_assert(std::is_same_v<std::variant_alternative_t<15, ValueVariant>, array_variant>,
                     "Index 15 must be array_variant");
    }

    /**
     * @brief Enhanced type-safe value with perfect legacy compatibility
     *
     * Key improvements over variant_value:
     * 1. Type indices match value_types enum exactly
     * 2. Support for array_value (type 15)
     * 3. Platform-aware llong/ullong handling
     * 4. Zero data loss on serialization round-trips
     * 5. Backward compatible with legacy value system
     */
    class value
    {
    public:
        /**
         * @brief Default constructor - creates null value
         */
        value() : name_(""), data_(std::in_place_index<0>) {}

        /**
         * @brief Construct with name (null value)
         */
        explicit value(std::string_view name)
            : name_(name), data_(std::in_place_index<0>) {}

        /**
         * @brief Construct with name and typed value
         */
        template<typename T>
        value(std::string_view name, T&& value)
            : name_(name), data_(std::forward<T>(value)) {}

        /**
         * @brief Construct from value_types and raw data (legacy compatibility)
         */
        value(std::string_view name, value_types type, const std::vector<uint8_t>& raw_data);

        /**
         * @brief Copy constructor (thread-safe)
         */
        value(const value& other);

        /**
         * @brief Move constructor
         */
        value(value&& other) noexcept;

        /**
         * @brief Copy assignment (thread-safe)
         */
        value& operator=(const value& other);

        /**
         * @brief Move assignment
         */
        value& operator=(value&& other) noexcept;

        /**
         * @brief Get the name of this value (lock-free, immutable)
         */
        std::string_view name() const noexcept { return name_; }

        /**
         * @brief Get the value_types enum (NOT variant::index()!)
         *
         * IMPORTANT: This returns the logical type, which matches:
         * - value_types enum (0-15)
         * - Wire format type byte
         * - Legacy system type
         *
         * On platforms where llong == int64_t, llong_value returns as long_value.
         */
        value_types type() const;

        /**
         * @brief Get variant index (for internal use only)
         */
        size_t variant_index() const {
            std::shared_lock lock(mutex_);
            return data_.index();
        }

        /**
         * @brief Check if null value
         */
        bool is_null() const {
            std::shared_lock lock(mutex_);
            return data_.index() == 0;
        }

        /**
         * @brief Type-safe getter with optional return
         * @tparam T The type to retrieve (must be exact variant type)
         * @return std::optional containing value if type matches
         */
        template<typename T>
        std::optional<T> get() const {
            std::shared_lock lock(mutex_);
            if (auto* ptr = std::get_if<T>(&data_)) {
                return *ptr;
            }
            return std::nullopt;
        }

        /**
         * @brief Type-safe setter
         * @tparam T The type to store (must be valid variant type)
         */
        template<typename T>
        void set(T&& value) {
            std::unique_lock lock(mutex_);
            data_ = std::forward<T>(value);
            write_count_.fetch_add(1, std::memory_order_relaxed);
        }

        /**
         * @brief Apply visitor to contained value (const)
         */
        template<typename Visitor>
        auto visit(Visitor&& vis) const {
            std::shared_lock lock(mutex_);
            read_count_.fetch_add(1, std::memory_order_relaxed);
            return std::visit(std::forward<Visitor>(vis), data_);
        }

        /**
         * @brief Apply visitor to contained value (mutable)
         */
        template<typename Visitor>
        auto visit_mut(Visitor&& vis) {
            std::unique_lock lock(mutex_);
            write_count_.fetch_add(1, std::memory_order_relaxed);
            return std::visit(std::forward<Visitor>(vis), data_);
        }

        /**
         * @brief Convert to string representation
         */
        std::string to_string() const;

        /**
         * @brief Convert to JSON representation
         */
        std::string to_json() const;

        /**
         * @brief Serialize to binary format (legacy compatible)
         *
         * Wire format:
         * [name_len:4][name:UTF-8][type:1][data:variable]
         *
         * Type byte matches value_types enum EXACTLY (0-15)
         */
        std::vector<uint8_t> serialize() const;

        /**
         * @brief Deserialize from binary format (legacy compatible)
         *
         * Supports both:
         * - New value serialized data
         * - Legacy value class serialized data
         *
         * @return std::optional containing value if deserialization succeeds
         */
        static std::optional<value> deserialize(const std::vector<uint8_t>& data);

        /**
         * @brief Get read/write statistics
         */
        size_t read_count() const { return read_count_.load(std::memory_order_relaxed); }
        size_t write_count() const { return write_count_.load(std::memory_order_relaxed); }

        /**
         * @brief Comparison operators (thread-safe)
         */
        bool operator==(const value& other) const;
        bool operator!=(const value& other) const { return !(*this == other); }
        bool operator<(const value& other) const;

    private:
        /**
         * @brief Serialize data portion based on type
         */
        void serialize_data(std::vector<uint8_t>& result) const;

        /**
         * @brief Deserialize data portion based on type
         */
        static bool deserialize_data(value& result,
                                     value_types type,
                                     const std::vector<uint8_t>& data,
                                     size_t& offset);

        const std::string name_;  // Immutable for lock-free access
        ValueVariant data_;
        mutable std::shared_mutex mutex_;
        mutable std::atomic<size_t> read_count_{0};
        mutable std::atomic<size_t> write_count_{0};
    };

    /**
     * @brief Type traits for variant types
     */
    template<typename T>
    struct is_variant_type_v2 : std::false_type {};

    // Specialize for each variant type
    template<> struct is_variant_type_v2<std::monostate> : std::true_type {};
    template<> struct is_variant_type_v2<bool> : std::true_type {};
    template<> struct is_variant_type_v2<int16_t> : std::true_type {};
    template<> struct is_variant_type_v2<uint16_t> : std::true_type {};
    template<> struct is_variant_type_v2<int32_t> : std::true_type {};
    template<> struct is_variant_type_v2<uint32_t> : std::true_type {};
    template<> struct is_variant_type_v2<int64_t> : std::true_type {};
    template<> struct is_variant_type_v2<uint64_t> : std::true_type {};
    template<> struct is_variant_type_v2<float> : std::true_type {};
    template<> struct is_variant_type_v2<double> : std::true_type {};
    template<> struct is_variant_type_v2<std::vector<uint8_t>> : std::true_type {};
    template<> struct is_variant_type_v2<std::string> : std::true_type {};
    template<> struct is_variant_type_v2<std::shared_ptr<thread_safe_container>> : std::true_type {};
    template<> struct is_variant_type_v2<array_variant> : std::true_type {};

    template<typename T>
    inline constexpr bool is_variant_type_v2_v = is_variant_type_v2<T>::value;

} // namespace container_module
