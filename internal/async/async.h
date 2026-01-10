/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
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

/**
 * @file internal/async/async.h
 * @brief Main header for C++20 coroutine async support
 *
 * Include this header to use async features:
 * @code
 * #include <container/internal/async/async.h>
 *
 * using namespace container_module::async;
 *
 * task<int> compute() {
 *     co_return 42;
 * }
 * @endcode
 *
 * @see container_module::async::task
 * @see container_module::async::generator
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

namespace container_module::async
{
    /**
     * @brief Check if coroutines are available at compile time
     */
    inline constexpr bool has_coroutine_support = true;

} // namespace container_module::async

#else

// Fallback when coroutines are not available
namespace container_module::async
{
    /**
     * @brief Check if coroutines are available at compile time
     */
    inline constexpr bool has_coroutine_support = false;

} // namespace container_module::async

#endif // CONTAINER_HAS_COROUTINES
