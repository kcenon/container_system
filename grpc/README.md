# Container System gRPC Integration

A gRPC protocol layer for the container_system that enables remote container exchange via Protocol Buffers. This integration follows a **minimal modification** approach with zero changes to existing container system code.

## Features

- **Bidirectional Conversion**: Seamless conversion between native `value_container` and gRPC proto messages
- **Full Type Support**: All container value types are supported (bool, integers, floats, strings, bytes, nested containers)
- **Zero Modification**: Existing container_system code remains unchanged
- **Isolated Build**: Separate CMake configuration; builds independently
- **Thread-Safe**: Server supports concurrent client connections

## Architecture

```
container_system/grpc/
├── proto/                    # Protocol Buffer definitions
│   └── container_service.proto
├── adapters/                 # Conversion layer
│   ├── container_adapter.h/cpp
│   └── value_mapper.h
├── server/                   # gRPC server implementation
│   ├── grpc_server.h/cpp
│   └── service_impl.h/cpp
├── client/                   # gRPC client implementation
│   └── grpc_client.h/cpp
├── examples/                 # Usage examples
│   ├── server_example.cpp
│   └── client_example.cpp
├── tests/                    # Unit and integration tests
│   ├── adapter_test.cpp
│   └── service_test.cpp
└── CMakeLists.txt           # Isolated build configuration
```

## Quick Start

### Building

```bash
cd container_system/grpc
mkdir build && cd build
cmake .. -DCONTAINER_GRPC_BUILD_TESTS=ON -DCONTAINER_GRPC_BUILD_EXAMPLES=ON
make
```

### Server Example

```cpp
#include "server/grpc_server.h"
#include "core/container.h"

int main() {
    container_grpc::grpc_server server("0.0.0.0:50051");

    // Optional: Set custom processor
    server.set_processor([](auto container) {
        // Process incoming container
        container->add_value("processed", container_module::value_types::bool_value, true);
        return container;
    });

    server.start();
    server.wait();  // Block until shutdown

    return 0;
}
```

### Client Example

```cpp
#include "client/grpc_client.h"
#include "core/container.h"

int main() {
    container_grpc::grpc_client client("localhost:50051");

    auto container = std::make_shared<container_module::value_container>();
    container->set_message_type("request");
    container->add_value("count", container_module::value_types::int_value, 42);

    auto result = client.process(container);

    if (result) {
        std::cout << "Response type: " << result.value->message_type() << std::endl;
    } else {
        std::cerr << "Error: " << result.error_message << std::endl;
    }

    return 0;
}
```

## API Reference

### grpc_server

```cpp
namespace container_grpc {

struct server_config {
    std::string address = "0.0.0.0:50051";
    int max_receive_message_size = 64 * 1024 * 1024;
    int max_send_message_size = 64 * 1024 * 1024;
    int num_completion_queues = 1;
    bool enable_reflection = false;
};

class grpc_server {
public:
    explicit grpc_server(const std::string& address);
    explicit grpc_server(const server_config& config);

    bool start();
    void stop(int deadline_ms = 5000);
    void wait();

    bool is_running() const noexcept;
    std::string address() const noexcept;

    void set_processor(container_processor processor);

    size_t request_count() const noexcept;
    size_t error_count() const noexcept;
};

}
```

### grpc_client

```cpp
namespace container_grpc {

struct client_config {
    std::string target_address = "localhost:50051";
    std::chrono::milliseconds timeout{30000};
    int max_retries = 3;
    bool use_ssl = false;
    std::string client_id = "";
};

template<typename T>
struct client_result {
    bool success = false;
    std::string error_message;
    T value;

    explicit operator bool() const noexcept { return success; }
};

class grpc_client {
public:
    explicit grpc_client(const std::string& target);
    explicit grpc_client(const client_config& config);

    // Unary operations
    client_result<std::shared_ptr<value_container>> send(std::shared_ptr<value_container> container);
    client_result<std::shared_ptr<value_container>> process(std::shared_ptr<value_container> container);

    // Streaming operations
    bool stream(std::shared_ptr<value_container> request, stream_callback callback);
    client_result<std::vector<std::shared_ptr<value_container>>> send_batch(
        const std::vector<std::shared_ptr<value_container>>& containers);

    // Status
    bool ping();
    std::optional<std::pair<int64_t, int64_t>> get_status();

    // Configuration
    void set_timeout(std::chrono::milliseconds timeout);
    std::chrono::milliseconds timeout() const noexcept;
    std::string target() const noexcept;
};

}
```

### container_adapter

```cpp
namespace container_grpc {

class container_adapter {
public:
    // Main conversion functions
    static GrpcContainer to_grpc(const container_module::value_container& container);
    static std::shared_ptr<container_module::value_container> from_grpc(const GrpcContainer& grpc_container);

    // Value conversion
    static GrpcValue to_grpc_value(const container_module::optimized_value& value);
    static container_module::optimized_value from_grpc_value(const GrpcValue& grpc_value);

    // Type mapping
    static ValueType to_grpc_type(container_module::value_types type);
    static container_module::value_types from_grpc_type(ValueType type);

    // Validation
    static bool can_convert(const container_module::value_container& container) noexcept;
    static bool can_convert(const GrpcContainer& grpc_container) noexcept;
};

}
```

## Supported Value Types

| Native Type | Proto Type | Notes |
|-------------|------------|-------|
| `null_value` | `NULL_VALUE` | Empty/null marker |
| `bool_value` | `BOOL_VALUE` | Boolean |
| `short_value` | `SHORT_VALUE` | int16, stored as int32 |
| `ushort_value` | `USHORT_VALUE` | uint16, stored as uint32 |
| `int_value` | `INT_VALUE` | int32 |
| `uint_value` | `UINT_VALUE` | uint32 |
| `long_value` | `LONG_VALUE` | int64 |
| `ulong_value` | `ULONG_VALUE` | uint64 |
| `llong_value` | `LLONG_VALUE` | int64 (platform alias) |
| `ullong_value` | `ULLONG_VALUE` | uint64 (platform alias) |
| `float_value` | `FLOAT_VALUE` | 32-bit float |
| `double_value` | `DOUBLE_VALUE` | 64-bit double |
| `string_value` | `STRING_VALUE` | UTF-8 string |
| `bytes_value` | `BYTES_VALUE` | Binary data |
| `container_value` | `CONTAINER_VALUE` | Nested container |
| `array_value` | `ARRAY_VALUE` | Heterogeneous array |

## gRPC Service Definition

```protobuf
service ContainerService {
    // Unary RPC: Send a single container and receive response
    rpc SendContainer(SendContainerRequest) returns (SendContainerResponse);

    // Unary RPC: Process a container and receive transformed result
    rpc ProcessContainer(GrpcContainer) returns (GrpcContainer);

    // Server streaming: Subscribe to containers matching criteria
    rpc StreamContainers(SendContainerRequest) returns (stream GrpcContainer);

    // Client streaming: Send multiple containers, receive summary
    rpc CollectContainers(stream GrpcContainer) returns (BatchContainerResponse);

    // Bidirectional streaming: Full duplex container exchange
    rpc ProcessStream(stream GrpcContainer) returns (stream GrpcContainer);

    // Utility: Get stream status
    rpc GetStreamStatus(SendContainerRequest) returns (StreamStatus);
}
```

## Configuration Options

### Server Configuration

| Option | Default | Description |
|--------|---------|-------------|
| `address` | `0.0.0.0:50051` | Server bind address |
| `max_receive_message_size` | 64MB | Maximum incoming message size |
| `max_send_message_size` | 64MB | Maximum outgoing message size |
| `num_completion_queues` | 1 | Number of completion queues |
| `enable_reflection` | false | Enable gRPC reflection |

### Client Configuration

| Option | Default | Description |
|--------|---------|-------------|
| `target_address` | `localhost:50051` | Server address to connect |
| `timeout` | 30000ms | Request timeout |
| `max_retries` | 3 | Maximum retry attempts |
| `use_ssl` | false | Enable SSL/TLS |
| `client_id` | "" | Client identifier |

## Error Handling

All client operations return a `client_result<T>` that includes:
- `success`: Boolean indicating operation success
- `error_message`: Detailed error description on failure
- `value`: Result value on success

```cpp
auto result = client.process(container);
if (!result) {
    std::cerr << "Operation failed: " << result.error_message << std::endl;
    // Handle error...
}
```

## Running Tests

```bash
cd build

# Run adapter unit tests
./grpc_adapter_test

# Run service integration tests
./grpc_service_test
```

## Dependencies

- **gRPC**: >= 1.50.0
- **Protobuf**: >= 3.21.0
- **C++ Standard**: C++20
- **GTest**: For testing (optional)

## Design Principles

1. **Zero Modification**: No changes to existing container_system code
2. **Read-Only Access**: Adapter layer only reads from native containers
3. **Stateless Conversion**: All conversion functions are stateless and thread-safe
4. **Graceful Degradation**: Missing gRPC dependencies do not affect core functionality
5. **Maximum Nesting Depth**: 32 levels to prevent stack overflow

## Performance Notes

- Conversion overhead is minimal for primitive types (stack-allocated)
- Large binary data and strings are zero-copy where possible
- Nested containers incur recursive conversion cost
- Consider batch operations for high-throughput scenarios

## License

BSD 3-Clause License. See LICENSE file for details.
