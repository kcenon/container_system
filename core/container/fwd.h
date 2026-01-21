/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
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

namespace container_module
{
	// Forward declaration of main container class
	class value_container;

	// Note: value_types enum is defined in value_types.h
	// Cannot forward declare enum class without matching underlying type

	// Forward declaration of optimized_value struct
	struct optimized_value;

	// Forward declaration of pool_stats struct
	struct pool_stats;

	// Type alias for container shared pointer
	// Note: Prefer message_buffer_ptr for new code (see Issue #321)
	using value_container_ptr [[deprecated("Use message_buffer_ptr instead (Issue #321)")]]
		= std::shared_ptr<value_container>;

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

} // namespace container_module
