// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file core/serializers/msgpack_serializer.h
 * @brief MessagePack format serializer implementation
 *
 * Implements the serializer_strategy interface for MessagePack format.
 * Part of the unified serialization API (Issue #314).
 */

#pragma once

#include "serializer_strategy.h"

namespace kcenon::container
{

	/**
	 * @brief MessagePack format serializer
	 *
	 * Serializes value_container to MessagePack binary format.
	 */
	class msgpack_serializer : public serializer_strategy
	{
	public:
		msgpack_serializer() = default;
		~msgpack_serializer() override = default;

#if CONTAINER_HAS_RESULT
		/**
		 * @brief Serialize a value_container to MessagePack format
		 *
		 * @param container The container to serialize
		 * @return Result containing serialized bytes or error
		 * @exception_safety No-throw guarantee
		 */
		[[nodiscard]] kcenon::common::Result<std::vector<uint8_t>>
			serialize(const value_container& container) const noexcept override;
#endif

		/**
		 * @brief Get the format this serializer handles
		 *
		 * @return serialization_format::msgpack
		 */
		[[nodiscard]] serialization_format format() const noexcept override
		{
			return serialization_format::msgpack;
		}

		/**
		 * @brief Get a human-readable name for this serializer
		 *
		 * @return "MessagePack"
		 */
		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "MessagePack";
		}
	};

} // namespace kcenon::container
