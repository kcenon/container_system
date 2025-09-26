/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Container System Project
 */

#include <iostream>
#include <string>
#include <vector>
#include "container.h"
#include "values/string_value.h"
#include "values/bool_value.h"
#include "values/bytes_value.h"
#include "values/container_value.h"

using namespace container_module;

int main() {
    std::cout << "=== Container System - Basic Usage Example ===" << std::endl;
    
    // 1. Basic container creation and value setting
    std::cout << "\n1. Basic Container Operations:" << std::endl;
    
    auto container = std::make_shared<value_container>();
    container->set_message_type("user_profile");
    
    // Set various types of values (note: all values need a name parameter)
    container->add(std::make_shared<string_value>("user_id", "12345"));
    container->add(std::make_shared<string_value>("username", "john_doe"));
    container->add(std::make_shared<string_value>("age", "30"));
    container->add(std::make_shared<bool_value>("is_active", true));
    container->add(std::make_shared<string_value>("balance", "1000.50"));
    
    std::cout << "Container message type: " << container->message_type() << std::endl;
    // Note: size() method is not available, using value count instead
    auto values = container->value_array("");
    std::cout << "Container has " << values.size() << " values" << std::endl;
    
    // 2. Reading values from container
    std::cout << "\n2. Reading Values:" << std::endl;
    
    auto user_id = container->get_value("user_id");
    if (user_id) {
        std::cout << "User ID: " << user_id->to_string() << std::endl;
    }
    
    auto username = container->get_value("username");
    if (username) {
        std::cout << "Username: " << username->to_string() << std::endl;
    }
    
    auto is_active = container->get_value("is_active");
    if (is_active) {
        std::cout << "Is Active: " << (is_active->to_boolean() ? "Yes" : "No") << std::endl;
    }
    
    // 3. Nested containers
    std::cout << "\n3. Nested Containers:" << std::endl;
    
    auto address_container = std::make_shared<value_container>();
    address_container->set_message_type("address");
    address_container->add(std::make_shared<string_value>("street", "123 Main St"));
    address_container->add(std::make_shared<string_value>("city", "New York"));
    address_container->add(std::make_shared<string_value>("zip", "10001"));

    // Create a container_value that holds the address container
    auto address_value = std::make_shared<container_value>("address");
    address_value->add(address_container->value_array(""));
    container->add(address_value);
    
    auto address = container->get_value("address");
    if (address && address->type() == value_types::container_value) {
        // Get child values from the container_value
        auto addr_values = address->children();
        std::shared_ptr<value> street = nullptr;
        std::shared_ptr<value> city = nullptr;
        for (auto& val : addr_values) {
            if (val->name() == "street") street = val;
            if (val->name() == "city") city = val;
        }
        
        if (street && city) {
            std::cout << "Address: " << street->to_string() << ", " << city->to_string() << std::endl;
        }
    }
    
    // 4. Binary data handling
    std::cout << "\n4. Binary Data:" << std::endl;
    
    std::vector<uint8_t> binary_data = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
    container->add(std::make_shared<bytes_value>("avatar", binary_data));
    
    auto avatar = container->get_value("avatar");
    if (avatar && avatar->type() == value_types::bytes_value) {
        auto data = avatar->to_bytes();
        std::cout << "Avatar data size: " << data.size() << " bytes" << std::endl;
        std::cout << "Avatar data (as text): ";
        for (auto byte : data) {
            std::cout << static_cast<char>(byte);
        }
        std::cout << std::endl;
    }
    
    // 5. Container serialization
    std::cout << "\n5. Serialization:" << std::endl;
    
    std::string serialized = container->serialize();
    std::cout << "Serialized container size: " << serialized.length() << " characters" << std::endl;
    std::cout << "Serialized data preview: " << serialized.substr(0, 100) << "..." << std::endl;
    
    // 6. Container deserialization
    std::cout << "\n6. Deserialization:" << std::endl;
    
    auto restored_container = std::make_shared<value_container>(serialized);
    std::cout << "Restored container message type: " << restored_container->message_type() << std::endl;
    auto restored_values = restored_container->value_array("");
    std::cout << "Restored container has " << restored_values.size() << " values" << std::endl;
    
    auto restored_username = restored_container->get_value("username");
    if (restored_username) {
        std::cout << "Restored username: " << restored_username->to_string() << std::endl;
    }
    
    // 7. Container iteration
    std::cout << "\n7. Container Iteration:" << std::endl;
    
    std::cout << "All values in container:" << std::endl;
    auto all_values = container->value_array("");
    for (const auto& val : all_values) {
        std::cout << "  " << val->name() << ": " << val->to_string()
                  << " (type: " << static_cast<int>(val->type()) << ")" << std::endl;
    }
    
    std::cout << "\n=== Example completed successfully ===" << std::endl;
    return 0;
}