// BSD 3-Clause License
//
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * @file container.h
 * @brief Main include file for the container module
 *
 * @deprecated This header path is deprecated. Please use the canonical path:
 *             #include <kcenon/container/container.h>
 *             This header will be removed in v0.5.0.0.
 *
 * This file includes all the public headers from the container module.
 * Users should include this file to use the container functionality.
 */

#pragma once

// =============================================================================
// DEPRECATION WARNING
// =============================================================================
// This header path is deprecated and will be removed in v0.5.0.0.
// Please migrate to the canonical include path:
//     #include <kcenon/container/container.h>
// =============================================================================
#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC warning "Deprecated: Use <kcenon/container/container.h> instead. This header will be removed in v0.5.0.0."
#elif defined(_MSC_VER)
    #pragma message("warning: Deprecated: Use <kcenon/container/container.h> instead. This header will be removed in v0.5.0.0.")
#endif

// Core components (suppress nested deprecation warnings)
#define CONTAINER_INTERNAL_INCLUDE
#include "container/core/container.h"
#undef CONTAINER_INTERNAL_INCLUDE
#include "container/core/value_types.h"

// Integration features (conditional)
#if defined(HAS_MESSAGING_FEATURES) || defined(HAS_EXTERNAL_INTEGRATION) || defined(HAS_PERFORMANCE_METRICS)
#include "container/integration/messaging_integration.h"
#endif

// Note: Internal components like variant_value, thread_safe_container, and simd_processor
// are not exposed in the public API