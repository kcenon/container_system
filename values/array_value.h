/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#pragma once

#include "container/core/value.h"
#include <vector>
#include <memory>

namespace container_module
{
	/**
	 * @class array_value
	 * @brief A specialized value for storing arrays/lists of values.
	 *
	 * ArrayValue (type 15) is an extension to support homogeneous or heterogeneous
	 * collections of values, similar to JSON arrays. This enables cross-language
	 * compatibility with array structures in Node.js, Python, etc.
	 *
	 * Wire format:
	 * [type:1=15][name_len:4 LE][name:UTF-8][value_size:4 LE][count:4 LE][values...]
	 */
	class array_value : public value
	{
	public:
		/**
		 * @brief Default constructor. Initializes an empty array.
		 */
		array_value();

		/**
		 * @brief Construct with a name.
		 * @param target_name The name/key for this array value
		 */
		array_value(const std::string& target_name);

		/**
		 * @brief Construct with a name and initial values.
		 * @param target_name The name/key for this array value
		 * @param values Initial vector of values to store
		 */
		array_value(const std::string& target_name,
		           const std::vector<std::shared_ptr<value>>& values);

		/**
		 * @brief Copy constructor
		 */
		array_value(const array_value& target);

		/**
		 * @brief Virtual destructor
		 */
		virtual ~array_value() = default;

		/**
		 * @brief Assignment operator
		 */
		array_value& operator=(const array_value& target);

	public:
		/**
		 * @brief Get the type discriminator
		 * @return value_types::array_value (15)
		 */
		value_types type(void) const override;

		/**
		 * @brief Serialize to binary format
		 * @return Vector of bytes representing the serialized array
		 */
		std::vector<uint8_t> serialize(void) override;

		/**
		 * @brief Deserialize from binary data
		 * @param byte_vector The binary data to deserialize from
		 * @return Shared pointer to the deserialized array_value
		 */
		static std::shared_ptr<array_value> deserialize(
			const std::vector<uint8_t>& byte_vector);

	public:
		/**
		 * @brief Add a value to the end of the array
		 * @param target_value The value to add
		 */
		void push_back(std::shared_ptr<value> target_value);

		/**
		 * @brief Get value at index
		 * @param index The index to retrieve
		 * @return Shared pointer to the value at index
		 * @throws std::out_of_range if index is invalid
		 */
		std::shared_ptr<value> at(size_t index) const;

		/**
		 * @brief Get the number of elements in the array
		 * @return The size of the array
		 */
		size_t size(void) const;

		/**
		 * @brief Check if the array is empty
		 * @return true if empty, false otherwise
		 */
		bool empty(void) const;

		/**
		 * @brief Clear all elements from the array
		 */
		void clear(void);

		/**
		 * @brief Get all values in the array
		 * @return Const reference to the internal vector of values
		 */
		const std::vector<std::shared_ptr<value>>& values(void) const;

	private:
		std::vector<std::shared_ptr<value>> values_;
	};
}
