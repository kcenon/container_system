// BSD 3-Clause License
//
// Copyright (c) 2021-2025
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
 * @file simd_tests.cpp
 * @brief Unit tests for SIMD processor functionality
 *
 * Tests cover:
 * - SIMD support detection (SSE2, SSE4.2, AVX2, AVX-512, NEON)
 * - SIMD operations (sum, min, max)
 * - Runtime feature detection
 * - Fallback to scalar operations
 */

#include <gtest/gtest.h>
#include <internal/simd_processor.h>
#include <vector>
#include <random>
#include <numeric>
#include <cmath>
#include <limits>

using namespace container_module::simd;

class SIMDSupportTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

class SIMDOperationsTest : public ::testing::Test {
protected:
    std::vector<container_module::ValueVariant> float_values;
    std::vector<container_module::ValueVariant> double_values;
    std::mt19937 rng{42};

    void SetUp() override {
        // Create test data with known values
        float_values.clear();
        double_values.clear();

        for (int i = 0; i < 100; ++i) {
            float_values.push_back(static_cast<float>(i + 1));
            double_values.push_back(static_cast<double>(i + 1));
        }
    }

    void TearDown() override {
        float_values.clear();
        double_values.clear();
    }
};

// ============================================================================
// SIMD Support Detection Tests
// ============================================================================

TEST_F(SIMDSupportTest, GetSIMDInfo) {
    std::string info = simd_support::get_simd_info();
    EXPECT_FALSE(info.empty());
    EXPECT_NE(info.find("SIMD Support:"), std::string::npos);
    EXPECT_NE(info.find("Width:"), std::string::npos);

    // Output for debugging
    std::cout << "SIMD Info: " << info << std::endl;
}

TEST_F(SIMDSupportTest, GetOptimalWidth) {
    size_t width = simd_support::get_optimal_width();

    // Width should be a power of 2 and reasonable
    EXPECT_GE(width, 1u);
    EXPECT_LE(width, 16u);

    // Check it's a power of 2
    EXPECT_EQ(width & (width - 1), 0u);
}

TEST_F(SIMDSupportTest, GetBestSIMDLevel) {
    simd_level level = simd_support::get_best_simd_level();

    // Level should be valid
    EXPECT_GE(static_cast<int>(level), static_cast<int>(simd_level::none));
    EXPECT_LE(static_cast<int>(level), static_cast<int>(simd_level::neon));

    std::cout << "Best SIMD Level: " << static_cast<int>(level) << std::endl;
}

TEST_F(SIMDSupportTest, RuntimeDetectionConsistency) {
    // If AVX-512 is available, AVX2 should also be available
    if (simd_support::has_avx512f()) {
        EXPECT_TRUE(simd_support::has_avx2());
        EXPECT_TRUE(simd_support::has_sse42());
        EXPECT_TRUE(simd_support::has_sse2());
    }

    // If AVX2 is available, SSE4.2 should also be available
    if (simd_support::has_avx2()) {
        EXPECT_TRUE(simd_support::has_sse42());
        EXPECT_TRUE(simd_support::has_sse2());
    }

    // If SSE4.2 is available, SSE2 should also be available
    if (simd_support::has_sse42()) {
        EXPECT_TRUE(simd_support::has_sse2());
    }
}

TEST_F(SIMDSupportTest, AVX512SubfeatureConsistency) {
    // If AVX-512DQ/BW/VL is available, AVX-512F should be available
    if (simd_support::has_avx512dq()) {
        EXPECT_TRUE(simd_support::has_avx512f());
    }
    if (simd_support::has_avx512bw()) {
        EXPECT_TRUE(simd_support::has_avx512f());
    }
    if (simd_support::has_avx512vl()) {
        EXPECT_TRUE(simd_support::has_avx512f());
    }
}

// ============================================================================
// SIMD Float Operations Tests
// ============================================================================

TEST_F(SIMDOperationsTest, SumFloatsBasic) {
    float sum = simd_processor::sum_floats(float_values);

    // Sum of 1 to 100 = 100 * 101 / 2 = 5050
    EXPECT_FLOAT_EQ(sum, 5050.0F);
}

TEST_F(SIMDOperationsTest, SumFloatsEmpty) {
    std::vector<container_module::ValueVariant> empty;
    float sum = simd_processor::sum_floats(empty);
    EXPECT_FLOAT_EQ(sum, 0.0F);
}

TEST_F(SIMDOperationsTest, SumFloatsLargeDataset) {
    std::vector<container_module::ValueVariant> large_data;
    large_data.reserve(10000);

    for (int i = 0; i < 10000; ++i) {
        large_data.push_back(1.0F);
    }

    float sum = simd_processor::sum_floats(large_data);
    EXPECT_NEAR(sum, 10000.0F, 0.01F);
}

TEST_F(SIMDOperationsTest, SumFloatsMixedTypes) {
    // Test with mixed types (only floats should be summed)
    std::vector<container_module::ValueVariant> mixed;
    mixed.push_back(1.0F);
    mixed.push_back(std::string("ignore"));
    mixed.push_back(2.0F);
    mixed.push_back(42);  // int, should be ignored
    mixed.push_back(3.0F);

    float sum = simd_processor::sum_floats(mixed);
    EXPECT_FLOAT_EQ(sum, 6.0F);
}

// ============================================================================
// SIMD Min/Max Operations Tests
// ============================================================================

TEST_F(SIMDOperationsTest, MinFloatBasic) {
    auto min_val = simd_processor::min_float(float_values);
    ASSERT_TRUE(min_val.has_value());
    EXPECT_FLOAT_EQ(*min_val, 1.0F);
}

TEST_F(SIMDOperationsTest, MaxFloatBasic) {
    auto max_val = simd_processor::max_float(float_values);
    ASSERT_TRUE(max_val.has_value());
    EXPECT_FLOAT_EQ(*max_val, 100.0F);
}

TEST_F(SIMDOperationsTest, MinFloatEmpty) {
    std::vector<container_module::ValueVariant> empty;
    auto min_val = simd_processor::min_float(empty);
    EXPECT_FALSE(min_val.has_value());
}

TEST_F(SIMDOperationsTest, MaxFloatEmpty) {
    std::vector<container_module::ValueVariant> empty;
    auto max_val = simd_processor::max_float(empty);
    EXPECT_FALSE(max_val.has_value());
}

TEST_F(SIMDOperationsTest, MinFloatWithNegatives) {
    std::vector<container_module::ValueVariant> data;
    data.push_back(-100.0F);
    data.push_back(50.0F);
    data.push_back(-200.0F);
    data.push_back(0.0F);

    auto min_val = simd_processor::min_float(data);
    ASSERT_TRUE(min_val.has_value());
    EXPECT_FLOAT_EQ(*min_val, -200.0F);
}

TEST_F(SIMDOperationsTest, MaxFloatWithNegatives) {
    std::vector<container_module::ValueVariant> data;
    data.push_back(-100.0F);
    data.push_back(50.0F);
    data.push_back(-200.0F);
    data.push_back(0.0F);

    auto max_val = simd_processor::max_float(data);
    ASSERT_TRUE(max_val.has_value());
    EXPECT_FLOAT_EQ(*max_val, 50.0F);
}

TEST_F(SIMDOperationsTest, MinMaxFloatLargeDataset) {
    std::vector<container_module::ValueVariant> large_data;
    large_data.reserve(10000);

    std::uniform_real_distribution<float> dist(-1000.0F, 1000.0F);

    float expected_min = std::numeric_limits<float>::max();
    float expected_max = std::numeric_limits<float>::lowest();

    for (int i = 0; i < 10000; ++i) {
        float val = dist(rng);
        large_data.push_back(val);
        expected_min = std::min(expected_min, val);
        expected_max = std::max(expected_max, val);
    }

    auto min_val = simd_processor::min_float(large_data);
    auto max_val = simd_processor::max_float(large_data);

    ASSERT_TRUE(min_val.has_value());
    ASSERT_TRUE(max_val.has_value());
    EXPECT_FLOAT_EQ(*min_val, expected_min);
    EXPECT_FLOAT_EQ(*max_val, expected_max);
}

// ============================================================================
// SIMD Double Operations Tests
// ============================================================================

TEST_F(SIMDOperationsTest, SumDoublesBasic) {
    double sum = simd_processor::sum_doubles(double_values);

    // Sum of 1 to 100 = 100 * 101 / 2 = 5050
    EXPECT_DOUBLE_EQ(sum, 5050.0);
}

TEST_F(SIMDOperationsTest, SumDoublesEmpty) {
    std::vector<container_module::ValueVariant> empty;
    double sum = simd_processor::sum_doubles(empty);
    EXPECT_DOUBLE_EQ(sum, 0.0);
}

// ============================================================================
// SIMD Find Operations Tests
// ============================================================================

TEST_F(SIMDOperationsTest, FindEqualFloatsBasic) {
    auto indices = simd_processor::find_equal_floats(float_values, 50.0F);
    ASSERT_EQ(indices.size(), 1u);
    EXPECT_EQ(indices[0], 49u);  // 0-indexed, value 50 is at index 49
}

TEST_F(SIMDOperationsTest, FindEqualFloatsNotFound) {
    auto indices = simd_processor::find_equal_floats(float_values, 999.0F);
    EXPECT_TRUE(indices.empty());
}

TEST_F(SIMDOperationsTest, FindEqualFloatsMultipleMatches) {
    std::vector<container_module::ValueVariant> data;
    data.push_back(1.0F);
    data.push_back(2.0F);
    data.push_back(1.0F);
    data.push_back(3.0F);
    data.push_back(1.0F);

    auto indices = simd_processor::find_equal_floats(data, 1.0F);
    ASSERT_EQ(indices.size(), 3u);
    EXPECT_EQ(indices[0], 0u);
    EXPECT_EQ(indices[1], 2u);
    EXPECT_EQ(indices[2], 4u);
}

// ============================================================================
// SIMD Memory Operations Tests
// ============================================================================

TEST_F(SIMDOperationsTest, FastCopyBasic) {
    std::vector<uint8_t> src = {1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<uint8_t> dst(src.size());

    simd_processor::fast_copy(src.data(), dst.data(), src.size());

    EXPECT_EQ(src, dst);
}

TEST_F(SIMDOperationsTest, FastCompareEqual) {
    std::vector<uint8_t> a = {1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<uint8_t> b = {1, 2, 3, 4, 5, 6, 7, 8};

    EXPECT_TRUE(simd_processor::fast_compare(a.data(), b.data(), a.size()));
}

TEST_F(SIMDOperationsTest, FastCompareNotEqual) {
    std::vector<uint8_t> a = {1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<uint8_t> b = {1, 2, 3, 4, 5, 6, 7, 9};  // Last byte differs

    EXPECT_FALSE(simd_processor::fast_compare(a.data(), b.data(), a.size()));
}

// ============================================================================
// SIMD Edge Cases Tests
// ============================================================================

TEST_F(SIMDOperationsTest, SingleElementSum) {
    std::vector<container_module::ValueVariant> single;
    single.push_back(42.0F);

    float sum = simd_processor::sum_floats(single);
    EXPECT_FLOAT_EQ(sum, 42.0F);
}

TEST_F(SIMDOperationsTest, SingleElementMinMax) {
    std::vector<container_module::ValueVariant> single;
    single.push_back(42.0F);

    auto min_val = simd_processor::min_float(single);
    auto max_val = simd_processor::max_float(single);

    ASSERT_TRUE(min_val.has_value());
    ASSERT_TRUE(max_val.has_value());
    EXPECT_FLOAT_EQ(*min_val, 42.0F);
    EXPECT_FLOAT_EQ(*max_val, 42.0F);
}

TEST_F(SIMDOperationsTest, NonAlignedSizeSum) {
    // Test with size that doesn't align to SIMD width
    std::vector<container_module::ValueVariant> data;
    for (int i = 1; i <= 17; ++i) {  // 17 elements (not aligned to 4, 8, or 16)
        data.push_back(static_cast<float>(i));
    }

    float sum = simd_processor::sum_floats(data);

    // Sum of 1 to 17 = 17 * 18 / 2 = 153
    EXPECT_FLOAT_EQ(sum, 153.0F);
}

// ============================================================================
// SIMD Traits Tests
// ============================================================================

TEST(SIMDTraitsTest, FloatTraits) {
#if defined(HAS_X86_SIMD) || defined(HAS_ARM_NEON)
    EXPECT_TRUE(simd_traits<float>::supported);
    EXPECT_GE(simd_traits<float>::width, 1u);
#endif
}

TEST(SIMDTraitsTest, DoubleTraits) {
#if defined(HAS_X86_SIMD) || defined(HAS_ARM_NEON)
    EXPECT_TRUE(simd_traits<double>::supported);
    EXPECT_GE(simd_traits<double>::width, 1u);
#endif
}

TEST(SIMDTraitsTest, Int32Traits) {
#if defined(HAS_X86_SIMD) || defined(HAS_ARM_NEON)
    EXPECT_TRUE(simd_traits<int32_t>::supported);
    EXPECT_GE(simd_traits<int32_t>::width, 1u);
#endif
}

// ============================================================================
// Platform-specific Tests
// ============================================================================

#if defined(HAS_AVX512)
TEST(AVX512Test, DetectionWorks) {
    // If compiled with AVX-512, runtime detection should confirm it
    // (unless running on a machine without AVX-512)
    std::cout << "AVX-512 compile-time support: enabled" << std::endl;
    std::cout << "AVX-512F runtime support: "
              << (simd_support::has_avx512f() ? "yes" : "no") << std::endl;
}
#endif

#if defined(HAS_AVX2)
TEST(AVX2Test, DetectionWorks) {
    std::cout << "AVX2 compile-time support: enabled" << std::endl;
    std::cout << "AVX2 runtime support: "
              << (simd_support::has_avx2() ? "yes" : "no") << std::endl;
}
#endif

#if defined(HAS_ARM_NEON)
TEST(NEONTest, DetectionWorks) {
    EXPECT_TRUE(simd_support::has_neon());
    std::cout << "ARM NEON support: enabled" << std::endl;
}
#endif

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
