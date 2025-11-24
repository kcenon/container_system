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

/**
 * @file test_config.h
 * @brief Centralized test configuration for container system integration tests
 *
 * Provides environment detection, threshold adjustments, and configuration
 * settings to ensure tests run reliably across different environments:
 * - Local development (macOS, Linux, Windows)
 * - CI environments (GitHub Actions, GitLab CI, etc.)
 * - Different build types (Debug, Release, RelWithDebInfo)
 */

#pragma once

#include <cstdlib>
#include <string>
#include <algorithm>
#include <limits>

namespace container_module
{
namespace testing
{

/**
 * @brief Centralized configuration for integration tests
 *
 * Singleton pattern for consistent configuration across all test files.
 */
class TestConfig
{
public:
    /**
     * @brief Get singleton instance
     */
    static TestConfig& instance()
    {
        static TestConfig config;
        return config;
    }

    // Delete copy/move constructors
    TestConfig(const TestConfig&) = delete;
    TestConfig& operator=(const TestConfig&) = delete;
    TestConfig(TestConfig&&) = delete;
    TestConfig& operator=(TestConfig&&) = delete;

    /**
     * @brief Check if running in CI environment
     *
     * Detects common CI environment variables:
     * - CI (set by most CI systems)
     * - GITHUB_ACTIONS
     * - GITLAB_CI
     * - JENKINS_HOME
     * - TRAVIS
     * - CIRCLECI
     */
    bool is_ci_environment() const { return ci_environment_; }

    /**
     * @brief Check if this is a debug build
     */
    bool is_debug_build() const { return debug_build_; }

    /**
     * @brief Check if running on Ubuntu
     */
    bool is_ubuntu() const { return is_ubuntu_; }

    /**
     * @brief Check if running on macOS
     */
    bool is_macos() const { return is_macos_; }

    /**
     * @brief Check if running on Windows
     */
    bool is_windows() const { return is_windows_; }

    /**
     * @brief Get platform name
     */
    std::string platform_name() const
    {
        if (is_ubuntu_) return "Ubuntu";
        if (is_macos_) return "macOS";
        if (is_windows_) return "Windows";
        return "Unknown";
    }

    /**
     * @brief Adjust throughput threshold for CI environment
     *
     * CI runners are typically slower and more variable than local machines.
     * This function applies conservative scaling to avoid false negatives.
     *
     * @param baseline Expected throughput in local environment (ops/second)
     * @param min_threshold Minimum acceptable threshold (default: 10.0)
     * @param ci_scale Scale factor for CI (default: 0.001 = 0.1%)
     * @return Adjusted threshold
     */
    double adjust_throughput_threshold(double baseline,
                                      double min_threshold = 10.0,
                                      double ci_scale = 0.001) const
    {
        if (!ci_environment_) {
            // Local environment: use relaxed but meaningful baseline
            // Allow 20% of baseline to account for machine variations
            // and variant-based storage overhead after migration
            return std::max(min_threshold, baseline * 0.2);
        }

        // CI environment: very relaxed thresholds
        double adjusted = baseline * ci_scale;
        return std::max(min_threshold, adjusted);
    }

    /**
     * @brief Adjust duration threshold for CI environment
     *
     * @param baseline_microseconds Expected duration in local environment (μs)
     * @param ci_multiplier Multiplier for CI timeout (default: 10x)
     * @return Adjusted duration threshold in microseconds
     */
    int64_t adjust_duration_threshold(int64_t baseline_microseconds,
                                     double ci_multiplier = 10.0) const
    {
        if (!ci_environment_) {
            // Local: allow 2x baseline
            return baseline_microseconds * 2;
        }

        // CI: much more relaxed
        return static_cast<int64_t>(baseline_microseconds * ci_multiplier);
    }

    /**
     * @brief Get appropriate epsilon for floating-point comparisons
     *
     * Different platforms may have slightly different floating-point behavior.
     *
     * @return Epsilon value for EXPECT_NEAR
     */
    double get_double_epsilon() const
    {
        // Standard epsilon for serialization roundtrip tests
        // Allows for ~6-7 significant digits of precision
        return 1e-6;
    }

    /**
     * @brief Check if performance tests should be skipped
     *
     * Performance tests are unreliable in CI due to:
     * - Shared resources
     * - Variable machine specs
     * - Background processes
     */
    bool should_skip_performance_checks() const
    {
        return ci_environment_;
    }

    /**
     * @brief Check if verbose diagnostics should be enabled
     *
     * Enabled in CI or when INTEGRATION_TEST_VERBOSE is set
     */
    bool enable_verbose_diagnostics() const
    {
        return verbose_diagnostics_;
    }

    /**
     * @brief Get test timeout multiplier
     *
     * CI environments need longer timeouts
     */
    double get_timeout_multiplier() const
    {
        return ci_environment_ ? 3.0 : 1.0;
    }

    /**
     * @brief Check if test should fail fast on first error
     */
    bool should_fail_fast() const
    {
        const char* fail_fast = std::getenv("INTEGRATION_TEST_FAIL_FAST");
        return fail_fast && (std::string(fail_fast) == "1" || std::string(fail_fast) == "true");
    }

private:
    TestConfig()
    {
        detect_environment();
        detect_build_type();
        detect_platform();
        detect_verbose_mode();
    }

    void detect_environment()
    {
        // Check common CI environment variables
        const char* ci = std::getenv("CI");
        const char* github_actions = std::getenv("GITHUB_ACTIONS");
        const char* gitlab_ci = std::getenv("GITLAB_CI");
        const char* jenkins = std::getenv("JENKINS_HOME");
        const char* travis = std::getenv("TRAVIS");
        const char* circle = std::getenv("CIRCLECI");

        ci_environment_ = (ci && (std::string(ci) == "true" || std::string(ci) == "1")) ||
                         github_actions != nullptr ||
                         gitlab_ci != nullptr ||
                         jenkins != nullptr ||
                         travis != nullptr ||
                         circle != nullptr;
    }

    void detect_build_type()
    {
#ifdef NDEBUG
        debug_build_ = false;
#else
        debug_build_ = true;
#endif
    }

    void detect_platform()
    {
#if defined(__linux__)
        is_ubuntu_ = true;
        is_macos_ = false;
        is_windows_ = false;
#elif defined(__APPLE__) && defined(__MACH__)
        is_ubuntu_ = false;
        is_macos_ = true;
        is_windows_ = false;
#elif defined(_WIN32) || defined(_WIN64)
        is_ubuntu_ = false;
        is_macos_ = false;
        is_windows_ = true;
#else
        is_ubuntu_ = false;
        is_macos_ = false;
        is_windows_ = false;
#endif
    }

    void detect_verbose_mode()
    {
        const char* verbose = std::getenv("INTEGRATION_TEST_VERBOSE");
        verbose_diagnostics_ = ci_environment_ ||
                              (verbose && (std::string(verbose) == "1" || std::string(verbose) == "true"));
    }

    bool ci_environment_;
    bool debug_build_;
    bool is_ubuntu_;
    bool is_macos_;
    bool is_windows_;
    bool verbose_diagnostics_;
};

} // namespace testing
} // namespace container_module
