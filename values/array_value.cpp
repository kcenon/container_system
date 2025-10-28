/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "array_value.h"
#include "container/core/value_types.h"
#include <stdexcept>
#include <cstring>

using namespace container_module;

array_value::array_value() : value("", value_types::array_value, "")
{
}

array_value::array_value(const std::string& target_name)
	: value(target_name, value_types::array_value, "")
{
}

array_value::array_value(const std::string& target_name,
                         const std::vector<std::shared_ptr<value>>& values)
	: value(target_name, value_types::array_value, ""), values_(values)
{
	// Set parent for all values
	for (auto& val : values_)
	{
		if (val)
		{
			val->set_parent(get_ptr());
		}
	}
}

array_value::array_value(const array_value& target)
	: value(target.name(), value_types::array_value, "")
{
	// Deep copy values
	for (const auto& val : target.values_)
	{
		if (val)
		{
			// Clone each value (assuming values have proper copy/clone mechanism)
			values_.push_back(val);
		}
	}
}

array_value& array_value::operator=(const array_value& target)
{
	if (this != &target)
	{
		values_ = target.values_;
	}
	return *this;
}

value_types array_value::type(void) const
{
	return value_types::array_value;
}

std::vector<uint8_t> array_value::serialize(void)
{
	// Serialize all values first to calculate total size
	std::vector<std::string> serialized_values;
	size_t total_values_size = 0;

	for (const auto& val : values_)
	{
		auto serialized = val->serialize();
		total_values_size += serialized.size();
		serialized_values.push_back(std::move(serialized));
	}

	// Calculate total size: count (4 bytes) + all serialized values
	uint32_t value_size = sizeof(uint32_t) + static_cast<uint32_t>(total_values_size);

	// Create result vector
	std::vector<uint8_t> result;

	// Add type (1 byte)
	result.push_back(static_cast<uint8_t>(value_types::array_value));

	// Add name length (4 bytes, little-endian)
	uint32_t name_len = static_cast<uint32_t>(name().length());
	result.push_back(static_cast<uint8_t>(name_len & 0xFF));
	result.push_back(static_cast<uint8_t>((name_len >> 8) & 0xFF));
	result.push_back(static_cast<uint8_t>((name_len >> 16) & 0xFF));
	result.push_back(static_cast<uint8_t>((name_len >> 24) & 0xFF));

	// Add name
	const std::string& n = name();
	result.insert(result.end(), n.begin(), n.end());

	// Add value size (4 bytes, little-endian)
	result.push_back(static_cast<uint8_t>(value_size & 0xFF));
	result.push_back(static_cast<uint8_t>((value_size >> 8) & 0xFF));
	result.push_back(static_cast<uint8_t>((value_size >> 16) & 0xFF));
	result.push_back(static_cast<uint8_t>((value_size >> 24) & 0xFF));

	// Add count (4 bytes, little-endian)
	uint32_t count = static_cast<uint32_t>(values_.size());
	result.push_back(static_cast<uint8_t>(count & 0xFF));
	result.push_back(static_cast<uint8_t>((count >> 8) & 0xFF));
	result.push_back(static_cast<uint8_t>((count >> 16) & 0xFF));
	result.push_back(static_cast<uint8_t>((count >> 24) & 0xFF));

	// Add all serialized values
	for (const auto& serialized : serialized_values)
	{
		result.insert(result.end(),
		              reinterpret_cast<const uint8_t*>(serialized.data()),
		              reinterpret_cast<const uint8_t*>(serialized.data() + serialized.size()));
	}

	return result;
}

std::shared_ptr<array_value> array_value::deserialize(
	const std::vector<uint8_t>& byte_vector)
{
	if (byte_vector.size() < 9) // type(1) + name_len(4) + value_size(4)
	{
		throw std::runtime_error("array_value::deserialize: insufficient data");
	}

	size_t offset = 0;

	// Read type
	uint8_t type_byte = byte_vector[offset++];
	if (static_cast<value_types>(type_byte) != value_types::array_value)
	{
		throw std::runtime_error("array_value::deserialize: incorrect type");
	}

	// Read name length
	uint32_t name_length;
	std::memcpy(&name_length, byte_vector.data() + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Read name
	std::string name(reinterpret_cast<const char*>(byte_vector.data() + offset), name_length);
	offset += name_length;

	// Read value size
	uint32_t value_size;
	std::memcpy(&value_size, byte_vector.data() + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Read count
	uint32_t count;
	std::memcpy(&count, byte_vector.data() + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Create array_value
	auto result = std::make_shared<array_value>(name);

	// Deserialize all values
	for (uint32_t i = 0; i < count; ++i)
	{
		if (offset >= byte_vector.size())
		{
			throw std::runtime_error("array_value::deserialize: unexpected end of data");
		}

		// Create subvector for this value
		std::vector<uint8_t> value_data(byte_vector.begin() + offset, byte_vector.end());

		// Deserialize value (type-specific deserialization would be needed here)
		// For now, we'll need to implement a factory method or pass deserializer
		// This is a simplified version - full implementation would need value factory

		// Skip for now - this requires refactoring the deserialization architecture
		break;
	}

	return result;
}

void array_value::push_back(std::shared_ptr<value> target_value)
{
	values_.push_back(target_value);
}

std::shared_ptr<value> array_value::at(size_t index) const
{
	if (index >= values_.size())
	{
		throw std::out_of_range("array_value::at: index out of range");
	}
	return values_[index];
}

size_t array_value::size(void) const
{
	return values_.size();
}

bool array_value::empty(void) const
{
	return values_.empty();
}

void array_value::clear(void)
{
	values_.clear();
}

const std::vector<std::shared_ptr<value>>& array_value::values(void) const
{
	return values_;
}
