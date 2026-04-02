// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

#include "xml_serializer.h"
#include "../container.h"
#include "../container/variant_helpers.h"
#include "utilities/core/formatter.h"

namespace kcenon::container
{

#if CONTAINER_HAS_RESULT
kcenon::common::Result<std::vector<uint8_t>>
xml_serializer::serialize(const value_container& container) const noexcept
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
				"Memory allocation failed during XML serialization",
				"xml_serializer"});
	}
	catch (const std::exception& e)
	{
		return kcenon::common::Result<std::vector<uint8_t>>(
			kcenon::common::error_info{
				-1,
				std::string("XML serialization failed: ") + e.what(),
				"xml_serializer"});
	}
}

std::string xml_serializer::serialize_to_string(const value_container& container) const
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
	utility_module::formatter::format_to(std::back_inserter(result), "<container>");

	// Header section
	utility_module::formatter::format_to(std::back_inserter(result), "<header>");
	if (message_type != "data_container")
	{
		utility_module::formatter::format_to(std::back_inserter(result),
							 "<target_id>{}</target_id>",
							 variant_helpers::xml_encode(target_id));
		utility_module::formatter::format_to(std::back_inserter(result),
							 "<target_sub_id>{}</target_sub_id>",
							 variant_helpers::xml_encode(target_sub_id));
		utility_module::formatter::format_to(std::back_inserter(result),
							 "<source_id>{}</source_id>",
							 variant_helpers::xml_encode(source_id));
		utility_module::formatter::format_to(std::back_inserter(result),
							 "<source_sub_id>{}</source_sub_id>",
							 variant_helpers::xml_encode(source_sub_id));
	}
	utility_module::formatter::format_to(std::back_inserter(result),
						 "<message_type>{}</message_type>",
						 variant_helpers::xml_encode(message_type));
	utility_module::formatter::format_to(std::back_inserter(result),
						 "<version>{}</version>",
						 variant_helpers::xml_encode(version));
	utility_module::formatter::format_to(std::back_inserter(result), "</header>");

	// Values section
	utility_module::formatter::format_to(std::back_inserter(result), "<values>");
	for (const auto& u : values)
	{
		std::string value_str = variant_helpers::to_string(u.data, u.type);
		utility_module::formatter::format_to(std::back_inserter(result), "<{}>{}</{}>",
							 u.name, variant_helpers::xml_encode(value_str),
							 u.name);
	}
	utility_module::formatter::format_to(std::back_inserter(result), "</values>");
	utility_module::formatter::format_to(std::back_inserter(result), "</container>");

	return result;
}
#endif

} // namespace kcenon::container
