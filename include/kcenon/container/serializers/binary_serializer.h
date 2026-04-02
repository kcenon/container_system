// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file core/serializers/binary_serializer.h
 * @brief Binary format serializer implementation
 *
 * Implements the serializer_strategy interface for the custom binary format
 * (@header{};@data{};). Part of the unified serialization API (Issue #314).
 */

#pragma once

#include "serializer_strategy.h"

namespace kcenon::container
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

#if CONTAINER_HAS_RESULT
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

} // namespace kcenon::container
