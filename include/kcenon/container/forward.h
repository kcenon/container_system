#pragma once

/**
 * @file forward.h
 * @brief Forward declarations for container_system types
 *
 * This header provides forward declarations for commonly used types
 * in the container_system module to reduce compilation dependencies.
 */

namespace kcenon::container {

// Core container types
class container;
class value;
class optimized_container;
class optimized_value;

// Core classes
namespace core {
    class value_store;
    class value_pool;
    template<typename T> class typed_value;
    enum class value_type;
}

// Internal implementation
namespace internal {
    class thread_safe_container;
    class memory_pool;
    class variant_value_factory;
    class simd_processor;
}

// Integration
namespace integration {
    class messaging_integration;
    class serialization_handler;
}

// Optimizations
namespace optimizations {
    class fast_parser;
    class bulk_operations;
}

// Utilities
template<typename T> class container_iterator;
template<typename T> class const_container_iterator;

} // namespace kcenon::container