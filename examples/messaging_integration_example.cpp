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
 * @file messaging_integration_example.cpp
 * @brief Example demonstrating messaging integration features
 *
 * This example shows how to use the container system with
 * messaging-specific optimizations using the variant-based API.
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>

// Include the container system
#include "container.h"

using namespace container_module;

void demonstrate_basic_usage() {
    std::cout << "\n=== Basic Container Usage ===\n";

    // Create container using set_value API
    auto container = std::make_shared<value_container>();
    container->set_source("client_01", "session_123");
    container->set_target("server", "main_handler");
    container->set_message_type("user_data");

    // Add values using set_value
    container->set("user_id", static_cast<int64_t>(12345L));
    container->set("username", std::string("john_doe"));
    container->set("balance", 1500.75);
    container->set("active", true);

    std::cout << "Created container with 4 values\n";
    std::cout << "Message type: " << container->message_type() << "\n";
    std::cout << "Source: " << container->source_id() << ":" << container->source_sub_id() << "\n";
    std::cout << "Target: " << container->target_id() << ":" << container->target_sub_id() << "\n";

    // Serialize
    std::string serialized = container->serialize_string(value_container::serialization_format::binary).value();
    std::cout << "Serialized size: " << serialized.size() << " bytes\n";
}

void demonstrate_value_access() {
    std::cout << "\n=== Value Access Patterns ===\n";

    auto container = std::make_shared<value_container>();
    container->set_message_type("value_access_demo");

    // Add various values
    container->set("request_id", static_cast<int32_t>(789));
    container->set("priority", static_cast<int32_t>(1));
    container->set("payload", std::string("Important data"));
    container->set("timestamp", static_cast<int64_t>(1672531200L));
    container->set("is_urgent", true);

    // Access values using get_value and std::get_if
    if (auto val = container->get_value("request_id")) {
        if (auto* v = std::get_if<int32_t>(&val->data)) {
            std::cout << "Request ID: " << *v << "\n";
        }
    }

    if (auto val = container->get_value("payload")) {
        if (auto* v = std::get_if<std::string>(&val->data)) {
            std::cout << "Payload: " << *v << "\n";
        }
    }

    if (auto val = container->get_value("is_urgent")) {
        if (auto* v = std::get_if<bool>(&val->data)) {
            std::cout << "Is Urgent: " << (*v ? "yes" : "no") << "\n";
        }
    }

    // Iterate over all values
    std::cout << "\nAll values in container:\n";
    for (const auto& val : *container) {
        std::cout << "  - " << val.name << " (type: " << static_cast<int>(val.type) << ")\n";
    }
}

void demonstrate_serialization_roundtrip() {
    std::cout << "\n=== Serialization Round-trip ===\n";

    // Create source container
    auto source = std::make_shared<value_container>();
    source->set_source("sender", "app_1");
    source->set_target("receiver", "app_2");
    source->set_message_type("roundtrip_test");

    source->set("int_val", static_cast<int32_t>(42));
    source->set("double_val", 3.14159);
    source->set("string_val", std::string("Hello, World!"));
    source->set("bool_val", true);
    source->set("long_val", static_cast<int64_t>(9876543210L));

    // Serialize
    std::string serialized = source->serialize_string(value_container::serialization_format::binary).value();
    std::cout << "Serialized " << source->size() << " values to " << serialized.size() << " bytes\n";

    // Deserialize
    auto restored = std::make_shared<value_container>(serialized);
    std::cout << "Restored container: " << restored->message_type() << "\n";

    // Verify values
    bool all_match = true;

    if (auto val = restored->get_value("int_val")) {
        if (auto* v = std::get_if<int32_t>(&val->data)) {
            if (*v != 42) all_match = false;
        }
    }

    if (auto val = restored->get_value("string_val")) {
        if (auto* v = std::get_if<std::string>(&val->data)) {
            if (*v != "Hello, World!") all_match = false;
        }
    }

    std::cout << "Round-trip verification: " << (all_match ? "PASSED" : "FAILED") << "\n";
}

void demonstrate_compatibility() {
    std::cout << "\n=== Messaging System Compatibility ===\n";

    // Show that the container system can be used with both aliases
    std::cout << "This container system provides compatibility aliases:\n";
    std::cout << "- ContainerSystem::container (standalone usage)\n";
    std::cout << "- MessagingSystem::container (messaging system integration)\n\n";

    // Demonstrate that the same container can be used in different contexts
    auto container = std::make_shared<value_container>();
    container->set_message_type("compatibility_test");
    container->set("demo_value", static_cast<int32_t>(123));

    std::cout << "Container can be used standalone or as part of messaging system\n";
    std::cout << "Type safety and performance remain consistent across usage patterns\n";
}

void performance_comparison() {
    std::cout << "\n=== Performance Comparison ===\n";

    const int iterations = 1000;

    // Test set_value performance
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto container = std::make_shared<value_container>();
        container->set_message_type("perf_test");
        container->set("index", static_cast<int32_t>(i));
        container->set("data", std::string("test_data"));
        std::string serialized = container->serialize_string(value_container::serialization_format::binary).value();
    }
    auto standard_time = std::chrono::high_resolution_clock::now() - start;

    auto standard_ms = std::chrono::duration_cast<std::chrono::milliseconds>(standard_time).count();
    std::cout << "set_value API: " << standard_ms << " ms for " << iterations << " operations\n";

    if (standard_ms > 0) {
        double rate = (iterations * 1000.0) / standard_ms;
        std::cout << "Rate: " << std::fixed << std::setprecision(2) << rate << " containers/second\n";
    }
}

void demonstrate_memory_efficiency() {
    std::cout << "\n=== Memory Efficiency ===\n";

    auto container = std::make_shared<value_container>();
    container->set_message_type("memory_test");

    // Add various values
    container->set("small_int", static_cast<int32_t>(42));
    container->set("large_string", std::string(1000, 'x'));
    container->set("double_val", 123.456);
    container->set("bool_val", true);
    container->set("long_val", static_cast<int64_t>(9999999999L));

    // Get memory stats
    auto [heap, stack] = container->memory_stats();
    std::cout << "Container memory stats:\n";
    std::cout << "  Heap allocations: " << heap << "\n";
    std::cout << "  Stack allocations: " << stack << "\n";
    std::cout << "  Total footprint: " << container->memory_footprint() << " bytes\n";
}

int main() {
    std::cout << "Container System - Messaging Integration Example\n";
    std::cout << "================================================\n";
    std::cout << "Using variant-based API (set_value/get_value)\n";

    try {
        // Demonstrate various features
        demonstrate_basic_usage();
        demonstrate_value_access();
        demonstrate_serialization_roundtrip();
        demonstrate_compatibility();
        performance_comparison();
        demonstrate_memory_efficiency();

        std::cout << "\n=== Example Completed Successfully ===\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
