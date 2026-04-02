// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file container.h
 * @brief Main include file for the container module
 *
 * This file includes all the public headers from the container module.
 * Users should include this file to use the container functionality.
 *
 * @code
 * // Recommended usage:
 * #include <container.h>
 *
 * // Create a message buffer (preferred name for serializable containers)
 * kcenon::container::message_buffer msg;
 * msg.set("name", "Alice");
 * msg.set("age", 30);
 *
 * // value_container is still supported (legacy name)
 * kcenon::container::value_container container;  // Same as message_buffer
 * @endcode
 *
 * @note Since v2.0.0, `message_buffer` is the preferred name for `value_container`.
 *       Both names refer to the same class and can be used interchangeably.
 */

#pragma once

// Core components
#include "core/container.h"
#include "core/value_types.h"

// Integration features (conditional)
#if defined(HAS_MESSAGING_FEATURES) || defined(HAS_EXTERNAL_INTEGRATION) || defined(HAS_PERFORMANCE_METRICS)
#include "integration/messaging_integration.h"
#endif

// Note: Internal components like variant_value, thread_safe_container, and simd_processor
// are not exposed in the public API