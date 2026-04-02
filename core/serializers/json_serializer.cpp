// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

#include "json_serializer.h"
#include "../container.h"
#include "../container/variant_helpers.h"
#include "utilities/core/formatter.h"

namespace kcenon::container
{

#if CONTAINER_HAS_RESULT
kcenon::common::Result<std::vector<uint8_t>>
json_serializer::serialize(const value_container& container) const noexcept
{
	try
	{
		auto str = serialize_to_string(container);
		return kcenon::common::ok(std::vector<uint8_t>(str.begin(), str.end()));
	}
	catch (const std::bad_alloc&)
	{
		return kcenon::common::Result<std::vector<uint8_t>>(
			kcenon::common::error_info{
				-2,
				"Memory allocation failed during JSON serialization",
				"json_serializer"});
	}
	catch (const std::exception& e)
	{
		return kcenon::common::Result<std::vector<uint8_t>>(
			kcenon::common::error_info{
				-1,
				std::string("JSON serialization failed: ") + e.what(),
				"json_serializer"});
	}
}

std::string json_serializer::serialize_to_string(const value_container& container) const
{
	// Get container data through public accessors
	auto message_type = container.message_type();
	auto target_id = container.target_id();
	auto target_sub_id = container.target_sub_id();
	auto source_id = container.source_id();
	auto source_sub_id = container.source_sub_id();
	auto version = container.version();
	auto values = container.get_variant_values();

	std::string result;
	utility_module::formatter::format_to(std::back_inserter(result), "{{");

	// Header section
	utility_module::formatter::format_to(std::back_inserter(result), "\"header\":{{");
	if (message_type != "data_container")
	{
		utility_module::formatter::format_to(std::back_inserter(result),
							 "\"target_id\":\"{}\",",
							 variant_helpers::json_escape(target_id));
		utility_module::formatter::format_to(std::back_inserter(result),
							 "\"target_sub_id\":\"{}\",",
							 variant_helpers::json_escape(target_sub_id));
		utility_module::formatter::format_to(std::back_inserter(result),
							 "\"source_id\":\"{}\",",
							 variant_helpers::json_escape(source_id));
		utility_module::formatter::format_to(std::back_inserter(result),
							 "\"source_sub_id\":\"{}\",",
							 variant_helpers::json_escape(source_sub_id));
	}
	utility_module::formatter::format_to(std::back_inserter(result),
						 "\"message_type\":\"{}\"",
						 variant_helpers::json_escape(message_type));
	utility_module::formatter::format_to(std::back_inserter(result), ",\"version\":\"{}\"",
						 variant_helpers::json_escape(version));
	utility_module::formatter::format_to(std::back_inserter(result), "}},"); // end header

	// Values section
	utility_module::formatter::format_to(std::back_inserter(result), "\"values\":{{");
	bool first = true;
	for (const auto& u : values)
	{
		if (!first)
		{
			utility_module::formatter::format_to(std::back_inserter(result), ",");
		}
		std::string value_str = variant_helpers::to_string(u.data, u.type);
		std::string escaped_name = variant_helpers::json_escape(u.name);

		// String and bytes values need quotes
		if (u.type == value_types::string_value || u.type == value_types::bytes_value)
		{
			std::string escaped_value = variant_helpers::json_escape(value_str);
			utility_module::formatter::format_to(std::back_inserter(result), "\"{}\":\"{}\"",
								 escaped_name, escaped_value);
		}
		else
		{
			utility_module::formatter::format_to(std::back_inserter(result), "\"{}\":{}",
								 escaped_name, value_str);
		}
		first = false;
	}
	utility_module::formatter::format_to(std::back_inserter(result), "}}"); // end values
	utility_module::formatter::format_to(std::back_inserter(result), "}}");

	return result;
}
#endif

} // namespace kcenon::container
