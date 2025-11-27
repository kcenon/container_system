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
 * @file client_example.cpp
 * @brief Example demonstrating gRPC client usage with container system
 *
 * This example shows how to:
 * - Create and configure a gRPC client
 * - Build containers with various value types
 * - Send requests and handle responses
 * - Use streaming capabilities
 *
 * Usage:
 *   ./grpc_client_example [host:port]
 *
 * Default target is localhost:50051 if not specified.
 */

#include "client/grpc_client.h"
#include "core/container.h"

#include <iostream>
#include <iomanip>
#include <string>

namespace {

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n"
              << "\nOptions:\n"
              << "  -t, --target HOST:PORT  Server address (default: localhost:50051)\n"
              << "  -h, --help              Show this help message\n"
              << "\nExample:\n"
              << "  " << program_name << " --target localhost:50051\n";
}

void print_container(const std::shared_ptr<container_module::value_container>& container,
                     const std::string& label) {
    std::cout << "\n--- " << label << " ---" << std::endl;
    std::cout << "  Source: " << container->source_id();
    if (!container->source_sub_id().empty()) {
        std::cout << "/" << container->source_sub_id();
    }
    std::cout << std::endl;

    std::cout << "  Target: " << container->target_id();
    if (!container->target_sub_id().empty()) {
        std::cout << "/" << container->target_sub_id();
    }
    std::cout << std::endl;

    std::cout << "  Type: " << container->message_type() << std::endl;
    std::cout << "  Values:" << std::endl;

    auto values = container->get_variant_values();
    for (const auto& val : values) {
        std::cout << "    - " << val.name << ": ";
        std::cout << container_module::variant_helpers::to_string(val.data, val.type);
        std::cout << std::endl;
    }
}

bool demo_simple_request(container_grpc::grpc_client& client) {
    std::cout << "\n=== Demo 1: Simple Request ===" << std::endl;

    // Create a request container
    auto request = std::make_shared<container_module::value_container>();
    request->set_source("client_example", "demo1");
    request->set_target("server", "processor");
    request->set_message_type("simple_request");

    // Add various value types
    request->add_value("greeting", std::string("Hello, gRPC!"));
    request->add_value("count", 42);
    request->add_value("temperature", 36.5);
    request->add_value("enabled", true);

    print_container(request, "Request");

    // Send request and get response
    auto result = client.process(request);
    if (!result) {
        std::cerr << "Request failed: " << result.error_message << std::endl;
        return false;
    }

    print_container(result.value, "Response");
    return true;
}

bool demo_batch_request(container_grpc::grpc_client& client) {
    std::cout << "\n=== Demo 2: Batch Request ===" << std::endl;

    // Create multiple containers for batch processing
    std::vector<std::shared_ptr<container_module::value_container>> batch;

    for (int i = 0; i < 3; ++i) {
        auto container = std::make_shared<container_module::value_container>();
        container->set_source("client_example", "demo2");
        container->set_target("server", "batch_processor");
        container->set_message_type("batch_item");
        container->add_value("item_id", i);
        container->add_value("data", std::string("Item " + std::to_string(i)));
        batch.push_back(container);
    }

    std::cout << "Sending batch of " << batch.size() << " containers..." << std::endl;

    auto result = client.send_batch(batch);
    if (!result) {
        std::cerr << "Batch request failed: " << result.error_message << std::endl;
        return false;
    }

    std::cout << "Received " << result.value.size() << " responses" << std::endl;
    for (size_t i = 0; i < result.value.size(); ++i) {
        print_container(result.value[i], "Batch Response " + std::to_string(i));
    }

    return true;
}

bool demo_health_check(container_grpc::grpc_client& client) {
    std::cout << "\n=== Demo 3: Health Check ===" << std::endl;

    if (client.ping()) {
        std::cout << "Server is healthy!" << std::endl;
    } else {
        std::cout << "Server health check failed!" << std::endl;
        return false;
    }

    auto status = client.get_status();
    if (status) {
        std::cout << "Server status:" << std::endl;
        std::cout << "  Total requests: " << status->first << std::endl;
        std::cout << "  Active connections: " << status->second << std::endl;
    }

    return true;
}

bool demo_streaming(container_grpc::grpc_client& client) {
    std::cout << "\n=== Demo 4: Streaming ===" << std::endl;

    // Create subscription request
    auto request = std::make_shared<container_module::value_container>();
    request->set_source("client_example", "demo4");
    request->set_target("server", "streamer");
    request->set_message_type("subscribe");
    request->add_value("topic", std::string("updates"));
    request->add_value("max_items", 5);

    int received_count = 0;

    // Start streaming with callback
    bool started = client.stream(request,
        [&received_count](std::shared_ptr<container_module::value_container> container) {
            ++received_count;
            std::cout << "  Received stream item #" << received_count << std::endl;
            auto values = container->get_variant_values();
            for (const auto& val : values) {
                std::cout << "    " << val.name << ": "
                          << container_module::variant_helpers::to_string(val.data, val.type)
                          << std::endl;
            }
        });

    if (!started) {
        std::cout << "Streaming not available or failed to start." << std::endl;
        std::cout << "(This is expected if server doesn't support streaming)" << std::endl;
        return true;  // Not a failure - streaming may not be supported
    }

    std::cout << "Streaming completed. Received " << received_count << " items." << std::endl;
    return true;
}

} // anonymous namespace

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string target = "localhost:50051";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if ((arg == "-t" || arg == "--target") && i + 1 < argc) {
            target = argv[++i];
        }
    }

    std::cout << "=== Container System gRPC Client Example ===" << std::endl;
    std::cout << "Connecting to " << target << "..." << std::endl;

    // Create client with configuration
    container_grpc::client_config config;
    config.target_address = target;
    config.timeout = std::chrono::milliseconds(30000);  // 30 second timeout
    config.max_retries = 3;
    config.client_id = "example_client";

    container_grpc::grpc_client client(config);

    // Check connection
    if (!client.is_connected()) {
        std::cout << "Note: Connection will be established on first request." << std::endl;
    }

    std::cout << "\nRunning demos..." << std::endl;

    // Run demos
    bool all_passed = true;

    if (!demo_simple_request(client)) {
        std::cerr << "Simple request demo failed!" << std::endl;
        all_passed = false;
    }

    if (!demo_batch_request(client)) {
        std::cerr << "Batch request demo failed!" << std::endl;
        all_passed = false;
    }

    if (!demo_health_check(client)) {
        std::cerr << "Health check demo failed!" << std::endl;
        all_passed = false;
    }

    if (!demo_streaming(client)) {
        std::cerr << "Streaming demo failed!" << std::endl;
        all_passed = false;
    }

    std::cout << "\n=== Summary ===" << std::endl;
    if (all_passed) {
        std::cout << "All demos completed successfully!" << std::endl;
        return 0;
    } else {
        std::cout << "Some demos failed. Check output above for details." << std::endl;
        return 1;
    }
}
