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
        benchmark_memory_efficiency();

        std::cout << "\n=== All benchmarks completed ===" << std::endl;
    }

private:
    void benchmark_basic_operations() {
        std::cout << "\n1. Basic Operations Benchmark:" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        const int iterations = 10000;
        auto container = std::make_shared<value_container>();
        container->set_message_type("benchmark_container");

        // Benchmark: set_value operations
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            std::string key = "key_" + std::to_string(i);
            std::string value = "value_" + std::to_string(i);
            container->set(key, value);
        }
        auto end = std::chrono::high_resolution_clock::now();

        auto add_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double add_ops_per_sec = (double)iterations / add_duration.count() * 1000000;

        std::cout << "set_value operations:" << std::endl;
        std::cout << "  " << iterations << " operations in " << add_duration.count() << " us" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(2) << add_ops_per_sec << " ops/sec" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(3) << (double)add_duration.count() / iterations << " us/op" << std::endl;

        // Benchmark: get_value operations
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            std::string key = "key_" + std::to_string(i);
            auto value = container->get_value(key);
            volatile bool exists = value.has_value();
            (void)exists;
        }
        end = std::chrono::high_resolution_clock::now();

        auto get_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double get_ops_per_sec = (double)iterations / get_duration.count() * 1000000;

        std::cout << "get_value operations:" << std::endl;
        std::cout << "  " << iterations << " operations in " << get_duration.count() << " us" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(2) << get_ops_per_sec << " ops/sec" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(3) << (double)get_duration.count() / iterations << " us/op" << std::endl;
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
            auto result = container->serialize_string(value_container::serialization_format::binary);
            std::string serialized = result.is_ok() ? result.value() : "";
            auto end = std::chrono::high_resolution_clock::now();

            auto serialize_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            // Benchmark deserialization
            start = std::chrono::high_resolution_clock::now();
            auto restored = std::make_shared<value_container>(serialized);
            end = std::chrono::high_resolution_clock::now();

            auto deserialize_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            std::cout << "Container with " << size << " entries:" << std::endl;
            std::cout << "  Serialization: " << serialize_duration.count() << " us" << std::endl;
            std::cout << "  Deserialization: " << deserialize_duration.count() << " us" << std::endl;
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
                container->set("str_" + std::to_string(i), std::string("value_" + std::to_string(i)));
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "String values: " << duration.count() << " us for " << iterations << " ops" << std::endl;
        }

        // Integer values
        {
            auto container = std::make_shared<value_container>();
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < iterations; ++i) {
                container->set("int_" + std::to_string(i), static_cast<int32_t>(i));
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Integer values: " << duration.count() << " us for " << iterations << " ops" << std::endl;
        }

        // Double values
        {
            auto container = std::make_shared<value_container>();
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < iterations; ++i) {
                container->set("dbl_" + std::to_string(i), static_cast<double>(i * 1.5));
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Double values: " << duration.count() << " us for " << iterations << " ops" << std::endl;
        }

        // Boolean values
        {
            auto container = std::make_shared<value_container>();
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < iterations; ++i) {
                container->set("bool_" + std::to_string(i), i % 2 == 0);
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Boolean values: " << duration.count() << " us for " << iterations << " ops" << std::endl;
        }
    }

    void benchmark_memory_efficiency() {
        std::cout << "\n4. Memory Efficiency Benchmark:" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        const int iterations = 1000;
        auto container = std::make_shared<value_container>();

        for (int i = 0; i < iterations; ++i) {
            container->set("key_" + std::to_string(i), static_cast<int32_t>(i));
        }

        auto [heap, stack] = container->memory_stats();
        size_t footprint = container->memory_footprint();

        std::cout << "Container with " << iterations << " int values:" << std::endl;
        std::cout << "  Heap allocations: " << heap << std::endl;
        std::cout << "  Stack allocations: " << stack << std::endl;
        std::cout << "  Total memory footprint: " << footprint << " bytes" << std::endl;
        std::cout << "  Bytes per value: " << footprint / iterations << std::endl;
        std::cout << "  optimized_value size: " << sizeof(optimized_value) << " bytes" << std::endl;
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
                    container->set(key, std::string("value_" + std::to_string(i)));
                    break;
                case 1:
                    container->set(key, static_cast<int32_t>(i));
                    break;
                case 2:
                    container->set(key, static_cast<double>(i * 1.5));
                    break;
                case 3:
                    container->set(key, i % 2 == 0);
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
