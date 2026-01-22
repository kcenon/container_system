// BSD 3-Clause License
//
// Copyright (c) 2024, kcenon
// All rights reserved.
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
 * @file simd_policies_tests.cpp
 * @brief Unit tests for compile-time SIMD policy selection
 *
 * Tests verify:
 * - Policy concept compliance
 * - Scalar policy correctness (baseline)
 * - Platform-specific policy correctness
 * - Result consistency across policies
 * - Edge cases (empty arrays, single element, non-aligned sizes)
 *
 * @see Issue #338 for design rationale
 */

#include <gtest/gtest.h>
#include <container/internal/simd_policies.h>
#include <vector>
#include <random>
#include <numeric>
#include <cmath>
#include <limits>

using namespace container_module::simd;

// ============================================================================
// Scalar Policy Tests (Baseline)
// ============================================================================

class ScalarPolicyTest : public ::testing::Test {
protected:
    simd_ops<scalar_simd_policy> ops;
    std::vector<float> test_data;
    std::mt19937 rng{42};

    void SetUp() override {
        test_data.clear();
        for (int i = 1; i <= 100; ++i) {
            test_data.push_back(static_cast<float>(i));
        }
    }
};

TEST_F(ScalarPolicyTest, PolicyName) {
    EXPECT_EQ(simd_ops<scalar_simd_policy>::policy_name(), "scalar");
}

TEST_F(ScalarPolicyTest, SimdWidth) {
    EXPECT_EQ(simd_ops<scalar_simd_policy>::simd_width(), 1u);
}

TEST_F(ScalarPolicyTest, SumFloatsBasic) {
    float sum = ops.sum_floats(test_data.data(), test_data.size());
    // Sum of 1 to 100 = 100 * 101 / 2 = 5050
    EXPECT_FLOAT_EQ(sum, 5050.0f);
}

TEST_F(ScalarPolicyTest, SumFloatsEmpty) {
    float sum = ops.sum_floats(nullptr, 0);
    EXPECT_FLOAT_EQ(sum, 0.0f);
}

TEST_F(ScalarPolicyTest, SumFloatsSingleElement) {
    float val = 42.0f;
    float sum = ops.sum_floats(&val, 1);
    EXPECT_FLOAT_EQ(sum, 42.0f);
}

TEST_F(ScalarPolicyTest, MinFloatBasic) {
    float min_val = ops.min_float(test_data.data(), test_data.size());
    EXPECT_FLOAT_EQ(min_val, 1.0f);
}

TEST_F(ScalarPolicyTest, MinFloatEmpty) {
    float min_val = ops.min_float(nullptr, 0);
    EXPECT_FLOAT_EQ(min_val, std::numeric_limits<float>::max());
}

TEST_F(ScalarPolicyTest, MinFloatWithNegatives) {
    std::vector<float> data = {-100.0f, 50.0f, -200.0f, 0.0f};
    float min_val = ops.min_float(data.data(), data.size());
    EXPECT_FLOAT_EQ(min_val, -200.0f);
}

TEST_F(ScalarPolicyTest, MaxFloatBasic) {
    float max_val = ops.max_float(test_data.data(), test_data.size());
    EXPECT_FLOAT_EQ(max_val, 100.0f);
}

TEST_F(ScalarPolicyTest, MaxFloatEmpty) {
    float max_val = ops.max_float(nullptr, 0);
    EXPECT_FLOAT_EQ(max_val, std::numeric_limits<float>::lowest());
}

TEST_F(ScalarPolicyTest, MaxFloatWithNegatives) {
    std::vector<float> data = {-100.0f, 50.0f, -200.0f, 0.0f};
    float max_val = ops.max_float(data.data(), data.size());
    EXPECT_FLOAT_EQ(max_val, 50.0f);
}

TEST_F(ScalarPolicyTest, SumDoublesBasic) {
    std::vector<double> doubles;
    for (int i = 1; i <= 100; ++i) {
        doubles.push_back(static_cast<double>(i));
    }
    double sum = ops.sum_doubles(doubles.data(), doubles.size());
    EXPECT_DOUBLE_EQ(sum, 5050.0);
}

// ============================================================================
// Default Policy Tests
// ============================================================================

class DefaultPolicyTest : public ::testing::Test {
protected:
    simd_ops<> ops;  // Uses default_simd_policy
    std::vector<float> test_data;
    std::mt19937 rng{42};

    void SetUp() override {
        test_data.clear();
        for (int i = 1; i <= 100; ++i) {
            test_data.push_back(static_cast<float>(i));
        }
    }
};

TEST_F(DefaultPolicyTest, PolicyNameNotEmpty) {
    std::string_view name = simd_ops<>::policy_name();
    EXPECT_FALSE(name.empty());
    std::cout << "Default SIMD policy: " << name << std::endl;
}

TEST_F(DefaultPolicyTest, SimdWidthPositive) {
    size_t width = simd_ops<>::simd_width();
    EXPECT_GE(width, 1u);
    EXPECT_LE(width, 16u);
    std::cout << "Default SIMD width: " << width << std::endl;
}

TEST_F(DefaultPolicyTest, SumFloatsBasic) {
    float sum = ops.sum_floats(test_data.data(), test_data.size());
    EXPECT_FLOAT_EQ(sum, 5050.0f);
}

TEST_F(DefaultPolicyTest, SumFloatsLargeDataset) {
    std::vector<float> large_data(10000, 1.0f);
    float sum = ops.sum_floats(large_data.data(), large_data.size());
    EXPECT_NEAR(sum, 10000.0f, 0.01f);
}

TEST_F(DefaultPolicyTest, MinFloatBasic) {
    float min_val = ops.min_float(test_data.data(), test_data.size());
    EXPECT_FLOAT_EQ(min_val, 1.0f);
}

TEST_F(DefaultPolicyTest, MaxFloatBasic) {
    float max_val = ops.max_float(test_data.data(), test_data.size());
    EXPECT_FLOAT_EQ(max_val, 100.0f);
}

TEST_F(DefaultPolicyTest, NonAlignedSizeSum) {
    // Test with size that doesn't align to any SIMD width
    std::vector<float> data;
    for (int i = 1; i <= 17; ++i) {
        data.push_back(static_cast<float>(i));
    }
    float sum = ops.sum_floats(data.data(), data.size());
    // Sum of 1 to 17 = 17 * 18 / 2 = 153
    EXPECT_FLOAT_EQ(sum, 153.0f);
}

TEST_F(DefaultPolicyTest, MinMaxLargeDataset) {
    std::vector<float> large_data;
    large_data.reserve(10000);

    std::uniform_real_distribution<float> dist(-1000.0f, 1000.0f);

    float expected_min = std::numeric_limits<float>::max();
    float expected_max = std::numeric_limits<float>::lowest();

    for (int i = 0; i < 10000; ++i) {
        float val = dist(rng);
        large_data.push_back(val);
        expected_min = std::min(expected_min, val);
        expected_max = std::max(expected_max, val);
    }

    float min_val = ops.min_float(large_data.data(), large_data.size());
    float max_val = ops.max_float(large_data.data(), large_data.size());

    EXPECT_FLOAT_EQ(min_val, expected_min);
    EXPECT_FLOAT_EQ(max_val, expected_max);
}

// ============================================================================
// Cross-Policy Consistency Tests
// ============================================================================

class CrossPolicyConsistencyTest : public ::testing::Test {
protected:
    std::vector<float> test_data;
    std::mt19937 rng{42};

    void SetUp() override {
        test_data.clear();
        std::uniform_real_distribution<float> dist(-1000.0f, 1000.0f);
        for (int i = 0; i < 1000; ++i) {
            test_data.push_back(dist(rng));
        }
    }
};

TEST_F(CrossPolicyConsistencyTest, SumFloatsConsistency) {
    simd_ops<scalar_simd_policy> scalar_ops;
    simd_ops<> default_ops;

    float scalar_sum = scalar_ops.sum_floats(test_data.data(), test_data.size());
    float default_sum = default_ops.sum_floats(test_data.data(), test_data.size());

    // Allow small floating-point differences due to operation order
    EXPECT_NEAR(scalar_sum, default_sum, std::abs(scalar_sum) * 1e-5f);
}

TEST_F(CrossPolicyConsistencyTest, MinFloatConsistency) {
    simd_ops<scalar_simd_policy> scalar_ops;
    simd_ops<> default_ops;

    float scalar_min = scalar_ops.min_float(test_data.data(), test_data.size());
    float default_min = default_ops.min_float(test_data.data(), test_data.size());

    EXPECT_FLOAT_EQ(scalar_min, default_min);
}

TEST_F(CrossPolicyConsistencyTest, MaxFloatConsistency) {
    simd_ops<scalar_simd_policy> scalar_ops;
    simd_ops<> default_ops;

    float scalar_max = scalar_ops.max_float(test_data.data(), test_data.size());
    float default_max = default_ops.max_float(test_data.data(), test_data.size());

    EXPECT_FLOAT_EQ(scalar_max, default_max);
}

// ============================================================================
// Edge Cases Tests
// ============================================================================

class EdgeCasesTest : public ::testing::Test {
protected:
    simd_ops<> ops;
};

TEST_F(EdgeCasesTest, SingleElement) {
    float val = 42.0f;

    EXPECT_FLOAT_EQ(ops.sum_floats(&val, 1), 42.0f);
    EXPECT_FLOAT_EQ(ops.min_float(&val, 1), 42.0f);
    EXPECT_FLOAT_EQ(ops.max_float(&val, 1), 42.0f);
}

TEST_F(EdgeCasesTest, TwoElements) {
    std::vector<float> data = {10.0f, 20.0f};

    EXPECT_FLOAT_EQ(ops.sum_floats(data.data(), data.size()), 30.0f);
    EXPECT_FLOAT_EQ(ops.min_float(data.data(), data.size()), 10.0f);
    EXPECT_FLOAT_EQ(ops.max_float(data.data(), data.size()), 20.0f);
}

TEST_F(EdgeCasesTest, AllSameValues) {
    std::vector<float> data(100, 5.0f);

    EXPECT_FLOAT_EQ(ops.sum_floats(data.data(), data.size()), 500.0f);
    EXPECT_FLOAT_EQ(ops.min_float(data.data(), data.size()), 5.0f);
    EXPECT_FLOAT_EQ(ops.max_float(data.data(), data.size()), 5.0f);
}

TEST_F(EdgeCasesTest, VeryLargeValues) {
    std::vector<float> data = {1e38f, 1e37f, 1e36f};

    // Should not overflow for min/max
    EXPECT_FLOAT_EQ(ops.min_float(data.data(), data.size()), 1e36f);
    EXPECT_FLOAT_EQ(ops.max_float(data.data(), data.size()), 1e38f);
}

TEST_F(EdgeCasesTest, VerySmallValues) {
    std::vector<float> data = {1e-38f, 1e-37f, 1e-36f};

    EXPECT_FLOAT_EQ(ops.min_float(data.data(), data.size()), 1e-38f);
    EXPECT_FLOAT_EQ(ops.max_float(data.data(), data.size()), 1e-36f);
}

TEST_F(EdgeCasesTest, MixedPositiveNegative) {
    std::vector<float> data;
    for (int i = -50; i <= 50; ++i) {
        data.push_back(static_cast<float>(i));
    }

    // Sum of -50 to 50 = 0
    EXPECT_FLOAT_EQ(ops.sum_floats(data.data(), data.size()), 0.0f);
    EXPECT_FLOAT_EQ(ops.min_float(data.data(), data.size()), -50.0f);
    EXPECT_FLOAT_EQ(ops.max_float(data.data(), data.size()), 50.0f);
}

// ============================================================================
// Platform-Specific Policy Tests (conditional)
// ============================================================================

#if defined(CONTAINER_HAS_SSE42) || defined(CONTAINER_HAS_SSE2)
class SSEPolicyTest : public ::testing::Test {
protected:
    simd_ops<sse_simd_policy> ops;
};

TEST_F(SSEPolicyTest, PolicyName) {
    EXPECT_EQ(simd_ops<sse_simd_policy>::policy_name(), "sse");
}

TEST_F(SSEPolicyTest, SimdWidth) {
    EXPECT_EQ(simd_ops<sse_simd_policy>::simd_width(), 4u);
}

TEST_F(SSEPolicyTest, SumFloatsBasic) {
    std::vector<float> data;
    for (int i = 1; i <= 100; ++i) {
        data.push_back(static_cast<float>(i));
    }
    float sum = ops.sum_floats(data.data(), data.size());
    EXPECT_FLOAT_EQ(sum, 5050.0f);
}
#endif

#if defined(CONTAINER_HAS_AVX2)
class AVX2PolicyTest : public ::testing::Test {
protected:
    simd_ops<avx2_simd_policy> ops;
};

TEST_F(AVX2PolicyTest, PolicyName) {
    EXPECT_EQ(simd_ops<avx2_simd_policy>::policy_name(), "avx2");
}

TEST_F(AVX2PolicyTest, SimdWidth) {
    EXPECT_EQ(simd_ops<avx2_simd_policy>::simd_width(), 8u);
}

TEST_F(AVX2PolicyTest, SumFloatsBasic) {
    std::vector<float> data;
    for (int i = 1; i <= 100; ++i) {
        data.push_back(static_cast<float>(i));
    }
    float sum = ops.sum_floats(data.data(), data.size());
    EXPECT_FLOAT_EQ(sum, 5050.0f);
}
#endif

#if defined(CONTAINER_HAS_AVX512)
class AVX512PolicyTest : public ::testing::Test {
protected:
    simd_ops<avx512_simd_policy> ops;
};

TEST_F(AVX512PolicyTest, PolicyName) {
    EXPECT_EQ(simd_ops<avx512_simd_policy>::policy_name(), "avx512");
}

TEST_F(AVX512PolicyTest, SimdWidth) {
    EXPECT_EQ(simd_ops<avx512_simd_policy>::simd_width(), 16u);
}

TEST_F(AVX512PolicyTest, SumFloatsBasic) {
    std::vector<float> data;
    for (int i = 1; i <= 100; ++i) {
        data.push_back(static_cast<float>(i));
    }
    float sum = ops.sum_floats(data.data(), data.size());
    EXPECT_FLOAT_EQ(sum, 5050.0f);
}
#endif

#if defined(CONTAINER_HAS_ARM_NEON)
class NEONPolicyTest : public ::testing::Test {
protected:
    simd_ops<neon_simd_policy> ops;
};

TEST_F(NEONPolicyTest, PolicyName) {
    EXPECT_EQ(simd_ops<neon_simd_policy>::policy_name(), "neon");
}

TEST_F(NEONPolicyTest, SimdWidth) {
    EXPECT_EQ(simd_ops<neon_simd_policy>::simd_width(), 4u);
}

TEST_F(NEONPolicyTest, SumFloatsBasic) {
    std::vector<float> data;
    for (int i = 1; i <= 100; ++i) {
        data.push_back(static_cast<float>(i));
    }
    float sum = ops.sum_floats(data.data(), data.size());
    EXPECT_FLOAT_EQ(sum, 5050.0f);
}
#endif

// ============================================================================
// Concept Verification Tests
// ============================================================================

TEST(ConceptTest, ScalarPolicySatisfiesConcept) {
    EXPECT_TRUE(SimdPolicy<scalar_simd_policy>);
}

#if defined(CONTAINER_HAS_SSE42) || defined(CONTAINER_HAS_SSE2)
TEST(ConceptTest, SSEPolicySatisfiesConcept) {
    EXPECT_TRUE(SimdPolicy<sse_simd_policy>);
}
#endif

#if defined(CONTAINER_HAS_AVX2)
TEST(ConceptTest, AVX2PolicySatisfiesConcept) {
    EXPECT_TRUE(SimdPolicy<avx2_simd_policy>);
}
#endif

#if defined(CONTAINER_HAS_AVX512)
TEST(ConceptTest, AVX512PolicySatisfiesConcept) {
    EXPECT_TRUE(SimdPolicy<avx512_simd_policy>);
}
#endif

#if defined(CONTAINER_HAS_ARM_NEON)
TEST(ConceptTest, NEONPolicySatisfiesConcept) {
    EXPECT_TRUE(SimdPolicy<neon_simd_policy>);
}
#endif

int main(int argc, char **argv) {
    std::cout << "=== SIMD Policy Tests ===" << std::endl;
    std::cout << "Default policy: " << simd_ops<>::policy_name() << std::endl;
    std::cout << "SIMD width: " << simd_ops<>::simd_width() << std::endl;
    std::cout << "=========================" << std::endl;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
