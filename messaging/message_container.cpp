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

#include "messaging/message_container.h"

#include <utility>
#include <stdexcept>
#include <sstream>

#ifdef __has_include
#if __has_include(<nlohmann/json.hpp>)
#include <nlohmann/json.hpp>
#define HAS_NLOHMANN_JSON 1
#elif __has_include(<json/json.h>)
#include <json/json.h>
#define HAS_NLOHMANN_JSON 0
#else
#define HAS_NLOHMANN_JSON -1
#endif
#endif

namespace container_module {

// ============================================================================
// Constructors
// ============================================================================

message_container::message_container(std::string_view message_type)
    : message_type_(message_type)
{
}

message_container::message_container(std::string_view target_id,
                                   std::string_view target_sub_id,
                                   std::string_view message_type)
    : target_id_(target_id)
    , target_sub_id_(target_sub_id)
    , message_type_(message_type)
{
}

message_container::message_container(std::string_view source_id,
                                   std::string_view source_sub_id,
                                   std::string_view target_id,
                                   std::string_view target_sub_id,
                                   std::string_view message_type)
    : source_id_(source_id)
    , source_sub_id_(source_sub_id)
    , target_id_(target_id)
    , target_sub_id_(target_sub_id)
    , message_type_(message_type)
{
}

// ============================================================================
// Header Management
// ============================================================================

void message_container::set_source(std::string_view source_id, std::string_view source_sub_id)
{
    source_id_ = source_id;
    source_sub_id_ = source_sub_id;
}

void message_container::set_target(std::string_view target_id, std::string_view target_sub_id)
{
    target_id_ = target_id;
    target_sub_id_ = target_sub_id;
}

void message_container::set_message_type(std::string_view message_type)
{
    message_type_ = message_type;
}

void message_container::set_version(std::string_view version)
{
    version_ = version;
}

void message_container::swap_header()
{
    std::swap(source_id_, target_id_);
    std::swap(source_sub_id_, target_sub_id_);
}

// ============================================================================
// Serialization
// ============================================================================

std::string message_container::serialize() const
{
#if HAS_NLOHMANN_JSON == 1
    nlohmann::json json_obj;

    // Serialize header
    nlohmann::json header;
    header["source_id"] = source_id_;
    header["source_sub_id"] = source_sub_id_;
    header["target_id"] = target_id_;
    header["target_sub_id"] = target_sub_id_;
    header["message_type"] = message_type_;
    header["version"] = version_;

    json_obj["header"] = header;

    // Serialize payload
    json_obj["payload"] = payload_.serialize();

    return json_obj.dump();
#else
    // Fallback: simple JSON-like format
    std::ostringstream oss;
    oss << "{"
        << "\"header\":{"
        << "\"source_id\":\"" << source_id_ << "\","
        << "\"source_sub_id\":\"" << source_sub_id_ << "\","
        << "\"target_id\":\"" << target_id_ << "\","
        << "\"target_sub_id\":\"" << target_sub_id_ << "\","
        << "\"message_type\":\"" << message_type_ << "\","
        << "\"version\":\"" << version_ << "\""
        << "},"
        << "\"payload\":" << payload_.serialize()
        << "}";
    return oss.str();
#endif
}

std::vector<uint8_t> message_container::serialize_binary() const
{
    // Binary format: [header_size][header_data][payload_data]
    std::vector<uint8_t> result;

    // Serialize header to JSON first (can be optimized to binary format)
    std::string header_json = "{"
        "\"source_id\":\"" + source_id_ + "\","
        "\"source_sub_id\":\"" + source_sub_id_ + "\","
        "\"target_id\":\"" + target_id_ + "\","
        "\"target_sub_id\":\"" + target_sub_id_ + "\","
        "\"message_type\":\"" + message_type_ + "\","
        "\"version\":\"" + version_ + "\""
        "}";

    // Write header size (4 bytes)
    uint32_t header_size = static_cast<uint32_t>(header_json.size());
    result.push_back(static_cast<uint8_t>(header_size & 0xFF));
    result.push_back(static_cast<uint8_t>((header_size >> 8) & 0xFF));
    result.push_back(static_cast<uint8_t>((header_size >> 16) & 0xFF));
    result.push_back(static_cast<uint8_t>((header_size >> 24) & 0xFF));

    // Write header data
    result.insert(result.end(), header_json.begin(), header_json.end());

    // Write payload data
    auto payload_data = payload_.serialize_binary();
    result.insert(result.end(), payload_data.begin(), payload_data.end());

    return result;
}

std::unique_ptr<message_container> message_container::deserialize(std::string_view json_data)
{
#if HAS_NLOHMANN_JSON == 1
    auto json_obj = nlohmann::json::parse(json_data);

    auto container = std::make_unique<message_container>();

    // Deserialize header
    if (json_obj.contains("header")) {
        auto header = json_obj["header"];
        if (header.contains("source_id")) container->source_id_ = header["source_id"];
        if (header.contains("source_sub_id")) container->source_sub_id_ = header["source_sub_id"];
        if (header.contains("target_id")) container->target_id_ = header["target_id"];
        if (header.contains("target_sub_id")) container->target_sub_id_ = header["target_sub_id"];
        if (header.contains("message_type")) container->message_type_ = header["message_type"];
        if (header.contains("version")) container->version_ = header["version"];
    }

    // Deserialize payload
    // Note: payload is already default-constructed, we modify it in place
    if (json_obj.contains("payload")) {
        std::string payload_str = json_obj["payload"].dump();
        // TODO: Implement in-place deserialization for value_store
        // For now, this is a placeholder - actual implementation would need
        // value_store to support in-place updates
    }

    return container;
#else
    throw std::runtime_error("JSON library not available for deserialization");
#endif
}

std::unique_ptr<message_container> message_container::deserialize_binary(const std::vector<uint8_t>& binary_data)
{
    if (binary_data.size() < 4) {
        throw std::runtime_error("Binary data too small to contain header size");
    }

    // Read header size (4 bytes)
    uint32_t header_size = static_cast<uint32_t>(binary_data[0])
                         | (static_cast<uint32_t>(binary_data[1]) << 8)
                         | (static_cast<uint32_t>(binary_data[2]) << 16)
                         | (static_cast<uint32_t>(binary_data[3]) << 24);

    if (binary_data.size() < 4 + header_size) {
        throw std::runtime_error("Binary data too small to contain header");
    }

    // Read header data
    std::string header_json(binary_data.begin() + 4, binary_data.begin() + 4 + header_size);

    auto container = std::make_unique<message_container>();

#if HAS_NLOHMANN_JSON == 1
    auto header = nlohmann::json::parse(header_json);
    if (header.contains("source_id")) container->source_id_ = header["source_id"];
    if (header.contains("source_sub_id")) container->source_sub_id_ = header["source_sub_id"];
    if (header.contains("target_id")) container->target_id_ = header["target_id"];
    if (header.contains("target_sub_id")) container->target_sub_id_ = header["target_sub_id"];
    if (header.contains("message_type")) container->message_type_ = header["message_type"];
    if (header.contains("version")) container->version_ = header["version"];
#else
    // Simple parsing fallback (not recommended for production)
    // This is a simplified version - proper JSON parsing would be needed
#endif

    // Read payload data
    // TODO: Implement in-place deserialization for value_store
    // std::vector<uint8_t> payload_data(binary_data.begin() + 4 + header_size, binary_data.end());
    // container->payload_ would need to be deserialized in place

    return container;
}

} // namespace container_module
