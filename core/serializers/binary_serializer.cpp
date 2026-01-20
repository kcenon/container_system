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

#include "binary_serializer.h"
#include "container/core/container.h"
#include "utilities/core/convert_string.h"

namespace container_module
{

#if KCENON_HAS_COMMON_SYSTEM
kcenon::common::Result<std::vector<uint8_t>>
binary_serializer::serialize(const value_container& container) const noexcept
{
	try
	{
		// Delegate to the container's existing serialize_string implementation
		auto str_result = container.serialize_string(
			value_container::serialization_format::binary);
		if (!str_result.is_ok())
		{
			return kcenon::common::Result<std::vector<uint8_t>>(str_result.error());
		}

		const auto& str = str_result.value();
		auto [arr, err] = utility_module::convert_string::to_array(str);
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
	catch (const std::exception& e)
	{
		return kcenon::common::Result<std::vector<uint8_t>>(
			kcenon::common::error_info{
				-1,
				std::string("Binary serialization failed: ") + e.what(),
				"binary_serializer"});
	}
}
#endif

} // namespace container_module
