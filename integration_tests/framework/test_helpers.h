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

#include "test_config.h"
#include <algorithm>
#include <chrono>
#include <container/core/container.h>
#include <cstdlib>
#include <iostream>
#include <random>
#include <regex>
#include <string>
#include <vector>

namespace container_module {
namespace testing {

/**
 * @brief Helper functions for container integration tests
 */
class TestHelpers {
public:
  /**
   * @brief Generate random string of specified length
   */
  static std::string GenerateRandomString(size_t length) {
    static const char charset[] = "0123456789"
                                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "abcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

    std::string result;
    result.reserve(length);

    for (size_t i = 0; i < length; ++i) {
      result += charset[dist(gen)];
    }

    return result;
  }

  /**
   * @brief Generate random byte array
   */
  static std::vector<uint8_t> GenerateRandomBytes(size_t size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 255);

    std::vector<uint8_t> result(size);
    for (auto &byte : result) {
      byte = static_cast<uint8_t>(dist(gen));
    }

    return result;
  }

  /**
   * @brief Check if value exists in container
   */
  static bool ValueExists(std::shared_ptr<value_container> container,
                          const std::string &key) {
    auto val = container->get_value(key);
    return val && val->type != value_types::null_value;
  }

  /**
   * @brief Count total number of values in container
   */
  static size_t CountValues(std::shared_ptr<value_container> container) {
    size_t count = 0;
    // Note: This is a simplified count - actual implementation may vary
    // based on container's internal structure
    for (size_t i = 0; i < 1000; ++i) {
      std::string key = "key_" + std::to_string(i);
      if (ValueExists(container, key)) {
        count++;
      } else {
        break;
      }
    }
    return count;
  }

  /**
   * @brief Verify that two containers are equal
   */
  static bool ContainersEqual(std::shared_ptr<value_container> c1,
                              std::shared_ptr<value_container> c2) {
    // Compare headers
    if (c1->source_id() != c2->source_id() ||
        c1->target_id() != c2->target_id() ||
        c1->message_type() != c2->message_type()) {
      return false;
    }

    // Compare serialized forms (most reliable for deep comparison)
    return c1->serialize() == c2->serialize();
  }

  /**
   * @brief Create a nested container structure
   */
  static std::shared_ptr<value_container>
  CreateNestedContainer(size_t depth = 3) {
    auto root = std::make_shared<value_container>();
    root->set_message_type("root_level");

    if (depth > 0) {
      auto nested = CreateNestedContainer(depth - 1);
      std::string nested_data = nested->serialize();
      std::string key = "nested_" + std::to_string(depth);
      root->add_value(key, nested_data);
    }

    std::string key = "data_" + std::to_string(depth);
    std::string value = "level_" + std::to_string(depth);
    root->add_value(key, value);

    return root;
  }

  /**
   * @brief Measure operation throughput
   * @param operation Lambda function to execute
   * @param iterations Number of iterations
   * @return Operations per second
   */
  template <typename Func>
  static double MeasureThroughput(Func operation, size_t iterations = 1000) {
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < iterations; ++i) {
      operation();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count();

    if (duration <= 0) {
      // Extremely fast execution; treat as near-infinite throughput.
      return iterations == 0 ? 0.0 : iterations * 1000000.0;
    }

    // Convert to operations per second
    return (iterations * 1000000.0) / duration;
  }

  /**
   * @brief Detect whether tests are running in a CI environment.
   *
   * Checks common environment variables used by GitHub Actions and
   * other CI providers.
   *
   * @deprecated Use TestConfig::instance().is_ci_environment() instead
   */
  [[deprecated("Use TestConfig::instance().is_ci_environment()")]]
  static bool IsCiEnvironment() {
    return TestConfig::instance().is_ci_environment();
  }

  /**
   * @brief Adjust performance thresholds for CI environments.
   *
   * @param baseline Baseline threshold for local environments.
   * @param ci_floor Minimum acceptable threshold when running in CI.
   * @param ci_scale Scale factor applied to the baseline in CI.
   * @return Adjusted threshold value.
   *
   * @deprecated Use TestConfig::instance().adjust_throughput_threshold()
   * instead
   */
  [[deprecated("Use TestConfig::instance().adjust_throughput_threshold()")]]
  static double AdjustPerformanceThreshold(double baseline,
                                           double ci_floor = 5.0,
                                           double ci_scale = 0.0001) {
    return TestConfig::instance().adjust_throughput_threshold(
        baseline, ci_floor, ci_scale);
  }

  /**
   * @brief Adjust duration thresholds when running on CI.
   *
   * @param baseline_microseconds Baseline duration in microseconds.
   * @param ci_ceiling Relaxed ceiling for CI.
   * @return Adjusted threshold for duration comparisons.
   *
   * @deprecated Use TestConfig::instance().adjust_duration_threshold() instead
   */
  [[deprecated("Use TestConfig::instance().adjust_duration_threshold()")]]
  static int64_t AdjustDurationThreshold(int64_t baseline_microseconds,
                                         int64_t ci_ceiling = 500000) {
    return TestConfig::instance().adjust_duration_threshold(
        baseline_microseconds);
  }

  /**
   * @brief Validate serialized data format
   *
   * Performs comprehensive validation:
   * 1. Checks for required markers (@header and @data)
   * 2. Validates header format matches expected pattern (single or double
   * braces)
   * 3. Validates data section format
   * 4. Ensures proper termination with semicolons
   */
  static bool IsValidSerializedData(const std::string &data) {
    // Check for required markers
    if (data.find("@header") == std::string::npos ||
        data.find("@data") == std::string::npos) {
      return false;
    }

    // Validate header format - must match pattern with single or double braces
    // Pattern: @header={{...}}; or @header={...};
    std::regex header_pattern(R"(@header=\s*\{\{?[^\}]*\}\}?;)");
    if (!std::regex_search(data, header_pattern)) {
      return false;
    }

    // Validate data section format
    // Pattern: @data={{...}}; or @data={...};
    std::regex data_pattern(R"(@data=\s*\{\{?.*\}\}?;)");
    if (!std::regex_search(data, data_pattern)) {
      return false;
    }

    return true;
  }

  /**
   * @brief Generate test container with mixed value types
   */
  static std::shared_ptr<value_container> CreateMixedTypeContainer() {
    auto container = std::make_shared<value_container>();
    container->set_source("test", "");
    container->set_target("dest", "");
    container->set_message_type("mixed_types");

    // Add different value types
    container->add_value("str_val", "test_string");
    container->add_value("int_val", 42);
    container->add_value("long_val", 9223372036854775807LL);
    container->add_value("double_val", 3.14159);
    container->add_value("bool_val", true);

    std::vector<uint8_t> bytes = {0x01, 0x02, 0x03, 0x04};
    container->add_value("bytes_val", bytes);

    return container;
  }

  /**
   * @brief Calculate serialization overhead percentage
   */
  static double
  CalculateSerializationOverhead(std::shared_ptr<value_container> container) {
    std::string serialized = container->serialize();
    // Estimate raw data size (simplified)
    size_t raw_size = 100; // Header estimate

    // This is a simplified calculation
    return ((double)(serialized.size() - raw_size) / raw_size) * 100.0;
  }

  /**
   * @brief Print detailed serialization debug information
   *
   * Useful for diagnosing serialization/deserialization issues.
   * Prints header and data section boundaries, format validation results.
   *
   * @param serialized Serialized container data
   */
  static void PrintSerializationDebugInfo(const std::string &serialized) {
    std::cout << "=== Serialization Debug Info ===" << std::endl;
    std::cout << "Total size: " << serialized.size() << " bytes" << std::endl;

    // Find header position
    size_t header_pos = serialized.find("@header");
    size_t data_pos = serialized.find("@data");

    if (header_pos != std::string::npos) {
      std::cout << "Header found at position: " << header_pos << std::endl;

      // Extract header section (up to @data or end)
      size_t header_end =
          (data_pos != std::string::npos) ? data_pos : serialized.size();
      std::string header_section = serialized.substr(
          header_pos, std::min(size_t(200), header_end - header_pos));
      std::cout << "Header section: " << header_section << std::endl;
    } else {
      std::cout << "WARNING: @header marker not found!" << std::endl;
    }

    if (data_pos != std::string::npos) {
      std::cout << "Data found at position: " << data_pos << std::endl;

      // Extract data section preview
      std::string data_section = serialized.substr(
          data_pos, std::min(size_t(200), serialized.size() - data_pos));
      std::cout << "Data section: " << data_section << std::endl;
    } else {
      std::cout << "WARNING: @data marker not found!" << std::endl;
    }

    // Validate format
    bool is_valid = IsValidSerializedData(serialized);
    std::cout << "Format validation: " << (is_valid ? "PASS" : "FAIL")
              << std::endl;
    std::cout << "================================" << std::endl;
  }

  /**
   * @brief Verify roundtrip serialization with detailed diagnostics
   *
   * Performs serialization and deserialization while providing
   * detailed diagnostic information if failures occur.
   *
   * @param container Container to test
   * @return True if roundtrip successful, false otherwise
   */
  static bool
  VerifyRoundtripWithDiagnostics(std::shared_ptr<value_container> container) {
    try {
      // Serialize
      std::string serialized = container->serialize();

      if (serialized.empty()) {
        std::cerr << "ERROR: Serialization produced empty string" << std::endl;
        return false;
      }

      // Validate format
      if (!IsValidSerializedData(serialized)) {
        std::cerr << "ERROR: Serialized data has invalid format" << std::endl;
        PrintSerializationDebugInfo(serialized);
        return false;
      }

      // Deserialize
      auto restored = std::make_shared<value_container>(serialized, false);

      // Verify header preservation
      if (restored->source_id() != container->source_id()) {
        std::cerr << "ERROR: source_id mismatch after roundtrip" << std::endl;
        std::cerr << "  Expected: '" << container->source_id() << "'"
                  << std::endl;
        std::cerr << "  Got: '" << restored->source_id() << "'" << std::endl;
        PrintSerializationDebugInfo(serialized);
        return false;
      }

      if (restored->target_id() != container->target_id()) {
        std::cerr << "ERROR: target_id mismatch after roundtrip" << std::endl;
        std::cerr << "  Expected: '" << container->target_id() << "'"
                  << std::endl;
        std::cerr << "  Got: '" << restored->target_id() << "'" << std::endl;
        return false;
      }

      if (restored->message_type() != container->message_type()) {
        std::cerr << "ERROR: message_type mismatch after roundtrip"
                  << std::endl;
        std::cerr << "  Expected: '" << container->message_type() << "'"
                  << std::endl;
        std::cerr << "  Got: '" << restored->message_type() << "'" << std::endl;
        return false;
      }

      return true;

    } catch (const std::exception &e) {
      std::cerr << "ERROR: Exception during roundtrip: " << e.what()
                << std::endl;
      return false;
    }
  }

  /**
   * @brief Dump container state for debugging
   *
   * Prints container header information and value count (if available).
   *
   * @param container Container to dump
   * @param label Optional label for output
   */
  static void DumpContainerState(std::shared_ptr<value_container> container,
                                 const std::string &label = "") {
    std::cout << "=== Container State";
    if (!label.empty()) {
      std::cout << " (" << label << ")";
    }
    std::cout << " ===" << std::endl;

    std::cout << "Source: " << container->source_id();
    if (!container->source_sub_id().empty()) {
      std::cout << " (sub: " << container->source_sub_id() << ")";
    }
    std::cout << std::endl;

    std::cout << "Target: " << container->target_id();
    if (!container->target_sub_id().empty()) {
      std::cout << " (sub: " << container->target_sub_id() << ")";
    }
    std::cout << std::endl;

    std::cout << "Message type: " << container->message_type() << std::endl;
    std::cout << "================================" << std::endl;
  }

  /**
   * @brief Create container with specific size
   */
  static std::shared_ptr<value_container>
  CreateContainerWithSize(size_t target_bytes) {
    auto container = std::make_shared<value_container>();
    container->set_message_type("sized_container");

    // Add string values until we reach target size
    size_t current_size = 0;
    size_t counter = 0;

    while (current_size < target_bytes) {
      std::string key = "key_" + std::to_string(counter);
      std::string value = GenerateRandomString(100);
      container->add_value(key, value);

      current_size = container->serialize().size();
      counter++;
    }

    return container;
  }
};

} // namespace testing
} // namespace container_module
