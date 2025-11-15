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

#include <gtest/gtest.h>
#include <container/core/container.h>
#include <memory>
#include <string>
#include <vector>
#include <chrono>

namespace container_module
{
namespace testing
{

/**
 * @brief Base fixture for container system integration tests
 *
 * Provides common setup/teardown and helper methods for container testing.
 * Follows the pattern established in common_system, thread_system, logger_system.
 */
class ContainerSystemFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create fresh container for each test
        container = std::make_shared<value_container>();

        // Set default header values for testing
        container->set_source("test_source", "test_sub");
        container->set_target("test_target", "");
        container->set_message_type("test_message");

        start_time = std::chrono::high_resolution_clock::now();
    }

    void TearDown() override
    {
        // Cleanup
        container.reset();

        // Calculate test duration
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time);

        // Log if test took longer than expected
        if (duration.count() > 1000) {
            std::cout << "Warning: Test took " << duration.count() << "ms" << std::endl;
        }
    }

    /**
     * @brief Create a container with predefined test values
     * @param num_values Number of test values to add
     * @return Shared pointer to created container
     */
    std::shared_ptr<value_container> CreateTestContainer(size_t num_values = 10)
    {
        auto test_container = std::make_shared<value_container>();
        test_container->set_source("source", "sub");
        test_container->set_target("target", "");
        test_container->set_message_type("test_data");

        for (size_t i = 0; i < num_values; ++i) {
            std::string key = "key_" + std::to_string(i);
            std::string value = "value_" + std::to_string(i);
            test_container->add(std::make_shared<string_value>(key, value));
        }

        return test_container;
    }

    /**
     * @brief Add a string value to the container
     */
    void AddStringValue(const std::string& key, const std::string& value)
    {
        container->add(std::make_shared<string_value>(key, value));
    }

    /**
     * @brief Add a numeric value to the container
     */
    template<typename T>
    void AddNumericValue(const std::string& key, T value)
    {
        if constexpr (std::is_same_v<T, int>) {
            container->add(std::make_shared<int_value>(key, value));
        } else if constexpr (std::is_same_v<T, long long>) {
            container->add(std::make_shared<llong_value>(key, value));
        } else if constexpr (std::is_same_v<T, double>) {
            container->add(std::make_shared<double_value>(key, value));
        }
    }

    /**
     * @brief Add a boolean value to the container
     */
    void AddBoolValue(const std::string& key, bool value)
    {
        container->add(std::make_shared<bool_value>(key, value));
    }

    /**
     * @brief Add a bytes value to the container
     */
    void AddBytesValue(const std::string& key, const std::vector<uint8_t>& data)
    {
        container->add(std::make_shared<bytes_value>(key, data));
    }

    /**
     * @brief Verify container header values
     */
    void VerifyHeader(const std::string& expected_source,
                     const std::string& expected_target,
                     const std::string& expected_message_type)
    {
        EXPECT_EQ(container->source_id(), expected_source);
        EXPECT_EQ(container->target_id(), expected_target);
        EXPECT_EQ(container->message_type(), expected_message_type);
    }

    /**
     * @brief Serialize and deserialize container with full parsing
     *
     * This is the standard roundtrip test: serialize the container,
     * then deserialize it with full value parsing enabled.
     *
     * @return Fully deserialized container
     */
    std::shared_ptr<value_container> RoundTripSerialize()
    {
        std::string serialized = container->serialize();
        return std::make_shared<value_container>(serialized, false);  // false = parse all
    }

    /**
     * @brief Serialize and deserialize container with header-only parsing
     *
     * Useful for testing header preservation without the overhead
     * of parsing all values.
     *
     * @return Container with parsed header only
     */
    std::shared_ptr<value_container> RoundTripSerializeHeaderOnly()
    {
        std::string serialized = container->serialize();
        return std::make_shared<value_container>(serialized, true);  // true = header only
    }

    /**
     * @brief Measure serialization time
     * @return Duration in microseconds
     */
    int64_t MeasureSerializationTime()
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::string serialized = container->serialize();
        auto end = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();
    }

    /**
     * @brief Measure deserialization time
     * @param serialized_data Serialized container data
     * @return Duration in microseconds
     */
    int64_t MeasureDeserializationTime(const std::string& serialized_data)
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto restored = std::make_shared<value_container>(serialized_data, false);
        auto end = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();
    }

protected:
    std::shared_ptr<value_container> container;
    std::chrono::high_resolution_clock::time_point start_time;
};

} // namespace testing
} // namespace container_module
