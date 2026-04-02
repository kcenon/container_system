// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

#include "binary_serializer.h"
#include "../container.h"
#include "utilities/core/convert_string.h"
#include "utilities/core/formatter.h"

namespace kcenon::container
{

namespace
{
	// Header field IDs for binary format
	inline constexpr int TARGET_ID = 1;
	inline constexpr int TARGET_SUB_ID = 2;
	inline constexpr int SOURCE_ID = 3;
	inline constexpr int SOURCE_SUB_ID = 4;
	inline constexpr int MESSAGE_TYPE = 5;
	inline constexpr int MESSAGE_VERSION = 6;
} // anonymous namespace

#if CONTAINER_HAS_RESULT
kcenon::common::Result<std::vector<uint8_t>>
binary_serializer::serialize(const value_container& container) const noexcept
{
	try
	{
		// Get container data through public accessors
		auto message_type = container.message_type();
		auto target_id = container.target_id();
		auto target_sub_id = container.target_sub_id();
		auto source_id = container.source_id();
		auto source_sub_id = container.source_sub_id();
		auto version = container.version();
		auto data_section = container.datas();

		// Build header with pre-allocated buffer
		std::string result;
		result.reserve(200 + data_section.size());

		// Note: In fmt library, {{}} produces single {}, so {{{{}}}} produces {{}}
		utility_module::formatter::format_to(std::back_inserter(result), "@header={{{{");

		if (message_type != "data_container")
		{
			utility_module::formatter::format_to(std::back_inserter(result), "[{},{}];",
								 TARGET_ID, target_id);
			utility_module::formatter::format_to(std::back_inserter(result), "[{},{}];",
								 TARGET_SUB_ID, target_sub_id);
			utility_module::formatter::format_to(std::back_inserter(result), "[{},{}];",
								 SOURCE_ID, source_id);
			utility_module::formatter::format_to(std::back_inserter(result), "[{},{}];",
								 SOURCE_SUB_ID, source_sub_id);
		}
		utility_module::formatter::format_to(std::back_inserter(result), "[{},{}];",
							 MESSAGE_TYPE, message_type);
		utility_module::formatter::format_to(std::back_inserter(result), "[{},{}];",
							 MESSAGE_VERSION, version);
		utility_module::formatter::format_to(std::back_inserter(result), "}}}};");

		// Append data section
		result.append(data_section);

		// Convert to byte array
		auto [arr, err] = utility_module::convert_string::to_array(result);
		if (!err.empty())
		{
			return kcenon::common::Result<std::vector<uint8_t>>(
				kcenon::common::error_info{
					-1,
					"Failed to convert binary string to bytes: " + err,
					"binary_serializer"});
		}

		return kcenon::common::ok(std::move(arr));
	}
	catch (const std::bad_alloc&)
	{
		return kcenon::common::Result<std::vector<uint8_t>>(
			kcenon::common::error_info{
				-2,
				"Memory allocation failed during binary serialization",
				"binary_serializer"});
	}
	catch (const std::exception& e)
	{
		return kcenon::common::Result<std::vector<uint8_t>>(
			kcenon::common::error_info{
				-1,
				std::string("Binary serialization failed: ") + e.what(),
				"binary_serializer"});
	}
}

std::string binary_serializer::serialize_to_string(const value_container& container) const
{
	// Get container data through public accessors
	auto message_type = container.message_type();
	auto target_id = container.target_id();
	auto target_sub_id = container.target_sub_id();
	auto source_id = container.source_id();
	auto source_sub_id = container.source_sub_id();
	auto version = container.version();
	auto data_section = container.datas();

	// Build header with pre-allocated buffer
	std::string result;
	result.reserve(200 + data_section.size());

	utility_module::formatter::format_to(std::back_inserter(result), "@header={{{{");

	if (message_type != "data_container")
	{
		utility_module::formatter::format_to(std::back_inserter(result), "[{},{}];",
							 TARGET_ID, target_id);
		utility_module::formatter::format_to(std::back_inserter(result), "[{},{}];",
							 TARGET_SUB_ID, target_sub_id);
		utility_module::formatter::format_to(std::back_inserter(result), "[{},{}];",
							 SOURCE_ID, source_id);
		utility_module::formatter::format_to(std::back_inserter(result), "[{},{}];",
							 SOURCE_SUB_ID, source_sub_id);
	}
	utility_module::formatter::format_to(std::back_inserter(result), "[{},{}];",
						 MESSAGE_TYPE, message_type);
	utility_module::formatter::format_to(std::back_inserter(result), "[{},{}];",
						 MESSAGE_VERSION, version);
	utility_module::formatter::format_to(std::back_inserter(result), "}}}};");

	result.append(data_section);

	return result;
}
#endif

} // namespace kcenon::container
