# sources.cmake - source/header file lists for container_system
#
# Aggregates the file lists consumed by the static library target. This module
# is logically pure (no targets created, no flags applied) — it only sets the
# variables MAIN_HEADER, CORE_FILES, INTERNAL_FILES, INTEGRATION_FILES,
# MESSAGING_FILES, and ALL_FILES in the parent scope so cmake/targets.cmake
# can pass them to add_library().

# Main include file
set(MAIN_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/container.h
)

# Core files (public API)
set(CORE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/container.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/container.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/fwd.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/variant_helpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/schema.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/container_schema.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/value_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/value_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/value_store.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/value_store.cpp
    # Serializer strategy pattern (Issue #310, #313, #314)
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/serializers/serializer_strategy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/serializers/serializer_factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/serializers/serializer_factory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/serializers/binary_serializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/serializers/binary_serializer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/serializers/json_serializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/serializers/json_serializer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/serializers/xml_serializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/serializers/xml_serializer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/serializers/msgpack_serializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/serializers/msgpack_serializer.cpp
)

# Legacy value files removed - using variant-based storage only

# Internal implementation files
set(INTERNAL_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/internal/value.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal/value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/internal/variant_value_factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/internal/memory_pool.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/internal/pool_allocator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/internal/pool_allocator_adapter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/internal/thread_safe_container.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal/thread_safe_container.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/internal/simd_processor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal/simd_processor.cpp
)

# Async coroutine files (C++20)
if(CONTAINER_ENABLE_COROUTINES)
    list(APPEND INTERNAL_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/internal/async/async.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/internal/async/task.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/internal/async/generator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/internal/async/thread_pool_executor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/internal/async/async_container.h
    )
    message(STATUS "Container: C++20 coroutine support enabled")
endif()

# Integration files (conditional)
set(INTEGRATION_FILES)
if(ENABLE_MESSAGING_FEATURES OR ENABLE_EXTERNAL_INTEGRATION OR ENABLE_PERFORMANCE_METRICS)
    list(APPEND INTEGRATION_FILES
         ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/integration/messaging_integration.h
         ${CMAKE_CURRENT_SOURCE_DIR}/src/integration/messaging_integration.cpp
    )
    message(STATUS "Container: Integration modules included")
endif()

# Messaging files (domain-specific)
set(MESSAGING_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/kcenon/container/messaging/message_container.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/messaging/message_container.cpp
)

# Combine all files
set(ALL_FILES
    ${MAIN_HEADER}
    ${CORE_FILES}
    ${VALUE_FILES}
    ${INTERNAL_FILES}
    ${INTEGRATION_FILES}
    ${MESSAGING_FILES}
)
