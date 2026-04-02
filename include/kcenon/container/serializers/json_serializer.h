// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file core/serializers/json_serializer.h
 * @brief JSON format serializer implementation
 *
 * Implements the serializer_strategy interface for JSON format.
 * Part of the unified serialization API (Issue #314).
 */

#pragma once

#include "serializer_strategy.h"

namespace kcenon::container
{

	/**
	 * @brief JSON format serializer
	 *
	 * Serializes value_container to JSON format with proper escaping
	 * per RFC 8259.
	 */
	class json_serializer : public serializer_strategy
	{
	public:
		json_serializer() = default;
		~json_serializer() override = default;

#if CONTAINER_HAS_RESULT
		/**
		 * @brief Serialize a value_container to JSON format
		 *
		 * @param container The container to serialize
		 * @return Result containing serialized bytes or error
		 * @exception_safety No-throw guarantee
		 */
		[[nodiscard]] kcenon::common::Result<std::vector<uint8_t>>
			serialize(const value_container& container) const noexcept override;

		/**
		 * @brief Serialize a value_container to JSON string format
		 *
		 * @param container The container to serialize
		 * @return Serialized JSON string
		 * @throws std::bad_alloc on memory allocation failure
		 */
		[[nodiscard]] std::string serialize_to_string(const value_container& container) const;
#endif

		/**
		 * @brief Get the format this serializer handles
		 *
		 * @return serialization_format::json
		 */
		[[nodiscard]] serialization_format format() const noexcept override
		{
			return serialization_format::json;
		}

		/**
		 * @brief Get a human-readable name for this serializer
		 *
		 * @return "JSON"
		 */
		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "JSON";
		}
	};

} // namespace kcenon::container
