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
 * @file grpc_server.h
 * @brief gRPC server wrapper for the container system
 *
 * This server provides gRPC access to the container system functionality
 * without modifying any existing container system code. It uses the adapter
 * layer to convert between native containers and protobuf messages.
 */

#include "core/container.h"

#include <memory>
#include <string>
#include <atomic>
#include <functional>

// Forward declarations
namespace grpc {
class Server;
class ServerBuilder;
}

namespace container_grpc {

/**
 * @brief Configuration for gRPC server
 */
struct server_config {
    std::string address = "0.0.0.0:50051";
    int max_receive_message_size = 64 * 1024 * 1024;  // 64MB
    int max_send_message_size = 64 * 1024 * 1024;     // 64MB
    int num_completion_queues = 1;
    bool enable_reflection = false;
};

/**
 * @brief Callback type for processing containers
 */
using container_processor = std::function<
    std::shared_ptr<container_module::value_container>(
        std::shared_ptr<container_module::value_container>)>;

/**
 * @class grpc_server
 * @brief gRPC server for container system access
 *
 * This server wraps the container system and exposes it via gRPC.
 * It uses the adapter pattern to convert between native and proto formats.
 *
 * Example usage:
 * @code
 *   container_grpc::grpc_server server("0.0.0.0:50051");
 *
 *   // Set custom processor (optional)
 *   server.set_processor([](auto container) {
 *       // Process container
 *       return container;
 *   });
 *
 *   server.start();
 *   server.wait();  // Block until shutdown
 * @endcode
 */
class grpc_server {
public:
    /**
     * @brief Construct server with address string
     * @param address Server address in format "host:port"
     */
    explicit grpc_server(const std::string& address);

    /**
     * @brief Construct server with configuration
     * @param config Server configuration
     */
    explicit grpc_server(const server_config& config);

    /**
     * @brief Destructor - stops server if running
     */
    ~grpc_server();

    // Non-copyable, non-movable
    grpc_server(const grpc_server&) = delete;
    grpc_server& operator=(const grpc_server&) = delete;
    grpc_server(grpc_server&&) = delete;
    grpc_server& operator=(grpc_server&&) = delete;

    /**
     * @brief Start the gRPC server
     * @return true if server started successfully
     */
    bool start();

    /**
     * @brief Stop the server gracefully
     * @param deadline_ms Shutdown timeout in milliseconds (0 = immediate)
     */
    void stop(int deadline_ms = 5000);

    /**
     * @brief Block until server is shut down
     */
    void wait();

    /**
     * @brief Check if server is running
     * @return true if server is active
     */
    bool is_running() const noexcept;

    /**
     * @brief Get the server's listening address
     * @return Address string
     */
    std::string address() const noexcept;

    /**
     * @brief Set custom container processor
     * @param processor Function to process containers
     *
     * The processor is called for each ProcessContainer RPC.
     * If not set, containers are returned unchanged.
     */
    void set_processor(container_processor processor);

    /**
     * @brief Get number of processed requests
     * @return Total request count
     */
    size_t request_count() const noexcept;

    /**
     * @brief Get number of failed requests
     * @return Failed request count
     */
    size_t error_count() const noexcept;

private:
    class impl;
    std::unique_ptr<impl> impl_;
};

} // namespace container_grpc
