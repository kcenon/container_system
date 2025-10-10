/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#pragma once

#include <container/core/container.h>
#include <container/values/bool_value.h>
#include <container/values/string_value.h>
#include <container/values/bytes_value.h>
#include <container/values/numeric_value.h>
#include <string>
#include <vector>
#include <random>
#include <chrono>

namespace container_module
{
namespace testing
{

/**
 * @brief Helper functions for container integration tests
 */
class TestHelpers
{
public:
    /**
     * @brief Generate random string of specified length
     */
    static std::string GenerateRandomString(size_t length)
    {
        static const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

        std::string result;
        result.reserve(length);

        for (size_t i = 0; i < length; ++i) {
            result += charset[dist(gen)];
        }

        return result;
    }

    /**
     * @brief Generate random byte array
     */
    static std::vector<uint8_t> GenerateRandomBytes(size_t size)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, 255);

        std::vector<uint8_t> result(size);
        for (auto& byte : result) {
            byte = static_cast<uint8_t>(dist(gen));
        }

        return result;
    }

    /**
     * @brief Check if value exists in container
     */
    static bool ValueExists(std::shared_ptr<value_container> container,
                           const std::string& key)
    {
        auto val = container->get_value(key);
        return val && !val->is_null();
    }

    /**
     * @brief Count total number of values in container
     */
    static size_t CountValues(std::shared_ptr<value_container> container)
    {
        size_t count = 0;
        // Note: This is a simplified count - actual implementation may vary
        // based on container's internal structure
        for (size_t i = 0; i < 1000; ++i) {
            std::string key = "key_" + std::to_string(i);
            if (ValueExists(container, key)) {
                count++;
            } else {
                break;
            }
        }
        return count;
    }

    /**
     * @brief Verify that two containers are equal
     */
    static bool ContainersEqual(std::shared_ptr<value_container> c1,
                               std::shared_ptr<value_container> c2)
    {
        // Compare headers
        if (c1->source_id() != c2->source_id() ||
            c1->target_id() != c2->target_id() ||
            c1->message_type() != c2->message_type()) {
            return false;
        }

        // Compare serialized forms (most reliable for deep comparison)
        return c1->serialize() == c2->serialize();
    }

    /**
     * @brief Create a nested container structure
     */
    static std::shared_ptr<value_container> CreateNestedContainer(size_t depth = 3)
    {
        auto root = std::make_shared<value_container>();
        root->set_message_type("root_level");

        if (depth > 0) {
            auto nested = CreateNestedContainer(depth - 1);
            std::string nested_data = nested->serialize();
            std::string key = "nested_" + std::to_string(depth);
            root->add(std::make_shared<value>(key, value_types::container_value,
                                             nested_data));
        }

        std::string key = "data_" + std::to_string(depth);
        std::string value = "level_" + std::to_string(depth);
        root->add(std::make_shared<string_value>(key, value));

        return root;
    }

    /**
     * @brief Measure operation throughput
     * @param operation Lambda function to execute
     * @param iterations Number of iterations
     * @return Operations per second
     */
    template<typename Func>
    static double MeasureThroughput(Func operation, size_t iterations = 1000)
    {
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < iterations; ++i) {
            operation();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();

        // Convert to operations per second
        return (iterations * 1000000.0) / duration;
    }

    /**
     * @brief Validate serialized data format
     */
    static bool IsValidSerializedData(const std::string& data)
    {
        // Basic validation - check for header markers
        return data.find("@header") != std::string::npos &&
               data.find("@data") != std::string::npos;
    }

    /**
     * @brief Generate test container with mixed value types
     */
    static std::shared_ptr<value_container> CreateMixedTypeContainer()
    {
        auto container = std::make_shared<value_container>();
        container->set_source("test", "");
        container->set_target("dest", "");
        container->set_message_type("mixed_types");

        // Add different value types
        container->add(std::make_shared<string_value>("str_val", "test_string"));
        container->add(std::make_shared<int_value>("int_val", 42));
        container->add(std::make_shared<llong_value>("long_val", 9223372036854775807LL));
        container->add(std::make_shared<double_value>("double_val", 3.14159));
        container->add(std::make_shared<bool_value>("bool_val", true));

        std::vector<uint8_t> bytes = {0x01, 0x02, 0x03, 0x04};
        container->add(std::make_shared<bytes_value>("bytes_val", bytes));

        return container;
    }

    /**
     * @brief Calculate serialization overhead percentage
     */
    static double CalculateSerializationOverhead(
        std::shared_ptr<value_container> container)
    {
        std::string serialized = container->serialize();
        // Estimate raw data size (simplified)
        size_t raw_size = 100; // Header estimate

        // This is a simplified calculation
        return ((double)(serialized.size() - raw_size) / raw_size) * 100.0;
    }

    /**
     * @brief Create container with specific size
     */
    static std::shared_ptr<value_container> CreateContainerWithSize(
        size_t target_bytes)
    {
        auto container = std::make_shared<value_container>();
        container->set_message_type("sized_container");

        // Add string values until we reach target size
        size_t current_size = 0;
        size_t counter = 0;

        while (current_size < target_bytes) {
            std::string key = "key_" + std::to_string(counter);
            std::string value = GenerateRandomString(100);
            container->add(std::make_shared<string_value>(key, value));

            current_size = container->serialize().size();
            counter++;
        }

        return container;
    }
};

} // namespace testing
} // namespace container_module
