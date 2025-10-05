/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Container System Project
 */

#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <iomanip>
#include <thread>
#include "container.h"

using namespace container_module;

class performance_benchmark {
public:
    void run_all_benchmarks() {
        std::cout << "=== Container System - Performance Benchmark ===" << std::endl;

        benchmark_basic_operations();
        benchmark_serialization();
        benchmark_value_types();

        std::cout << "\n=== All benchmarks completed ===" << std::endl;
    }

private:
    void benchmark_basic_operations() {
        std::cout << "\n1. Basic Operations Benchmark:" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        const int iterations = 10000;
        auto container = std::make_shared<value_container>();
        container->set_message_type("benchmark_container");

        // Benchmark: Add operations
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            std::string key = "key_" + std::to_string(i);
            std::string value = "value_" + std::to_string(i);
            container->add(std::make_shared<string_value>(key, value));
        }
        auto end = std::chrono::high_resolution_clock::now();

        auto add_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double add_ops_per_sec = (double)iterations / add_duration.count() * 1000000;

        std::cout << "Add operations:" << std::endl;
        std::cout << "  " << iterations << " operations in " << add_duration.count() << " μs" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(2) << add_ops_per_sec << " ops/sec" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(3) << (double)add_duration.count() / iterations << " μs/op" << std::endl;

        // Benchmark: Get operations
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            std::string key = "key_" + std::to_string(i);
            auto value = container->get_value(key);
            volatile bool exists = (value != nullptr);
            (void)exists;
        }
        end = std::chrono::high_resolution_clock::now();

        auto get_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double get_ops_per_sec = (double)iterations / get_duration.count() * 1000000;

        std::cout << "Get operations:" << std::endl;
        std::cout << "  " << iterations << " operations in " << get_duration.count() << " μs" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(2) << get_ops_per_sec << " ops/sec" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(3) << (double)get_duration.count() / iterations << " μs/op" << std::endl;
    }

    void benchmark_serialization() {
        std::cout << "\n2. Serialization Benchmark:" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        // Create containers of different sizes
        std::vector<int> sizes = {100, 1000, 5000};

        for (int size : sizes) {
            auto container = create_test_container(size);

            // Benchmark serialization
            auto start = std::chrono::high_resolution_clock::now();
            std::string serialized = container->serialize();
            auto end = std::chrono::high_resolution_clock::now();

            auto serialize_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            // Benchmark deserialization
            start = std::chrono::high_resolution_clock::now();
            auto restored = std::make_shared<value_container>(serialized);
            end = std::chrono::high_resolution_clock::now();

            auto deserialize_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            std::cout << "Container with " << size << " entries:" << std::endl;
            std::cout << "  Serialization: " << serialize_duration.count() << " μs" << std::endl;
            std::cout << "  Deserialization: " << deserialize_duration.count() << " μs" << std::endl;
            std::cout << "  Serialized size: " << serialized.size() << " bytes" << std::endl;
            std::cout << "  Bytes per entry: " << serialized.size() / size << std::endl;
        }
    }

    void benchmark_value_types() {
        std::cout << "\n3. Value Types Benchmark:" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        const int iterations = 10000;

        // String values
        {
            auto container = std::make_shared<value_container>();
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < iterations; ++i) {
                container->add(std::make_shared<string_value>("str_" + std::to_string(i), "value_" + std::to_string(i)));
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "String values: " << duration.count() << " μs for " << iterations << " ops" << std::endl;
        }

        // Integer values
        {
            auto container = std::make_shared<value_container>();
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < iterations; ++i) {
                container->add(std::make_shared<int_value>("int_" + std::to_string(i), i));
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Integer values: " << duration.count() << " μs for " << iterations << " ops" << std::endl;
        }

        // Double values
        {
            auto container = std::make_shared<value_container>();
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < iterations; ++i) {
                container->add(std::make_shared<double_value>("dbl_" + std::to_string(i), i * 1.5));
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Double values: " << duration.count() << " μs for " << iterations << " ops" << std::endl;
        }

        // Boolean values
        {
            auto container = std::make_shared<value_container>();
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < iterations; ++i) {
                container->add(std::make_shared<bool_value>("bool_" + std::to_string(i), i % 2 == 0));
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Boolean values: " << duration.count() << " μs for " << iterations << " ops" << std::endl;
        }
    }

    std::shared_ptr<value_container> create_test_container(int size) {
        auto container = std::make_shared<value_container>();
        container->set_message_type("test_container");

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> type_dist(0, 3);

        for (int i = 0; i < size; ++i) {
            std::string key = "key_" + std::to_string(i);
            int value_type = type_dist(gen);

            switch (value_type) {
                case 0:
                    container->add(std::make_shared<string_value>(key, "value_" + std::to_string(i)));
                    break;
                case 1:
                    container->add(std::make_shared<int_value>(key, i));
                    break;
                case 2:
                    container->add(std::make_shared<double_value>(key, i * 1.5));
                    break;
                case 3:
                    container->add(std::make_shared<bool_value>(key, i % 2 == 0));
                    break;
            }
        }

        return container;
    }
};

int main() {
    performance_benchmark bench;
    bench.run_all_benchmarks();
    return 0;
}
