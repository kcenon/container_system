# Integration Guide - Container System

> **Language:** **English** | [한국어](INTEGRATION_KO.md)

## Overview

This guide describes how to integrate container_system with other modules in the ecosystem. Container System provides type-safe serialization and data storage that seamlessly integrates with messaging, network, and database systems.

**Version:** 0.1.0.0
**Last Updated:** 2025-10-22

---

## Table of Contents

- [Quick Start](#quick-start)
- [Integration with messaging_system](#integration-with-messaging_system)
- [Integration with network_system](#integration-with-network_system)
- [Integration with database_system](#integration-with-database_system)
- [Integration with common_system](#integration-with-common_system)
- [Build Configuration](#build-configuration)
- [Performance Considerations](#performance-considerations)
- [Troubleshooting](#troubleshooting)

---

## Quick Start

### Basic Integration

```cpp
#include <container/container.h>

// Create a container
auto container = container_system::create_container();

// Add values
container->set_value("id", 12345);
container->set_value("name", "John Doe");
container->set_value("score", 98.5);

// Serialize for transmission
auto binary_data = container->serialize_to_binary();
```

### CMake Integration

```cmake
find_package(container_system CONFIG REQUIRED)

target_link_libraries(your_target PRIVATE
    kcenon::container_system
)
```

---

## Integration with messaging_system

Container System was originally part of messaging_system and provides seamless integration.

### Message Container Mapping

```cpp
#include <container/container.h>
#include <messaging/message.h>

// Convert container to message
auto container = container_system::create_container();
container->set_value("type", "request");
container->set_value("payload", data);

auto message = messaging_system::create_message_from_container(container);

// Convert message to container
auto received_message = receive_message();
auto received_container = received_message->to_container();
```

### Build Configuration

```cmake
# Enable messaging_system integration
set(BUILD_WITH_MESSAGING_SYSTEM ON)

find_package(container_system CONFIG REQUIRED)
find_package(messaging_system CONFIG REQUIRED)

target_link_libraries(your_target PRIVATE
    kcenon::container_system
    kcenon::messaging_system
)
```

### Example: Message Serialization

```cpp
#include <container/container.h>
#include <messaging/message_builder.h>

// Build message with container
auto message = messaging_system::message_builder()
    .set_type("command")
    .set_container(container)
    .build();

// Serialize entire message
auto bytes = message->serialize();

// Deserialize
auto restored_message = messaging_system::deserialize_message(bytes);
auto restored_container = restored_message->get_container();
```

---

## Integration with network_system

Container System provides efficient serialization for network transmission.

### Network Transport Integration

```cpp
#include <container/container.h>
#include <network_system/messaging_client.h>

// Prepare container for network send
auto container = container_system::create_container();
container->set_value("command", "ping");

// Serialize for network
auto payload = container->serialize_to_binary();

// Send via network
auto client = network_system::messaging_client("localhost", 8080);
client.send(payload);

// Receive and deserialize
auto received_data = client.receive();
auto received_container = container_system::deserialize_from_binary(received_data);
```

### Zero-Copy Optimization

```cpp
#include <container/container.h>
#include <network_system/pipeline.h>

// Use zero-copy pipeline
auto container = container_system::create_container();
container->set_value("large_data", large_blob);

// Zero-copy serialization
auto pipeline = network_system::create_pipeline();
pipeline->append_container(container); // No copy!

// Send via pipeline
client.send_pipeline(pipeline);
```

### Build Configuration

```cmake
# Enable network_system integration
set(BUILD_WITH_NETWORK_SYSTEM ON)

find_package(container_system CONFIG REQUIRED)
find_package(network_system CONFIG REQUIRED)

target_link_libraries(your_target PRIVATE
    kcenon::container_system
    kcenon::network_system
)
```

---

## Integration with database_system

Store and retrieve containers from database systems.

### Data Persistence

```cpp
#include <container/container.h>
#include <database_system/database_manager.h>

// Store container in database
auto container = container_system::create_container();
container->set_value("user_id", 42);
container->set_value("username", "john_doe");

auto db = database_system::create_manager();
db->execute_query(
    "INSERT INTO users (data) VALUES (?)",
    container->serialize_to_binary()
);

// Retrieve container from database
auto result = db->execute_query("SELECT data FROM users WHERE user_id = 42");
auto blob = result->get_blob(0);
auto loaded_container = container_system::deserialize_from_binary(blob);
```

### JSON Storage (PostgreSQL JSONB)

```cpp
#include <container/container.h>
#include <database_system/postgres_manager.h>

// Store as JSON for queryable fields
auto container = container_system::create_container();
container->set_value("name", "Product A");
container->set_value("price", 29.99);

auto json = container->serialize_to_json();

auto db = database_system::create_postgres_manager();
db->execute_query(
    "INSERT INTO products (data) VALUES (?::jsonb)",
    json
);

// Query JSON fields directly
auto results = db->execute_query(
    "SELECT data FROM products WHERE data->>'price' < '50'"
);
```

### Build Configuration

```cmake
# Enable database_system integration
set(BUILD_WITH_DATABASE_SYSTEM ON)

find_package(container_system CONFIG REQUIRED)
find_package(database_system CONFIG REQUIRED)

target_link_libraries(your_target PRIVATE
    kcenon::container_system
    kcenon::database_system
)
```

---

## Integration with common_system

Use Result<T> pattern for error handling.

### Result<T> Integration

```cpp
#include <container/container.h>
#include <kcenon/common/patterns/result.h>

common::Result<container_ptr> load_container(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        return common::error<container_ptr>(
            common::error_codes::NOT_FOUND,
            "Container file not found",
            "container_loader"
        );
    }

    auto data = read_file(path);
    auto container = container_system::deserialize_from_binary(data);

    return common::ok(container);
}

// Usage with monadic operations
auto result = load_container("config.bin")
    .and_then(validate_container)
    .map(transform_container);

if (result.is_ok()) {
    auto container = result.value();
    // Use container
} else {
    auto error = result.error();
    log_error(error.message);
}
```

### Build Configuration

```cmake
# Enable common_system integration (now mandatory)
# KCENON_HAS_COMMON_SYSTEM is automatically defined when common_system is found

find_package(container_system CONFIG REQUIRED)
find_package(common_system CONFIG REQUIRED)

target_link_libraries(your_target PRIVATE
    kcenon::container_system
    kcenon::common_system
)
```

---

## Build Configuration

### CMake Options

```cmake
# Optional integration flags
option(BUILD_WITH_COMMON_SYSTEM "Enable common_system integration" ON)
option(BUILD_WITH_MESSAGING_SYSTEM "Enable messaging_system integration" OFF)
option(BUILD_WITH_NETWORK_SYSTEM "Enable network_system integration" OFF)
option(BUILD_WITH_DATABASE_SYSTEM "Enable database_system integration" OFF)

# Feature flags
option(ENABLE_SIMD "Enable SIMD optimizations" ON)
option(ENABLE_JSON "Enable JSON serialization" ON)
option(ENABLE_XML "Enable XML serialization" ON)
```

### Full Integration Example

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_application)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find all required packages
find_package(container_system CONFIG REQUIRED)
find_package(network_system CONFIG REQUIRED)
find_package(database_system CONFIG REQUIRED)

add_executable(my_app main.cpp)

target_link_libraries(my_app PRIVATE
    kcenon::container_system
    kcenon::network_system
    kcenon::database_system
)
```

---

## Performance Considerations

### Serialization Format Selection

| Format | Speed | Size | Use Case |
|--------|-------|------|----------|
| **Binary** | Fastest (2M/s) | Smallest | Network, IPC |
| **JSON** | Medium (500K/s) | Medium | REST APIs, Debug |
| **XML** | Slowest (200K/s) | Largest | Legacy systems |

### Optimization Tips

1. **Use binary format for performance-critical paths**
   ```cpp
   // Fast path: binary
   auto data = container->serialize_to_binary();

   // Slow path: JSON (for debugging only)
   auto json = container->serialize_to_json();
   ```

2. **Enable SIMD for large data operations**
   ```cmake
   set(ENABLE_SIMD ON)
   ```

3. **Use zero-copy with network_system**
   ```cpp
   // Avoid: Copy overhead
   auto data = container->serialize_to_binary();
   client.send(data);

   // Better: Zero-copy
   client.send_container(container); // Uses internal zero-copy
   ```

4. **Reuse containers to reduce allocations**
   ```cpp
   // Reuse container for multiple operations
   auto container = create_container();
   for (const auto& item : items) {
       container->clear();
       container->set_value("item", item);
       send(container);
   }
   ```

---

## Troubleshooting

### Common Issues

#### 1. Link Error: `undefined reference to container_system::create_container`

**Solution**: Ensure container_system is properly linked
```cmake
target_link_libraries(your_target PRIVATE kcenon::container_system)
```

#### 2. Serialization Format Mismatch

**Problem**: Binary data deserialized as JSON
**Solution**: Use consistent format
```cpp
// Serialize
auto binary = container->serialize_to_binary();

// Deserialize with matching format
auto restored = container_system::deserialize_from_binary(binary); // OK
auto wrong = container_system::deserialize_from_json(binary); // ERROR!
```

#### 3. Type Mismatch on Retrieval

**Problem**: Retrieved value type doesn't match
```cpp
container->set_value("count", 42); // int
auto str = container->get_string("count"); // ERROR: type mismatch
```

**Solution**: Use correct type or check before retrieval
```cpp
if (container->value_type("count") == value_type::INT) {
    auto count = container->get_int("count");
}
```

#### 4. Performance Issues with Large Containers

**Problem**: Slow serialization of large containers
**Solution**: Enable SIMD and use binary format
```cmake
set(ENABLE_SIMD ON)
```

```cpp
// Use binary format (fastest)
auto data = container->serialize_to_binary();
```

---

## Example Applications

### Complete Integration Example

See [examples/integration_example/](examples/integration_example/) for a complete application demonstrating:
- Container creation and population
- Network transmission via network_system
- Database persistence via database_system
- Error handling with common_system Result<T>

### Running the Example

```bash
cd container_system
mkdir build && cd build
cmake .. -DBUILD_EXAMPLES=ON
make
./examples/integration_example
```

---

## Support

- **Documentation**: [docs/](docs/)
- **API Reference**: [docs/API_REFERENCE.md](docs/API_REFERENCE.md)
- **Issues**: [GitHub Issues](https://github.com/kcenon/container_system/issues)
- **Email**: kcenon@naver.com

---

**Last Updated**: 2025-10-22
**Maintainer**: kcenon@naver.com
