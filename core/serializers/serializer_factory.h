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
 * @file core/serializers/serializer_factory.h
 * @brief Factory for creating format-specific serializers
 *
 * This header provides the serializer_factory class that creates appropriate
 * serializer instances based on the requested format. Part of the unified
 * serialization API (Issue #310).
 */

#pragma once

#include "serializer_strategy.h"

#include <memory>

namespace container_module
{

	/**
	 * @brief Factory for creating format-specific serializers
	 *
	 * Creates serializer_strategy instances based on the requested serialization
	 * format. This centralizes serializer creation and allows for easy extension
	 * with new formats.
	 *
	 * @code
	 * auto serializer = serializer_factory::create(serialization_format::json);
	 * if (serializer) {
	 *     auto result = serializer->serialize(container);
	 * }
	 * @endcode
	 */
	class serializer_factory
	{
	public:
		/**
		 * @brief Create a serializer for the specified format
		 *
		 * @param fmt The serialization format
		 * @return Unique pointer to serializer, or nullptr for unsupported formats
		 *
		 * @note Returns nullptr for auto_detect and unknown formats
		 */
		[[nodiscard]] static std::unique_ptr<serializer_strategy>
			create(serialization_format fmt) noexcept;

		/**
		 * @brief Check if a format is supported for serialization
		 *
		 * @param fmt The serialization format to check
		 * @return true if the format is supported, false otherwise
		 */
		[[nodiscard]] static bool is_supported(serialization_format fmt) noexcept;

	private:
		serializer_factory() = delete;
	};

} // namespace container_module
