/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, ??

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
 * @file deprecation.h
 * @brief Deprecation warning macros and utilities
 *
 * Provides cross-platform macros for suppressing deprecation warnings
 * during the migration from legacy polymorphic value system to
 * variant-based system.
 */

#pragma once

// GCC/Clang deprecation warning suppression
#if defined(__GNUC__) || defined(__clang__)
    #define CONTAINER_SUPPRESS_DEPRECATION_START \
        _Pragma("GCC diagnostic push") \
        _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")

    #define CONTAINER_SUPPRESS_DEPRECATION_END \
        _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
    // MSVC deprecation warning suppression
    #define CONTAINER_SUPPRESS_DEPRECATION_START \
        __pragma(warning(push)) \
        __pragma(warning(disable: 4996))

    #define CONTAINER_SUPPRESS_DEPRECATION_END \
        __pragma(warning(pop))
#else
    // Fallback for unknown compilers
    #define CONTAINER_SUPPRESS_DEPRECATION_START
    #define CONTAINER_SUPPRESS_DEPRECATION_END
#endif

/**
 * @def CONTAINER_LEGACY_API_DEPRECATION_MESSAGE
 * @brief Standard deprecation message for legacy value types
 */
#define CONTAINER_LEGACY_API_DEPRECATION_MESSAGE \
    "Legacy polymorphic value types are deprecated. " \
    "Use variant_value_v2 and set_value()/get_value() API instead. " \
    "See docs/advanced/VARIANT_VALUE_V2_MIGRATION_GUIDE.md"

/**
 * Example usage:
 *
 * @code
 * CONTAINER_SUPPRESS_DEPRECATION_START
 * auto val = std::make_shared<int_value>("key", 42);  // No warning
 * CONTAINER_SUPPRESS_DEPRECATION_END
 * @endcode
 */
