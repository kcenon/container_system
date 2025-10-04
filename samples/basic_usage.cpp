/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Container System Project
 */

#include <iostream>
#include <string>
#include <vector>
#include "container.h"

using namespace container_module;

int main() {
    std::cout << "=== Container System - Basic Usage Example ===" << std::endl;

    // 1. Basic container creation and value setting
    std::cout << "\n1. Basic Container Operations:" << std::endl;

    auto container = std::make_shared<value_container>();
    container->set_message_type("user_profile");

    // Set various types of values
    container->add(std::make_shared<string_value>("user_id", "12345"));
    container->add(std::make_shared<string_value>("username", "john_doe"));
    container->add(std::make_shared<int_value>("age", 30));
    container->add(std::make_shared<bool_value>("is_active", true));
    container->add(std::make_shared<double_value>("balance", 1000.50));

    std::cout << "Container message type: " << container->message_type() << std::endl;

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

    auto age = container->get_value("age");
    if (age) {
        std::cout << "Age: " << age->to_int() << std::endl;
    }

    auto is_active = container->get_value("is_active");
    if (is_active) {
        std::cout << "Is Active: " << (is_active->to_boolean() ? "Yes" : "No") << std::endl;
    }

    auto balance = container->get_value("balance");
    if (balance) {
        std::cout << "Balance: $" << balance->to_double() << std::endl;
    }

    // 3. Multiple values with same name
    std::cout << "\n3. Multiple Values with Same Name:" << std::endl;

    container->add(std::make_shared<string_value>("tag", "cpp"));
    container->add(std::make_shared<string_value>("tag", "programming"));
    container->add(std::make_shared<string_value>("tag", "example"));

    auto tags = container->value_array("tag");
    std::cout << "User has " << tags.size() << " tags:" << std::endl;
    for (const auto& tag : tags) {
        std::cout << "  - " << tag->to_string() << std::endl;
    }
    
    // 4. Container serialization
    std::cout << "\n4. Serialization:" << std::endl;

    std::string serialized = container->serialize();
    std::cout << "Serialized container size: " << serialized.length() << " bytes" << std::endl;

    // 5. Container deserialization
    std::cout << "\n5. Deserialization:" << std::endl;

    auto restored_container = std::make_shared<value_container>(serialized);
    std::cout << "Restored container message type: " << restored_container->message_type() << std::endl;

    auto restored_username = restored_container->get_value("username");
    if (restored_username) {
        std::cout << "Restored username: " << restored_username->to_string() << std::endl;
    }

    auto restored_balance = restored_container->get_value("balance");
    if (restored_balance) {
        std::cout << "Restored balance: $" << restored_balance->to_double() << std::endl;
    }

    // 6. Container metadata
    std::cout << "\n6. Container Metadata:" << std::endl;

    container->set_source("client_app", "user_session_123");
    container->set_target("server_api", "profile_handler");

    std::cout << "Source: " << container->source_id() << "/" << container->source_sub_id() << std::endl;
    std::cout << "Target: " << container->target_id() << "/" << container->target_sub_id() << std::endl;
    std::cout << "Message type: " << container->message_type() << std::endl;

    std::cout << "\n=== Example completed successfully ===" << std::endl;
    return 0;
}