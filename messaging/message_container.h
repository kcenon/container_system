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

#pragma once

#include "container/core/value_store.h"
#include "container/internal/variant_value_v2.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace container_module {

/**
 * @brief Messaging-specific container
 *
 * Extends value_store with messaging-specific metadata such as
 * source_id, target_id, and message_type. This class is designed
 * for message-passing systems and network communication.
 *
 * Features:
 * - Inherits all value_store capabilities (key-value storage, serialization)
 * - Adds messaging metadata (source, target, message type, version)
 * - Header swapping for request/response patterns
 * - Backward compatible with value_container
 *
 * @note This class is part of Sprint 3: Domain Separation initiative
 * @see value_store for the domain-agnostic storage layer
 */
class message_container {
public:
    /**
     * @brief Default constructor
     */
    message_container() = default;

    /**
     * @brief Construct with message type
     * @param message_type The type of message
     */
    explicit message_container(std::string_view message_type);

    /**
     * @brief Construct with target and message type
     * @param target_id The target identifier
     * @param target_sub_id The target sub-identifier (optional)
     * @param message_type The type of message
     */
    message_container(std::string_view target_id,
                     std::string_view target_sub_id,
                     std::string_view message_type);

    /**
     * @brief Construct with full header information
     * @param source_id The source identifier
     * @param source_sub_id The source sub-identifier
     * @param target_id The target identifier
     * @param target_sub_id The target sub-identifier
     * @param message_type The type of message
     */
    message_container(std::string_view source_id,
                     std::string_view source_sub_id,
                     std::string_view target_id,
                     std::string_view target_sub_id,
                     std::string_view message_type);

    /**
     * @brief Destructor
     */
    virtual ~message_container() = default;

    // Copy and move semantics (deleted due to value_store mutex member)
    message_container(const message_container&) = delete;
    message_container(message_container&&) = delete;
    message_container& operator=(const message_container&) = delete;
    message_container& operator=(message_container&&) = delete;

    // =========================================================================
    // Messaging Header Management
    // =========================================================================

    /**
     * @brief Set source information
     * @param source_id The source identifier
     * @param source_sub_id The source sub-identifier
     */
    void set_source(std::string_view source_id, std::string_view source_sub_id = "");

    /**
     * @brief Set target information
     * @param target_id The target identifier
     * @param target_sub_id The target sub-identifier
     */
    void set_target(std::string_view target_id, std::string_view target_sub_id = "");

    /**
     * @brief Set message type
     * @param message_type The type of message
     */
    void set_message_type(std::string_view message_type);

    /**
     * @brief Set version string
     * @param version The version string
     */
    void set_version(std::string_view version);

    /**
     * @brief Get source identifier
     * @return Source identifier
     */
    std::string source_id() const { return source_id_; }

    /**
     * @brief Get source sub-identifier
     * @return Source sub-identifier
     */
    std::string source_sub_id() const { return source_sub_id_; }

    /**
     * @brief Get target identifier
     * @return Target identifier
     */
    std::string target_id() const { return target_id_; }

    /**
     * @brief Get target sub-identifier
     * @return Target sub-identifier
     */
    std::string target_sub_id() const { return target_sub_id_; }

    /**
     * @brief Get message type
     * @return Message type
     */
    std::string message_type() const { return message_type_; }

    /**
     * @brief Get version string
     * @return Version string
     */
    std::string version() const { return version_; }

    /**
     * @brief Swap source and target IDs
     *
     * Useful for creating response messages from requests.
     */
    void swap_header();

    // =========================================================================
    // Payload Access (value_store delegation)
    // =========================================================================

    /**
     * @brief Get mutable reference to payload
     * @return Reference to the underlying value_store
     */
    value_store& payload() { return payload_; }

    /**
     * @brief Get const reference to payload
     * @return Const reference to the underlying value_store
     */
    const value_store& payload() const { return payload_; }

    // =========================================================================
    // Serialization
    // =========================================================================

    /**
     * @brief Serialize to JSON string (header + payload)
     * @return JSON representation
     * @throws std::runtime_error if serialization fails
     */
    std::string serialize() const;

    /**
     * @brief Serialize to binary format (header + payload)
     * @return Binary data
     * @throws std::runtime_error if serialization fails
     */
    std::vector<uint8_t> serialize_binary() const;

    /**
     * @brief Deserialize from JSON string
     * @param json_data JSON string
     * @return Unique pointer to message_container instance
     * @throws std::runtime_error if deserialization fails
     */
    static std::unique_ptr<message_container> deserialize(std::string_view json_data);

    /**
     * @brief Deserialize from binary format
     * @param binary_data Binary data
     * @return Unique pointer to message_container instance
     * @throws std::runtime_error if deserialization fails
     */
    static std::unique_ptr<message_container> deserialize_binary(const std::vector<uint8_t>& binary_data);

private:
    // Messaging-specific header fields
    std::string source_id_;
    std::string source_sub_id_;
    std::string target_id_;
    std::string target_sub_id_;
    std::string message_type_;
    std::string version_{"1.0.0.0"};

    // Domain-agnostic payload storage
    value_store payload_;
};

} // namespace container_module
