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

#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <atomic>
#include <map>
#include <sstream>
#include <iomanip>
#include <condition_variable>

#include "container.h"

using namespace container_module;

/**
 * @brief Advanced Container System Example
 *
 * Demonstrates comprehensive usage of the container system including:
 * - Multi-threaded producer-consumer patterns
 * - Complex data structures
 * - Performance monitoring
 * - Error handling
 * - Serialization/deserialization
 */
class AdvancedContainerExample {
private:
    std::atomic<bool> running_{true};
    std::atomic<int> processed_containers_{0};
    std::atomic<int> processed_bytes_{0};

    // Statistics
    struct Statistics {
        std::atomic<int> created{0};
        std::atomic<int> serialized{0};
        std::atomic<int> deserialized{0};
        std::atomic<int> errors{0};
        std::chrono::steady_clock::time_point start_time;
    } stats_;

public:
    AdvancedContainerExample() {
        stats_.start_time = std::chrono::steady_clock::now();
        std::cout << "=== Advanced Container System Example ===" << std::endl;
        std::cout << "Using variant-based API (set_value/get_value)" << std::endl;
        std::cout << "===========================================" << std::endl;
    }

    ~AdvancedContainerExample() {
        running_ = false;
        print_final_statistics();
    }

    /**
     * @brief Demonstrates basic container operations using new API
     */
    void demonstrate_basic_operations() {
        std::cout << "\n--- Basic Container Operations ---" << std::endl;

        // Create a container with various value types using set_value API
        auto container = std::make_shared<value_container>();
        container->set_source("example_client", "session_001");
        container->set_target("example_server", "processor_main");
        container->set_message_type("user_profile_update");

        // Add different types of values using set_value
        container->set_value("username", std::string("john_doe"));
        container->set_value("user_id", static_cast<int32_t>(12345));
        container->set_value("account_balance", 1500.75);
        container->set_value("is_premium", true);
        container->set_value("last_login", static_cast<int64_t>(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()));

        // Display container information
        std::cout << "Container created:" << std::endl;
        std::cout << "  Source: " << container->source_id() << "/" << container->source_sub_id() << std::endl;
        std::cout << "  Target: " << container->target_id() << "/" << container->target_sub_id() << std::endl;
        std::cout << "  Type: " << container->message_type() << std::endl;

        // Demonstrate value access using get_value
        if (auto username_value = container->get_value("username")) {
            if (auto* str = std::get_if<std::string>(&username_value->data)) {
                std::cout << "  Username: " << *str << std::endl;
            }
        }

        if (auto balance_value = container->get_value("account_balance")) {
            if (auto* val = std::get_if<double>(&balance_value->data)) {
                std::cout << "  Balance: $" << std::fixed << std::setprecision(2) << *val << std::endl;
            }
        }

        // Demonstrate serialization
        std::string serialized = container->serialize();
        std::cout << "  Serialized size: " << serialized.size() << " bytes" << std::endl;

        // Demonstrate deserialization
        auto deserialized = std::make_shared<value_container>(serialized);
        std::cout << "  Deserialization successful" << std::endl;

        stats_.created++;
        stats_.serialized++;
        stats_.deserialized++;
    }

    /**
     * @brief Demonstrates multi-threaded producer-consumer pattern
     */
    void demonstrate_multithreaded_operations() {
        std::cout << "\n--- Multi-threaded Operations ---" << std::endl;

        const int num_producers = 2;
        const int num_consumers = 2;
        const int items_per_producer = 500;

        // Shared queue simulation using vector with mutex
        std::vector<std::shared_ptr<value_container>> shared_queue;
        std::mutex queue_mutex;
        std::condition_variable cv;
        std::atomic<bool> producers_done{false};

        // Producer threads
        std::vector<std::thread> producer_threads;
        for (int p = 0; p < num_producers; ++p) {
            producer_threads.emplace_back([&, p]() {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(1, 1000);

                for (int i = 0; i < items_per_producer; ++i) {
                    // Create container using set_value API
                    auto container = std::make_shared<value_container>();
                    container->set_source("producer_" + std::to_string(p), "thread_" + std::to_string(p));
                    container->set_target("consumer_pool", "any_available");
                    container->set_message_type("work_item");

                    container->set_value("producer_id", static_cast<int32_t>(p));
                    container->set_value("item_id", static_cast<int32_t>(i));
                    container->set_value("random_value", static_cast<int32_t>(dis(gen)));
                    container->set_value("timestamp", static_cast<int64_t>(
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch()).count()));

                    // Add to queue
                    {
                        std::lock_guard<std::mutex> lock(queue_mutex);
                        shared_queue.push_back(container);
                    }
                    cv.notify_one();

                    stats_.created++;

                    // Small delay to simulate work
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                }
            });
        }

        // Consumer threads
        std::vector<std::thread> consumer_threads;
        for (int c = 0; c < num_consumers; ++c) {
            consumer_threads.emplace_back([&, c]() {
                int items_processed = 0;

                while (running_) {
                    std::shared_ptr<value_container> container;

                    // Get item from queue
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        cv.wait(lock, [&]() { return !shared_queue.empty() || producers_done.load(); });

                        if (!shared_queue.empty()) {
                            container = shared_queue.back();
                            shared_queue.pop_back();
                        } else if (producers_done.load()) {
                            break;
                        }
                    }

                    if (container) {
                        // Process container (serialize/deserialize simulation)
                        std::string serialized = container->serialize();
                        processed_bytes_ += serialized.size();

                        auto processed = std::make_shared<value_container>(serialized);
                        items_processed++;
                        processed_containers_++;
                        stats_.serialized++;
                        stats_.deserialized++;

                        // Simulate processing time
                        std::this_thread::sleep_for(std::chrono::microseconds(50));
                    }
                }

                std::cout << "  Consumer " << c << " processed " << items_processed << " items" << std::endl;
            });
        }

        // Wait for producers to finish
        for (auto& thread : producer_threads) {
            thread.join();
        }

        producers_done = true;
        cv.notify_all();

        // Wait for consumers to finish
        for (auto& thread : consumer_threads) {
            thread.join();
        }

        std::cout << "Multi-threaded processing completed:" << std::endl;
        std::cout << "  Total items processed: " << processed_containers_.load() << std::endl;
        std::cout << "  Total bytes processed: " << processed_bytes_.load() << std::endl;
        std::cout << "  Remaining in queue: " << shared_queue.size() << std::endl;
    }

    /**
     * @brief Demonstrates error handling scenarios
     */
    void demonstrate_error_handling() {
        std::cout << "\n--- Error Handling ---" << std::endl;

        // Test edge cases
        std::cout << "Testing edge cases:" << std::endl;

        // Empty container serialization
        auto empty_container = std::make_shared<value_container>();
        std::string empty_serialized = empty_container->serialize();
        auto empty_deserialized = std::make_shared<value_container>(empty_serialized);
        std::cout << "  - Empty container serialization/deserialization works" << std::endl;

        // Large value handling
        std::string large_string(10000, 'A');
        auto large_container = std::make_shared<value_container>();
        large_container->set_message_type("large_data_test");
        large_container->set_value("large_data", large_string);

        std::string large_serialized = large_container->serialize();
        auto large_deserialized = std::make_shared<value_container>(large_serialized);

        if (auto recovered_value = large_deserialized->get_value("large_data")) {
            if (auto* str = std::get_if<std::string>(&recovered_value->data)) {
                if (*str == large_string) {
                    std::cout << "  - Large data handling works (" << large_string.size() << " bytes)" << std::endl;
                }
            }
        }

        stats_.created += 2;
        stats_.serialized += 2;
        stats_.deserialized += 2;
    }

    /**
     * @brief Demonstrates performance scenarios
     */
    void demonstrate_performance_scenarios() {
        std::cout << "\n--- Performance Scenarios ---" << std::endl;

        // Scenario 1: High-frequency small messages
        std::cout << "Scenario 1: High-frequency small messages" << std::endl;

        const int small_message_count = 10000;
        auto start_time = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < small_message_count; ++i) {
            auto container = std::make_shared<value_container>();
            container->set_source("high_freq_client", "session_" + std::to_string(i % 100));
            container->set_target("high_freq_server", "handler");
            container->set_message_type("ping");

            container->set_value("sequence", static_cast<int32_t>(i));
            container->set_value("timestamp", static_cast<int64_t>(
                std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count()));

            // Quick serialization test
            container->serialize();
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        double rate = (small_message_count * 1000000.0) / duration.count();
        std::cout << "  Rate: " << std::fixed << std::setprecision(2) << rate << " messages/second" << std::endl;

        // Scenario 2: Low-frequency large messages
        std::cout << "Scenario 2: Low-frequency large messages" << std::endl;

        const int large_message_count = 100;
        start_time = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < large_message_count; ++i) {
            auto container = std::make_shared<value_container>();
            container->set_source("large_msg_client", "upload_session");
            container->set_target("large_msg_server", "file_handler");
            container->set_message_type("file_upload");

            // Simulate large file data using string
            std::string file_data(50000, static_cast<char>(i % 256));
            container->set_value("file_content", file_data);
            container->set_value("filename", std::string("large_file_" + std::to_string(i) + ".dat"));
            container->set_value("file_size", static_cast<int32_t>(file_data.size()));

            // Serialization test
            std::string serialized = container->serialize();
            processed_bytes_ += serialized.size();
        }

        end_time = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        double large_rate = (large_message_count * 1000.0) / duration.count();
        std::cout << "  Rate: " << std::fixed << std::setprecision(2) << large_rate << " large messages/second" << std::endl;
        std::cout << "  Data processed: " << (processed_bytes_.load() / 1024 / 1024) << " MB" << std::endl;

        stats_.created += small_message_count + large_message_count;
        stats_.serialized += small_message_count + large_message_count;
    }

    /**
     * @brief Prints final statistics
     */
    void print_final_statistics() {
        auto end_time = std::chrono::steady_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - stats_.start_time);

        std::cout << "\n=== Final Statistics ===" << std::endl;
        std::cout << "Total runtime: " << total_duration.count() << " ms" << std::endl;
        std::cout << "Containers created: " << stats_.created.load() << std::endl;
        std::cout << "Serializations: " << stats_.serialized.load() << std::endl;
        std::cout << "Deserializations: " << stats_.deserialized.load() << std::endl;
        std::cout << "Errors encountered: " << stats_.errors.load() << std::endl;
        std::cout << "Total bytes processed: " << processed_bytes_.load() << std::endl;

        if (total_duration.count() > 0) {
            double containers_per_sec = (stats_.created.load() * 1000.0) / total_duration.count();
            double serializations_per_sec = (stats_.serialized.load() * 1000.0) / total_duration.count();

            std::cout << "Average creation rate: " << std::fixed << std::setprecision(2)
                      << containers_per_sec << " containers/second" << std::endl;
            std::cout << "Average serialization rate: " << serializations_per_sec
                      << " operations/second" << std::endl;
        }
        std::cout << "========================" << std::endl;
    }

    /**
     * @brief Runs all demonstrations
     */
    void run_all_demonstrations() {
        try {
            demonstrate_basic_operations();
            demonstrate_multithreaded_operations();
            demonstrate_error_handling();
            demonstrate_performance_scenarios();

        } catch (const std::exception& e) {
            std::cerr << "Exception in demonstration: " << e.what() << std::endl;
            stats_.errors++;
        }
    }
};

int main() {
    try {
        AdvancedContainerExample example;
        example.run_all_demonstrations();

        std::cout << "\nAdvanced Container System Example completed successfully!" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
