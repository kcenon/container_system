/**
 * @file fuzz_deserialize.cpp
 * @brief Fuzz target for value deserialization
 *
 * This fuzz target tests the value::deserialize function with random input
 * to detect memory safety issues and crashes.
 */

#include <cstdint>
#include <cstddef>
#include <vector>

#include "internal/value.h"

using namespace container_module;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Create input vector from fuzzer data
    std::vector<uint8_t> input(data, data + size);

    try {
        // Attempt to deserialize value from input data
        auto result = value::deserialize(input);

        if (result.has_value()) {
            // If successfully deserialized, serialize again to verify consistency
            auto serialized = result->serialize();
            (void)serialized;  // Suppress unused warning

            // Also test type conversion methods
            (void)result->name();
            (void)result->type();

            // Try to get various types (should not crash)
            (void)result->get<bool>();
            (void)result->get<int32_t>();
            (void)result->get<int64_t>();
            (void)result->get<double>();
            (void)result->get<std::string>();
        }
    } catch (const std::exception&) {
        // Exceptions are allowed for invalid input
    } catch (...) {
        // Catch any other exceptions
    }

    return 0;
}
