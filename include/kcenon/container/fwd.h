// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file core/container/fwd.h
 * @brief Forward declarations for container module types
 *
 * This header provides forward declarations for key types in the container
 * module, allowing them to be used without full type definitions. This helps
 * reduce compilation times and break circular dependencies.
 *
 * @note Include full headers when you need complete type definitions.
 */

#pragma once

#include <memory>
#include <variant>
#include <string>
#include <vector>
#include <cstdint>

namespace kcenon::container
{
	// Forward declaration of main container class
	class value_container;

	// Note: value_types enum is defined in value_types.h
	// Cannot forward declare enum class without matching underlying type

	// Forward declaration of optimized_value struct
	struct optimized_value;

	// Forward declaration of pool_stats struct
	struct pool_stats;

	// Forward declaration of value_variant (full definition in types.h)
	// Note: std::variant cannot be forward declared, so we provide a comment
	// for documentation purposes. Include types.h for the actual definition.

	// =======================================================================
	// Type Alias for Migration (Issue #321, #332)
	// =======================================================================

	/**
	 * @brief Forward declaration of message_buffer alias
	 *
	 * `message_buffer` is the preferred name for `value_container` since v2.0.0.
	 * This alias better describes the class's purpose: a serializable message
	 * buffer for network transmission.
	 *
	 * @see value_container for the actual implementation
	 */
	using message_buffer = value_container;

	/// @brief Shared pointer to message_buffer (preferred name)
	using message_buffer_ptr = std::shared_ptr<message_buffer>;

} // namespace kcenon::container

// =======================================================================
// Backward-Compatibility Namespace Alias (Issue #380, Phase 1)
// Temporary alias for one release cycle to ease migration.
// Downstream code using `container_module::` will continue to compile.
// Remove after all downstream systems have migrated to kcenon::container.
// =======================================================================
namespace container_module = kcenon::container;
