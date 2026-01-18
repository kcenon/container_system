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

/**
 * @file server_example.cpp
 * @brief Example demonstrating gRPC server usage with container system
 *
 * This example shows how to:
 * - Create and configure a gRPC server
 * - Set up a custom container processor
 * - Handle incoming requests
 * - Gracefully shutdown the server
 *
 * Usage:
 *   ./grpc_server_example [port]
 *
 * Default port is 50051 if not specified.
 */

#include "server/grpc_server.h"
#include "core/container.h"

#include <iostream>
#include <csignal>
#include <atomic>

namespace {

std::atomic<bool> g_shutdown_requested{false};

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nShutdown signal received..." << std::endl;
        g_shutdown_requested.store(true);
    }
}

/**
 * @brief Example container processor that echoes back with modifications
 *
 * This processor demonstrates how to:
 * - Read values from incoming container
 * - Create a response container
 * - Add processed values to response
 */
std::shared_ptr<container_module::value_container> echo_processor(
    std::shared_ptr<container_module::value_container> request) {

    // Create response container
    auto response = std::make_shared<container_module::value_container>();

    // Swap source and target for response
    response->set_source(request->target_id(), request->target_sub_id());
    response->set_target(request->source_id(), request->source_sub_id());
    response->set_message_type("response");

    // Echo back all values with "echo_" prefix
    auto values = request->get_variant_values();
    for (const auto& val : values) {
        container_module::optimized_value echo_val;
        echo_val.name = "echo_" + val.name;
        echo_val.type = val.type;
        echo_val.data = val.data;
        response->set(echo_val);
    }

    // Add processing metadata
    response->set("processed", true);
    response->set("original_type", request->message_type());

    return response;
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n"
              << "\nOptions:\n"
              << "  -p, --port PORT    Server port (default: 50051)\n"
              << "  -h, --help         Show this help message\n"
              << "\nExample:\n"
              << "  " << program_name << " --port 50051\n";
}

} // anonymous namespace

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string address = "0.0.0.0:50051";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if ((arg == "-p" || arg == "--port") && i + 1 < argc) {
            address = "0.0.0.0:" + std::string(argv[++i]);
        }
    }

    // Set up signal handlers for graceful shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "=== Container System gRPC Server Example ===" << std::endl;
    std::cout << "Starting server on " << address << "..." << std::endl;

    // Create server with configuration
    container_grpc::server_config config;
    config.address = address;
    config.max_receive_message_size = 64 * 1024 * 1024;  // 64MB
    config.max_send_message_size = 64 * 1024 * 1024;     // 64MB

    container_grpc::grpc_server server(config);

    // Set custom processor
    server.set_processor(echo_processor);

    // Start the server
    if (!server.start()) {
        std::cerr << "Failed to start server!" << std::endl;
        return 1;
    }

    std::cout << "Server started successfully!" << std::endl;
    std::cout << "Press Ctrl+C to shutdown..." << std::endl;
    std::cout << std::endl;

    // Main loop - periodically print stats
    while (!g_shutdown_requested.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        if (!g_shutdown_requested.load()) {
            std::cout << "Stats: requests=" << server.request_count()
                      << ", errors=" << server.error_count() << std::endl;
        }
    }

    // Graceful shutdown
    std::cout << "Shutting down server..." << std::endl;
    server.stop(5000);  // 5 second deadline

    std::cout << "Server shutdown complete." << std::endl;
    std::cout << "Final stats: requests=" << server.request_count()
              << ", errors=" << server.error_count() << std::endl;

    return 0;
}
