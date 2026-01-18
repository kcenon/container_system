/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, kcenon
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

/**
 * @file service_test.cpp
 * @brief Integration tests for gRPC server and client
 *
 * Tests verify end-to-end communication between gRPC client and server,
 * including server lifecycle, unary calls, and error handling.
 */

#include <gtest/gtest.h>

#include "client/grpc_client.h"
#include "server/grpc_server.h"
#include "core/container.h"

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <thread>

using namespace container_grpc;
using namespace container_module;

/**
 * Test fixture for gRPC service integration tests
 */
class GrpcServiceTest : public ::testing::Test {
protected:
    static constexpr const char* TEST_ADDRESS = "127.0.0.1:50099";
    static constexpr int STARTUP_TIMEOUT_MS = 5000;  // Timeout for server startup
    static constexpr int SHUTDOWN_WAIT_MS = 500;

    void SetUp() override {
        // Ensure clean state
        server_.reset();
        client_.reset();
    }

    void TearDown() override {
        // Clean shutdown
        if (client_) {
            client_.reset();
        }
        if (server_) {
            server_->stop(SHUTDOWN_WAIT_MS);
            server_.reset();
        }
        // Allow port to be released (acceptable grace period)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    /**
     * Wait for server to be ready using polling instead of fixed sleep
     */
    bool WaitForServerReady(int timeout_ms = STARTUP_TIMEOUT_MS) {
        auto start = std::chrono::steady_clock::now();
        while (!server_->is_running()) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count();
            if (elapsed > timeout_ms) {
                return false;
            }
            std::this_thread::yield();
        }
        return true;
    }

    void StartServer() {
        server_ = std::make_unique<grpc_server>(TEST_ADDRESS);
        ASSERT_TRUE(server_->start());
        ASSERT_TRUE(WaitForServerReady()) << "Server did not become ready within timeout";
    }

    void StartServerWithProcessor(container_processor processor) {
        server_ = std::make_unique<grpc_server>(TEST_ADDRESS);
        server_->set_processor(std::move(processor));
        ASSERT_TRUE(server_->start());
        ASSERT_TRUE(WaitForServerReady()) << "Server did not become ready within timeout";
    }

    void ConnectClient() {
        client_ = std::make_unique<grpc_client>(TEST_ADDRESS);
    }

    std::unique_ptr<grpc_server> server_;
    std::unique_ptr<grpc_client> client_;
};

// =============================================================================
// Server Lifecycle Tests
// =============================================================================

TEST_F(GrpcServiceTest, ServerStartAndStop) {
    server_ = std::make_unique<grpc_server>(TEST_ADDRESS);

    EXPECT_FALSE(server_->is_running());
    ASSERT_TRUE(server_->start());
    EXPECT_TRUE(server_->is_running());
    EXPECT_EQ(server_->address(), TEST_ADDRESS);

    server_->stop(SHUTDOWN_WAIT_MS);
    EXPECT_FALSE(server_->is_running());
}

TEST_F(GrpcServiceTest, ServerWithConfig) {
    server_config config;
    config.address = TEST_ADDRESS;
    config.max_receive_message_size = 32 * 1024 * 1024;
    config.max_send_message_size = 32 * 1024 * 1024;

    server_ = std::make_unique<grpc_server>(config);
    ASSERT_TRUE(server_->start());
    EXPECT_TRUE(server_->is_running());
}

TEST_F(GrpcServiceTest, ServerRequestCounting) {
    StartServer();
    ConnectClient();

    EXPECT_EQ(server_->request_count(), 0u);
    EXPECT_EQ(server_->error_count(), 0u);

    auto container = std::make_shared<value_container>();
    container->set_message_type("test");

    auto result = client_->process(container);
    EXPECT_TRUE(result.success);
    EXPECT_GE(server_->request_count(), 1u);
}

// =============================================================================
// Client Configuration Tests
// =============================================================================

TEST_F(GrpcServiceTest, ClientWithAddress) {
    StartServer();

    grpc_client client(TEST_ADDRESS);
    EXPECT_EQ(client.target(), TEST_ADDRESS);
}

TEST_F(GrpcServiceTest, ClientWithConfig) {
    StartServer();

    client_config config;
    config.target_address = TEST_ADDRESS;
    config.timeout = std::chrono::milliseconds(5000);
    config.max_retries = 2;
    config.client_id = "test_client";

    grpc_client client(config);
    EXPECT_EQ(client.target(), TEST_ADDRESS);
    EXPECT_EQ(client.timeout(), std::chrono::milliseconds(5000));
}

TEST_F(GrpcServiceTest, ClientTimeout) {
    StartServer();
    ConnectClient();

    auto initial = client_->timeout();
    client_->set_timeout(std::chrono::milliseconds(10000));
    EXPECT_EQ(client_->timeout(), std::chrono::milliseconds(10000));
}

// =============================================================================
// Unary RPC Tests
// =============================================================================

TEST_F(GrpcServiceTest, ProcessEmptyContainer) {
    StartServer();
    ConnectClient();

    auto container = std::make_shared<value_container>();
    container->set_message_type("empty_test");

    auto result = client_->process(container);

    EXPECT_TRUE(result.success);
    ASSERT_NE(result.value, nullptr);
    EXPECT_EQ(result.value->message_type(), "empty_test");
}

TEST_F(GrpcServiceTest, ProcessContainerWithValues) {
    StartServer();
    ConnectClient();

    auto container = std::make_shared<value_container>();
    container->set_source("client", "session1");
    container->set_target("server", "handler1");
    container->set_message_type("data_request");
    container->set("count", 42);
    container->set("name", std::string("test_name"));
    container->set("flag", true);

    auto result = client_->process(container);

    EXPECT_TRUE(result.success);
    ASSERT_NE(result.value, nullptr);

    // Verify values are preserved
    auto count = result.value->get_variant_value("count");
    ASSERT_TRUE(count.has_value());
    EXPECT_EQ(std::get<int>(count->data), 42);

    auto name = result.value->get_variant_value("name");
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(std::get<std::string>(name->data), "test_name");
}

TEST_F(GrpcServiceTest, ProcessWithCustomProcessor) {
    // Set up processor that modifies containers
    StartServerWithProcessor([](auto container) {
        container->set("processed", true);
        container->set("timestamp",
                        static_cast<int64_t>(
                            std::chrono::system_clock::now()
                                .time_since_epoch()
                                .count()));
        return container;
    });

    ConnectClient();

    auto container = std::make_shared<value_container>();
    container->set_message_type("process_me");

    auto result = client_->process(container);

    EXPECT_TRUE(result.success);
    ASSERT_NE(result.value, nullptr);

    auto processed = result.value->get_variant_value("processed");
    ASSERT_TRUE(processed.has_value());
    EXPECT_TRUE(std::get<bool>(processed->data));

    auto timestamp = result.value->get_variant_value("timestamp");
    EXPECT_TRUE(timestamp.has_value());
}

TEST_F(GrpcServiceTest, SendContainer) {
    StartServer();
    ConnectClient();

    auto container = std::make_shared<value_container>();
    container->set_message_type("send_test");
    container->set("data", std::string("test data"));

    auto result = client_->send(container);

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.error_message.empty());
}

// =============================================================================
// Ping and Health Tests
// =============================================================================

TEST_F(GrpcServiceTest, PingServer) {
    StartServer();
    ConnectClient();

    EXPECT_TRUE(client_->ping());
}

TEST_F(GrpcServiceTest, GetStreamStatus) {
    StartServer();
    ConnectClient();

    auto status = client_->get_status();

    ASSERT_TRUE(status.has_value());
    // Status returns (messages_sent, messages_received)
    EXPECT_GE(status->first, 0);
    EXPECT_GE(status->second, 0);
}

// =============================================================================
// Error Handling Tests
// =============================================================================

TEST_F(GrpcServiceTest, ClientWithoutServer) {
    // Connect to non-existent server
    grpc_client client("127.0.0.1:59999");

    auto container = std::make_shared<value_container>();
    container->set_message_type("test");

    auto result = client.process(container);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_message.empty());
}

TEST_F(GrpcServiceTest, ProcessNullContainer) {
    StartServer();
    ConnectClient();

    auto result = client_->process(nullptr);

    EXPECT_FALSE(result.success);
}

// =============================================================================
// Concurrent Request Tests
// =============================================================================

TEST_F(GrpcServiceTest, ConcurrentRequests) {
    StartServer();

    constexpr int NUM_CLIENTS = 5;
    constexpr int REQUESTS_PER_CLIENT = 10;

    std::vector<std::future<int>> futures;

    for (int client_id = 0; client_id < NUM_CLIENTS; ++client_id) {
        futures.push_back(std::async(std::launch::async, [this, client_id]() {
            grpc_client client(TEST_ADDRESS);
            int success_count = 0;

            for (int req = 0; req < REQUESTS_PER_CLIENT; ++req) {
                auto container = std::make_shared<value_container>();
                container->set_message_type("concurrent_test");
                container->set("client_id", client_id);
                container->set("request_id", req);

                auto result = client.process(container);
                if (result.success) {
                    ++success_count;
                }
            }

            return success_count;
        }));
    }

    int total_success = 0;
    for (auto& future : futures) {
        total_success += future.get();
    }

    EXPECT_EQ(total_success, NUM_CLIENTS * REQUESTS_PER_CLIENT);
}

// =============================================================================
// Large Message Tests
// =============================================================================

TEST_F(GrpcServiceTest, ProcessLargeContainer) {
    StartServer();
    ConnectClient();

    auto container = std::make_shared<value_container>();
    container->set_message_type("large_test");

    // Add 100 values
    for (int i = 0; i < 100; ++i) {
        container->set("int_" + std::to_string(i), i);
        container->set("str_" + std::to_string(i),
                        std::string(100, static_cast<char>('A' + (i % 26))));
    }

    auto result = client_->process(container);

    EXPECT_TRUE(result.success);
    ASSERT_NE(result.value, nullptr);

    // Verify some values
    auto int_50 = result.value->get_variant_value("int_50");
    ASSERT_TRUE(int_50.has_value());
    EXPECT_EQ(std::get<int>(int_50->data), 50);
}

TEST_F(GrpcServiceTest, ProcessContainerWithBinaryData) {
    StartServer();
    ConnectClient();

    auto container = std::make_shared<value_container>();
    container->set_message_type("binary_test");

    // Add 64KB of binary data
    std::vector<uint8_t> binary_data(64 * 1024);
    for (size_t i = 0; i < binary_data.size(); ++i) {
        binary_data[i] = static_cast<uint8_t>(i % 256);
    }
    container->set("binary", binary_data);

    auto result = client_->process(container);

    EXPECT_TRUE(result.success);
    ASSERT_NE(result.value, nullptr);

    auto restored = result.value->get_variant_value("binary");
    ASSERT_TRUE(restored.has_value());
    EXPECT_EQ(std::get<std::vector<uint8_t>>(restored->data).size(),
              binary_data.size());
}

// =============================================================================
// Nested Container Tests
// =============================================================================

TEST_F(GrpcServiceTest, ProcessNestedContainers) {
    StartServer();
    ConnectClient();

    auto inner = std::make_shared<value_container>();
    inner->set_message_type("inner");
    inner->set("inner_val", 999);

    auto container = std::make_shared<value_container>();
    container->set_message_type("outer");
    container->set("nested", inner);

    auto result = client_->process(container);

    EXPECT_TRUE(result.success);
    ASSERT_NE(result.value, nullptr);

    auto nested = result.value->get_variant_value("nested");
    ASSERT_TRUE(nested.has_value());

    auto nested_container =
        std::get<std::shared_ptr<value_container>>(nested->data);
    ASSERT_NE(nested_container, nullptr);
    EXPECT_EQ(nested_container->message_type(), "inner");

    auto inner_val = nested_container->get_variant_value("inner_val");
    ASSERT_TRUE(inner_val.has_value());
    EXPECT_EQ(std::get<int>(inner_val->data), 999);
}

// =============================================================================
// Batch Operations Tests
// =============================================================================

TEST_F(GrpcServiceTest, SendBatchContainers) {
    StartServer();
    ConnectClient();

    std::vector<std::shared_ptr<value_container>> containers;
    for (int i = 0; i < 5; ++i) {
        auto container = std::make_shared<value_container>();
        container->set_message_type("batch_item_" + std::to_string(i));
        container->set("index", i);
        containers.push_back(container);
    }

    auto result = client_->send_batch(containers);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.value.size(), containers.size());
}

// =============================================================================
// Main
// =============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
