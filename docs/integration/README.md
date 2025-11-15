# Container System Integration Guide

## Overview

This directory contains integration guides for using container_system with other KCENON systems.

## Integration Guides

- [With Common System](with-common-system.md) - Type utilities and concepts
- [With Logger System](with-logger.md) - Logging serialization operations
- [With Network System](with-network-system.md) - Network data serialization
- [With Database System](with-database-system.md) - Data model serialization

## Quick Start

### Basic variant_value Usage

```cpp
#include "container_system/variant_value.h"

using namespace container_system;

variant_value data;
data["user_id"] = 12345;
data["username"] = "john_doe";
data["active"] = true;
data["scores"] = variant_value::array{95, 87, 92};

// Serialize to MessagePack
auto msgpack = data.to_msgpack();

// Deserialize
auto deserialized = variant_value::from_msgpack(msgpack);
```

### JSON Serialization

```cpp
variant_value config;
config["server"]["host"] = "localhost";
config["server"]["port"] = 8080;
config["features"]["logging"] = true;

// To JSON string
std::string json = config.to_json_string();

// From JSON string
auto parsed = variant_value::from_json_string(json);
```

## Integration Patterns

### Configuration Management

```cpp
class AppConfig {
public:
    static AppConfig from_file(const std::string& filename) {
        auto json = read_file(filename);
        auto data = variant_value::from_json_string(json);

        AppConfig config;
        config.host = data["server"]["host"].as_string();
        config.port = data["server"]["port"].as_int();
        config.debug = data["debug"].as_bool();

        return config;
    }

    std::string host;
    int port;
    bool debug;
};
```

### Network Data Transfer

```cpp
// Sender
variant_value message;
message["type"] = "user_update";
message["user_id"] = 123;
message["data"]["email"] = "new@example.com";

auto serialized = message.to_msgpack();
connection->send(serialized);

// Receiver
auto received = connection->receive();
auto message = variant_value::from_msgpack(received);

if (message["type"].as_string() == "user_update") {
    int user_id = message["user_id"].as_int();
    std::string email = message["data"]["email"].as_string();
    update_user(user_id, email);
}
```

### Database ORM

```cpp
class User {
public:
    variant_value to_variant() const {
        variant_value v;
        v["id"] = id;
        v["username"] = username;
        v["email"] = email;
        v["created_at"] = created_at.time_since_epoch().count();
        return v;
    }

    static User from_variant(const variant_value& v) {
        User user;
        user.id = v["id"].as_int();
        user.username = v["username"].as_string();
        user.email = v["email"].as_string();
        user.created_at = std::chrono::system_clock::time_point(
            std::chrono::seconds(v["created_at"].as_int64())
        );
        return user;
    }

    int id;
    std::string username;
    std::string email;
    std::chrono::system_clock::time_point created_at;
};
```

## Common Use Cases

### 1. API Request/Response

```cpp
// Build API request
variant_value request;
request["method"] = "GET";
request["path"] = "/api/users/123";
request["headers"]["Authorization"] = "Bearer token";

// Send as JSON
auto json = request.to_json_string();
client->send(json);
```

### 2. Configuration Files

```cpp
// Load config
auto config = variant_value::from_json_file("config.json");

// Access nested values
std::string db_host = config["database"]["host"].as_string();
int db_port = config["database"]["port"].as_int();
bool ssl = config["database"]["ssl"].as_bool();
```

### 3. Caching Serializable Data

```cpp
// Serialize to cache
variant_value user_data = user.to_variant();
auto cached = user_data.to_msgpack();
cache->set("user:123", cached);

// Deserialize from cache
auto cached_data = cache->get("user:123");
auto user_data = variant_value::from_msgpack(cached_data);
auto user = User::from_variant(user_data);
```

## Best Practices

- Use MessagePack for binary efficiency
- Use JSON for human-readable config files
- Validate data after deserialization
- Handle missing keys gracefully
- Use type-safe accessors (as_int(), as_string(), etc.)

## Performance Comparison

| Format | Size | Encode Speed | Decode Speed | Use Case |
|--------|------|--------------|--------------|----------|
| MessagePack | Smallest | Fast | Fast | Network transfer, caching |
| JSON | Medium | Medium | Medium | Config files, APIs |
| Protocol Buffers | Small | Medium | Fast | Cross-language RPC |

## Additional Resources

- [Container System API Reference](../API_REFERENCE.md)
- [Value System Comparison](../VALUE_SYSTEM_COMPARISON_ANALYSIS.md)
- [Ecosystem Integration Guide](../../../ECOSYSTEM_INTEGRATION.md)
