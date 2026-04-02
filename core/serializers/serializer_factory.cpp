// BSD 3-Clause License
// Copyright (c) 2021, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

#include "serializer_factory.h"
#include "binary_serializer.h"
#include "json_serializer.h"
#include "xml_serializer.h"
#include "msgpack_serializer.h"

namespace kcenon::container
{

std::unique_ptr<serializer_strategy>
serializer_factory::create(serialization_format fmt) noexcept
{
	switch (fmt)
	{
		case serialization_format::binary:
			return std::make_unique<binary_serializer>();

		case serialization_format::json:
			return std::make_unique<json_serializer>();

		case serialization_format::xml:
			return std::make_unique<xml_serializer>();

		case serialization_format::msgpack:
			return std::make_unique<msgpack_serializer>();

		case serialization_format::auto_detect:
		case serialization_format::unknown:
		default:
			return nullptr;
	}
}

bool serializer_factory::is_supported(serialization_format fmt) noexcept
{
	switch (fmt)
	{
		case serialization_format::binary:
		case serialization_format::json:
		case serialization_format::xml:
		case serialization_format::msgpack:
			return true;

		case serialization_format::auto_detect:
		case serialization_format::unknown:
		default:
			return false;
	}
}

} // namespace kcenon::container
