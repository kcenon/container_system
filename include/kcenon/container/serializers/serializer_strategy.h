// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

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

// Unified Result<T> integration (Issue #335)
#include "../container/result_integration.h"

namespace kcenon::container
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

#if CONTAINER_HAS_RESULT
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

} // namespace kcenon::container
