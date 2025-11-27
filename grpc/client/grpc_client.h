/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, kcenon
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

/**
 * @file grpc_client.h
 * @brief gRPC client wrapper for the container system
 *
 * This client provides easy access to container services via gRPC.
 * It handles connection management, retries, and protocol conversion.
 */

#include "core/container.h"

#include <memory>
#include <string>
#include <chrono>
#include <functional>
#include <optional>

namespace container_grpc {

/**
 * @brief Configuration for gRPC client
 */
struct client_config {
    std::string target_address = "localhost:50051";
    std::chrono::milliseconds timeout{30000};
    int max_retries = 3;
    bool use_ssl = false;
    std::string client_id = "";
};

/**
 * @brief Result type for client operations
 */
template<typename T>
struct client_result {
    bool success = false;
    std::string error_message;
    T value;

    explicit operator bool() const noexcept { return success; }
};

/**
 * @brief Callback type for streaming responses
 */
using stream_callback = std::function<void(
    std::shared_ptr<container_module::value_container>)>;

/**
 * @class grpc_client
 * @brief gRPC client for container system access
 *
 * This client provides synchronous and asynchronous methods for
 * communicating with a container gRPC server.
 *
 * Example usage:
 * @code
 *   container_grpc::grpc_client client("localhost:50051");
 *
 *   auto container = std::make_shared<container_module::value_container>();
 *   container->set_message_type("request");
 *   container->add_value("count", 42);
 *
 *   auto result = client.process(container);
 *   if (result) {
 *       std::cout << result.value->message_type() << std::endl;
 *   }
 * @endcode
 */
class grpc_client {
public:
    /**
     * @brief Construct client with target address
     * @param target Target address in format "host:port"
     */
    explicit grpc_client(const std::string& target);

    /**
     * @brief Construct client with configuration
     * @param config Client configuration
     */
    explicit grpc_client(const client_config& config);

    /**
     * @brief Destructor
     */
    ~grpc_client();

    // Non-copyable, but movable
    grpc_client(const grpc_client&) = delete;
    grpc_client& operator=(const grpc_client&) = delete;
    grpc_client(grpc_client&&) noexcept;
    grpc_client& operator=(grpc_client&&) noexcept;

    /**
     * @brief Check if client is connected
     * @return true if connected to server
     */
    bool is_connected() const noexcept;

    /**
     * @brief Get target address
     * @return Target address string
     */
    std::string target() const noexcept;

    /**
     * @brief Set request timeout
     * @param timeout Timeout duration
     */
    void set_timeout(std::chrono::milliseconds timeout);

    /**
     * @brief Get current timeout
     * @return Timeout duration
     */
    std::chrono::milliseconds timeout() const noexcept;

    // =========================================================================
    // Unary Operations
    // =========================================================================

    /**
     * @brief Send a container and receive response
     * @param container Container to send
     * @return Result containing response container or error
     */
    client_result<std::shared_ptr<container_module::value_container>>
    send(std::shared_ptr<container_module::value_container> container);

    /**
     * @brief Process a container (simple unary call)
     * @param container Container to process
     * @return Result containing processed container or error
     */
    client_result<std::shared_ptr<container_module::value_container>>
    process(std::shared_ptr<container_module::value_container> container);

    // =========================================================================
    // Streaming Operations
    // =========================================================================

    /**
     * @brief Subscribe to container stream
     * @param request Initial request container
     * @param callback Callback for received containers
     * @return true if streaming started successfully
     */
    bool stream(
        std::shared_ptr<container_module::value_container> request,
        stream_callback callback);

    /**
     * @brief Send multiple containers (client streaming)
     * @param containers Containers to send
     * @return Result containing batch response
     */
    client_result<std::vector<std::shared_ptr<container_module::value_container>>>
    send_batch(
        const std::vector<std::shared_ptr<container_module::value_container>>& containers);

    // =========================================================================
    // Status and Health
    // =========================================================================

    /**
     * @brief Get stream status from server
     * @return Optional containing status or nullopt on error
     */
    std::optional<std::pair<int64_t, int64_t>> get_status();

    /**
     * @brief Check server health (sends ping)
     * @return true if server is healthy
     */
    bool ping();

private:
    class impl;
    std::unique_ptr<impl> impl_;
};

} // namespace container_grpc
