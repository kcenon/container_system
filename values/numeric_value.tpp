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

#pragma once

namespace container_module
{
	template <typename T, value_types TypeTag>
	numeric_value<T, TypeTag>::numeric_value(void) : value()
	{
		type_ = TypeTag;
		T zero_value = T{};
		data_.resize(sizeof(T));
		std::memcpy(data_.data(), &zero_value, sizeof(T));
	}

	template <typename T, value_types TypeTag>
	numeric_value<T, TypeTag>::numeric_value(const std::string& name, const T& initial_value) 
		: value(name)
	{
		type_ = TypeTag;
		data_.resize(sizeof(T));
		std::memcpy(data_.data(), &initial_value, sizeof(T));
	}

	template <typename T, value_types TypeTag>
	T numeric_value<T, TypeTag>::get_value(void) const
	{
		T result;
		std::memcpy(&result, data_.data(), sizeof(T));
		return result;
	}

	template <typename T, value_types TypeTag>
	bool numeric_value<T, TypeTag>::to_boolean(void) const
	{
		return get_value() != T{};
	}

	template <typename T, value_types TypeTag>
	short numeric_value<T, TypeTag>::to_short(void) const
	{
		return static_cast<short>(get_value());
	}

	template <typename T, value_types TypeTag>
	unsigned short numeric_value<T, TypeTag>::to_ushort(void) const
	{
		return static_cast<unsigned short>(get_value());
	}

	template <typename T, value_types TypeTag>
	int numeric_value<T, TypeTag>::to_int(void) const
	{
		return static_cast<int>(get_value());
	}

	template <typename T, value_types TypeTag>
	unsigned int numeric_value<T, TypeTag>::to_uint(void) const
	{
		return static_cast<unsigned int>(get_value());
	}

	template <typename T, value_types TypeTag>
	long numeric_value<T, TypeTag>::to_long(void) const
	{
		return static_cast<long>(get_value());
	}

	template <typename T, value_types TypeTag>
	unsigned long numeric_value<T, TypeTag>::to_ulong(void) const
	{
		return static_cast<unsigned long>(get_value());
	}

	template <typename T, value_types TypeTag>
	long long numeric_value<T, TypeTag>::to_llong(void) const
	{
		return static_cast<long long>(get_value());
	}

	template <typename T, value_types TypeTag>
	unsigned long long numeric_value<T, TypeTag>::to_ullong(void) const
	{
		return static_cast<unsigned long long>(get_value());
	}

	template <typename T, value_types TypeTag>
	float numeric_value<T, TypeTag>::to_float(void) const
	{
		return static_cast<float>(get_value());
	}

	template <typename T, value_types TypeTag>
	double numeric_value<T, TypeTag>::to_double(void) const
	{
		return static_cast<double>(get_value());
	}

	template <typename T, value_types TypeTag>
	std::string numeric_value<T, TypeTag>::to_string(const bool& original) const
	{
		(void)original; // unused parameter
		return std::to_string(get_value());
	}

	// ========================================================================
	// Template specialization for long_value (type 6) with range checking
	// ========================================================================

	template <>
	inline numeric_value<long, value_types::long_value>::numeric_value(
		const std::string& name, const long& initial_value)
		: value(name)
	{
		// Convert to int64_t for range checking
		int64_t value_as_64 = static_cast<int64_t>(initial_value);

		// Enforce strict 32-bit range policy
		if (!is_int32_range(value_as_64)) {
			throw std::overflow_error(
				"long_value: value " + std::to_string(value_as_64) +
				" exceeds 32-bit range [-2147483648, 2147483647]. "
				"Use llong_value for 64-bit values."
			);
		}

		type_ = value_types::long_value;

		// Always serialize as 4 bytes (int32_t)
		int32_t value_as_32 = static_cast<int32_t>(initial_value);
		data_.resize(sizeof(int32_t));
		std::memcpy(data_.data(), &value_as_32, sizeof(int32_t));
	}

	template <>
	inline long numeric_value<long, value_types::long_value>::get_value(void) const
	{
		// Deserialize as int32_t, then convert to long
		int32_t value_as_32;
		std::memcpy(&value_as_32, data_.data(), sizeof(int32_t));
		return static_cast<long>(value_as_32);
	}

	// ========================================================================
	// Template specialization for ulong_value (type 7) with range checking
	// ========================================================================

	template <>
	inline numeric_value<unsigned long, value_types::ulong_value>::numeric_value(
		const std::string& name, const unsigned long& initial_value)
		: value(name)
	{
		// Convert to uint64_t for range checking
		uint64_t value_as_64 = static_cast<uint64_t>(initial_value);

		// Enforce strict 32-bit range policy
		if (!is_uint32_range(value_as_64)) {
			throw std::overflow_error(
				"ulong_value: value " + std::to_string(value_as_64) +
				" exceeds 32-bit range [0, 4294967295]. "
				"Use ullong_value for 64-bit values."
			);
		}

		type_ = value_types::ulong_value;

		// Always serialize as 4 bytes (uint32_t)
		uint32_t value_as_32 = static_cast<uint32_t>(initial_value);
		data_.resize(sizeof(uint32_t));
		std::memcpy(data_.data(), &value_as_32, sizeof(uint32_t));
	}

	template <>
	inline unsigned long numeric_value<unsigned long, value_types::ulong_value>::get_value(void) const
	{
		// Deserialize as uint32_t, then convert to unsigned long
		uint32_t value_as_32;
		std::memcpy(&value_as_32, data_.data(), sizeof(uint32_t));
		return static_cast<unsigned long>(value_as_32);
	}

	// ========================================================================
	// End of template specializations
	// ========================================================================

} // namespace container_module