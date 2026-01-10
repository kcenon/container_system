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
 * @file async_coroutine_example.cpp
 * @brief Example demonstrating C++20 coroutine-based async API
 *
 * This example shows how to use the async container API with:
 * - Basic async serialization and deserialization
 * - Async file I/O operations
 * - Streaming with generators
 * - Integration with thread pool executors
 */

#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>
#include <filesystem>

#include "container.h"

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
 * @brief Helper to print error message
 */
void print_error(const std::string& message) {
    std::cout << "[ERROR] " << message << std::endl;
}

#if CONTAINER_HAS_COROUTINES

using namespace container_module::async;

/**
 * @brief Demonstrate basic async serialization
 */
task<void> demonstrate_async_serialization() {
    print_section("Async Serialization Demo");

    // Create a container with some data
    auto container = std::make_shared<value_container>();
    container->set_message_type("async_demo");
    container->set("name", std::string("John Doe"));
    container->set("age", static_cast<int32_t>(30));
    container->set("score", 95.5);
    container->set("active", true);

    std::cout << "Created container with 4 values" << std::endl;

    // Wrap in async_container
    async_container async_cont(container);

    // Async serialize
    std::cout << "Starting async serialization..." << std::endl;
    auto start = std::chrono::steady_clock::now();

#if CONTAINER_HAS_COMMON_RESULT
    auto result = co_await async_cont.serialize_async();
    if (result.is_ok()) {
        auto& bytes = result.value();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start).count();
        print_success("Serialized " + std::to_string(bytes.size()) +
                     " bytes in " + std::to_string(duration) + " us");

        // Async deserialize
        std::cout << "Starting async deserialization..." << std::endl;
        start = std::chrono::steady_clock::now();
        auto deser_result = co_await async_container::deserialize_async(bytes);
        if (deser_result.is_ok()) {
            duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start).count();
            print_success("Deserialized in " + std::to_string(duration) + " us");

            // Verify data
            auto restored = deser_result.value();
            auto name_val = restored->get_value("name");
            if (name_val && std::holds_alternative<std::string>(name_val->data)) {
                std::cout << "Verified: name = " <<
                    std::get<std::string>(name_val->data) << std::endl;
            }
        } else {
            print_error("Deserialization failed");
        }
    } else {
        print_error("Serialization failed");
    }
#else
    auto bytes = co_await async_cont.serialize_async();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count();
    print_success("Serialized " + std::to_string(bytes.size()) +
                 " bytes in " + std::to_string(duration) + " us");

    // Async deserialize
    std::cout << "Starting async deserialization..." << std::endl;
    start = std::chrono::steady_clock::now();
    auto restored = co_await async_container::deserialize_async(bytes);
    duration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count();
    print_success("Deserialized in " + std::to_string(duration) + " us");
#endif

    co_return;
}

/**
 * @brief Demonstrate async file I/O
 */
task<void> demonstrate_async_file_io() {
    print_section("Async File I/O Demo");

    // Create a container with data
    auto container = std::make_shared<value_container>();
    container->set_message_type("file_io_demo");

    // Add some test data
    for (int i = 0; i < 100; ++i) {
        container->set("key_" + std::to_string(i),
                            std::string("value_") + std::to_string(i));
    }
    std::cout << "Created container with 100 key-value pairs" << std::endl;

    async_container async_cont(container);

    // Save to file
    std::string temp_file = "/tmp/async_container_test.bin";
    std::cout << "Saving to " << temp_file << "..." << std::endl;

    auto start = std::chrono::steady_clock::now();

#if CONTAINER_HAS_COMMON_RESULT
    auto save_result = co_await async_cont.save_async(temp_file,
        [](size_t bytes, size_t total) {
            // Progress callback (optional)
        });

    if (save_result.is_ok()) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start).count();
        print_success("Saved in " + std::to_string(duration) + " us");

        // Load from file
        async_container loaded_cont;
        std::cout << "Loading from " << temp_file << "..." << std::endl;
        start = std::chrono::steady_clock::now();

        auto load_result = co_await loaded_cont.load_async(temp_file);
        if (load_result.is_ok()) {
            duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start).count();
            print_success("Loaded in " + std::to_string(duration) + " us");

            // Verify data
            auto loaded_container = loaded_cont.get_container();
            auto val = loaded_container->get_value("key_50");
            if (val && std::holds_alternative<std::string>(val->data)) {
                std::cout << "Verified: key_50 = " <<
                    std::get<std::string>(val->data) << std::endl;
            }
        } else {
            print_error("Load failed");
        }
    } else {
        print_error("Save failed");
    }
#else
    auto save_result = co_await async_cont.save_async(temp_file);
    if (save_result) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start).count();
        print_success("Saved in " + std::to_string(duration) + " us");

        // Load from file
        async_container loaded_cont;
        std::cout << "Loading from " << temp_file << "..." << std::endl;
        start = std::chrono::steady_clock::now();

        auto load_result = co_await loaded_cont.load_async(temp_file);
        if (load_result) {
            duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start).count();
            print_success("Loaded in " + std::to_string(duration) + " us");
        }
    }
#endif

    // Cleanup
    std::filesystem::remove(temp_file);
    std::cout << "Cleaned up temp file" << std::endl;

    co_return;
}

/**
 * @brief Demonstrate chunked streaming
 */
task<void> demonstrate_streaming() {
    print_section("Streaming (Generator) Demo");

    // Create a larger container
    auto container = std::make_shared<value_container>();
    container->set_message_type("streaming_demo");

    // Add more data to make streaming meaningful
    std::string large_data(10000, 'X');
    for (int i = 0; i < 50; ++i) {
        container->set("large_field_" + std::to_string(i), large_data);
    }
    std::cout << "Created container with 50 large fields" << std::endl;

    async_container async_cont(container);

    // Use generator to stream chunks
    std::cout << "Streaming serialization in 8KB chunks:" << std::endl;

    size_t chunk_count = 0;
    size_t total_bytes = 0;
    auto start = std::chrono::steady_clock::now();

    for (auto chunk : async_cont.serialize_chunked(8 * 1024)) {
        ++chunk_count;
        total_bytes += chunk.size();
        std::cout << "  Chunk " << chunk_count << ": " << chunk.size() << " bytes" << std::endl;
    }

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count();

    print_success("Streamed " + std::to_string(total_bytes) + " bytes in " +
                 std::to_string(chunk_count) + " chunks (" +
                 std::to_string(duration) + " us)");

    co_return;
}

/**
 * @brief Demonstrate executor context usage
 */
task<void> demonstrate_executor_context() {
    print_section("Executor Context Demo");

    std::cout << "Using default executor (std::thread fallback)..." << std::endl;

    auto container = std::make_shared<value_container>();
    container->set_message_type("executor_demo");
    container->set("test", std::string("value"));

    async_container async_cont(container);

    // Check if executor is available
    auto& ctx = async_executor_context::instance();
    if (ctx.has_executor()) {
        std::cout << "Custom executor is configured" << std::endl;
    } else {
        std::cout << "Using default thread-based execution" << std::endl;
    }

    // Perform async operation
    auto start = std::chrono::steady_clock::now();

#if CONTAINER_HAS_COMMON_RESULT
    auto result = co_await async_cont.serialize_async();
    if (result.is_ok()) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start).count();
        print_success("Operation completed in " + std::to_string(duration) + " us");
    }
#else
    auto bytes = co_await async_cont.serialize_async();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count();
    print_success("Operation completed in " + std::to_string(duration) + " us");
#endif

    co_return;
}

/**
 * @brief Entry point coroutine that runs all demos
 */
task<void> run_all_demos() {
    std::cout << std::string(60, '*') << std::endl;
    std::cout << "  Container System - Async Coroutine API Examples" << std::endl;
    std::cout << std::string(60, '*') << std::endl;

    co_await demonstrate_async_serialization();
    co_await demonstrate_async_file_io();
    co_await demonstrate_streaming();
    co_await demonstrate_executor_context();

    print_section("All Demos Complete");
    std::cout << "The async API provides non-blocking operations using" << std::endl;
    std::cout << "C++20 coroutines for efficient integration with" << std::endl;
    std::cout << "async I/O frameworks and event loops." << std::endl;
}

#endif // CONTAINER_HAS_COROUTINES

int main() {
#if CONTAINER_HAS_COROUTINES
    // Run the demo coroutine
    auto demo_task = run_all_demos();

    // Wait for completion (simple busy-wait for demo purposes)
    while (!demo_task.done()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "\nDemo completed successfully!" << std::endl;
#else
    std::cout << "C++20 coroutines are not available." << std::endl;
    std::cout << "Please compile with a C++20 compliant compiler:" << std::endl;
    std::cout << "  - GCC 10+ (full support in 11+)" << std::endl;
    std::cout << "  - Clang 13+" << std::endl;
    std::cout << "  - MSVC 2019 16.8+" << std::endl;
#endif

    return 0;
}
