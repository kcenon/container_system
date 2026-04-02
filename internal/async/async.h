// BSD 3-Clause License
// Copyright (c) 2024, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file internal/async/async.h
 * @brief Main header for C++20 coroutine async support
 *
 * Include this header to use async features:
 * @code
 * #include <internal/async/async.h>
 *
 * using namespace kcenon::container::async;
 *
 * task<int> compute() {
 *     co_return 42;
 * }
 * @endcode
 *
 * @see kcenon::container::async::task
 * @see kcenon::container::async::generator
 */

#pragma once

// Feature detection
#if defined(__cpp_impl_coroutine) && __cpp_impl_coroutine >= 201902L
    #define CONTAINER_HAS_COROUTINES 1
#elif defined(_MSC_VER) && _MSC_VER >= 1928
    // MSVC 2019 16.8+ has coroutine support
    #define CONTAINER_HAS_COROUTINES 1
#else
    #define CONTAINER_HAS_COROUTINES 0
#endif

#if CONTAINER_HAS_COROUTINES

#include "task.h"
#include "generator.h"
#include "thread_pool_executor.h"
#include "async_container.h"

namespace kcenon::container::async
{
    /**
     * @brief Check if coroutines are available at compile time
     */
    inline constexpr bool has_coroutine_support = true;

} // namespace kcenon::container::async

#else

// Fallback when coroutines are not available
namespace kcenon::container::async
{
    /**
     * @brief Check if coroutines are available at compile time
     */
    inline constexpr bool has_coroutine_support = false;

} // namespace kcenon::container::async

#endif // CONTAINER_HAS_COROUTINES
