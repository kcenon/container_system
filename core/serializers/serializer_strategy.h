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
 * @file core/serializers/serializer_strategy.h
 * @brief Abstract base class for serialization strategies
 *
 * This header defines the serializer_strategy interface that all format-specific
 * serializers must implement. Part of the unified serialization API (Issue #310).
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Include feature flags for unified macro detection
#if __has_include(<kcenon/common/config/feature_flags.h>)
    #include <kcenon/common/config/feature_flags.h>
#endif

// Optional common system integration
#if KCENON_HAS_COMMON_SYSTEM
    #if __has_include(<kcenon/common/patterns/result.h>)
        #include <kcenon/common/patterns/result.h>
    #elif __has_include(<common/patterns/result.h>)
        #include <common/patterns/result.h>
        namespace kcenon::common = ::common;
    #endif
#endif

namespace container_module
{
	// Forward declaration
	class value_container;

	/**
	 * @brief Serialization format enumeration (mirrors value_container::serialization_format)
	 */
	enum class serialization_format
	{
		binary,      ///< Custom binary format (@header{};@data{};)
		json,        ///< JSON format
		xml,         ///< XML format
		msgpack,     ///< MessagePack binary format
		auto_detect, ///< Auto-detect format during deserialization
		unknown      ///< Unknown or unrecognized format
	};

	/**
	 * @brief Abstract base class for serialization strategies
	 *
	 * Defines the interface for all format-specific serializers. Each serializer
	 * implements this interface to provide serialization for a specific format.
	 *
	 * @code
	 * class json_serializer : public serializer_strategy {
	 * public:
	 *     serialization_result serialize(const value_container& c) const noexcept override;
	 *     serialization_format format() const noexcept override { return serialization_format::json; }
	 * };
	 * @endcode
	 */
	class serializer_strategy
	{
	public:
		virtual ~serializer_strategy() = default;

#if KCENON_HAS_COMMON_SYSTEM
		/**
		 * @brief Serialize a value_container to bytes
		 *
		 * @param container The container to serialize
		 * @return Result containing serialized bytes or error
		 * @exception_safety No-throw guarantee
		 */
		[[nodiscard]] virtual kcenon::common::Result<std::vector<uint8_t>>
			serialize(const value_container& container) const noexcept = 0;
#endif

		/**
		 * @brief Get the format this serializer handles
		 *
		 * @return The serialization format this strategy implements
		 */
		[[nodiscard]] virtual serialization_format format() const noexcept = 0;

		/**
		 * @brief Get a human-readable name for this serializer
		 *
		 * @return Format name (e.g., "JSON", "XML", "MessagePack")
		 */
		[[nodiscard]] virtual std::string_view name() const noexcept = 0;

	protected:
		serializer_strategy() = default;
		serializer_strategy(const serializer_strategy&) = default;
		serializer_strategy(serializer_strategy&&) = default;
		serializer_strategy& operator=(const serializer_strategy&) = default;
		serializer_strategy& operator=(serializer_strategy&&) = default;
	};

} // namespace container_module
