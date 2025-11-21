# Container System Features

**Last Updated**: 2025-11-15

## Overview

This document provides comprehensive details about all features and capabilities of the Container System, including value types, serialization formats, advanced features, and integration capabilities.

## Core Capabilities

### Type Safety
- **Strongly-typed value system** with compile-time checks
- **Template metaprogramming** for type-safe value creation
- **Variant storage** with minimal overhead
- **Type validation** at construction and deserialization
- **Zero runtime overhead** for type checking

### Thread Safety
- **Lock-free read operations** without synchronization overhead
- **Thread-safe container wrapper** for concurrent write scenarios
- **Concurrent read access** - multiple threads can safely read simultaneously
- **Optional thread-safe write operations** via `thread_safe_container`
- **ThreadSanitizer validated** - zero data races detected

### SIMD Optimization
- **ARM NEON support** for Apple Silicon (M1/M2/M3)
- **x86 AVX2 support** for Intel/AMD processors
- **Automatic SIMD detection** based on platform capabilities
- **Numeric array acceleration** with 3.2x speedup on ARM
- **Bulk operation optimization** - 3.8 GB/s throughput

### Memory Efficiency
- **Variant storage** with minimal allocations
- **Smart pointer management** - 100% RAII compliance
- **Move semantics** - 4.2M zero-copy operations/second
- **Memory pool support** - 10-50x faster for small blocks
- **Automatic resource cleanup** - perfect AddressSanitizer score

### Serialization Formats
- **Binary format** - high-performance with minimal overhead
- **JSON format** - human-readable with pretty-printing
- **XML format** - schema-validated with namespace support
- **Automatic format detection** for deserialization
- **Streaming serialization** for large datasets

## Value Types

The Container System supports 15 distinct value types covering all common data scenarios:

### Primitive Types

| Type | Code | C++ Type | Size | Description |
|------|------|----------|------|-------------|
| `null_value` | '0' | - | 0 bytes | Null/empty value, represents absence of data |
| `bool_value` | '1' | `bool` | 1 byte | Boolean true/false |
| `char_value` | '2' | `char` | 1 byte | Single ASCII character |

### Integer Types

| Type | Code | C++ Type | Size | Range |
|------|------|----------|------|-------|
| `int8_value` | '3' | `int8_t` | 1 byte | -128 to 127 |
| `uint8_value` | '4' | `uint8_t` | 1 byte | 0 to 255 |
| `int16_value` | '5' | `int16_t` | 2 bytes | -32,768 to 32,767 |
| `uint16_value` | '6' | `uint16_t` | 2 bytes | 0 to 65,535 |
| `int32_value` | '7' | `int32_t` | 4 bytes | -2^31 to 2^31-1 |
| `uint32_value` | '8' | `uint32_t` | 4 bytes | 0 to 2^32-1 |
| `int64_value` | '9' | `int64_t` | 8 bytes | -2^63 to 2^63-1 |
| `uint64_value` | 'a' | `uint64_t` | 8 bytes | 0 to 2^64-1 |

### Floating-Point Types

| Type | Code | C++ Type | Size | Precision |
|------|------|----------|------|-----------|
| `float_value` | 'b' | `float` | 4 bytes | ~7 decimal digits |
| `double_value` | 'c' | `double` | 8 bytes | ~15 decimal digits |

### Complex Types

| Type | Code | C++ Type | Size | Description |
|------|------|----------|------|-------------|
| `bytes_value` | 'd' | `std::vector<uint8_t>` | Variable | Raw byte array, binary data |
| `container_value` | 'e' | `std::shared_ptr<value_container>` | Variable | Nested container for hierarchical data |
| `string_value` | 'f' | `std::string` | Variable | UTF-8 encoded string |

### Value Type Usage Examples

```cpp
#include <container/core/value_factory.h>
using namespace container_module;

// Primitive types
auto null_val = value_factory::create("empty", null_value, "");
auto bool_val = value_factory::create_bool("is_active", true);
auto char_val = value_factory::create("grade", char_value, "A");

// Integer types
auto int8_val = value_factory::create("tiny", int8_value, "127");
auto uint64_val = value_factory::create_uint64("big_number", 1000000000000ULL);

// Floating-point types
auto float_val = value_factory::create("pi_approx", float_value, "3.14159");
auto double_val = value_factory::create_double("price", 99.99);

// Complex types
auto bytes_val = value_factory::create("data", bytes_value, "binary_data_here");
auto string_val = value_factory::create_string("name", "John Doe");

// Nested container
auto nested = std::make_shared<value_container>();
nested->add_value(value_factory::create_string("city", "Seattle"));
auto container_val = std::make_shared<container_value>("address", nested);
```

## Enhanced Features

### Messaging Integration
- **Optimized containers** for messaging systems
- **Builder pattern** for fluent container construction
- **Header management** - source, target, message_type
- **Message routing** - automatic header swapping
- **Performance metrics** - real-time operation monitoring

```cpp
#include <container/integration/messaging_builder.h>

auto message = messaging_container_builder()
    .source("order_service", "instance_01")
    .target("fulfillment_service", "warehouse_west")
    .message_type("order_create")
    .add_value("order_id", "ORD-2025-001234")
    .add_value("total_amount", 299.99)
    .optimize_for_network()
    .build();
```

### Builder Pattern
- **Fluent API** with method chaining
- **Type-safe construction** with compile-time validation
- **Pre-allocation support** via `reserve_values()`
- **Optimization hints** - `optimize_for_speed()`, `optimize_for_network()`
- **Nested container support** via lambda callbacks

### Performance Metrics
- **Real-time monitoring** of container operations
- **Operation counters** - creation, serialization, deserialization
- **Timing metrics** - average latency per operation
- **Memory tracking** - allocation and deallocation counts
- **SIMD usage statistics** - operations accelerated by SIMD

### External Callbacks
- **Integration hooks** for external systems
- **Custom serialization handlers** for specialized formats
- **Validation callbacks** during deserialization
- **Event notifications** for container lifecycle
- **Performance profiling hooks**

### Dual Compatibility
- **Standalone mode** - independent container library
- **Messaging system integration** - seamless messaging_system compatibility
- **CMake aliases** - `ContainerSystem::container` or `MessagingSystem::container`
- **Namespace flexibility** - works with both `container_module` and `messaging_system`

## Technology Stack

### Modern C++ Foundation
- **C++17 compatibility** - template metaprogramming, variant, optional
- **Template metaprogramming** - compile-time type checking
- **Smart pointers** - automatic memory management
- **RAII principles** - resource acquisition is initialization
- **Move semantics** - zero-copy operations
- **Variant storage** - efficient polymorphic value storage

### SIMD Optimizations
- **ARM NEON intrinsics** for Apple Silicon
- **x86 AVX2 intrinsics** for Intel/AMD processors
- **Automatic platform detection** at compile-time
- **Fallback to scalar operations** when SIMD unavailable
- **Bulk data processing** - arrays of numeric values

### Design Patterns
- **Factory Pattern** - `value_factory` for type-safe value creation
- **Builder Pattern** - fluent API for container construction
- **Visitor Pattern** - type-safe value processing
- **Template Method Pattern** - customizable serialization
- **Strategy Pattern** - configurable serialization formats
- **RAII Pattern** - automatic resource management
- **Adapter Pattern** - Result&lt;T&gt; for error handling at boundaries

## Advanced Capabilities

### Nested Containers
```cpp
// Create hierarchical data structures
auto root = std::make_shared<value_container>();
root->set_message_type("order");

auto customer = std::make_shared<value_container>();
customer->set_message_type("customer_info");
customer->add_value(value_factory::create_string("name", "Alice"));
customer->add_value(value_factory::create_string("email", "alice@example.com"));

auto address = std::make_shared<value_container>();
address->set_message_type("address");
address->add_value(value_factory::create_string("street", "123 Main St"));
address->add_value(value_factory::create_string("city", "Seattle"));

customer->add_value(std::make_shared<container_value>("address", address));
root->add_value(std::make_shared<container_value>("customer", customer));
```

### Thread-Safe Operations
```cpp
#include <container/advanced/thread_safe_container.h>

auto container = std::make_shared<value_container>();
auto safe_container = std::make_shared<thread_safe_container>(container);

// Multiple threads can safely access
std::vector<std::thread> workers;
for (int i = 0; i < 4; ++i) {
    workers.emplace_back([&safe_container, i]() {
        // Thread-safe write
        safe_container->set_value("thread_" + std::to_string(i),
                                 int32_value,
                                 std::to_string(i));

        // Thread-safe read
        auto value = safe_container->get_value("thread_" + std::to_string(i));
    });
}

for (auto& worker : workers) {
    worker.join();
}
```

### SIMD Processing
```cpp
#include <container/advanced/simd_processor.h>

simd_processor processor;

// Process large numeric arrays with SIMD acceleration
std::vector<double> temperatures(10000);
std::iota(temperatures.begin(), temperatures.end(), 20.0);

auto processed = processor.process_array(temperatures);

// Check SIMD capabilities
if (processor.is_simd_available()) {
    std::cout << "Using SIMD: " << processor.get_simd_type() << std::endl;
    // Output: "Using SIMD: ARM NEON" or "Using SIMD: AVX2"
}
```

### Memory Optimization
```cpp
// Pre-allocate for known container sizes
auto container = std::make_shared<value_container>();
container->reserve_values(100);  // Avoid reallocations

// Use move semantics for large data
std::string large_data(1000000, 'x');
auto value = std::make_shared<string_value>("data", std::move(large_data));
container->add_value(std::move(value));

// Efficient bulk operations
std::vector<std::shared_ptr<value>> bulk_values;
bulk_values.reserve(100);
for (int i = 0; i < 100; ++i) {
    bulk_values.push_back(value_factory::create_int32("val_" + std::to_string(i), i));
}
container->set_values(bulk_values);  // Single operation
```

## Real-World Use Cases

### Financial Trading System
```cpp
auto market_data = messaging_container_builder()
    .source("trading_engine", "session_001")
    .target("risk_monitor", "main")
    .message_type("market_tick")
    .add_value("symbol", "AAPL")
    .add_value("price", 175.50)
    .add_value("volume", 1000000)
    .add_value("bid", 175.48)
    .add_value("ask", 175.52)
    .add_value("timestamp", std::chrono::system_clock::now().time_since_epoch().count())
    .optimize_for_speed()
    .build();

// High-frequency serialization (2M ops/sec)
std::string binary_data = market_data->serialize();
```

### IoT Sensor Data
```cpp
auto sensor_reading = std::make_shared<value_container>();
sensor_reading->set_source("sensor_array", "building_A_floor_3");
sensor_reading->set_message_type("environmental_reading");

// Bulk sensor data with SIMD optimization
std::vector<double> temperature_readings(1000);
std::vector<double> humidity_readings(1000);

simd_processor processor;
auto processed_temp = processor.process_array(temperature_readings);
auto processed_humidity = processor.process_array(humidity_readings);

sensor_reading->add_value(std::make_shared<bytes_value>("temp_data",
    reinterpret_cast<const char*>(processed_temp.data()),
    processed_temp.size() * sizeof(double)));
```

### Web API Response
```cpp
auto api_response = messaging_container_builder()
    .message_type("api_response")
    .add_value("status", 200)
    .add_value("success", true)
    .add_nested_container("data", [](auto& builder) {
        builder.add_value("user_id", 12345)
               .add_value("username", "john_doe")
               .add_value("email", "john@example.com");
    })
    .build();

// JSON serialization for HTTP response
std::string json_response = api_response->to_json();
```

### Database Storage
```cpp
auto record = std::make_shared<value_container>();
record->set_message_type("user_record");
record->add_value(value_factory::create_int64("id", 12345));
record->add_value(value_factory::create_string("username", "alice"));
record->add_value(value_factory::create_double("balance", 1500.75));
record->add_value(value_factory::create_bool("active", true));

// Compact binary format for BLOB storage
std::vector<uint8_t> blob_data = record->serialize_array();
// Store blob_data in database BLOB field
```

## Integration Capabilities

### With Messaging System
```cpp
#include <messaging_system/messaging_client.h>

auto client = std::make_shared<messaging_client>("client_01");

// Container seamlessly integrates with messaging
auto message = messaging_container_builder()
    .source("client_01", "session_123")
    .target("server", "main")
    .message_type("request")
    .add_value("action", "query")
    .build();

client->send_container(message);
```

### With Network System
```cpp
#include <network_system/tcp_client.h>

auto tcp_client = network_system::TcpClient::create("localhost", 8080);

// Serialize and send over network
std::string data = container->serialize();
tcp_client->send(data);

// Receive and deserialize
auto received = tcp_client->receive();
auto restored = std::make_shared<value_container>(received);
```

### With Database System
```cpp
#include <database_system/database_manager.h>

database_manager db;
db.connect("host=localhost dbname=app");

// Store container as BLOB
std::string serialized = container->serialize();
db.insert_query("INSERT INTO messages (data) VALUES (?)", serialized);

// Retrieve and restore
auto result = db.select_query("SELECT data FROM messages WHERE id = 1");
auto restored = std::make_shared<value_container>(result[0]["data"]);
```

## Performance Characteristics

### Serialization Performance

| Format | Throughput (ops/s) | Size Overhead | Best Use Case |
|--------|-------------------|---------------|---------------|
| **Binary** | 1.8M | 100% (baseline) | High-performance, network transmission |
| **JSON** | 950K | ~180% | Human-readable, debugging, APIs |
| **XML** | 720K | ~220% | Schema validation, enterprise integration |

### Memory Characteristics

| Component | Memory Usage | Notes |
|-----------|--------------|-------|
| Empty Container | ~128 bytes | Header + minimal allocations |
| String Value | ~64 bytes + length | Includes key + value |
| Numeric Value | ~48 bytes | Fixed-size allocation |
| Nested Container | Recursive | Sum of all child containers |
| SIMD Buffer | ~16-32 bytes | Alignment overhead |

### SIMD Acceleration

| Platform | SIMD Type | Speedup | Throughput |
|----------|-----------|---------|------------|
| Apple M1/M2/M3 | ARM NEON | 3.2x | 3.8 GB/s |
| Intel/AMD (AVX2) | x86 AVX2 | 2.5x | 2.9 GB/s |
| Fallback | Scalar | 1.0x | 1.2 GB/s |

## Error Handling

### Hybrid Adapter Pattern

The container system uses a sophisticated hybrid approach:
- **Internal operations**: C++ exceptions for performance
- **External API**: Result&lt;T&gt; adapters for type safety

```cpp
#include <container/adapters/common_result_adapter.h>
using namespace container::adapters;

// Serialization with Result<T>
auto serialize_result = serialization_result_adapter::serialize(*container);
if (!serialize_result) {
    std::cerr << "Error: " << serialize_result.error().message << "\n";
    return -1;
}
auto data = serialize_result.value();

// Deserialization with Result<T>
auto deserialize_result = deserialization_result_adapter::deserialize<value_container>(data);
if (!deserialize_result) {
    std::cerr << "Error: " << deserialize_result.error().message << "\n";
}

// Container operations with Result<T>
auto get_result = container_result_adapter::get_value<double>(container, "price");
if (!get_result) {
    std::cerr << "Error: " << get_result.error().message << "\n";
}
```

### Error Categories

| Category | Error Code Range | Examples |
|----------|-----------------|----------|
| Serialization | -400 to -409 | Invalid format, buffer overflow |
| Deserialization | -410 to -419 | Corrupt data, version mismatch |
| Validation | -420 to -429 | Type mismatch, missing required fields |
| Type Conversion | -430 to -439 | Invalid cast, overflow |
| SIMD Operations | -440 to -449 | Alignment error, unsupported operation |

## See Also

- [BENCHMARKS.md](BENCHMARKS.md) - Detailed performance benchmarks
- [PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md) - File organization and dependencies
- [PRODUCTION_QUALITY.md](PRODUCTION_QUALITY.md) - RAII compliance and sanitizer results
- [API_REFERENCE.md](API_REFERENCE.md) - Complete API documentation
- [USER_GUIDE.md](USER_GUIDE.md) - Usage patterns and best practices

---

**Last Updated**: 2025-11-15
**Version**: 1.0
