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
 * - Adding different types of values
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

    // String value
    std::string username_key = "username";
    std::string username_val = "john_doe";
    auto string_val = std::make_shared<string_value>(username_key, username_val);
    container->add(string_val);
    std::cout << "Added string value: username = " << string_val->to_string() << std::endl;

    // Integer value
    std::string user_id_key = "user_id";
    auto int_val = std::make_shared<int_value>(user_id_key, 12345);
    container->add(int_val);
    std::cout << "Added int value: user_id = " << int_val->to_int() << std::endl;

    // Long value
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::string timestamp_key = "timestamp";
    auto long_val = std::make_shared<long_value>(timestamp_key, timestamp);
    container->add(long_val);
    std::cout << "Added long value: timestamp = " << long_val->to_long() << std::endl;

    // Float value
    std::string score_key = "score";
    auto float_val = std::make_shared<float_value>(score_key, 98.5f);
    container->add(float_val);
    std::cout << "Added float value: score = " << float_val->to_float() << std::endl;

    // Double value
    std::string balance_key = "account_balance";
    auto double_val = std::make_shared<double_value>(balance_key, 1500.75);
    container->add(double_val);
    std::cout << "Added double value: account_balance = " << double_val->to_double() << std::endl;

    // Boolean value
    std::string active_key = "is_active";
    auto bool_val = std::make_shared<bool_value>(active_key, true);
    container->add(bool_val);
    std::cout << "Added bool value: is_active = " << (bool_val->to_boolean() ? "true" : "false") << std::endl;

    std::cout << "Total values added: 6" << std::endl;
}

void demonstrate_serialization() {
    std::cout << "\n=== Serialization Demonstration ===" << std::endl;

    // Create container with various data
    auto container = std::make_shared<value_container>();
    container->set_source("serialize_test", "test_session");
    container->set_target("deserialize_test", "test_handler");
    container->set_message_type("serialization_test");

    std::string msg_key = "message";
    std::string msg_val = "Hello, Serialization!";
    container->add(std::make_shared<string_value>(msg_key, msg_val));
    std::string count_key = "count";
    container->add(std::make_shared<int_value>(count_key, 42));
    std::string pi_key = "pi";
    container->add(std::make_shared<double_value>(pi_key, 3.14159));
    std::string success_key = "success";
    container->add(std::make_shared<bool_value>(success_key, true));

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

    // Verify specific values
    auto message_value = new_container->get_value("message");
    auto count_value = new_container->get_value("count");

    if (message_value) {
        std::cout << "  Message: " << message_value->to_string() << std::endl;
    }

    if (count_value) {
        std::cout << "  Count: " << count_value->to_int() << std::endl;
    }
}

void demonstrate_value_access() {
    std::cout << "\n=== Value Access Demonstration ===" << std::endl;

    auto container = std::make_shared<value_container>();
    container->set_message_type("value_access_test");

    // Add sample data
    std::string product_key = "product_name";
    std::string product_val = "Super Widget";
    container->add(std::make_shared<string_value>(product_key, product_val));
    std::string price_key = "price";
    container->add(std::make_shared<double_value>(price_key, 29.99));
    std::string qty_key = "quantity";
    container->add(std::make_shared<int_value>(qty_key, 100));
    std::string stock_key = "in_stock";
    container->add(std::make_shared<bool_value>(stock_key, true));

    std::cout << "Container contains 4 values" << std::endl;

    // Access values by key
    std::cout << "\nAccessing values by key:" << std::endl;

    auto product_name = container->get_value("product_name");
    if (product_name) {
        std::cout << "  Product: " << product_name->to_string() << std::endl;
    }

    auto price = container->get_value("price");
    if (price) {
        std::cout << "  Price: $" << price->to_double() << std::endl;
    }

    auto quantity = container->get_value("quantity");
    if (quantity) {
        std::cout << "  Quantity: " << quantity->to_int() << std::endl;
    }

    auto in_stock = container->get_value("in_stock");
    if (in_stock) {
        std::cout << "  In Stock: " << (in_stock->to_boolean() ? "yes" : "no") << std::endl;
    }
}

void demonstrate_multiple_values() {
    std::cout << "\n=== Multiple Values with Same Name ===" << std::endl;

    auto container = std::make_shared<value_container>();
    container->set_message_type("multi_value_test");

    // Add multiple values with same name
    std::string item_key = "item";
    std::string first_val = "first";
    std::string second_val = "second";
    std::string third_val = "third";
    container->add(std::make_shared<string_value>(item_key, first_val));
    container->add(std::make_shared<string_value>(item_key, second_val));
    container->add(std::make_shared<string_value>(item_key, third_val));

    std::cout << "Added 3 values with name 'item'" << std::endl;

    // Get all values with that name
    auto items = container->value_array("item");
    std::cout << "Found " << items.size() << " items:" << std::endl;

    for (size_t i = 0; i < items.size(); ++i) {
        std::cout << "  [" << i << "]: " << items[i]->to_string() << std::endl;
    }

    // Access by index
    auto first_item = container->get_value("item", 0);
    auto second_item = container->get_value("item", 1);
    auto third_item = container->get_value("item", 2);

    if (first_item) std::cout << "  First item: " << first_item->to_string() << std::endl;
    if (second_item) std::cout << "  Second item: " << second_item->to_string() << std::endl;
    if (third_item) std::cout << "  Third item: " << third_item->to_string() << std::endl;
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

        std::string index_key = "index";
        container->add(std::make_shared<int_value>(index_key, i));
        std::string data_key = "data";
        std::string data_val = "test_data_" + std::to_string(i);
        container->add(std::make_shared<string_value>(data_key, data_val));

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
        demonstrate_multiple_values();
        demonstrate_performance_basics();

        std::cout << "\n=== Basic Example Completed Successfully ===" << std::endl;
        std::cout << "This example demonstrated:" << std::endl;
        std::cout << "• Basic container creation and configuration" << std::endl;
        std::cout << "• All supported value types" << std::endl;
        std::cout << "• Serialization and deserialization" << std::endl;
        std::cout << "• Value access patterns" << std::endl;
        std::cout << "• Multiple values with same name" << std::endl;
        std::cout << "• Basic performance characteristics" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error in basic example: " << e.what() << std::endl;
        return 1;
    }
}
