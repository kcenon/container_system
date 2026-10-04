// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

#pragma once

/**
 * @file internal/pool_allocator_adapter.h
 * @brief STL-compatible allocator adapter over @ref pool_allocator
 *
 * Provides an `std::allocator_traits`-compliant wrapper around the existing
 * thread-local @ref kcenon::container::internal::pool_allocator singleton so
 * that standard library containers (std::vector, std::list,
 * std::unordered_map, etc.) can use the pool transparently.
 *
 * Design notes:
 * - Stateless: all instances are interchangeable. `is_always_equal = true_type`.
 * - Uses the thread-local singleton from pool_allocator.h for the actual
 *   small/medium size-class allocation fast path. Allocations that exceed the
 *   pool's medium threshold fall back to the singleton's heap path, matching
 *   the existing behaviour.
 * - `propagate_on_container_*` are all `false_type` (the default) because the
 *   allocator is stateless and always equal.
 * - Supports rebind through the class template itself (see @c rebind nested
 *   template), which in turn makes it compatible with pre-C++20 allocator
 *   machinery in addition to @c std::allocator_traits.
 */

#include "pool_allocator.h"

#include <cstddef>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>

namespace kcenon::container::internal {

/**
 * @brief STL-compatible allocator adapter that routes allocations through
 *        @ref pool_allocator.
 *
 * Satisfies the C++ AllocatorAwareContainer and Allocator named requirements
 * so it can be used as the template argument to any STL container.
 *
 * Example:
 * @code
 *   std::vector<int, kcenon::container::internal::pool_allocator_adapter<int>> v;
 *   v.reserve(16);
 *   v.push_back(42);
 * @endcode
 *
 * @tparam T Element type. Must be destructible.
 */
template<typename T>
class pool_allocator_adapter {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // Propagation traits: adapter is stateless, so propagation is unnecessary.
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap = std::false_type;
    using is_always_equal = std::true_type;

    /**
     * @brief Rebind mechanism required for node-based containers
     *        (e.g. std::list, std::map) that allocate internal node types.
     */
    template<typename U>
    struct rebind {
        using other = pool_allocator_adapter<U>;
    };

    /// Default constructor. The adapter is stateless.
    constexpr pool_allocator_adapter() noexcept = default;

    /// Copy constructor. Stateless: nothing to copy.
    constexpr pool_allocator_adapter(const pool_allocator_adapter&) noexcept = default;

    /// Rebind constructor. Required by std::allocator_traits.
    template<typename U>
    constexpr pool_allocator_adapter(const pool_allocator_adapter<U>&) noexcept {}

    ~pool_allocator_adapter() = default;

    /**
     * @brief Allocate storage for @p n objects of type @c T.
     *
     * Routes through the thread-local @ref pool_allocator singleton, which
     * selects the small, medium, or heap path based on total byte size. Throws
     * @c std::bad_alloc on failure to mirror @c std::allocator behaviour.
     *
     * @param n Number of elements to allocate.
     * @return Pointer to uninitialised storage for @p n objects.
     */
    [[nodiscard]] pointer allocate(size_type n) {
        if (n == 0) {
            return nullptr;
        }
        if (n > max_size()) {
            throw std::bad_alloc();
        }

        const size_type bytes = n * sizeof(T);
        void* ptr = pool_allocator::instance().allocate(bytes);
        if (!ptr) {
            throw std::bad_alloc();
        }
        return static_cast<pointer>(ptr);
    }

    /**
     * @brief Deallocate storage previously obtained from @ref allocate.
     *
     * @param p Pointer returned by a prior call to @ref allocate.
     * @param n Number of elements originally requested.
     */
    void deallocate(pointer p, size_type n) noexcept {
        if (!p) {
            return;
        }
        const size_type bytes = n * sizeof(T);
        pool_allocator::instance().deallocate(static_cast<void*>(p), bytes);
    }

    /**
     * @brief Maximum number of elements that can be allocated in a single call.
     */
    constexpr size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }
};

/**
 * @brief Equality: all adapter instances are interchangeable (stateless).
 */
template<typename T, typename U>
constexpr bool operator==(const pool_allocator_adapter<T>&,
                          const pool_allocator_adapter<U>&) noexcept {
    return true;
}

/**
 * @brief Inequality: negation of @ref operator==.
 */
template<typename T, typename U>
constexpr bool operator!=(const pool_allocator_adapter<T>& a,
                          const pool_allocator_adapter<U>& b) noexcept {
    return !(a == b);
}

} // namespace kcenon::container::internal
