// BSD 3-Clause License
//
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace utility_module {

/**
 * @brief String conversion utilities
 */
class convert_string {
public:
    /**
     * @brief Convert string to byte array
     */
    static std::pair<std::vector<uint8_t>, std::string> to_array(const std::string& str) {
        try {
            std::vector<uint8_t> result(str.begin(), str.end());
            return {result, ""};
        } catch (const std::exception& e) {
            return {{}, e.what()};
        }
    }

    /**
     * @brief Convert byte array to string
     */
    static std::pair<std::string, std::string> to_string(const std::vector<uint8_t>& arr) {
        try {
            std::string result(arr.begin(), arr.end());
            return {result, ""};
        } catch (const std::exception& e) {
            return {"", e.what()};
        }
    }

    /**
     * @brief Convert data to base64
     */
    static std::pair<std::string, std::string> to_base64(const std::vector<uint8_t>& data) {
        try {
            const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::string result;
            int val = 0;
            int valb = -6;

            for (uint8_t c : data) {
                val = (val << 8) + c;
                valb += 8;
                while (valb >= 0) {
                    result.push_back(chars[(val >> valb) & 0x3F]);
                    valb -= 6;
                }
            }

            if (valb > -6) {
                result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
            }

            while (result.size() % 4) {
                result.push_back('=');
            }

            return {result, ""};
        } catch (const std::exception& e) {
            return {"", e.what()};
        }
    }

    /**
     * @brief Convert from base64
     */
    static std::pair<std::vector<uint8_t>, std::string> from_base64(const std::string& str) {
        try {
            const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::vector<uint8_t> result;
            int val = 0;
            int valb = -8;

            for (char c : str) {
                if (c == '=') break;
                size_t pos = chars.find(c);
                if (pos == std::string::npos) continue;

                val = (val << 6) + pos;
                valb += 6;
                if (valb >= 0) {
                    result.push_back((val >> valb) & 0xFF);
                    valb -= 8;
                }
            }

            return {result, ""};
        } catch (const std::exception& e) {
            return {{}, e.what()};
        }
    }

    /**
     * @brief Replace all occurrences of a substring
     */
    static void replace(std::string& str, const std::string& from, const std::string& to) {
        if (from.empty()) return;

        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    /**
     * @brief Convert string to string (identity function for compatibility)
     */
    static std::pair<std::string, std::string> to_string(const std::string& str) {
        return {str, ""};
    }
};

} // namespace utility_module