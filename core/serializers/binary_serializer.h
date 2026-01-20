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
 * @file core/serializers/binary_serializer.h
 * @brief Binary format serializer implementation
 *
 * Implements the serializer_strategy interface for the custom binary format
 * (@header{};@data{};). Part of the unified serialization API (Issue #314).
 */

#pragma once

#include "serializer_strategy.h"

namespace container_module
{

	/**
	 * @brief Binary format serializer
	 *
	 * Serializes value_container to the custom binary format:
	 * @header{{[key,value];...}};@data{{[key,value];...}};
	 */
	class binary_serializer : public serializer_strategy
	{
	public:
		binary_serializer() = default;
		~binary_serializer() override = default;

#if KCENON_HAS_COMMON_SYSTEM
		/**
		 * @brief Serialize a value_container to binary format
		 *
		 * @param container The container to serialize
		 * @return Result containing serialized bytes or error
		 * @exception_safety No-throw guarantee
		 */
		[[nodiscard]] kcenon::common::Result<std::vector<uint8_t>>
			serialize(const value_container& container) const noexcept override;

		/**
		 * @brief Serialize a value_container to binary string format
		 *
		 * @param container The container to serialize
		 * @return Serialized binary string
		 * @throws std::bad_alloc on memory allocation failure
		 */
		[[nodiscard]] std::string serialize_to_string(const value_container& container) const;
#endif

		/**
		 * @brief Get the format this serializer handles
		 *
		 * @return serialization_format::binary
		 */
		[[nodiscard]] serialization_format format() const noexcept override
		{
			return serialization_format::binary;
		}

		/**
		 * @brief Get a human-readable name for this serializer
		 *
		 * @return "Binary"
		 */
		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "Binary";
		}
	};

} // namespace container_module
