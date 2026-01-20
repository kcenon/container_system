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

#include "serializer_factory.h"

namespace container_module
{

std::unique_ptr<serializer_strategy>
serializer_factory::create(serialization_format fmt) noexcept
{
	// Concrete serializer implementations will be added in Issue #314
	// For now, return nullptr to indicate format-specific serializers are not yet available
	switch (fmt)
	{
		case serialization_format::binary:
		case serialization_format::json:
		case serialization_format::xml:
		case serialization_format::msgpack:
			// TODO(#314): Return concrete serializer implementations
			return nullptr;

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

} // namespace container_module
