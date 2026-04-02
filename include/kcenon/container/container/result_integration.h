// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file core/container/result_integration.h
 * @brief Unified Result<T> integration header
 *
 * This header centralizes all Result<T> integration boilerplate code,
 * eliminating duplication across multiple files. It provides:
 *
 * - Feature flag detection (KCENON_HAS_COMMON_SYSTEM)
 * - Result<T> header inclusion (with fallback paths)
 * - Namespace aliases for compatibility
 * - Unified CONTAINER_HAS_RESULT macro
 *
 * @note Include this header instead of manually checking for Result<T> availability.
 *
 * @code
 * #include "core/container/result_integration.h"
 *
 * #if CONTAINER_HAS_RESULT
 * kcenon::common::Result<int> get_value() { ... }
 * #endif
 * @endcode
 *
 * @see Issue #335: Extract Result<T> integration boilerplate
 */

#pragma once

// =============================================================================
// Feature Flag Detection
// =============================================================================

#if __has_include(<kcenon/common/config/feature_flags.h>)
    #include <kcenon/common/config/feature_flags.h>
#endif

// Ensure KCENON_HAS_COMMON_SYSTEM is defined (default to 0 if not)
#ifndef KCENON_HAS_COMMON_SYSTEM
    #define KCENON_HAS_COMMON_SYSTEM 0
#endif

// =============================================================================
// Result<T> Integration
// =============================================================================

#if KCENON_HAS_COMMON_SYSTEM
    #if __has_include(<kcenon/common/patterns/result.h>)
        #include <kcenon/common/patterns/result.h>
        #define CONTAINER_HAS_RESULT 1
    #elif __has_include(<common/patterns/result.h>)
        #include <common/patterns/result.h>
        #define CONTAINER_HAS_RESULT 1

        // Create namespace fallback for compatibility
        #ifndef KCENON_COMMON_RESULT_FALLBACK_DEFINED
        #define KCENON_COMMON_RESULT_FALLBACK_DEFINED
        namespace kcenon {
        namespace common {
            using ::common::error_info;

            template<typename T>
            using Result = ::common::Result<T>;

            using VoidResult = ::common::VoidResult;

            using ::common::ok;
            using ::common::error;
            using ::common::is_ok;
            using ::common::is_error;
            using ::common::get_value;
            using ::common::get_error;
            using ::common::get_if_ok;
            using ::common::get_if_error;
            using ::common::value_or;
            using ::common::map;
            using ::common::and_then;
            using ::common::or_else;
            using ::common::try_catch;

            // Import common system error codes as namespace alias
            namespace error_codes = ::common::error_codes;
        } // namespace common
        } // namespace kcenon
        #endif // KCENON_COMMON_RESULT_FALLBACK_DEFINED
    #else
        #define CONTAINER_HAS_RESULT 0
    #endif
#else
    #define CONTAINER_HAS_RESULT 0
#endif

// =============================================================================
// Deprecated Macro Aliases (for backward compatibility)
// =============================================================================

// Map old per-file macros to unified macro
#ifndef CONTAINER_HAS_COMMON_RESULT
    #define CONTAINER_HAS_COMMON_RESULT CONTAINER_HAS_RESULT
#endif

#ifndef POLICY_CONTAINER_HAS_COMMON_RESULT
    #define POLICY_CONTAINER_HAS_COMMON_RESULT CONTAINER_HAS_RESULT
#endif
