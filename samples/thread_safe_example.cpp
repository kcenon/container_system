/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Container System Project
 */

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <random>
#include <mutex>
#include "container.h"

using namespace container_module;

int main() {
    std::cout << "=== Container System - Thread Safety Example ===" << std::endl;

    // 1. Multi-threaded container operations with mutex protection
    std::cout << "\n1. Multi-threaded Container Operations:" << std::endl;

    auto container = std::make_shared<value_container>();
    container->set_message_type("shared_data");
    std::mutex container_mutex;

    // Initialize some shared data using new set_value API
    container->set("counter", static_cast<int32_t>(0));
    container->set("total_operations", static_cast<int32_t>(0));

    std::cout << "Container initialized for multi-threaded access" << std::endl;

    // 2. Concurrent read/write operations with proper synchronization
    std::cout << "\n2. Concurrent Operations Test:" << std::endl;

    const int num_threads = 8;
    const int operations_per_thread = 1000;
    std::atomic<int> global_counter{0};
    std::atomic<int> completed_threads{0};

    std::vector<std::thread> workers;

    // Create worker threads
    for (int i = 0; i < num_threads; ++i) {
        workers.emplace_back([&container, &container_mutex, &global_counter, &completed_threads,
                             operations_per_thread, i]() {

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(1, 100);

            for (int op = 0; op < operations_per_thread; ++op) {
                // Simulate different types of operations
                int operation_type = dis(gen) % 4;

                switch (operation_type) {
                    case 0: { // Read operation
                        std::lock_guard<std::mutex> lock(container_mutex);
                        if (auto counter_val = container->get_value("counter")) {
                            if (auto* val = std::get_if<int32_t>(&counter_val->data)) {
                                volatile int read_val = *val;
                                (void)read_val;
                            }
                        }
                        break;
                    }
                    case 1: { // Write operation (increment counter)
                        std::lock_guard<std::mutex> lock(container_mutex);
                        if (auto current = container->get_value("counter")) {
                            if (auto* val = std::get_if<int32_t>(&current->data)) {
                                container->set("counter", static_cast<int32_t>(*val + 1));
                                global_counter.fetch_add(1, std::memory_order_relaxed);
                            }
                        }
                        break;
                    }
                    case 2: { // Add thread-specific data
                        std::lock_guard<std::mutex> lock(container_mutex);
                        std::string thread_key = "thread_" + std::to_string(i);
                        std::string thread_data = "data_from_thread_" + std::to_string(i) + "_op_" + std::to_string(op);
                        container->set(thread_key, thread_data);
                        break;
                    }
                    case 3: { // Update total operations
                        std::lock_guard<std::mutex> lock(container_mutex);
                        if (auto total_ops = container->get_value("total_operations")) {
                            if (auto* val = std::get_if<int32_t>(&total_ops->data)) {
                                container->set("total_operations", static_cast<int32_t>(*val + 1));
                            }
                        }
                        break;
                    }
                }

                // Small delay to increase chance of race conditions if not properly protected
                if (op % 100 == 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            }

            completed_threads.fetch_add(1, std::memory_order_release);
            std::cout << "Thread " << i << " completed " << operations_per_thread << " operations" << std::endl;
        });
    }

    // Wait for all threads to complete
    for (auto& worker : workers) {
        worker.join();
    }

    std::cout << "\nAll threads completed!" << std::endl;

    // 3. Verify results
    std::cout << "\n3. Results Verification:" << std::endl;

    {
        std::lock_guard<std::mutex> lock(container_mutex);
        if (auto final_counter = container->get_value("counter")) {
            if (auto* val = std::get_if<int32_t>(&final_counter->data)) {
                std::cout << "Final counter value: " << *val << std::endl;
            }
        }
        if (auto total_ops = container->get_value("total_operations")) {
            if (auto* val = std::get_if<int32_t>(&total_ops->data)) {
                std::cout << "Total operations recorded: " << *val << std::endl;
            }
        }
        std::cout << "Global counter (atomic): " << global_counter.load() << std::endl;
    }

    // 4. Performance test
    std::cout << "\n4. Performance Test:" << std::endl;

    const int perf_iterations = 10000;
    auto start_time = std::chrono::high_resolution_clock::now();

    // Sequential operations
    auto perf_container = std::make_shared<value_container>();
    perf_container->set_message_type("performance_test");

    for (int i = 0; i < perf_iterations; ++i) {
        std::string key = "perf_key_" + std::to_string(i);
        std::string value = "perf_value_" + std::to_string(i);
        perf_container->set(key, value);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    std::cout << "Sequential operations (" << perf_iterations << " ops): "
              << duration << " us" << std::endl;
    std::cout << "Average time per operation: " << (double)duration / perf_iterations << " us" << std::endl;

    // 5. Serialization test
    std::cout << "\n5. Serialization Test:" << std::endl;

    std::string serialized;
    {
        std::lock_guard<std::mutex> lock(container_mutex);
        auto result = container->serialize_string(value_container::serialization_format::binary);
        serialized = result.is_ok() ? result.value() : "";
    }

    std::cout << "Container serialized successfully" << std::endl;
    std::cout << "Serialized size: " << serialized.length() << " bytes" << std::endl;

    auto restored_container = std::make_shared<value_container>(serialized);
    std::cout << "Container restored successfully" << std::endl;

    if (auto restored_counter = restored_container->get_value("counter")) {
        if (auto* val = std::get_if<int32_t>(&restored_counter->data)) {
            std::cout << "Restored counter value: " << *val << std::endl;
        }
    }

    std::cout << "\n=== Thread Safety Example completed successfully ===" << std::endl;
    return 0;
}
