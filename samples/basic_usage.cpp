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

    // Set various types of values using new set_value API
    container->set_value("user_id", std::string("12345"));
    container->set_value("username", std::string("john_doe"));
    container->set_value("age", static_cast<int32_t>(30));
    container->set_value("is_active", true);
    container->set_value("balance", 1000.50);

    std::cout << "Container message type: " << container->message_type() << std::endl;

    // 2. Reading values from container
    std::cout << "\n2. Reading Values:" << std::endl;

    if (auto user_id = container->get_value("user_id")) {
        if (auto* str = std::get_if<std::string>(&user_id->data)) {
            std::cout << "User ID: " << *str << std::endl;
        }
    }

    if (auto username = container->get_value("username")) {
        if (auto* str = std::get_if<std::string>(&username->data)) {
            std::cout << "Username: " << *str << std::endl;
        }
    }

    if (auto age = container->get_value("age")) {
        if (auto* val = std::get_if<int32_t>(&age->data)) {
            std::cout << "Age: " << *val << std::endl;
        }
    }

    if (auto is_active = container->get_value("is_active")) {
        if (auto* val = std::get_if<bool>(&is_active->data)) {
            std::cout << "Is Active: " << (*val ? "Yes" : "No") << std::endl;
        }
    }

    if (auto balance = container->get_value("balance")) {
        if (auto* val = std::get_if<double>(&balance->data)) {
            std::cout << "Balance: $" << *val << std::endl;
        }
    }

    // 3. Using iterators for all values
    std::cout << "\n3. Iterating Over All Values:" << std::endl;

    for (const auto& val : *container) {
        std::cout << "  - " << val.name << " (type: " << static_cast<int>(val.type) << ")" << std::endl;
    }

    // 4. Container serialization
    std::cout << "\n4. Serialization:" << std::endl;

    std::string serialized = container->serialize();
    std::cout << "Serialized container size: " << serialized.length() << " bytes" << std::endl;

    // 5. Container deserialization
    std::cout << "\n5. Deserialization:" << std::endl;

    auto restored_container = std::make_shared<value_container>(serialized);
    std::cout << "Restored container message type: " << restored_container->message_type() << std::endl;

    if (auto restored_username = restored_container->get_value("username")) {
        if (auto* str = std::get_if<std::string>(&restored_username->data)) {
            std::cout << "Restored username: " << *str << std::endl;
        }
    }

    if (auto restored_balance = restored_container->get_value("balance")) {
        if (auto* val = std::get_if<double>(&restored_balance->data)) {
            std::cout << "Restored balance: $" << *val << std::endl;
        }
    }

    // 6. Container metadata
    std::cout << "\n6. Container Metadata:" << std::endl;

    container->set_source("client_app", "user_session_123");
    container->set_target("server_api", "profile_handler");

    std::cout << "Source: " << container->source_id() << "/" << container->source_sub_id() << std::endl;
    std::cout << "Target: " << container->target_id() << "/" << container->target_sub_id() << std::endl;
    std::cout << "Message type: " << container->message_type() << std::endl;

    // 7. Memory efficiency demonstration
    std::cout << "\n7. Memory Efficiency:" << std::endl;

    auto [heap, stack] = container->memory_stats();
    std::cout << "Heap allocations: " << heap << std::endl;
    std::cout << "Stack allocations: " << stack << std::endl;
    std::cout << "Total memory footprint: " << container->memory_footprint() << " bytes" << std::endl;

    std::cout << "\n=== Example completed successfully ===" << std::endl;
    return 0;
}
