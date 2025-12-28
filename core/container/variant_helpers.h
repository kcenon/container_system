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
 * @file core/container/variant_helpers.h
 * @brief Helper functions for variant value manipulation
 *
 * This header provides utility functions for working with value_variant:
 * - JSON string escaping (RFC 8259 compliant)
 * - XML string encoding (XML 1.0 specification)
 * - String conversion for variant values
 * - Data size calculation
 */

#pragma once

#include "container/core/container/types.h"

#include <string>
#include <string_view>
#include <cstdio>

namespace container_module
{
	/**
	 * @brief Helper functions for variant value manipulation
	 */
	namespace variant_helpers
	{
		/**
		 * @brief Escape a string for JSON output per RFC 8259
		 * @param input Raw string
		 * @return JSON-escaped string (without surrounding quotes)
		 */
		inline std::string json_escape(std::string_view input)
		{
			std::string result;
			result.reserve(input.size() + input.size() / 8);

			for (char c : input)
			{
				switch (c)
				{
				case '"':
					result += "\\\"";
					break;
				case '\\':
					result += "\\\\";
					break;
				case '\b':
					result += "\\b";
					break;
				case '\f':
					result += "\\f";
					break;
				case '\n':
					result += "\\n";
					break;
				case '\r':
					result += "\\r";
					break;
				case '\t':
					result += "\\t";
					break;
				default:
					if (static_cast<unsigned char>(c) < 0x20)
					{
						// Control character - use \uXXXX format
						char buf[8];
						std::snprintf(buf, sizeof(buf), "\\u%04x",
									  static_cast<unsigned char>(c));
						result += buf;
					}
					else
					{
						result += c;
					}
					break;
				}
			}

			return result;
		}

		/**
		 * @brief Encode a string for XML output per XML 1.0 specification
		 * @param input Raw string
		 * @return XML-encoded string with entity references
		 */
		inline std::string xml_encode(std::string_view input)
		{
			std::string result;
			result.reserve(input.size() + input.size() / 8);

			for (char c : input)
			{
				switch (c)
				{
				case '&':
					result += "&amp;";
					break;
				case '<':
					result += "&lt;";
					break;
				case '>':
					result += "&gt;";
					break;
				case '"':
					result += "&quot;";
					break;
				case '\'':
					result += "&apos;";
					break;
				default:
					if (static_cast<unsigned char>(c) < 0x20
						&& c != '\t' && c != '\n' && c != '\r')
					{
						// Control character - use numeric character reference
						char buf[16];
						std::snprintf(buf, sizeof(buf), "&#x%02x;",
									  static_cast<unsigned char>(c));
						result += buf;
					}
					else
					{
						result += c;
					}
					break;
				}
			}

			return result;
		}

		/**
		 * @brief Convert value_variant to string representation
		 */
		inline std::string to_string(const value_variant& var, value_types type)
		{
			switch (type)
			{
			case value_types::null_value:
				return "";
			case value_types::bool_value:
				return std::get<bool>(var) ? "true" : "false";
			case value_types::short_value:
				return std::to_string(std::get<short>(var));
			case value_types::ushort_value:
				return std::to_string(std::get<unsigned short>(var));
			case value_types::int_value:
				return std::to_string(std::get<int>(var));
			case value_types::uint_value:
				return std::to_string(std::get<unsigned int>(var));
			case value_types::long_value:
				return std::to_string(std::get<long>(var));
			case value_types::ulong_value:
				return std::to_string(std::get<unsigned long>(var));
			case value_types::llong_value:
				return std::to_string(std::get<long long>(var));
			case value_types::ullong_value:
				return std::to_string(std::get<unsigned long long>(var));
			case value_types::float_value:
				return std::to_string(std::get<float>(var));
			case value_types::double_value:
				return std::to_string(std::get<double>(var));
			case value_types::string_value:
				return std::get<std::string>(var);
			default:
				return "";
			}
		}

		/**
		 * @brief Get size in bytes of variant data
		 */
		inline size_t data_size(const value_variant& var, value_types type)
		{
			switch (type)
			{
			case value_types::null_value:
				return 0;
			case value_types::bool_value:
				return sizeof(bool);
			case value_types::short_value:
				return sizeof(short);
			case value_types::ushort_value:
				return sizeof(unsigned short);
			case value_types::int_value:
				return sizeof(int);
			case value_types::uint_value:
				return sizeof(unsigned int);
			case value_types::long_value:
				return sizeof(long);
			case value_types::ulong_value:
				return sizeof(unsigned long);
			case value_types::llong_value:
				return sizeof(long long);
			case value_types::ullong_value:
				return sizeof(unsigned long long);
			case value_types::float_value:
				return sizeof(float);
			case value_types::double_value:
				return sizeof(double);
			case value_types::string_value:
				return std::get<std::string>(var).size();
			case value_types::bytes_value:
				return std::get<std::vector<uint8_t>>(var).size();
			default:
				return 0;
			}
		}
	} // namespace variant_helpers

} // namespace container_module
