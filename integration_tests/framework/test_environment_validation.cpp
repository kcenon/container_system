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
 * @file test_environment_validation.cpp
 * @brief Validation test for CI environment detection and configuration
 *
 * This test validates that TestConfig correctly detects the runtime environment
 * and provides appropriate configuration for tests. It's designed to run in both
 * local and CI environments to ensure proper detection.
 */

#include <gtest/gtest.h>
#include "test_config.h"
#include <utilities/core/formatter.h>
#include <iostream>
#include <cstdlib>

using namespace container_module::testing;

/**
 * @brief Test fixture for environment validation
 */
class EnvironmentValidationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Print environment for debugging
        PrintEnvironmentInfo();
    }

    void PrintEnvironmentInfo() const
    {
        std::cout << "\n=== Test Environment Information ===" << std::endl;
        std::cout << "CI Environment: " << (TestConfig::instance().is_ci_environment() ? "YES" : "NO") << std::endl;
        std::cout << "Platform: " << TestConfig::instance().platform_name() << std::endl;
        std::cout << "Debug Build: " << (TestConfig::instance().is_debug_build() ? "YES" : "NO") << std::endl;
        std::cout << "Skip Performance: " << (TestConfig::instance().should_skip_performance_checks() ? "YES" : "NO") << std::endl;
        std::cout << "Verbose Diagnostics: " << (TestConfig::instance().enable_verbose_diagnostics() ? "YES" : "NO") << std::endl;

        // Print formatter mode
        std::cout << "\nFormatter Configuration:" << std::endl;
        #if defined(__has_include)
            #if __has_include(<fmt/format.h>)
                std::cout << "  fmt library: AVAILABLE" << std::endl;
            #else
                std::cout << "  fmt library: NOT AVAILABLE" << std::endl;
            #endif
            #if __has_include(<format>)
                std::cout << "  std::format: AVAILABLE" << std::endl;
            #else
                std::cout << "  std::format: NOT AVAILABLE" << std::endl;
            #endif
        #endif
        // Check actual HAS_STD_FORMAT value from utilities/core/formatter.h
        #if HAS_STD_FORMAT == 1
            std::cout << "  Active mode: std::format (HAS_STD_FORMAT=1)" << std::endl;
        #elif HAS_STD_FORMAT == 0
            std::cout << "  Active mode: fmt library (HAS_STD_FORMAT=0)" << std::endl;
        #elif HAS_STD_FORMAT == -1
            std::cout << "  Active mode: FALLBACK - NO FORMATTING (HAS_STD_FORMAT=-1)" << std::endl;
            std::cout << "  ⚠️  WARNING: Using fallback mode will cause test failures!" << std::endl;
        #endif

        // Print relevant environment variables
        std::cout << "\nEnvironment Variables:" << std::endl;
        const char* ci_vars[] = {"CI", "GITHUB_ACTIONS", "GITLAB_CI", "JENKINS_HOME", "TRAVIS", "CIRCLECI"};
        for (const char* var : ci_vars) {
            const char* value = std::getenv(var);
            std::cout << "  " << var << ": " << (value ? value : "<not set>") << std::endl;
        }
        std::cout << "===================================\n" << std::endl;
    }
};

/**
 * Test 1: Verify environment detection
 */
TEST_F(EnvironmentValidationTest, EnvironmentDetection)
{
    // This test always passes but provides diagnostic information
    // Check if any CI environment variable is set
    bool any_ci_var_set = false;
    const char* ci_vars[] = {"CI", "GITHUB_ACTIONS", "GITLAB_CI", "JENKINS_HOME", "TRAVIS", "CIRCLECI"};

    for (const char* var : ci_vars) {
        if (std::getenv(var) != nullptr) {
            any_ci_var_set = true;
            break;
        }
    }

    // Verify TestConfig detection matches environment
    if (any_ci_var_set) {
        EXPECT_TRUE(TestConfig::instance().is_ci_environment())
            << "CI environment variable detected but TestConfig says not CI";
    }

    // Always pass - this is diagnostic
    SUCCEED() << "Environment detection check completed";
}

/**
 * Test 2: Verify platform detection
 */
TEST_F(EnvironmentValidationTest, PlatformDetection)
{
    std::string platform = TestConfig::instance().platform_name();

    // Should detect one of the known platforms
    EXPECT_TRUE(platform == "Ubuntu" || platform == "macOS" || platform == "Windows" || platform == "Unknown")
        << "Unexpected platform name: " << platform;

    // At least one platform flag should be true
    bool has_platform = TestConfig::instance().is_ubuntu() ||
                       TestConfig::instance().is_macos() ||
                       TestConfig::instance().is_windows();

    EXPECT_TRUE(has_platform || platform == "Unknown")
        << "No platform detected";

    SUCCEED() << "Platform detected as: " << platform;
}

/**
 * Test 3: Verify threshold adjustments are reasonable
 */
TEST_F(EnvironmentValidationTest, ThresholdAdjustments)
{
    double baseline_throughput = 100000.0;  // 100K ops/sec baseline
    int64_t baseline_duration = 10000;      // 10ms baseline

    double adjusted_throughput = TestConfig::instance().adjust_throughput_threshold(
        baseline_throughput, 500.0, 0.001);
    int64_t adjusted_duration = TestConfig::instance().adjust_duration_threshold(
        baseline_duration, 10.0);

    std::cout << "Throughput adjustment: " << baseline_throughput << " -> " << adjusted_throughput << std::endl;
    std::cout << "Duration adjustment: " << baseline_duration << "μs -> " << adjusted_duration << "μs" << std::endl;

    // Adjusted values should be less than or equal to baseline
    EXPECT_LE(adjusted_throughput, baseline_throughput)
        << "Adjusted throughput should not exceed baseline";
    EXPECT_GE(adjusted_duration, baseline_duration)
        << "Adjusted duration should not be less than baseline";

    // Minimum threshold check
    EXPECT_GE(adjusted_throughput, 500.0)
        << "Adjusted throughput should meet minimum threshold";

    // CI environment should have more relaxed thresholds
    if (TestConfig::instance().is_ci_environment()) {
        std::cout << "CI environment detected - using relaxed thresholds" << std::endl;
        // In CI, throughput threshold should be significantly lower
        EXPECT_LT(adjusted_throughput, baseline_throughput * 0.1)
            << "CI throughput threshold should be very relaxed (<10% of baseline)";
    } else {
        std::cout << "Local environment - using standard thresholds" << std::endl;
        // In local, should be 20% of baseline (accounts for machine variations
        // and variant-based storage overhead after migration)
        EXPECT_GT(adjusted_throughput, baseline_throughput * 0.15)
            << "Local throughput threshold should be reasonable (>15% of baseline)";
        EXPECT_LE(adjusted_throughput, baseline_throughput * 0.25)
            << "Local throughput threshold should not exceed 25% of baseline";
    }

    SUCCEED() << "Threshold adjustments validated";
}

/**
 * Test 4: Verify epsilon for floating-point comparisons
 */
TEST_F(EnvironmentValidationTest, EpsilonValue)
{
    double epsilon = TestConfig::instance().get_double_epsilon();

    std::cout << "Double epsilon: " << epsilon << std::endl;

    // Epsilon should be a small positive value
    EXPECT_GT(epsilon, 0.0) << "Epsilon should be positive";
    EXPECT_LT(epsilon, 0.01) << "Epsilon should be small (< 0.01)";

    // Standard value is 1e-6
    EXPECT_DOUBLE_EQ(epsilon, 1e-6) << "Epsilon should be 1e-6";

    SUCCEED() << "Epsilon value validated";
}

/**
 * Test 5: Verify timeout multiplier
 */
TEST_F(EnvironmentValidationTest, TimeoutMultiplier)
{
    double multiplier = TestConfig::instance().get_timeout_multiplier();

    std::cout << "Timeout multiplier: " << multiplier << std::endl;

    // Multiplier should be >= 1.0
    EXPECT_GE(multiplier, 1.0) << "Timeout multiplier should be at least 1.0";

    // CI should have higher multiplier
    if (TestConfig::instance().is_ci_environment()) {
        EXPECT_GT(multiplier, 1.0) << "CI timeout multiplier should be > 1.0";
    }

    SUCCEED() << "Timeout multiplier validated";
}

/**
 * Test 6: Verify build type detection
 */
TEST_F(EnvironmentValidationTest, BuildTypeDetection)
{
    bool is_debug = TestConfig::instance().is_debug_build();

    std::cout << "Build type: " << (is_debug ? "Debug" : "Release") << std::endl;

#ifdef NDEBUG
    EXPECT_FALSE(is_debug) << "NDEBUG defined but TestConfig says Debug";
#else
    EXPECT_TRUE(is_debug) << "NDEBUG not defined but TestConfig says Release";
#endif

    SUCCEED() << "Build type detection validated";
}

/**
 * Main function
 */
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════╗\n";
    std::cout << "║   Integration Test Environment Validation Suite          ║\n";
    std::cout << "║                                                           ║\n";
    std::cout << "║   This test validates that TestConfig correctly detects  ║\n";
    std::cout << "║   the runtime environment and provides appropriate       ║\n";
    std::cout << "║   configuration for all integration tests.                ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════╝\n";
    std::cout << std::endl;

    return RUN_ALL_TESTS();
}
