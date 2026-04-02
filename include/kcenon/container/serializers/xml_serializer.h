// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file core/serializers/xml_serializer.h
 * @brief XML format serializer implementation
 *
 * Implements the serializer_strategy interface for XML format.
 * Part of the unified serialization API (Issue #314).
 */

#pragma once

#include "serializer_strategy.h"

namespace kcenon::container
{

	/**
	 * @brief XML format serializer
	 *
	 * Serializes value_container to XML format with proper entity encoding
	 * per XML 1.0 specification.
	 */
	class xml_serializer : public serializer_strategy
	{
	public:
		xml_serializer() = default;
		~xml_serializer() override = default;

#if CONTAINER_HAS_RESULT
		/**
		 * @brief Serialize a value_container to XML format
		 *
		 * @param container The container to serialize
		 * @return Result containing serialized bytes or error
		 * @exception_safety No-throw guarantee
		 */
		[[nodiscard]] kcenon::common::Result<std::vector<uint8_t>>
			serialize(const value_container& container) const noexcept override;

		/**
		 * @brief Serialize a value_container to XML string format
		 *
		 * @param container The container to serialize
		 * @return Serialized XML string
		 * @throws std::bad_alloc on memory allocation failure
		 */
		[[nodiscard]] std::string serialize_to_string(const value_container& container) const;
#endif

		/**
		 * @brief Get the format this serializer handles
		 *
		 * @return serialization_format::xml
		 */
		[[nodiscard]] serialization_format format() const noexcept override
		{
			return serialization_format::xml;
		}

		/**
		 * @brief Get a human-readable name for this serializer
		 *
		 * @return "XML"
		 */
		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "XML";
		}
	};

} // namespace kcenon::container
