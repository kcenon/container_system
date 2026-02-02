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

#include "msgpack_serializer.h"
#include "../container.h"
#include "../container/msgpack.h"

namespace container_module
{

#if CONTAINER_HAS_RESULT

namespace
{
	// Helper function to serialize a value_container to msgpack
	// This allows recursive serialization of nested containers
	std::vector<uint8_t> serialize_container_to_msgpack(const value_container& container);

	void write_value_to_encoder(msgpack_encoder& encoder, const optimized_value& unit)
	{
		switch (unit.type)
		{
		case value_types::null_value:
			encoder.write_nil();
			break;

		case value_types::bool_value:
			encoder.write_bool(std::get<bool>(unit.data));
			break;

		case value_types::short_value:
			encoder.write_int(std::get<short>(unit.data));
			break;

		case value_types::ushort_value:
			encoder.write_uint(std::get<unsigned short>(unit.data));
			break;

		case value_types::int_value:
			encoder.write_int(std::get<int>(unit.data));
			break;

		case value_types::uint_value:
			encoder.write_uint(std::get<unsigned int>(unit.data));
			break;

		case value_types::long_value:
			encoder.write_int(std::get<long>(unit.data));
			break;

		case value_types::ulong_value:
			encoder.write_uint(std::get<unsigned long>(unit.data));
			break;

		case value_types::llong_value:
			encoder.write_int(std::get<long long>(unit.data));
			break;

		case value_types::ullong_value:
			encoder.write_uint(std::get<unsigned long long>(unit.data));
			break;

		case value_types::float_value:
			encoder.write_float(std::get<float>(unit.data));
			break;

		case value_types::double_value:
			encoder.write_double(std::get<double>(unit.data));
			break;

		case value_types::string_value:
			encoder.write_string(std::get<std::string>(unit.data));
			break;

		case value_types::bytes_value:
			encoder.write_binary(std::get<std::vector<uint8_t>>(unit.data));
			break;

		case value_types::container_value:
			{
				// Nested container: serialize recursively
				auto nested = std::get<std::shared_ptr<value_container>>(unit.data);
				if (nested)
				{
					auto nested_data = serialize_container_to_msgpack(*nested);
					encoder.write_binary(nested_data);
				}
				else
				{
					encoder.write_nil();
				}
			}
			break;

		case value_types::array_value:
			// Array values are stored as containers
			encoder.write_nil();
			break;

		default:
			encoder.write_nil();
			break;
		}
	}

	std::vector<uint8_t> serialize_container_to_msgpack(const value_container& container)
	{
		// Get container data through public accessors
		auto message_type = container.message_type();
		auto target_id = container.target_id();
		auto target_sub_id = container.target_sub_id();
		auto source_id = container.source_id();
		auto source_sub_id = container.source_sub_id();
		auto version = container.version();
		auto values = container.get_variant_values();

		msgpack_encoder encoder;

		// Estimate buffer size: header (~100 bytes) + values
		encoder.reserve(200 + values.size() * 32);

		// MessagePack structure:
		// {
		//   "header": { ... },
		//   "values": { ... }
		// }

		// Write outer map with 2 entries (header + values)
		encoder.write_map_header(2);

		// Write "header" key
		encoder.write_string("header");

		// Calculate header entries count
		size_t header_count = 2; // message_type and version are always present
		if (message_type != "data_container")
		{
			header_count += 4; // target_id, target_sub_id, source_id, source_sub_id
		}

		// Write header map
		encoder.write_map_header(header_count);

		if (message_type != "data_container")
		{
			encoder.write_string("target_id");
			encoder.write_string(target_id);

			encoder.write_string("target_sub_id");
			encoder.write_string(target_sub_id);

			encoder.write_string("source_id");
			encoder.write_string(source_id);

			encoder.write_string("source_sub_id");
			encoder.write_string(source_sub_id);
		}

		encoder.write_string("message_type");
		encoder.write_string(message_type);

		encoder.write_string("version");
		encoder.write_string(version);

		// Write "values" key
		encoder.write_string("values");

		// Write values map
		encoder.write_map_header(values.size());

		for (const auto& unit : values)
		{
			// Write key
			encoder.write_string(unit.name);

			// Write value based on type
			write_value_to_encoder(encoder, unit);
		}

		return encoder.finish();
	}
} // anonymous namespace

kcenon::common::Result<std::vector<uint8_t>>
msgpack_serializer::serialize(const value_container& container) const noexcept
{
	try
	{
		auto data = serialize_container_to_msgpack(container);
		return kcenon::common::ok(std::move(data));
	}
	catch (const std::bad_alloc&)
	{
		return kcenon::common::Result<std::vector<uint8_t>>(
			kcenon::common::error_info{
				-2,
				"Memory allocation failed during MessagePack serialization",
				"msgpack_serializer"});
	}
	catch (const std::exception& e)
	{
		return kcenon::common::Result<std::vector<uint8_t>>(
			kcenon::common::error_info{
				-1,
				std::string("MessagePack serialization failed: ") + e.what(),
				"msgpack_serializer"});
	}
}
#endif

} // namespace container_module
