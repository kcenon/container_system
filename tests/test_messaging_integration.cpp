// BSD 3-Clause License
//
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <future>
#include <atomic>

#ifdef HAS_MESSAGING_FEATURES
#include "integration/messaging_integration.h"
#endif

#include "core/container.h"

using namespace container_module;

class MessagingIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup
    }

    void TearDown() override {
        // Test cleanup
    }
};

#ifdef HAS_MESSAGING_FEATURES

TEST_F(MessagingIntegrationTest, BuilderPatternBasicConstruction) {
    auto container = integration::messaging_container_builder()
        .source("client_01", "session_123")
        .target("server", "handler_01")
        .message_type("test_message")
        .set("test_key", std::string("test_value"))
        .set("numeric_key", 42)
        .set("boolean_key", true)
        .build();

    ASSERT_NE(container, nullptr);
    EXPECT_EQ(container->source_id(), "client_01");
    EXPECT_EQ(container->source_sub_id(), "session_123");
    EXPECT_EQ(container->target_id(), "server");
    EXPECT_EQ(container->target_sub_id(), "handler_01");
    EXPECT_EQ(container->message_type(), "test_message");

    // Check that values were added
    EXPECT_TRUE(container->get_variant_value("test_key").has_value());
    EXPECT_TRUE(container->get_variant_value("numeric_key").has_value());
    EXPECT_TRUE(container->get_variant_value("boolean_key").has_value());
}

TEST_F(MessagingIntegrationTest, BuilderPatternComplexTypes) {
    auto nested_container = std::make_shared<value_container>();
    nested_container->set_message_type("nested");

    auto container = integration::messaging_container_builder()
        .source("producer", "batch_01")
        .target("consumer", "worker_01")
        .message_type("complex_data")
        .set("nested_data", nested_container)
        .set("pi_value", 3.14159)
        .set("large_number", 9223372036854775807LL)
        .build();

    ASSERT_NE(container, nullptr);
    // Check that values were added
    EXPECT_TRUE(container->get_variant_value("nested_data").has_value());
    EXPECT_TRUE(container->get_variant_value("pi_value").has_value());
    EXPECT_TRUE(container->get_variant_value("large_number").has_value());

    // Verify nested container data exists
    auto nested_value = container->get_variant_value("nested_data");
    ASSERT_TRUE(nested_value.has_value());
    EXPECT_EQ(nested_value->type, value_types::bytes_value);  // Nested containers are stored as bytes
}

TEST_F(MessagingIntegrationTest, BuilderPatternFluentChaining) {
    auto builder = integration::messaging_container_builder();

    // Test fluent interface
    auto& builder_ref = builder
        .source("test_source")
        .target("test_target")
        .message_type("chain_test");

    // Should return same builder instance
    EXPECT_EQ(&builder, &builder_ref);

    auto container = builder.build();
    ASSERT_NE(container, nullptr);
    EXPECT_EQ(container->message_type(), "chain_test");
}

TEST_F(MessagingIntegrationTest, OptimizationSettings) {
    auto container1 = integration::messaging_container_builder()
        .source("perf_client")
        .target("perf_server")
        .message_type("speed_test")
        .set("data", std::string("speed_optimized"))
        .optimize_for_speed()
        .build();

    auto container2 = integration::messaging_container_builder()
        .source("memory_client")
        .target("memory_server")
        .message_type("memory_test")
        .set("data", std::string("memory_optimized"))
        // .optimize_for_memory() // Method not available
        .build();

    ASSERT_NE(container1, nullptr);
    ASSERT_NE(container2, nullptr);

    // Both containers should be valid regardless of optimization
    EXPECT_EQ(container1->message_type(), "speed_test");
    EXPECT_EQ(container2->message_type(), "memory_test");
}

TEST_F(MessagingIntegrationTest, SerializationIntegration) {
    auto container = integration::messaging_container_builder()
        .source("serialization_test")
        .target("deserialization_test")
        .message_type("serialization_message")
        .set("string_data", std::string("Hello, World!"))
        .set("int_data", 12345)
        .set("double_data", 98.76)
        .set("bool_data", false)
        .build();

    // Test enhanced serialization
    std::string serialized = integration::messaging_integration::serialize_for_messaging(container);
    EXPECT_FALSE(serialized.empty());

    // Test deserialization
    auto deserialized = integration::messaging_integration::deserialize_from_messaging(serialized);
    ASSERT_NE(deserialized, nullptr);

    EXPECT_EQ(deserialized->source_id(), "serialization_test");
    EXPECT_EQ(deserialized->target_id(), "deserialization_test");
    EXPECT_EQ(deserialized->message_type(), "serialization_message");

    // Check that deserialized values exist
    EXPECT_TRUE(deserialized->get_variant_value("string_data").has_value());
    EXPECT_TRUE(deserialized->get_variant_value("int_data").has_value());
    EXPECT_TRUE(deserialized->get_variant_value("double_data").has_value());
    EXPECT_TRUE(deserialized->get_variant_value("bool_data").has_value());
}

#ifdef HAS_PERFORMANCE_METRICS
TEST_F(MessagingIntegrationTest, PerformanceMonitoring) {
    // Reset metrics for clean test
    integration::messaging_integration::reset_metrics();

    // Simulate container creation operations
    for (int i = 0; i < 10; ++i) {
        auto container = integration::messaging_integration::create_optimized_container("perf_test");
        ASSERT_NE(container, nullptr);
    }

    // Simulate serialization operations
    auto container = integration::messaging_container_builder()
        .source("perf_test")
        .target("perf_target")
        .message_type("performance_test")
        .set("test_data", std::string("performance_monitoring"))
        .build();

    for (int i = 0; i < 10; ++i) {
        std::string serialized = integration::messaging_integration::serialize_for_messaging(container);
        EXPECT_FALSE(serialized.empty());
    }

    // Get metrics summary
    auto& metrics = integration::messaging_integration::get_metrics();
    EXPECT_GT(metrics.containers_created, 0);
    EXPECT_GT(metrics.serializations_performed, 0);
}
#endif

#ifdef HAS_EXTERNAL_INTEGRATION
TEST_F(MessagingIntegrationTest, ExternalCallbacks) {
    std::atomic<int> creation_callback_count{0};
    std::atomic<int> serialization_callback_count{0};

    // Register creation callback
    integration::messaging_integration::register_creation_callback(
        [&creation_callback_count](const std::shared_ptr<value_container>& container) {
            creation_callback_count++;
        }
    );

    // Register serialization callback
    integration::messaging_integration::register_serialization_callback(
        [&serialization_callback_count](const std::shared_ptr<value_container>& container) {
            serialization_callback_count++;
        }
    );

    // Test creation callback
    auto container = integration::messaging_integration::create_optimized_container("callback_test");
    EXPECT_EQ(creation_callback_count.load(), 1);

    // Test serialization callback
    std::string serialized = integration::messaging_integration::serialize_for_messaging(container);
    EXPECT_EQ(serialization_callback_count.load(), 1);

    // Test multiple operations
    auto container2 = integration::messaging_integration::create_optimized_container("callback_test_2");
    std::string serialized2 = integration::messaging_integration::serialize_for_messaging(container2);

    EXPECT_EQ(creation_callback_count.load(), 2);
    EXPECT_EQ(serialization_callback_count.load(), 2);

    // Cleanup
    integration::messaging_integration::unregister_callbacks();
}
#endif

TEST_F(MessagingIntegrationTest, ThreadSafetyStress) {
    const int num_threads = 4;
    const int operations_per_thread = 100;
    std::vector<std::future<void>> futures;
    std::atomic<int> success_count{0};

    for (int t = 0; t < num_threads; ++t) {
        futures.emplace_back(std::async(std::launch::async, [&, t]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                try {
                    auto container = integration::messaging_container_builder()
                        .source("thread_" + std::to_string(t))
                        .target("target_" + std::to_string(t))
                        .message_type("thread_test")
                        .set("iteration", i)
                        .set("thread_id", t)
                        .build();

                    if (container && container->get_variant_value("iteration").has_value()) {
                        success_count++;
                    }
                } catch (const std::exception&) {
                    // Expected in stress test
                }
            }
        }));
    }

    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
    }

    // Should have most operations succeed
    EXPECT_GT(success_count.load(), num_threads * operations_per_thread * 0.9);
}

TEST_F(MessagingIntegrationTest, ErrorHandling) {
    // Test empty source
    auto container1 = integration::messaging_container_builder()
        .source("")  // Empty source
        .target("test_target")
        .message_type("error_test")
        .build();

    // Should still create container but with empty source
    ASSERT_NE(container1, nullptr);
    EXPECT_EQ(container1->source_id(), "");

    // Test with invalid value types
    auto container2 = integration::messaging_container_builder()
        .source("error_test")
        .target("error_target")
        .message_type("error_handling")
        .build();

    ASSERT_NE(container2, nullptr);
    // Container should be empty (no specific check available)
}

TEST_F(MessagingIntegrationTest, LargeDataHandling) {
    std::string large_string(10000, 'A');  // 10KB string
    std::vector<int> large_vector(1000, 42);  // Large vector

    auto container = integration::messaging_container_builder()
        .source("large_data_test")
        .target("large_data_target")
        .message_type("large_data")
        .set("large_string", large_string)
        .set("item_count", static_cast<int>(large_vector.size()))
        .build();

    ASSERT_NE(container, nullptr);

    // Verify large data serialization
    std::string serialized = integration::messaging_integration::serialize_for_messaging(container);
    EXPECT_GT(serialized.size(), large_string.size());

    auto deserialized = integration::messaging_integration::deserialize_from_messaging(serialized);
    ASSERT_NE(deserialized, nullptr);

    auto string_value = deserialized->get_variant_value("large_string");
    ASSERT_TRUE(string_value.has_value());
    auto str = std::get_if<std::string>(&string_value->data);
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(*str, large_string);
}

#else
TEST_F(MessagingIntegrationTest, MessagingFeaturesDisabled) {
    // When messaging features are disabled, we should still have basic container functionality
    auto container = std::make_shared<value_container>();
    container->set_source("basic_test", "sub_test");
    container->set_target("basic_target", "sub_target");
    container->set_message_type("basic_message");

    EXPECT_EQ(container->source_id(), "basic_test");
    EXPECT_EQ(container->target_id(), "basic_target");
    EXPECT_EQ(container->message_type(), "basic_message");
}
#endif

// Benchmark test for integration features
class MessagingIntegrationBenchmark : public ::testing::Test {
protected:
    static constexpr int BENCHMARK_ITERATIONS = 1000;
};

#ifdef HAS_MESSAGING_FEATURES
TEST_F(MessagingIntegrationBenchmark, BuilderPerformance) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < BENCHMARK_ITERATIONS; ++i) {
        auto container = integration::messaging_container_builder()
            .source("benchmark_source")
            .target("benchmark_target")
            .message_type("benchmark_test")
            .set("iteration", i)
            .set("timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count())
            .build();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Log performance metrics
    double containers_per_second = (BENCHMARK_ITERATIONS * 1000000.0) / duration.count();
    std::cout << "Builder pattern performance: " << containers_per_second
              << " containers/second" << std::endl;

    // Should create at least 1000 containers per second
    EXPECT_GT(containers_per_second, 1000.0);
}

TEST_F(MessagingIntegrationBenchmark, SerializationPerformance) {
    // Create test container
    auto container = integration::messaging_container_builder()
        .source("perf_test")
        .target("perf_target")
        .message_type("serialization_benchmark")
        .set("data1", std::string("performance_test_data"))
        .set("data2", 123456789)
        .set("data3", 3.14159265359)
        .set("data4", true)
        .build();

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < BENCHMARK_ITERATIONS; ++i) {
        std::string serialized = integration::messaging_integration::serialize_for_messaging(container);
        auto deserialized = integration::messaging_integration::deserialize_from_messaging(serialized);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    double operations_per_second = (BENCHMARK_ITERATIONS * 1000000.0) / duration.count();
    std::cout << "Serialization performance: " << operations_per_second
              << " serialize+deserialize/second" << std::endl;

    // Should handle at least 100 serialization cycles per second
    EXPECT_GT(operations_per_second, 100.0);
}
#endif