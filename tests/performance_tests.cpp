// BSD 3-Clause License
//
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this
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
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <future>
#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <thread>
#include <vector>

#include "container/core/container.h"

#ifdef HAS_MESSAGING_FEATURES
#include "container/integration/messaging_integration.h"
#endif

using namespace container_module;

class PerformanceTest : public ::testing::Test {
protected:
  // CI-friendly iteration counts to prevent timeouts
  // Can be overridden via environment variable for local benchmarking
  static constexpr int WARM_UP_ITERATIONS = 10;
  static constexpr int BENCHMARK_ITERATIONS = 1000; // Reduced from 10000 for CI
  static constexpr int STRESS_ITERATIONS = 10000; // Reduced from 100000 for CI

  void SetUp() override {
    // Warm up the system
    for (int i = 0; i < WARM_UP_ITERATIONS; ++i) {
      auto container = std::make_shared<value_container>();
      container->set_message_type("warmup");
    }
  }

  // Helper function to measure execution time
  template <typename Func> std::chrono::microseconds measure_time(Func &&func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  }

  // Helper function to adjust threshold based on sanitizer presence
  double adjust_threshold_for_sanitizers(double base_threshold) {
    const char *tsan_opts = std::getenv("TSAN_OPTIONS");
    const char *asan_opts = std::getenv("ASAN_OPTIONS");
    const char *ubsan_opts = std::getenv("UBSAN_OPTIONS");

    if (tsan_opts || asan_opts || ubsan_opts) {
      // Sanitizers can slow down execution by 2-10x
      // Use more conservative threshold
      return base_threshold * 0.1; // 10% of normal threshold
    }
    return base_threshold;
  }

  // Helper function to calculate statistics
  struct Statistics {
    double mean;
    double median;
    double min;
    double max;
    double std_dev;
  };

  Statistics calculate_stats(const std::vector<double> &data) {
    Statistics stats{};
    if (data.empty())
      return stats;

    auto sorted_data = data;
    std::sort(sorted_data.begin(), sorted_data.end());

    stats.min = sorted_data.front();
    stats.max = sorted_data.back();
    stats.median = sorted_data[sorted_data.size() / 2];

    stats.mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();

    double variance = 0.0;
    for (double value : data) {
      variance += (value - stats.mean) * (value - stats.mean);
    }
    stats.std_dev = std::sqrt(variance / data.size());

    return stats;
  }

  void print_performance_report(const std::string &test_name,
                                const Statistics &stats,
                                const std::string &unit = "ops/sec") {
    std::cout << "\n=== " << test_name
              << " Performance Report ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Mean: " << stats.mean << " " << unit << std::endl;
    std::cout << "Median: " << stats.median << " " << unit << std::endl;
    std::cout << "Min: " << stats.min << " " << unit << std::endl;
    std::cout << "Max: " << stats.max << " " << unit << std::endl;
    std::cout << "Std Dev: " << stats.std_dev << " " << unit << std::endl;
    std::cout << "=======================================" << std::endl;
  }
};

TEST_F(PerformanceTest, ContainerCreationPerformance) {
  std::vector<double> creation_rates;
  const int num_runs = 10;

  for (int run = 0; run < num_runs; ++run) {
    auto duration = measure_time([&]() {
      for (int i = 0; i < BENCHMARK_ITERATIONS; ++i) {
        auto container = std::make_shared<value_container>();
        container->set_source("perf_test", "session_" + std::to_string(i));
        container->set_target("perf_target", "handler");
        container->set_message_type("performance_test");
      }
    });

    double rate = (BENCHMARK_ITERATIONS * 1000000.0) / duration.count();
    creation_rates.push_back(rate);
  }

  auto stats = calculate_stats(creation_rates);
  print_performance_report("Container Creation", stats);

  // Performance requirement: Should create at least 40K containers per second
  // Windows CI is 2-3x slower than Unix, so threshold is conservative
  EXPECT_GT(stats.mean, adjust_threshold_for_sanitizers(40000.0))
      << "Container creation performance below threshold (threshold="
      << adjust_threshold_for_sanitizers(40000.0) << ")";
}

TEST_F(PerformanceTest, ValueAdditionPerformance) {
  std::vector<double> addition_rates;
  const int num_runs = 10;
  const int values_per_container = 10;

  for (int run = 0; run < num_runs; ++run) {
    auto duration = measure_time([&]() {
      for (int i = 0; i < BENCHMARK_ITERATIONS; ++i) {
        // Create fresh container for each iteration to prevent memory bloat
        auto container = std::make_shared<value_container>();
        container->set_message_type("value_addition_test");

        for (int j = 0; j < values_per_container; ++j) {
          std::string key = "key_" + std::to_string(j);
          container->add_value(key, i * j);
        }
      }
    });

    double rate = (BENCHMARK_ITERATIONS * values_per_container * 1000000.0) /
                  duration.count();
    addition_rates.push_back(rate);
  }

  auto stats = calculate_stats(addition_rates);
  print_performance_report("Value Addition", stats);

  // Performance requirement: Should add at least 40K values per second
  // Windows CI is significantly slower; this threshold is conservative
  EXPECT_GT(stats.mean, adjust_threshold_for_sanitizers(40000.0))
      << "Value addition performance below threshold (threshold="
      << adjust_threshold_for_sanitizers(40000.0) << ")";
}

TEST_F(PerformanceTest, SerializationPerformance) {
  // Create a container with diverse value types
  auto container = std::make_shared<value_container>();
  container->set_source("serialization_test", "perf_session");
  container->set_target("serialization_target", "perf_handler");
  container->set_message_type("serialization_benchmark");

  // Add various types of values
  container->add_value(
      "string_data",
      std::string("Lorem ipsum dolor sit amet, consectetur adipiscing elit"));
  container->add_value("int_data", 123456789);
  container->add_value("long_data", 9223372036854775807LL);
  container->add_value("double_data", 3.141592653589793);
  container->add_value("bool_data", true);

  // Add binary data
  std::vector<uint8_t> binary_data(1024, 0xAB);
  container->add_value("bytes_data", binary_data);

  std::vector<double> serialization_rates;
  const int num_runs = 10;

  for (int run = 0; run < num_runs; ++run) {
    std::vector<std::string> serialized_data;
    serialized_data.reserve(BENCHMARK_ITERATIONS);

    auto duration = measure_time([&]() {
      for (int i = 0; i < BENCHMARK_ITERATIONS; ++i) {
        serialized_data.push_back(container->serialize());
      }
    });

    double rate = (BENCHMARK_ITERATIONS * 1000000.0) / duration.count();
    serialization_rates.push_back(rate);
  }

  auto stats = calculate_stats(serialization_rates);
  print_performance_report("Serialization", stats);

  // Performance requirement: Should serialize at least 5K containers per second
  // Adjusted for slower Windows CI environment
  EXPECT_GT(stats.mean, adjust_threshold_for_sanitizers(5000.0))
      << "Serialization performance below threshold (threshold="
      << adjust_threshold_for_sanitizers(5000.0) << ")";
}

TEST_F(PerformanceTest, DeserializationPerformance) {
  // Create and serialize a test container
  auto original = std::make_shared<value_container>();
  original->set_source("deserialization_test", "perf_session");
  original->set_target("deserialization_target", "perf_handler");
  original->set_message_type("deserialization_benchmark");

  original->add_value("test_string", std::string("Performance test data"));
  original->add_value("test_int", 42);
  original->add_value("test_double", 2.71828);

  std::string serialized_data = original->serialize();

  std::vector<double> deserialization_rates;
  const int num_runs = 10;

  for (int run = 0; run < num_runs; ++run) {
    auto duration = measure_time([&]() {
      for (int i = 0; i < BENCHMARK_ITERATIONS; ++i) {
        auto container = std::make_shared<value_container>();
        container->deserialize(serialized_data);
      }
    });

    double rate = (BENCHMARK_ITERATIONS * 1000000.0) / duration.count();
    deserialization_rates.push_back(rate);
  }

  auto stats = calculate_stats(deserialization_rates);
  print_performance_report("Deserialization", stats);

  // Performance requirement: Should deserialize at least 400 containers per
  // second Deserialization is slow; Windows CI is even slower Sanitizers
  // significantly reduce performance, so lower threshold when running under
  // sanitizers
  double threshold = 400.0;

  // Check if running under sanitizers (they set specific environment variables)
  const char *tsan_opts = std::getenv("TSAN_OPTIONS");
  const char *asan_opts = std::getenv("ASAN_OPTIONS");
  const char *ubsan_opts = std::getenv("UBSAN_OPTIONS");
  const char *ci_env = std::getenv("CI");

  if (tsan_opts || asan_opts || ubsan_opts) {
    threshold = 150.0; // Much lower threshold when sanitizers are active
  } else if (ci_env) {
    threshold = 190.0; // Lower threshold for CI environment variability
  }

  EXPECT_GT(stats.mean, threshold)
      << "Deserialization performance below threshold (threshold=" << threshold
      << ")";
}

TEST_F(PerformanceTest, ThreadSafetyStressTest) {
  const int num_threads = std::thread::hardware_concurrency();
  const int operations_per_thread = STRESS_ITERATIONS / num_threads;
  const int remaining_operations =
      STRESS_ITERATIONS % num_threads; // Handle remainder

  std::vector<std::future<std::chrono::microseconds>> futures;
  std::atomic<int> total_operations{0};

  auto start_time = std::chrono::high_resolution_clock::now();

  for (int t = 0; t < num_threads; ++t) {
    // Last thread handles remaining operations from integer division
    int thread_operations = operations_per_thread;
    if (t == num_threads - 1) {
      thread_operations += remaining_operations;
    }

    futures.emplace_back(std::async(
        std::launch::async,
        [&, t, thread_operations]() -> std::chrono::microseconds {
          auto thread_start = std::chrono::high_resolution_clock::now();

          for (int i = 0; i < thread_operations; ++i) {
            auto container = std::make_shared<value_container>();
            container->set_source("thread_" + std::to_string(t),
                                  "op_" + std::to_string(i));
            container->set_target("stress_target", "handler");
            container->set_message_type("stress_test");

            // Add random values
            container->add_value("iteration", i);
            container->add_value("thread_id", t);
            container->add_value(
                "data", std::string("stress_test_data_" + std::to_string(i)));

            // Serialize occasionally
            if (i % 100 == 0) {
              container->serialize();
            }

            // Use fetch_add for atomic increment
            total_operations.fetch_add(1, std::memory_order_relaxed);
          }

          auto thread_end = std::chrono::high_resolution_clock::now();
          return std::chrono::duration_cast<std::chrono::microseconds>(
              thread_end - thread_start);
        }));
  }

  // Wait for all threads and collect timing data
  std::vector<double> thread_rates;
  for (auto &future : futures) {
    auto thread_duration = future.get();
    double rate = (operations_per_thread * 1000000.0) / thread_duration.count();
    thread_rates.push_back(rate);
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      end_time - start_time);

  double overall_rate =
      (total_operations.load() * 1000000.0) / total_duration.count();
  auto thread_stats = calculate_stats(thread_rates);

  std::cout << "\n=== Thread Safety Stress Test ===" << std::endl;
  std::cout << "Threads: " << num_threads << std::endl;
  std::cout << "Operations per thread: " << operations_per_thread << " + "
            << remaining_operations << " (remainder)" << std::endl;
  std::cout << "Total Operations: " << total_operations.load() << std::endl;
  std::cout << "Expected Operations: " << STRESS_ITERATIONS << std::endl;
  std::cout << "Overall Rate: " << std::fixed << std::setprecision(2)
            << overall_rate << " ops/sec" << std::endl;
  std::cout << "Per-Thread Mean Rate: " << thread_stats.mean << " ops/sec"
            << std::endl;
  std::cout << "=================================" << std::endl;

  // Verify all operations completed successfully
  EXPECT_EQ(total_operations.load(), STRESS_ITERATIONS)
      << "Expected " << STRESS_ITERATIONS << " operations but got "
      << total_operations.load();
  // Windows CI has fewer cores and is slower; threshold adjusted
  EXPECT_GT(overall_rate, adjust_threshold_for_sanitizers(10000.0))
      << "Multi-threaded performance below threshold (threshold="
      << adjust_threshold_for_sanitizers(10000.0) << ")";
}

TEST_F(PerformanceTest, MemoryUsageTest) {
  const int num_containers = 10000;
  std::vector<std::shared_ptr<value_container>> containers;
  containers.reserve(num_containers);

  // Measure memory usage during container creation
  auto creation_start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < num_containers; ++i) {
    auto container = std::make_shared<value_container>();
    container->set_source("memory_test", "session_" + std::to_string(i));
    container->set_target("memory_target", "handler");
    container->set_message_type("memory_benchmark");

    // Add some values
    container->add_value("index", i);
    container->add_value("description", std::string("Memory test container " +
                                                    std::to_string(i)));
    container->add_value("value", i * 3.14159);

    containers.push_back(container);
  }

  auto creation_end = std::chrono::high_resolution_clock::now();
  auto creation_duration =
      std::chrono::duration_cast<std::chrono::microseconds>(creation_end -
                                                            creation_start);

  // Test serialization memory efficiency
  std::vector<std::string> serialized_containers;
  serialized_containers.reserve(num_containers);

  auto serialization_start = std::chrono::high_resolution_clock::now();

  for (const auto &container : containers) {
    serialized_containers.push_back(container->serialize());
  }

  auto serialization_end = std::chrono::high_resolution_clock::now();
  auto serialization_duration =
      std::chrono::duration_cast<std::chrono::microseconds>(
          serialization_end - serialization_start);

  // Calculate performance metrics
  double creation_rate =
      (num_containers * 1000000.0) / creation_duration.count();
  double serialization_rate =
      (num_containers * 1000000.0) / serialization_duration.count();

  std::cout << "\n=== Memory Usage Test ===" << std::endl;
  std::cout << "Containers Created: " << num_containers << std::endl;
  std::cout << "Creation Rate: " << std::fixed << std::setprecision(2)
            << creation_rate << " containers/sec" << std::endl;
  std::cout << "Serialization Rate: " << serialization_rate << " containers/sec"
            << std::endl;
  std::cout << "=========================" << std::endl;

  // Performance requirements
  EXPECT_GT(creation_rate, adjust_threshold_for_sanitizers(10000.0))
      << "Bulk creation performance below threshold (threshold="
      << adjust_threshold_for_sanitizers(10000.0) << ")";
  EXPECT_GT(serialization_rate, adjust_threshold_for_sanitizers(5000.0))
      << "Bulk serialization performance below threshold (threshold="
      << adjust_threshold_for_sanitizers(5000.0) << ")";

  // Cleanup
  containers.clear();
  serialized_containers.clear();
}

#ifdef HAS_MESSAGING_FEATURES
TEST_F(PerformanceTest, MessagingIntegrationPerformance) {
  std::vector<double> builder_rates;
  const int num_runs = 5;

  for (int run = 0; run < num_runs; ++run) {
    auto duration = measure_time([&]() {
      for (int i = 0; i < BENCHMARK_ITERATIONS; ++i) {
        auto container =
            integration::messaging_container_builder()
                .source("perf_client_" + std::to_string(i % 100))
                .target("perf_server", "handler_" + std::to_string(i % 10))
                .message_type("messaging_performance_test")
                .add_value("iteration", i)
                .add_value(
                    "timestamp",
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count())
                .add_value("data", std::string("performance_test_data_" +
                                               std::to_string(i)))
                .optimize_for_speed()
                .build();
      }
    });

    double rate = (BENCHMARK_ITERATIONS * 1000000.0) / duration.count();
    builder_rates.push_back(rate);
  }

  auto stats = calculate_stats(builder_rates);
  print_performance_report("Messaging Builder Pattern", stats);

  // Performance requirement: Builder should create at least 8K containers per
  // second Messaging integration adds overhead; Windows CI is slower
  EXPECT_GT(stats.mean, adjust_threshold_for_sanitizers(8000.0))
      << "Messaging builder performance below threshold (threshold="
      << adjust_threshold_for_sanitizers(8000.0) << ")";
}

TEST_F(PerformanceTest, MessagingSerializationPerformance) {
  auto container = integration::messaging_container_builder()
                       .source("serialization_perf")
                       .target("serialization_target")
                       .message_type("messaging_serialization_test")
                       .add_value("large_string", std::string(1000, 'X'))
                       .add_value("numeric_data", 123456789)
                       .add_value("floating_data", 3.141592653589793)
                       .optimize_for_speed()
                       .build();

  std::vector<double> serialization_rates;
  const int num_runs = 5;

  for (int run = 0; run < num_runs; ++run) {
    auto duration = measure_time([&]() {
      for (int i = 0; i < BENCHMARK_ITERATIONS / 10;
           ++i) { // Reduced iterations for enhanced serialization
        std::string serialized =
            integration::messaging_integration::serialize_for_messaging(
                container);
        auto deserialized =
            integration::messaging_integration::deserialize_from_messaging(
                serialized);
      }
    });

    double rate = ((BENCHMARK_ITERATIONS / 10) * 1000000.0) / duration.count();
    serialization_rates.push_back(rate);
  }

  auto stats = calculate_stats(serialization_rates);
  print_performance_report("Messaging Enhanced Serialization", stats);

  // Performance requirement: Enhanced serialization should handle at least 100
  // cycles per second Enhanced serialization is expensive; Windows CI makes it
  // even slower
  EXPECT_GT(stats.mean, adjust_threshold_for_sanitizers(100.0))
      << "Messaging serialization performance below threshold (threshold="
      << adjust_threshold_for_sanitizers(100.0) << ")";
}
#endif

// Large-scale stress test
TEST_F(PerformanceTest, LargeScaleStressTest) {
  // Reduced for CI environment - still tests memory and serialization at scale
  const int stress_containers = 5000; // Reduced from 50000
  const int stress_values_per_container = 20;

  std::cout << "\n=== Large-Scale Stress Test ===" << std::endl;
  std::cout << "Creating " << stress_containers << " containers with "
            << stress_values_per_container << " values each..." << std::endl;

  auto total_start = std::chrono::high_resolution_clock::now();

  std::vector<std::shared_ptr<value_container>> stress_containers_vec;
  stress_containers_vec.reserve(stress_containers);

  // Phase 1: Creation
  auto creation_start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < stress_containers; ++i) {
    auto container = std::make_shared<value_container>();
    container->set_source("stress_client_" + std::to_string(i % 1000), "");
    container->set_target("stress_server", "batch_" + std::to_string(i / 1000));
    container->set_message_type("large_scale_stress_test");

    for (int j = 0; j < stress_values_per_container; ++j) {
      std::string key = "key_" + std::to_string(j);
      switch (j % 5) {
      case 0:
        container->add_value(key,
                             std::string("stress_test_" + std::to_string(i)));
        break;
      case 1:
        container->add_value(key, i + j);
        break;
      case 2:
        container->add_value(key, (i + j) * 0.001);
        break;
      case 3:
        container->add_value(key, (i + j) % 2 == 0);
        break;
      case 4:
        container->add_value(key, (i * 1000 + j) % 2147483647);
        break;
      }
    }

    stress_containers_vec.push_back(container);

    if ((i + 1) % 10000 == 0) {
      std::cout << "Created " << (i + 1) << " containers..." << std::endl;
    }
  }
  auto creation_end = std::chrono::high_resolution_clock::now();

  // Phase 2: Serialization
  auto serialization_start = std::chrono::high_resolution_clock::now();
  std::vector<std::string> serialized_data;
  serialized_data.reserve(stress_containers);

  for (size_t i = 0; i < stress_containers_vec.size(); ++i) {
    serialized_data.push_back(stress_containers_vec[i]->serialize());

    if ((i + 1) % 10000 == 0) {
      std::cout << "Serialized " << (i + 1) << " containers..." << std::endl;
    }
  }
  auto serialization_end = std::chrono::high_resolution_clock::now();

  auto total_end = std::chrono::high_resolution_clock::now();

  // Calculate and report metrics
  auto creation_duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(creation_end -
                                                            creation_start);
  auto serialization_duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          serialization_end - serialization_start);
  auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      total_end - total_start);

  double creation_rate =
      (stress_containers * 1000.0) / creation_duration.count();
  double serialization_rate =
      (stress_containers * 1000.0) / serialization_duration.count();
  double total_values = stress_containers * stress_values_per_container;

  std::cout << "\n=== Stress Test Results ===" << std::endl;
  std::cout << "Total Containers: " << stress_containers << std::endl;
  std::cout << "Total Values: " << total_values << std::endl;
  std::cout << "Creation Time: " << creation_duration.count() << " ms"
            << std::endl;
  std::cout << "Serialization Time: " << serialization_duration.count() << " ms"
            << std::endl;
  std::cout << "Total Time: " << total_duration.count() << " ms" << std::endl;
  std::cout << "Creation Rate: " << std::fixed << std::setprecision(2)
            << creation_rate << " containers/sec" << std::endl;
  std::cout << "Serialization Rate: " << serialization_rate << " containers/sec"
            << std::endl;
  std::cout << "===========================" << std::endl;

  // Verify all data was created successfully
  EXPECT_EQ(stress_containers_vec.size(), stress_containers);
  EXPECT_EQ(serialized_data.size(), stress_containers);

  // Performance requirements for stress test
  EXPECT_GT(creation_rate, adjust_threshold_for_sanitizers(1000.0))
      << "Stress test creation rate below threshold (threshold="
      << adjust_threshold_for_sanitizers(1000.0) << ")";
  EXPECT_GT(serialization_rate, adjust_threshold_for_sanitizers(500.0))
      << "Stress test serialization rate below threshold (threshold="
      << adjust_threshold_for_sanitizers(500.0) << ")";

  // Memory cleanup
  stress_containers_vec.clear();
  serialized_data.clear();
}