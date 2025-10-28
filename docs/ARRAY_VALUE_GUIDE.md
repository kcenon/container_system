# ArrayValue Implementation Guide

## Overview

`ArrayValue` (type 15) is a container type that stores ordered collections of heterogeneous or homogeneous values, providing JSON-array-like functionality with full cross-language compatibility across C++, Rust, Go, Python, .NET, and Node.js implementations.

## Architecture

### Class Hierarchy

```
Value (abstract base)
├── BoolValue (type 1)
├── NumericValue (types 2-11)
├── BytesValue (type 12)
├── StringValue (type 13)
├── ContainerValue (type 14)
└── ArrayValue (type 15)  ← New heterogeneous collection type
```

### Class Diagram

```
┌─────────────────────────────────────┐
│           value (base)              │
├─────────────────────────────────────┤
│ - name_: string                     │
│ - type_: value_types                │
│ - data_: string                     │
├─────────────────────────────────────┤
│ + type(): value_types               │
│ + serialize(): vector<uint8_t>      │
│ + is_container(): bool              │
│ + is_array(): bool                  │
└─────────────────────────────────────┘
                ▲
                │
                │ inherits
                │
┌─────────────────────────────────────┐
│         array_value                 │
├─────────────────────────────────────┤
│ - values_: vector<shared_ptr<value>>│
├─────────────────────────────────────┤
│ + array_value(name)                 │
│ + array_value(name, values)         │
│ + push_back(value): void            │
│ + at(index): shared_ptr<value>      │
│ + size(): size_t                    │
│ + empty(): bool                     │
│ + clear(): void                     │
│ + values(): vector&                 │
│ + serialize(): vector<uint8_t>      │
│ + deserialize(bytes): shared_ptr    │
└─────────────────────────────────────┘
```

## Wire Protocol Format

### Binary Format

```
[type:1 byte = 15]
[name_length:4 bytes LE]
[name:UTF-8 string]
[value_size:4 bytes LE]
[element_count:4 bytes LE]
[element1_serialized]
[element2_serialized]
...
[elementN_serialized]
```

### Text Format (C++ Wire Protocol)

```
[name,array_value,count];[element1][element2]...[elementN]
```

**Example:**
```
[colors,array_value,3];[,string_value,red];[,string_value,green];[,string_value,blue];
```

## Usage Examples

### Basic Creation and Population

```cpp
#include "container/values/array_value.h"
#include "container/values/numeric_value.h"
#include "container/values/string_value.h"

using namespace container_module;

// Create empty array
auto numbers = std::make_shared<array_value>("numbers");

// Add elements
numbers->push_back(std::make_shared<int_value>("", 10));
numbers->push_back(std::make_shared<int_value>("", 20));
numbers->push_back(std::make_shared<int_value>("", 30));

std::cout << "Size: " << numbers->size() << std::endl; // Output: Size: 3
```

### Constructor with Initial Values

```cpp
// Create array with initial values
std::vector<std::shared_ptr<value>> initial_values = {
    std::make_shared<string_value>("", "Alice"),
    std::make_shared<string_value>("", "Bob"),
    std::make_shared<string_value>("", "Charlie")
};

auto names = std::make_shared<array_value>("names", initial_values);
```

### Heterogeneous Arrays

```cpp
// Mix different value types in one array
auto mixed = std::make_shared<array_value>("mixed_data");

mixed->push_back(std::make_shared<int_value>("", 42));
mixed->push_back(std::make_shared<string_value>("", "hello"));
mixed->push_back(std::make_shared<double_value>("", 3.14));
mixed->push_back(std::make_shared<bool_value>("", true));

// Access elements
auto first = mixed->at(0);  // int_value with value 42
auto second = mixed->at(1); // string_value with value "hello"
```

### Using with ValueContainer

```cpp
#include "container/core/container.h"

// Create container
auto container = std::make_shared<value_container>();
container->set_message_type("user_data");

// Create and populate array
auto scores = std::make_shared<array_value>("test_scores");
scores->push_back(std::make_shared<int_value>("", 95));
scores->push_back(std::make_shared<int_value>("", 87));
scores->push_back(std::make_shared<int_value>("", 92));

// Add to container
container->add(scores);

// Serialize to binary
auto serialized = container->serialize();

// Serialize to text format (cross-language compatible)
auto wire_format = container->to_string();
// Output: @header={{...}};@data={{[test_scores,array_value,3];...}};
```

### Iteration and Access

```cpp
auto array = std::make_shared<array_value>("data");
// ... populate array ...

// Iterate through elements
for (size_t i = 0; i < array->size(); ++i) {
    auto element = array->at(i);
    std::cout << "Element " << i << ": "
              << element->to_string() << std::endl;
}

// Access all values
const auto& all_values = array->values();
for (const auto& val : all_values) {
    // Process each value
}

// Check if empty
if (array->empty()) {
    std::cout << "Array is empty" << std::endl;
}

// Clear all elements
array->clear();
```

## Deserialization

### From Binary Format

```cpp
// Binary data received from network or file
std::vector<uint8_t> binary_data = /* ... */;

// Deserialize
auto restored = array_value::deserialize(binary_data);

std::cout << "Array name: " << restored->name() << std::endl;
std::cout << "Element count: " << restored->size() << std::endl;
```

### From Value Container

```cpp
// Receive serialized container
std::vector<uint8_t> container_data = /* ... */;

// Deserialize container
auto container = std::make_shared<value_container>(container_data);

// Extract array
auto array = container->value("test_scores");
if (auto arr = std::dynamic_pointer_cast<array_value>(array)) {
    std::cout << "Found array with " << arr->size() << " elements" << std::endl;
}
```

## Integration with Factory Pattern

The `array_value` is automatically registered in the container's factory pattern:

```cpp
// In container.cpp constructor
data_type_map_.insert(
    { value_types::array_value,
      [this](const std::string& n, const std::string& d)
      {
          return std::make_shared<array_value>(n);
      } });
```

This enables automatic deserialization when parsing binary or text formats.

## Cross-Language Interoperability

### Sending from C++ to Python

```cpp
// C++ side
auto container = std::make_shared<value_container>();
auto array = std::make_shared<array_value>("colors");
array->push_back(std::make_shared<string_value>("", "red"));
array->push_back(std::make_shared<string_value>("", "blue"));
container->add(array);

auto wire_data = container->to_string();
// Send wire_data to Python...
```

```python
# Python side
from container_module import ValueContainer

# Receive wire_data from C++
container = ValueContainer.from_string(wire_data)
array = container.get_value("colors")

print(f"Received {array.count()} colors")
for i in range(array.count()):
    print(f"  Color {i}: {array.at(i).to_string()}")
```

### Sending from Python to C++

```python
# Python side
from container_module import ValueContainer
from container_module.values import ArrayValue, IntValue

container = ValueContainer()
numbers = ArrayValue("scores")
numbers.push_back(IntValue("", 100))
numbers.push_back(IntValue("", 95))
container.add(numbers)

wire_data = container.serialize()
# Send to C++...
```

```cpp
// C++ side
std::vector<uint8_t> received_data = /* ... */;
auto container = std::make_shared<value_container>(received_data);

auto scores = std::dynamic_pointer_cast<array_value>(
    container->value("scores"));

std::cout << "Received " << scores->size() << " scores" << std::endl;
```

## Performance Considerations

### Memory Management

- Uses `std::shared_ptr` for automatic memory management
- Elements maintain parent-child relationships
- Copy constructor performs deep copy of all elements

### Capacity Planning

```cpp
// For large arrays, consider pre-allocation
auto large_array = std::make_shared<array_value>("large");
// Note: Current implementation uses std::vector default growth
// Elements are added dynamically via push_back()

for (int i = 0; i < 10000; ++i) {
    large_array->push_back(std::make_shared<int_value>("", i));
}
```

## Error Handling

### Bounds Checking

```cpp
auto array = std::make_shared<array_value>("data");
array->push_back(std::make_shared<int_value>("", 42));

// Safe access with bounds checking
try {
    auto element = array->at(10); // Index out of range
} catch (const std::out_of_range& e) {
    std::cerr << "Index error: " << e.what() << std::endl;
}
```

### Type Checking

```cpp
auto element = array->at(0);

// Check value type before casting
if (element->type() == value_types::int_value) {
    if (auto int_val = std::dynamic_pointer_cast<int_value>(element)) {
        int value = int_val->to_long();
    }
}
```

## Best Practices

1. **Naming**: Use descriptive names for arrays to aid debugging and serialization
   ```cpp
   // Good
   auto user_ids = std::make_shared<array_value>("user_ids");

   // Avoid
   auto arr = std::make_shared<array_value>("a");
   ```

2. **Type Consistency**: For homogeneous arrays, maintain consistent element types
   ```cpp
   auto integers = std::make_shared<array_value>("integers");
   integers->push_back(std::make_shared<int_value>("", 1));
   integers->push_back(std::make_shared<int_value>("", 2));
   // Don't mix: integers->push_back(std::make_shared<string_value>("", "three"));
   ```

3. **Empty Checks**: Always check before accessing elements
   ```cpp
   if (!array->empty()) {
       auto first = array->at(0);
       // Process first element
   }
   ```

4. **Clear Resources**: Use clear() when reusing arrays
   ```cpp
   array->clear();
   // Array is now empty, ready for reuse
   ```

## Migration from ContainerValue

If you were using `container_value` for array-like storage:

### Before (using container_value)

```cpp
auto container = std::make_shared<container_value>("items", 3);
container->add(std::make_shared<int_value>("0", 10));
container->add(std::make_shared<int_value>("1", 20));
container->add(std::make_shared<int_value>("2", 30));
```

### After (using array_value)

```cpp
auto array = std::make_shared<array_value>("items");
array->push_back(std::make_shared<int_value>("", 10));
array->push_back(std::make_shared<int_value>("", 20));
array->push_back(std::make_shared<int_value>("", 30));
```

**Benefits:**
- Cleaner API (no need for numeric string keys)
- Explicit array semantics
- Better cross-language mapping to native arrays
- Index-based access with `at(index)`

## See Also

- [Value Types Reference](VALUE_TYPES.md)
- [Container Architecture](ARCHITECTURE.md)
- [Wire Protocol Specification](WIRE_PROTOCOL.md)
- [Cross-Language Guide](CROSS_LANGUAGE.md)
