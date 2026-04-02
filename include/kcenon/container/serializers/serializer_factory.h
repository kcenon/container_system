// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

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

namespace kcenon::container
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

} // namespace kcenon::container
