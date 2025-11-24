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

#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <iomanip>

#include "container.h"

using namespace container_module;

/**
 * @brief Basic Container System Example
 *
 * This example demonstrates fundamental usage of the container system:
 * - Creating containers
 * - Adding different types of values using set_value() API
 * - Serialization and deserialization
 * - Basic error handling
 */

void demonstrate_basic_usage() {
    std::cout << "=== Basic Container Usage ===" << std::endl;

    // Create a new container
    auto container = std::make_shared<value_container>();

    // Set container metadata
    container->set_source("example_client", "session_001");
    container->set_target("example_server", "main_handler");
    container->set_message_type("user_data");

    std::cout << "Container created with:" << std::endl;
    std::cout << "  Source: " << container->source_id() << "/" << container->source_sub_id() << std::endl;
    std::cout << "  Target: " << container->target_id() << "/" << container->target_sub_id() << std::endl;
    std::cout << "  Type: " << container->message_type() << std::endl;
}

void demonstrate_value_types() {
    std::cout << "\n=== Value Types Demonstration ===" << std::endl;

    auto container = std::make_shared<value_container>();
    container->set_message_type("value_types_demo");

    // String value using new set_value API
    container->set_value("username", std::string("john_doe"));
    std::cout << "Added string value: username = john_doe" << std::endl;

    // Integer value
    container->set_value("user_id", static_cast<int32_t>(12345));
    std::cout << "Added int value: user_id = 12345" << std::endl;

    // Long value (timestamp)
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    container->set_value("timestamp", static_cast<int64_t>(timestamp));
    std::cout << "Added long value: timestamp = " << timestamp << std::endl;

    // Float value
    container->set_value("score", 98.5f);
    std::cout << "Added float value: score = 98.5" << std::endl;

    // Double value
    container->set_value("account_balance", 1500.75);
    std::cout << "Added double value: account_balance = 1500.75" << std::endl;

    // Boolean value
    container->set_value("is_active", true);
    std::cout << "Added bool value: is_active = true" << std::endl;

    std::cout << "Total values added: 6" << std::endl;
}

void demonstrate_serialization() {
    std::cout << "\n=== Serialization Demonstration ===" << std::endl;

    // Create container with various data
    auto container = std::make_shared<value_container>();
    container->set_source("serialize_test", "test_session");
    container->set_target("deserialize_test", "test_handler");
    container->set_message_type("serialization_test");

    container->set_value("message", std::string("Hello, Serialization!"));
    container->set_value("count", static_cast<int32_t>(42));
    container->set_value("pi", 3.14159);
    container->set_value("success", true);

    // Serialize
    std::cout << "Serializing container..." << std::endl;
    std::string serialized_data = container->serialize();
    std::cout << "Serialized size: " << serialized_data.size() << " bytes" << std::endl;

    // Deserialize
    std::cout << "Deserializing container..." << std::endl;
    auto new_container = std::make_shared<value_container>(serialized_data);

    std::cout << "Deserialization successful!" << std::endl;
    std::cout << "Deserialized container:" << std::endl;
    std::cout << "  Source: " << new_container->source_id() << "/" << new_container->source_sub_id() << std::endl;
    std::cout << "  Target: " << new_container->target_id() << "/" << new_container->target_sub_id() << std::endl;
    std::cout << "  Type: " << new_container->message_type() << std::endl;

    // Verify specific values using new get_value API
    if (auto message_value = new_container->get_value("message")) {
        if (auto* str = std::get_if<std::string>(&message_value->data)) {
            std::cout << "  Message: " << *str << std::endl;
        }
    }

    if (auto count_value = new_container->get_value("count")) {
        if (auto* val = std::get_if<int32_t>(&count_value->data)) {
            std::cout << "  Count: " << *val << std::endl;
        }
    }
}

void demonstrate_value_access() {
    std::cout << "\n=== Value Access Demonstration ===" << std::endl;

    auto container = std::make_shared<value_container>();
    container->set_message_type("value_access_test");

    // Add sample data using set_value
    container->set_value("product_name", std::string("Super Widget"));
    container->set_value("price", 29.99);
    container->set_value("quantity", static_cast<int32_t>(100));
    container->set_value("in_stock", true);

    std::cout << "Container contains 4 values" << std::endl;

    // Access values by key using new API
    std::cout << "\nAccessing values by key:" << std::endl;

    if (auto product_name = container->get_value("product_name")) {
        if (auto* str = std::get_if<std::string>(&product_name->data)) {
            std::cout << "  Product: " << *str << std::endl;
        }
    }

    if (auto price = container->get_value("price")) {
        if (auto* val = std::get_if<double>(&price->data)) {
            std::cout << "  Price: $" << *val << std::endl;
        }
    }

    if (auto quantity = container->get_value("quantity")) {
        if (auto* val = std::get_if<int32_t>(&quantity->data)) {
            std::cout << "  Quantity: " << *val << std::endl;
        }
    }

    if (auto in_stock = container->get_value("in_stock")) {
        if (auto* val = std::get_if<bool>(&in_stock->data)) {
            std::cout << "  In Stock: " << (*val ? "yes" : "no") << std::endl;
        }
    }
}

void demonstrate_iteration() {
    std::cout << "\n=== Container Iteration ===" << std::endl;

    auto container = std::make_shared<value_container>();
    container->set_message_type("iteration_test");

    // Add multiple items
    container->set_value("item_1", std::string("first"));
    container->set_value("item_2", std::string("second"));
    container->set_value("item_3", std::string("third"));

    std::cout << "Added 3 values with different names" << std::endl;

    // Iterate over all values
    std::cout << "Iterating over container values:" << std::endl;
    for (const auto& val : *container) {
        std::cout << "  - " << val.name << " (type: " << static_cast<int>(val.type) << ")" << std::endl;
    }
}

void demonstrate_performance_basics() {
    std::cout << "\n=== Basic Performance Demonstration ===" << std::endl;

    const int num_operations = 1000;

    // Container creation performance
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::shared_ptr<value_container>> containers;
    containers.reserve(num_operations);

    for (int i = 0; i < num_operations; ++i) {
        auto container = std::make_shared<value_container>();
        container->set_source("perf_client", "session_" + std::to_string(i));
        container->set_target("perf_server", "handler");
        container->set_message_type("performance_test");

        container->set_value("index", static_cast<int32_t>(i));
        container->set_value("data", std::string("test_data_" + std::to_string(i)));

        containers.push_back(container);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    double containers_per_second = (num_operations * 1000000.0) / duration.count();

    std::cout << "Performance results:" << std::endl;
    std::cout << "  Created " << num_operations << " containers in "
              << duration.count() << " microseconds" << std::endl;
    std::cout << "  Rate: " << std::fixed << std::setprecision(2)
              << containers_per_second << " containers/second" << std::endl;

    // Serialization performance
    start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::string> serialized_data;
    serialized_data.reserve(num_operations);

    for (const auto& container : containers) {
        serialized_data.push_back(container->serialize());
    }

    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    double serializations_per_second = (num_operations * 1000000.0) / duration.count();

    std::cout << "  Serialized " << num_operations << " containers in "
              << duration.count() << " microseconds" << std::endl;
    std::cout << "  Rate: " << std::fixed << std::setprecision(2)
              << serializations_per_second << " serializations/second" << std::endl;

    // Calculate total data size
    size_t total_size = 0;
    for (const auto& data : serialized_data) {
        total_size += data.size();
    }

    std::cout << "  Total serialized data: " << total_size << " bytes" << std::endl;
    std::cout << "  Average per container: " << (total_size / num_operations) << " bytes" << std::endl;
}

int main() {
    try {
        std::cout << "Container System Basic Example" << std::endl;
        std::cout << "==============================" << std::endl;

        demonstrate_basic_usage();
        demonstrate_value_types();
        demonstrate_serialization();
        demonstrate_value_access();
        demonstrate_iteration();
        demonstrate_performance_basics();

        std::cout << "\n=== Basic Example Completed Successfully ===" << std::endl;
        std::cout << "This example demonstrated:" << std::endl;
        std::cout << "  - Basic container creation and configuration" << std::endl;
        std::cout << "  - All supported value types using set_value() API" << std::endl;
        std::cout << "  - Serialization and deserialization" << std::endl;
        std::cout << "  - Value access patterns using get_value() API" << std::endl;
        std::cout << "  - Container iteration" << std::endl;
        std::cout << "  - Basic performance characteristics" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error in basic example: " << e.what() << std::endl;
        return 1;
    }
}
