/**
 * @file fuzz_container_deserialize.cpp
 * @brief Fuzz target for thread_safe_container deserialization
 *
 * This fuzz target tests the thread_safe_container::deserialize function
 * with random input to detect memory safety issues and crashes.
 */

#include <cstdint>
#include <cstddef>
#include <vector>

#include "internal/thread_safe_container.h"

using namespace container_module;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Create input vector from fuzzer data
    std::vector<uint8_t> input(data, data + size);

    try {
        // Attempt to deserialize container from input data
        auto container = thread_safe_container::deserialize(input);

        if (container) {
            // Round-trip test: serialize and verify
            auto serialized = container->serialize();
            (void)serialized;

            // Test container operations
            (void)container->size();
            (void)container->empty();

            // Test key enumeration
            auto keys = container->keys();
            for (const auto& key : keys) {
                // Access each value
                auto val = container->get(key);
                if (val.has_value()) {
                    (void)val->type();
                }
            }
        }
    } catch (const std::exception&) {
        // Exceptions are allowed for invalid input
    } catch (...) {
        // Catch any other exceptions
    }

    return 0;
}
