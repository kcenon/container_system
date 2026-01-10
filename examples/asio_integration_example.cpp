// BSD 3-Clause License
//
// Copyright (c) 2021-2025, kcenon
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

/**
 * @file asio_integration_example.cpp
 * @brief Example demonstrating integration with Boost.Asio / standalone Asio
 *
 * This example shows how to use container_system's async API alongside
 * Asio's async I/O facilities. The pattern demonstrated here can be
 * adapted for network applications using Asio.
 *
 * @note This example requires Asio (standalone or Boost.Asio).
 *       It is conditionally compiled based on availability.
 */

#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "container.h"

// Check for Asio availability
#if __has_include(<asio.hpp>)
    #define HAS_STANDALONE_ASIO 1
    #include <asio.hpp>
#elif __has_include(<boost/asio.hpp>)
    #define HAS_BOOST_ASIO 1
    #include <boost/asio.hpp>
    namespace asio = boost::asio;
#else
    #define HAS_ASIO 0
#endif

#if CONTAINER_HAS_COROUTINES
#include "internal/async/async.h"
#endif

using namespace container_module;

/**
 * @brief Helper to print section headers
 */
void print_section(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

/**
 * @brief Helper to print success message
 */
void print_success(const std::string& message) {
    std::cout << "[OK] " << message << std::endl;
}

/**
 * @brief Helper to print info message
 */
void print_info(const std::string& message) {
    std::cout << "[INFO] " << message << std::endl;
}

#if defined(HAS_STANDALONE_ASIO) || defined(HAS_BOOST_ASIO)

/**
 * @brief Simple message processor using Asio strand for serialization
 *
 * This class demonstrates how to integrate container_system serialization
 * with Asio's execution model.
 */
class message_processor {
public:
    explicit message_processor(asio::io_context& io_ctx)
        : io_ctx_(io_ctx)
        , strand_(asio::make_strand(io_ctx))
    {
    }

    /**
     * @brief Queue a container for async processing
     */
    void process_async(std::shared_ptr<value_container> container,
                      std::function<void(std::vector<uint8_t>)> callback) {
        // Post work to the strand for thread-safe execution
        asio::post(strand_, [this, container, callback]() {
            auto serialized = container->serialize_array();
            print_info("Processed container: " + std::to_string(serialized.size()) + " bytes");
            if (callback) {
                callback(std::move(serialized));
            }
        });
    }

    /**
     * @brief Deserialize data asynchronously
     */
    void deserialize_async(std::vector<uint8_t> data,
                          std::function<void(std::shared_ptr<value_container>)> callback) {
        asio::post(strand_, [data = std::move(data), callback]() {
            auto container = std::make_shared<value_container>(data, false);
            print_info("Deserialized container");
            if (callback) {
                callback(container);
            }
        });
    }

private:
    asio::io_context& io_ctx_;
    asio::strand<asio::io_context::executor_type> strand_;
};

/**
 * @brief Demonstrate basic Asio integration
 */
void demonstrate_basic_asio_integration() {
    print_section("Basic Asio Integration");

    asio::io_context io_ctx;
    message_processor processor(io_ctx);

    // Create a container
    auto container = std::make_shared<value_container>();
    container->set_message_type("asio_demo");
    container->set("request_id", static_cast<int32_t>(12345));
    container->set("action", std::string("process_data"));
    container->set("payload", std::string("Hello from Asio!"));

    std::cout << "Created container with request data" << std::endl;

    // Process asynchronously
    std::atomic<bool> completed{false};
    std::vector<uint8_t> result_data;

    processor.process_async(container,
        [&completed, &result_data](std::vector<uint8_t> serialized) {
            result_data = std::move(serialized);
            completed.store(true);
        });

    // Run the io_context
    std::cout << "Running Asio io_context..." << std::endl;
    io_ctx.run();

    if (completed.load()) {
        print_success("Async processing completed: " +
                     std::to_string(result_data.size()) + " bytes");
    }
}

/**
 * @brief Demonstrate timer-based scheduled processing
 */
void demonstrate_scheduled_processing() {
    print_section("Scheduled Processing with Asio Timer");

    asio::io_context io_ctx;
    asio::steady_timer timer(io_ctx);

    auto container = std::make_shared<value_container>();
    container->set_message_type("scheduled_task");
    container->set("timestamp", static_cast<int64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()));

    std::cout << "Scheduling container processing in 100ms..." << std::endl;

    timer.expires_after(std::chrono::milliseconds(100));
    timer.async_wait([container](const asio::error_code& ec) {
        if (!ec) {
            auto serialized = container->serialize_array();
            print_success("Timer triggered! Serialized " +
                         std::to_string(serialized.size()) + " bytes");
        }
    });

    io_ctx.run();
}

/**
 * @brief Demonstrate concurrent processing with work guard
 */
void demonstrate_concurrent_processing() {
    print_section("Concurrent Processing with Thread Pool");

    asio::io_context io_ctx;
    auto work_guard = asio::make_work_guard(io_ctx);

    // Start worker threads
    std::vector<std::thread> workers;
    for (int i = 0; i < 4; ++i) {
        workers.emplace_back([&io_ctx]() {
            io_ctx.run();
        });
    }

    std::cout << "Started 4 worker threads" << std::endl;

    // Submit multiple containers for processing
    std::atomic<int> completed_count{0};
    const int total_tasks = 10;

    for (int i = 0; i < total_tasks; ++i) {
        asio::post(io_ctx, [i, &completed_count, total_tasks]() {
            auto container = std::make_shared<value_container>();
            container->set_message_type("concurrent_task");
            container->set("task_id", static_cast<int32_t>(i));
            container->set("data", std::string("Task data " + std::to_string(i)));

            auto serialized = container->serialize_array();

            int count = completed_count.fetch_add(1) + 1;
            std::cout << "  Task " << i << " completed (" << count << "/" << total_tasks << ")" << std::endl;
        });
    }

    // Wait for completion
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    work_guard.reset();

    for (auto& t : workers) {
        t.join();
    }

    print_success("All " + std::to_string(total_tasks) + " tasks completed");
}

/**
 * @brief Demonstrate message queue pattern
 */
void demonstrate_message_queue() {
    print_section("Message Queue Pattern");

    asio::io_context io_ctx;
    auto work_guard = asio::make_work_guard(io_ctx);

    // Message queue with mutex protection
    std::queue<std::vector<uint8_t>> message_queue;
    std::mutex queue_mutex;
    std::atomic<bool> producer_done{false};
    std::atomic<int> consumed_count{0};

    // Producer thread
    std::thread producer([&]() {
        for (int i = 0; i < 5; ++i) {
            auto container = std::make_shared<value_container>();
            container->set_message_type("queue_message");
            container->set("seq", static_cast<int32_t>(i));
            container->set("body", std::string("Message " + std::to_string(i)));

            auto serialized = container->serialize_array();

            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                message_queue.push(std::move(serialized));
            }

            std::cout << "  Produced message " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        producer_done.store(true);
    });

    // Consumer in Asio context
    std::function<void()> consume_next;
    consume_next = [&]() {
        std::vector<uint8_t> data;
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (!message_queue.empty()) {
                data = std::move(message_queue.front());
                message_queue.pop();
            }
        }

        if (!data.empty()) {
            auto container = std::make_shared<value_container>(data, false);
            auto seq = container->get_value("seq");
            if (seq && std::holds_alternative<int32_t>(seq->data)) {
                std::cout << "  Consumed message " << std::get<int32_t>(seq->data) << std::endl;
            }
            consumed_count.fetch_add(1);
        }

        // Check if we should continue
        if (!producer_done.load() || !message_queue.empty()) {
            asio::post(io_ctx, consume_next);
        } else {
            work_guard.reset();
        }
    };

    asio::post(io_ctx, consume_next);

    // Run consumer
    std::thread consumer([&io_ctx]() {
        io_ctx.run();
    });

    producer.join();
    consumer.join();

    print_success("Processed " + std::to_string(consumed_count.load()) + " messages");
}

#endif // HAS_ASIO

#if CONTAINER_HAS_COROUTINES && (defined(HAS_STANDALONE_ASIO) || defined(HAS_BOOST_ASIO))

using namespace container_module::async;

/**
 * @brief Demonstrate mixing container coroutines with Asio
 *
 * This shows how container_system's task<T> can be used alongside
 * Asio's async operations in a hybrid approach.
 */
void demonstrate_coroutine_asio_hybrid() {
    print_section("Coroutine + Asio Hybrid Approach");

    std::cout << "This example shows how to use container_system's" << std::endl;
    std::cout << "coroutine-based async API alongside Asio." << std::endl;
    std::cout << std::endl;

    // Create container and use async API
    auto container = std::make_shared<value_container>();
    container->set_message_type("hybrid_demo");
    container->set("mode", std::string("coroutine_asio"));

    async_container async_cont(container);

    // Run coroutine-based serialization
    auto serialize_task = async_cont.serialize_async();
    while (!serialize_task.done()) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

#if CONTAINER_HAS_COMMON_RESULT
    auto result = std::move(serialize_task).get();
    if (result.is_ok()) {
        std::cout << "Coroutine serialization: " << result.value().size() << " bytes" << std::endl;

        // Now use the serialized data with Asio
        asio::io_context io_ctx;
        asio::post(io_ctx, [data = result.value()]() {
            // Simulate network send
            print_info("Would send " + std::to_string(data.size()) + " bytes over network");
        });
        io_ctx.run();
    }
#else
    auto bytes = std::move(serialize_task).get();
    std::cout << "Coroutine serialization: " << bytes.size() << " bytes" << std::endl;

    // Now use the serialized data with Asio
    asio::io_context io_ctx;
    asio::post(io_ctx, [bytes = std::move(bytes)]() {
        print_info("Would send " + std::to_string(bytes.size()) + " bytes over network");
    });
    io_ctx.run();
#endif

    print_success("Hybrid approach demonstration complete");
}

#endif // CONTAINER_HAS_COROUTINES && HAS_ASIO

int main() {
    std::cout << std::string(60, '*') << std::endl;
    std::cout << "  Container System - Asio Integration Examples" << std::endl;
    std::cout << std::string(60, '*') << std::endl;

#if defined(HAS_STANDALONE_ASIO)
    std::cout << "Using: Standalone Asio" << std::endl;
#elif defined(HAS_BOOST_ASIO)
    std::cout << "Using: Boost.Asio" << std::endl;
#else
    std::cout << "Asio not available!" << std::endl;
    std::cout << "Install standalone Asio or Boost.Asio to run this example." << std::endl;
    std::cout << std::endl;
    std::cout << "Install via vcpkg:" << std::endl;
    std::cout << "  vcpkg install asio" << std::endl;
    std::cout << "  # or" << std::endl;
    std::cout << "  vcpkg install boost-asio" << std::endl;
    return 1;
#endif

#if defined(HAS_STANDALONE_ASIO) || defined(HAS_BOOST_ASIO)
    demonstrate_basic_asio_integration();
    demonstrate_scheduled_processing();
    demonstrate_concurrent_processing();
    demonstrate_message_queue();

#if CONTAINER_HAS_COROUTINES
    demonstrate_coroutine_asio_hybrid();
#else
    std::cout << "\nNote: C++20 coroutine demos skipped (compiler support required)" << std::endl;
#endif

    print_section("All Asio Integration Examples Complete");
    std::cout << "These patterns can be adapted for real network applications" << std::endl;
    std::cout << "using TCP/UDP sockets with Asio." << std::endl;
#endif

    return 0;
}
