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
 * @file core/container/msgpack.h
 * @brief MessagePack serialization format support
 *
 * This header provides MessagePack encoding/decoding utilities following
 * the MessagePack specification (https://msgpack.org/):
 * - Encoder class for serializing values to MessagePack format
 * - Decoder class for deserializing MessagePack data
 * - Type mapping between container_system and MessagePack types
 *
 * @see https://github.com/msgpack/msgpack/blob/master/spec.md
 */

#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <string_view>
#include <cstring>
#include <stdexcept>
#include <optional>

namespace container_module
{
	/**
	 * @brief MessagePack format type codes
	 */
	namespace msgpack_format
	{
		// Positive fixint: 0x00 - 0x7f
		constexpr uint8_t POSITIVE_FIXINT_MAX = 0x7f;

		// Fixmap: 0x80 - 0x8f
		constexpr uint8_t FIXMAP_PREFIX = 0x80;
		constexpr uint8_t FIXMAP_MAX_SIZE = 0x0f;

		// Fixarray: 0x90 - 0x9f
		constexpr uint8_t FIXARRAY_PREFIX = 0x90;
		constexpr uint8_t FIXARRAY_MAX_SIZE = 0x0f;

		// Fixstr: 0xa0 - 0xbf
		constexpr uint8_t FIXSTR_PREFIX = 0xa0;
		constexpr uint8_t FIXSTR_MAX_SIZE = 0x1f;

		// Nil, false, true
		constexpr uint8_t NIL = 0xc0;
		constexpr uint8_t FALSE = 0xc2;
		constexpr uint8_t TRUE = 0xc3;

		// Binary
		constexpr uint8_t BIN8 = 0xc4;
		constexpr uint8_t BIN16 = 0xc5;
		constexpr uint8_t BIN32 = 0xc6;

		// Float
		constexpr uint8_t FLOAT32 = 0xca;
		constexpr uint8_t FLOAT64 = 0xcb;

		// Unsigned integers
		constexpr uint8_t UINT8 = 0xcc;
		constexpr uint8_t UINT16 = 0xcd;
		constexpr uint8_t UINT32 = 0xce;
		constexpr uint8_t UINT64 = 0xcf;

		// Signed integers
		constexpr uint8_t INT8 = 0xd0;
		constexpr uint8_t INT16 = 0xd1;
		constexpr uint8_t INT32 = 0xd2;
		constexpr uint8_t INT64 = 0xd3;

		// String
		constexpr uint8_t STR8 = 0xd9;
		constexpr uint8_t STR16 = 0xda;
		constexpr uint8_t STR32 = 0xdb;

		// Array
		constexpr uint8_t ARRAY16 = 0xdc;
		constexpr uint8_t ARRAY32 = 0xdd;

		// Map
		constexpr uint8_t MAP16 = 0xde;
		constexpr uint8_t MAP32 = 0xdf;

		// Negative fixint: 0xe0 - 0xff
		constexpr uint8_t NEGATIVE_FIXINT_PREFIX = 0xe0;
	} // namespace msgpack_format

	/**
	 * @brief MessagePack type enumeration for decoder
	 */
	enum class msgpack_type
	{
		nil,
		boolean,
		positive_int,
		negative_int,
		float32,
		float64,
		str,
		bin,
		array,
		map,
		unknown
	};

	/**
	 * @brief MessagePack encoder for binary serialization
	 */
	class msgpack_encoder
	{
	public:
		msgpack_encoder() = default;

		/**
		 * @brief Write nil value
		 */
		void write_nil()
		{
			buffer_.push_back(msgpack_format::NIL);
		}

		/**
		 * @brief Write boolean value
		 */
		void write_bool(bool value)
		{
			buffer_.push_back(value ? msgpack_format::TRUE : msgpack_format::FALSE);
		}

		/**
		 * @brief Write signed integer with automatic size selection
		 */
		void write_int(int64_t value)
		{
			if (value >= 0)
			{
				write_uint(static_cast<uint64_t>(value));
			}
			else if (value >= -32)
			{
				// Negative fixint
				buffer_.push_back(static_cast<uint8_t>(value));
			}
			else if (value >= INT8_MIN)
			{
				buffer_.push_back(msgpack_format::INT8);
				buffer_.push_back(static_cast<uint8_t>(value));
			}
			else if (value >= INT16_MIN)
			{
				buffer_.push_back(msgpack_format::INT16);
				write_be16(static_cast<uint16_t>(value));
			}
			else if (value >= INT32_MIN)
			{
				buffer_.push_back(msgpack_format::INT32);
				write_be32(static_cast<uint32_t>(value));
			}
			else
			{
				buffer_.push_back(msgpack_format::INT64);
				write_be64(static_cast<uint64_t>(value));
			}
		}

		/**
		 * @brief Write unsigned integer with automatic size selection
		 */
		void write_uint(uint64_t value)
		{
			if (value <= msgpack_format::POSITIVE_FIXINT_MAX)
			{
				// Positive fixint
				buffer_.push_back(static_cast<uint8_t>(value));
			}
			else if (value <= UINT8_MAX)
			{
				buffer_.push_back(msgpack_format::UINT8);
				buffer_.push_back(static_cast<uint8_t>(value));
			}
			else if (value <= UINT16_MAX)
			{
				buffer_.push_back(msgpack_format::UINT16);
				write_be16(static_cast<uint16_t>(value));
			}
			else if (value <= UINT32_MAX)
			{
				buffer_.push_back(msgpack_format::UINT32);
				write_be32(static_cast<uint32_t>(value));
			}
			else
			{
				buffer_.push_back(msgpack_format::UINT64);
				write_be64(value);
			}
		}

		/**
		 * @brief Write 32-bit float
		 */
		void write_float(float value)
		{
			buffer_.push_back(msgpack_format::FLOAT32);
			uint32_t bits = 0;
			std::memcpy(&bits, &value, sizeof(bits));
			write_be32(bits);
		}

		/**
		 * @brief Write 64-bit double
		 */
		void write_double(double value)
		{
			buffer_.push_back(msgpack_format::FLOAT64);
			uint64_t bits = 0;
			std::memcpy(&bits, &value, sizeof(bits));
			write_be64(bits);
		}

		/**
		 * @brief Write string value
		 */
		void write_string(std::string_view value)
		{
			size_t len = value.size();
			if (len <= msgpack_format::FIXSTR_MAX_SIZE)
			{
				buffer_.push_back(static_cast<uint8_t>(msgpack_format::FIXSTR_PREFIX | len));
			}
			else if (len <= UINT8_MAX)
			{
				buffer_.push_back(msgpack_format::STR8);
				buffer_.push_back(static_cast<uint8_t>(len));
			}
			else if (len <= UINT16_MAX)
			{
				buffer_.push_back(msgpack_format::STR16);
				write_be16(static_cast<uint16_t>(len));
			}
			else
			{
				buffer_.push_back(msgpack_format::STR32);
				write_be32(static_cast<uint32_t>(len));
			}
			buffer_.insert(buffer_.end(), value.begin(), value.end());
		}

		/**
		 * @brief Write binary data
		 */
		void write_binary(const std::vector<uint8_t>& value)
		{
			size_t len = value.size();
			if (len <= UINT8_MAX)
			{
				buffer_.push_back(msgpack_format::BIN8);
				buffer_.push_back(static_cast<uint8_t>(len));
			}
			else if (len <= UINT16_MAX)
			{
				buffer_.push_back(msgpack_format::BIN16);
				write_be16(static_cast<uint16_t>(len));
			}
			else
			{
				buffer_.push_back(msgpack_format::BIN32);
				write_be32(static_cast<uint32_t>(len));
			}
			buffer_.insert(buffer_.end(), value.begin(), value.end());
		}

		/**
		 * @brief Write array header (elements follow separately)
		 */
		void write_array_header(size_t count)
		{
			if (count <= msgpack_format::FIXARRAY_MAX_SIZE)
			{
				buffer_.push_back(static_cast<uint8_t>(msgpack_format::FIXARRAY_PREFIX | count));
			}
			else if (count <= UINT16_MAX)
			{
				buffer_.push_back(msgpack_format::ARRAY16);
				write_be16(static_cast<uint16_t>(count));
			}
			else
			{
				buffer_.push_back(msgpack_format::ARRAY32);
				write_be32(static_cast<uint32_t>(count));
			}
		}

		/**
		 * @brief Write map header (key-value pairs follow separately)
		 */
		void write_map_header(size_t count)
		{
			if (count <= msgpack_format::FIXMAP_MAX_SIZE)
			{
				buffer_.push_back(static_cast<uint8_t>(msgpack_format::FIXMAP_PREFIX | count));
			}
			else if (count <= UINT16_MAX)
			{
				buffer_.push_back(msgpack_format::MAP16);
				write_be16(static_cast<uint16_t>(count));
			}
			else
			{
				buffer_.push_back(msgpack_format::MAP32);
				write_be32(static_cast<uint32_t>(count));
			}
		}

		/**
		 * @brief Get the encoded data and clear the buffer
		 */
		std::vector<uint8_t> finish()
		{
			return std::move(buffer_);
		}

		/**
		 * @brief Get current buffer size
		 */
		size_t size() const { return buffer_.size(); }

		/**
		 * @brief Reserve buffer capacity
		 */
		void reserve(size_t capacity) { buffer_.reserve(capacity); }

	private:
		void write_be16(uint16_t value)
		{
			buffer_.push_back(static_cast<uint8_t>(value >> 8));
			buffer_.push_back(static_cast<uint8_t>(value));
		}

		void write_be32(uint32_t value)
		{
			buffer_.push_back(static_cast<uint8_t>(value >> 24));
			buffer_.push_back(static_cast<uint8_t>(value >> 16));
			buffer_.push_back(static_cast<uint8_t>(value >> 8));
			buffer_.push_back(static_cast<uint8_t>(value));
		}

		void write_be64(uint64_t value)
		{
			buffer_.push_back(static_cast<uint8_t>(value >> 56));
			buffer_.push_back(static_cast<uint8_t>(value >> 48));
			buffer_.push_back(static_cast<uint8_t>(value >> 40));
			buffer_.push_back(static_cast<uint8_t>(value >> 32));
			buffer_.push_back(static_cast<uint8_t>(value >> 24));
			buffer_.push_back(static_cast<uint8_t>(value >> 16));
			buffer_.push_back(static_cast<uint8_t>(value >> 8));
			buffer_.push_back(static_cast<uint8_t>(value));
		}

		std::vector<uint8_t> buffer_;
	};

	/**
	 * @brief MessagePack decoder for binary deserialization
	 */
	class msgpack_decoder
	{
	public:
		explicit msgpack_decoder(const uint8_t* data, size_t size)
			: data_(data), size_(size), offset_(0)
		{
		}

		explicit msgpack_decoder(const std::vector<uint8_t>& data)
			: data_(data.data()), size_(data.size()), offset_(0)
		{
		}

		/**
		 * @brief Peek the type of the next value without consuming it
		 */
		msgpack_type peek_type() const
		{
			if (offset_ >= size_)
			{
				return msgpack_type::unknown;
			}

			uint8_t byte = data_[offset_];

			// Positive fixint
			if (byte <= msgpack_format::POSITIVE_FIXINT_MAX)
			{
				return msgpack_type::positive_int;
			}

			// Fixmap
			if ((byte & 0xf0) == msgpack_format::FIXMAP_PREFIX)
			{
				return msgpack_type::map;
			}

			// Fixarray
			if ((byte & 0xf0) == msgpack_format::FIXARRAY_PREFIX)
			{
				return msgpack_type::array;
			}

			// Fixstr
			if ((byte & 0xe0) == msgpack_format::FIXSTR_PREFIX)
			{
				return msgpack_type::str;
			}

			// Negative fixint
			if (byte >= msgpack_format::NEGATIVE_FIXINT_PREFIX)
			{
				return msgpack_type::negative_int;
			}

			switch (byte)
			{
			case msgpack_format::NIL:
				return msgpack_type::nil;
			case msgpack_format::FALSE:
			case msgpack_format::TRUE:
				return msgpack_type::boolean;
			case msgpack_format::BIN8:
			case msgpack_format::BIN16:
			case msgpack_format::BIN32:
				return msgpack_type::bin;
			case msgpack_format::FLOAT32:
				return msgpack_type::float32;
			case msgpack_format::FLOAT64:
				return msgpack_type::float64;
			case msgpack_format::UINT8:
			case msgpack_format::UINT16:
			case msgpack_format::UINT32:
			case msgpack_format::UINT64:
				return msgpack_type::positive_int;
			case msgpack_format::INT8:
			case msgpack_format::INT16:
			case msgpack_format::INT32:
			case msgpack_format::INT64:
				return msgpack_type::negative_int;
			case msgpack_format::STR8:
			case msgpack_format::STR16:
			case msgpack_format::STR32:
				return msgpack_type::str;
			case msgpack_format::ARRAY16:
			case msgpack_format::ARRAY32:
				return msgpack_type::array;
			case msgpack_format::MAP16:
			case msgpack_format::MAP32:
				return msgpack_type::map;
			default:
				return msgpack_type::unknown;
			}
		}

		/**
		 * @brief Read nil value
		 * @return true if nil was read, false otherwise
		 */
		bool read_nil()
		{
			if (offset_ >= size_ || data_[offset_] != msgpack_format::NIL)
			{
				return false;
			}
			++offset_;
			return true;
		}

		/**
		 * @brief Read boolean value
		 */
		std::optional<bool> read_bool()
		{
			if (offset_ >= size_)
			{
				return std::nullopt;
			}

			uint8_t byte = data_[offset_];
			if (byte == msgpack_format::TRUE)
			{
				++offset_;
				return true;
			}
			if (byte == msgpack_format::FALSE)
			{
				++offset_;
				return false;
			}
			return std::nullopt;
		}

		/**
		 * @brief Read signed integer
		 */
		std::optional<int64_t> read_int()
		{
			if (offset_ >= size_)
			{
				return std::nullopt;
			}

			uint8_t byte = data_[offset_];

			// Positive fixint
			if (byte <= msgpack_format::POSITIVE_FIXINT_MAX)
			{
				++offset_;
				return static_cast<int64_t>(byte);
			}

			// Negative fixint
			if (byte >= msgpack_format::NEGATIVE_FIXINT_PREFIX)
			{
				++offset_;
				return static_cast<int64_t>(static_cast<int8_t>(byte));
			}

			switch (byte)
			{
			case msgpack_format::INT8:
				if (offset_ + 2 > size_)
				{
					return std::nullopt;
				}
				++offset_;
				return static_cast<int64_t>(static_cast<int8_t>(data_[offset_++]));

			case msgpack_format::INT16:
				if (offset_ + 3 > size_)
				{
					return std::nullopt;
				}
				++offset_;
				return static_cast<int64_t>(static_cast<int16_t>(read_be16()));

			case msgpack_format::INT32:
				if (offset_ + 5 > size_)
				{
					return std::nullopt;
				}
				++offset_;
				return static_cast<int64_t>(static_cast<int32_t>(read_be32()));

			case msgpack_format::INT64:
				if (offset_ + 9 > size_)
				{
					return std::nullopt;
				}
				++offset_;
				return static_cast<int64_t>(read_be64());

			case msgpack_format::UINT8:
				if (offset_ + 2 > size_)
				{
					return std::nullopt;
				}
				++offset_;
				return static_cast<int64_t>(data_[offset_++]);

			case msgpack_format::UINT16:
				if (offset_ + 3 > size_)
				{
					return std::nullopt;
				}
				++offset_;
				return static_cast<int64_t>(read_be16());

			case msgpack_format::UINT32:
				if (offset_ + 5 > size_)
				{
					return std::nullopt;
				}
				++offset_;
				return static_cast<int64_t>(read_be32());

			case msgpack_format::UINT64:
				if (offset_ + 9 > size_)
				{
					return std::nullopt;
				}
				++offset_;
				{
					uint64_t val = read_be64();
					if (val > static_cast<uint64_t>(INT64_MAX))
					{
						return std::nullopt; // Overflow
					}
					return static_cast<int64_t>(val);
				}

			default:
				return std::nullopt;
			}
		}

		/**
		 * @brief Read unsigned integer
		 */
		std::optional<uint64_t> read_uint()
		{
			if (offset_ >= size_)
			{
				return std::nullopt;
			}

			uint8_t byte = data_[offset_];

			// Positive fixint
			if (byte <= msgpack_format::POSITIVE_FIXINT_MAX)
			{
				++offset_;
				return static_cast<uint64_t>(byte);
			}

			switch (byte)
			{
			case msgpack_format::UINT8:
				if (offset_ + 2 > size_)
				{
					return std::nullopt;
				}
				++offset_;
				return static_cast<uint64_t>(data_[offset_++]);

			case msgpack_format::UINT16:
				if (offset_ + 3 > size_)
				{
					return std::nullopt;
				}
				++offset_;
				return static_cast<uint64_t>(read_be16());

			case msgpack_format::UINT32:
				if (offset_ + 5 > size_)
				{
					return std::nullopt;
				}
				++offset_;
				return static_cast<uint64_t>(read_be32());

			case msgpack_format::UINT64:
				if (offset_ + 9 > size_)
				{
					return std::nullopt;
				}
				++offset_;
				return read_be64();

			default:
				return std::nullopt;
			}
		}

		/**
		 * @brief Read 32-bit float
		 */
		std::optional<float> read_float()
		{
			if (offset_ >= size_ || data_[offset_] != msgpack_format::FLOAT32)
			{
				return std::nullopt;
			}
			if (offset_ + 5 > size_)
			{
				return std::nullopt;
			}
			++offset_;
			uint32_t bits = read_be32();
			float value = 0.0f;
			std::memcpy(&value, &bits, sizeof(value));
			return value;
		}

		/**
		 * @brief Read 64-bit double
		 */
		std::optional<double> read_double()
		{
			if (offset_ >= size_)
			{
				return std::nullopt;
			}

			uint8_t byte = data_[offset_];

			// Also handle float32 -> double conversion
			if (byte == msgpack_format::FLOAT32)
			{
				auto f = read_float();
				if (f)
				{
					return static_cast<double>(*f);
				}
				return std::nullopt;
			}

			if (byte != msgpack_format::FLOAT64)
			{
				return std::nullopt;
			}
			if (offset_ + 9 > size_)
			{
				return std::nullopt;
			}
			++offset_;
			uint64_t bits = read_be64();
			double value = 0.0;
			std::memcpy(&value, &bits, sizeof(value));
			return value;
		}

		/**
		 * @brief Read string value
		 */
		std::optional<std::string> read_string()
		{
			if (offset_ >= size_)
			{
				return std::nullopt;
			}

			uint8_t byte = data_[offset_];
			size_t len = 0;

			// Fixstr
			if ((byte & 0xe0) == msgpack_format::FIXSTR_PREFIX)
			{
				len = byte & 0x1f;
				++offset_;
			}
			else
			{
				switch (byte)
				{
				case msgpack_format::STR8:
					if (offset_ + 2 > size_) return std::nullopt;
					++offset_;
					len = data_[offset_++];
					break;

				case msgpack_format::STR16:
					if (offset_ + 3 > size_) return std::nullopt;
					++offset_;
					len = read_be16();
					break;

				case msgpack_format::STR32:
					if (offset_ + 5 > size_) return std::nullopt;
					++offset_;
					len = read_be32();
					break;

				default:
					return std::nullopt;
				}
			}

			if (offset_ + len > size_)
			{
				return std::nullopt;
			}

			std::string result(reinterpret_cast<const char*>(data_ + offset_), len);
			offset_ += len;
			return result;
		}

		/**
		 * @brief Read binary data
		 */
		std::optional<std::vector<uint8_t>> read_binary()
		{
			if (offset_ >= size_)
			{
				return std::nullopt;
			}

			uint8_t byte = data_[offset_];
			size_t len = 0;

			switch (byte)
			{
			case msgpack_format::BIN8:
				if (offset_ + 2 > size_) return std::nullopt;
				++offset_;
				len = data_[offset_++];
				break;

			case msgpack_format::BIN16:
				if (offset_ + 3 > size_) return std::nullopt;
				++offset_;
				len = read_be16();
				break;

			case msgpack_format::BIN32:
				if (offset_ + 5 > size_) return std::nullopt;
				++offset_;
				len = read_be32();
				break;

			default:
				return std::nullopt;
			}

			if (offset_ + len > size_)
			{
				return std::nullopt;
			}

			std::vector<uint8_t> result(data_ + offset_, data_ + offset_ + len);
			offset_ += len;
			return result;
		}

		/**
		 * @brief Read array header and return element count
		 */
		std::optional<size_t> read_array_header()
		{
			if (offset_ >= size_)
			{
				return std::nullopt;
			}

			uint8_t byte = data_[offset_];

			// Fixarray
			if ((byte & 0xf0) == msgpack_format::FIXARRAY_PREFIX)
			{
				++offset_;
				return static_cast<size_t>(byte & 0x0f);
			}

			switch (byte)
			{
			case msgpack_format::ARRAY16:
				if (offset_ + 3 > size_) return std::nullopt;
				++offset_;
				return static_cast<size_t>(read_be16());

			case msgpack_format::ARRAY32:
				if (offset_ + 5 > size_) return std::nullopt;
				++offset_;
				return static_cast<size_t>(read_be32());

			default:
				return std::nullopt;
			}
		}

		/**
		 * @brief Read map header and return element count
		 */
		std::optional<size_t> read_map_header()
		{
			if (offset_ >= size_)
			{
				return std::nullopt;
			}

			uint8_t byte = data_[offset_];

			// Fixmap
			if ((byte & 0xf0) == msgpack_format::FIXMAP_PREFIX)
			{
				++offset_;
				return static_cast<size_t>(byte & 0x0f);
			}

			switch (byte)
			{
			case msgpack_format::MAP16:
				if (offset_ + 3 > size_) return std::nullopt;
				++offset_;
				return static_cast<size_t>(read_be16());

			case msgpack_format::MAP32:
				if (offset_ + 5 > size_) return std::nullopt;
				++offset_;
				return static_cast<size_t>(read_be32());

			default:
				return std::nullopt;
			}
		}

		/**
		 * @brief Check if at end of data
		 */
		bool eof() const { return offset_ >= size_; }

		/**
		 * @brief Get current position
		 */
		size_t position() const { return offset_; }

		/**
		 * @brief Get remaining bytes
		 */
		size_t remaining() const { return size_ - offset_; }

	private:
		uint16_t read_be16()
		{
			uint16_t result = (static_cast<uint16_t>(data_[offset_]) << 8)
							| static_cast<uint16_t>(data_[offset_ + 1]);
			offset_ += 2;
			return result;
		}

		uint32_t read_be32()
		{
			uint32_t result = (static_cast<uint32_t>(data_[offset_]) << 24)
							| (static_cast<uint32_t>(data_[offset_ + 1]) << 16)
							| (static_cast<uint32_t>(data_[offset_ + 2]) << 8)
							| static_cast<uint32_t>(data_[offset_ + 3]);
			offset_ += 4;
			return result;
		}

		uint64_t read_be64()
		{
			uint64_t result = (static_cast<uint64_t>(data_[offset_]) << 56)
							| (static_cast<uint64_t>(data_[offset_ + 1]) << 48)
							| (static_cast<uint64_t>(data_[offset_ + 2]) << 40)
							| (static_cast<uint64_t>(data_[offset_ + 3]) << 32)
							| (static_cast<uint64_t>(data_[offset_ + 4]) << 24)
							| (static_cast<uint64_t>(data_[offset_ + 5]) << 16)
							| (static_cast<uint64_t>(data_[offset_ + 6]) << 8)
							| static_cast<uint64_t>(data_[offset_ + 7]);
			offset_ += 8;
			return result;
		}

		const uint8_t* data_;
		size_t size_;
		size_t offset_;
	};

} // namespace container_module
