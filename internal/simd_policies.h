/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, kcenon
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
 * @file simd_policies.h
 * @brief Compile-time SIMD policy selection for vectorized operations.
 *
 * This header provides policy classes for SIMD operations that are selected
 * at compile time based on the target platform. This eliminates runtime
 * branching overhead and enables full compiler optimization.
 *
 * Usage:
 * @code
 * // Default usage - automatically uses best policy for platform
 * simd_ops<> ops;
 * float result = ops.sum_floats(data, count);
 *
 * // Explicit policy (for testing or special cases)
 * simd_ops<scalar_simd_policy> scalar_ops;
 * @endcode
 *
 * @see Issue #338 for design rationale
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <string_view>
#include <concepts>

// Platform-specific SIMD headers
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #define CONTAINER_HAS_X86_SIMD 1
    #if defined(__AVX512F__) || defined(HAS_AVX512)
        #ifndef CONTAINER_HAS_AVX512
            #define CONTAINER_HAS_AVX512 1
        #endif
        #ifndef CONTAINER_HAS_AVX2
            #define CONTAINER_HAS_AVX2 1
        #endif
        #include <immintrin.h>
    #elif defined(__AVX2__) || defined(HAS_AVX2)
        #ifndef CONTAINER_HAS_AVX2
            #define CONTAINER_HAS_AVX2 1
        #endif
        #include <immintrin.h>
    #elif defined(__SSE4_2__) || defined(HAS_SSE42)
        #ifndef CONTAINER_HAS_SSE42
            #define CONTAINER_HAS_SSE42 1
        #endif
        #include <nmmintrin.h>
        #include <smmintrin.h>
        #include <tmmintrin.h>
    #elif defined(__SSE2__)
        #ifndef CONTAINER_HAS_SSE2
            #define CONTAINER_HAS_SSE2 1
        #endif
        #include <emmintrin.h>
    #endif
    #if defined(CONTAINER_HAS_SSE42) || defined(CONTAINER_HAS_SSE2)
        #include <xmmintrin.h>
        #include <emmintrin.h>
        #include <pmmintrin.h>
    #endif
#elif defined(__ARM_NEON) || defined(__aarch64__)
    #define CONTAINER_HAS_ARM_NEON 1
    #include <arm_neon.h>
#endif

namespace container_module::simd {

/**
 * @brief Concept for SIMD policy classes.
 *
 * A valid SIMD policy must provide:
 * - name() -> string_view: Returns the policy name
 * - simd_width -> size_t: SIMD register width in floats
 * - sum_floats(data, count) -> float: Sum float array
 * - min_float(data, count) -> float: Find minimum
 * - max_float(data, count) -> float: Find maximum
 */
template<typename T>
concept SimdPolicy = requires(const T& policy, const float* data, size_t count) {
    { T::name() } -> std::convertible_to<std::string_view>;
    { T::simd_width } -> std::convertible_to<size_t>;
    { policy.sum_floats(data, count) } -> std::same_as<float>;
    { policy.min_float(data, count) } -> std::same_as<float>;
    { policy.max_float(data, count) } -> std::same_as<float>;
};

// ============================================================================
// Scalar Policy (Fallback)
// ============================================================================

/**
 * @brief Scalar (non-SIMD) implementation of operations.
 *
 * Used as fallback when no SIMD instructions are available,
 * or for testing/comparison purposes.
 */
struct scalar_simd_policy {
    static constexpr std::string_view name() noexcept { return "scalar"; }
    static constexpr size_t simd_width = 1;

    [[nodiscard]] float sum_floats(const float* data, size_t count) const noexcept {
        float sum = 0.0f;
        for (size_t i = 0; i < count; ++i) {
            sum += data[i];
        }
        return sum;
    }

    [[nodiscard]] float min_float(const float* data, size_t count) const noexcept {
        if (count == 0) return std::numeric_limits<float>::max();
        float min_val = data[0];
        for (size_t i = 1; i < count; ++i) {
            if (data[i] < min_val) {
                min_val = data[i];
            }
        }
        return min_val;
    }

    [[nodiscard]] float max_float(const float* data, size_t count) const noexcept {
        if (count == 0) return std::numeric_limits<float>::lowest();
        float max_val = data[0];
        for (size_t i = 1; i < count; ++i) {
            if (data[i] > max_val) {
                max_val = data[i];
            }
        }
        return max_val;
    }

    [[nodiscard]] double sum_doubles(const double* data, size_t count) const noexcept {
        double sum = 0.0;
        for (size_t i = 0; i < count; ++i) {
            sum += data[i];
        }
        return sum;
    }
};

// ============================================================================
// SSE Policy (x86)
// ============================================================================

#if defined(CONTAINER_HAS_SSE42) || defined(CONTAINER_HAS_SSE2)
/**
 * @brief SSE implementation of SIMD operations (128-bit).
 */
struct sse_simd_policy {
    static constexpr std::string_view name() noexcept { return "sse"; }
    static constexpr size_t simd_width = 4;

    [[nodiscard]]
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((target("sse3")))
#endif
    float sum_floats(const float* data, size_t count) const noexcept {
        __m128 sum_vec = _mm_setzero_ps();
        size_t simd_end = count - (count % 4);

        for (size_t i = 0; i < simd_end; i += 4) {
            __m128 vec = _mm_loadu_ps(&data[i]);
            sum_vec = _mm_add_ps(sum_vec, vec);
        }

        // Horizontal sum using hadd
        sum_vec = _mm_hadd_ps(sum_vec, sum_vec);
        sum_vec = _mm_hadd_ps(sum_vec, sum_vec);

        float sum = _mm_cvtss_f32(sum_vec);

        // Handle remaining elements
        for (size_t i = simd_end; i < count; ++i) {
            sum += data[i];
        }

        return sum;
    }

    [[nodiscard]]
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((target("sse2")))
#endif
    float min_float(const float* data, size_t count) const noexcept {
        if (count == 0) return std::numeric_limits<float>::max();

        __m128 min_vec = _mm_set1_ps(std::numeric_limits<float>::max());
        size_t simd_end = count - (count % 4);

        for (size_t i = 0; i < simd_end; i += 4) {
            __m128 vec = _mm_loadu_ps(&data[i]);
            min_vec = _mm_min_ps(min_vec, vec);
        }

        // Extract minimum from vector
        alignas(16) float result[4];
        _mm_store_ps(result, min_vec);
        float min_val = result[0];
        for (int i = 1; i < 4; ++i) {
            if (result[i] < min_val) min_val = result[i];
        }

        // Handle remaining elements
        for (size_t i = simd_end; i < count; ++i) {
            if (data[i] < min_val) min_val = data[i];
        }

        return min_val;
    }

    [[nodiscard]]
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((target("sse2")))
#endif
    float max_float(const float* data, size_t count) const noexcept {
        if (count == 0) return std::numeric_limits<float>::lowest();

        __m128 max_vec = _mm_set1_ps(std::numeric_limits<float>::lowest());
        size_t simd_end = count - (count % 4);

        for (size_t i = 0; i < simd_end; i += 4) {
            __m128 vec = _mm_loadu_ps(&data[i]);
            max_vec = _mm_max_ps(max_vec, vec);
        }

        // Extract maximum from vector
        alignas(16) float result[4];
        _mm_store_ps(result, max_vec);
        float max_val = result[0];
        for (int i = 1; i < 4; ++i) {
            if (result[i] > max_val) max_val = result[i];
        }

        // Handle remaining elements
        for (size_t i = simd_end; i < count; ++i) {
            if (data[i] > max_val) max_val = data[i];
        }

        return max_val;
    }

    [[nodiscard]] double sum_doubles(const double* data, size_t count) const noexcept {
        // Use scalar for doubles in SSE (simpler)
        double sum = 0.0;
        for (size_t i = 0; i < count; ++i) {
            sum += data[i];
        }
        return sum;
    }
};
#endif

// ============================================================================
// AVX2 Policy (x86)
// ============================================================================

#if defined(CONTAINER_HAS_AVX2)
/**
 * @brief AVX2 implementation of SIMD operations (256-bit).
 */
struct avx2_simd_policy {
    static constexpr std::string_view name() noexcept { return "avx2"; }
    static constexpr size_t simd_width = 8;

    [[nodiscard]]
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((target("avx2")))
#endif
    float sum_floats(const float* data, size_t count) const noexcept {
        __m256 sum_vec = _mm256_setzero_ps();
        size_t simd_end = count - (count % 8);

        for (size_t i = 0; i < simd_end; i += 8) {
            __m256 vec = _mm256_loadu_ps(&data[i]);
            sum_vec = _mm256_add_ps(sum_vec, vec);
        }

        // Horizontal sum
        __m128 low = _mm256_castps256_ps128(sum_vec);
        __m128 high = _mm256_extractf128_ps(sum_vec, 1);
        __m128 sum128 = _mm_add_ps(low, high);
        sum128 = _mm_hadd_ps(sum128, sum128);
        sum128 = _mm_hadd_ps(sum128, sum128);

        float sum = _mm_cvtss_f32(sum128);

        // Handle remaining elements
        for (size_t i = simd_end; i < count; ++i) {
            sum += data[i];
        }

        return sum;
    }

    [[nodiscard]]
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((target("avx2")))
#endif
    float min_float(const float* data, size_t count) const noexcept {
        if (count == 0) return std::numeric_limits<float>::max();

        __m256 min_vec = _mm256_set1_ps(std::numeric_limits<float>::max());
        size_t simd_end = count - (count % 8);

        for (size_t i = 0; i < simd_end; i += 8) {
            __m256 vec = _mm256_loadu_ps(&data[i]);
            min_vec = _mm256_min_ps(min_vec, vec);
        }

        // Extract minimum from vector
        alignas(32) float result[8];
        _mm256_store_ps(result, min_vec);
        float min_val = result[0];
        for (int i = 1; i < 8; ++i) {
            if (result[i] < min_val) min_val = result[i];
        }

        // Handle remaining elements
        for (size_t i = simd_end; i < count; ++i) {
            if (data[i] < min_val) min_val = data[i];
        }

        return min_val;
    }

    [[nodiscard]]
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((target("avx2")))
#endif
    float max_float(const float* data, size_t count) const noexcept {
        if (count == 0) return std::numeric_limits<float>::lowest();

        __m256 max_vec = _mm256_set1_ps(std::numeric_limits<float>::lowest());
        size_t simd_end = count - (count % 8);

        for (size_t i = 0; i < simd_end; i += 8) {
            __m256 vec = _mm256_loadu_ps(&data[i]);
            max_vec = _mm256_max_ps(max_vec, vec);
        }

        // Extract maximum from vector
        alignas(32) float result[8];
        _mm256_store_ps(result, max_vec);
        float max_val = result[0];
        for (int i = 1; i < 8; ++i) {
            if (result[i] > max_val) max_val = result[i];
        }

        // Handle remaining elements
        for (size_t i = simd_end; i < count; ++i) {
            if (data[i] > max_val) max_val = data[i];
        }

        return max_val;
    }

    [[nodiscard]] double sum_doubles(const double* data, size_t count) const noexcept {
        double sum = 0.0;
        for (size_t i = 0; i < count; ++i) {
            sum += data[i];
        }
        return sum;
    }
};
#endif

// ============================================================================
// AVX-512 Policy (x86)
// ============================================================================

#if defined(CONTAINER_HAS_AVX512)
/**
 * @brief AVX-512 implementation of SIMD operations (512-bit).
 */
struct avx512_simd_policy {
    static constexpr std::string_view name() noexcept { return "avx512"; }
    static constexpr size_t simd_width = 16;

    [[nodiscard]]
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((target("avx512f")))
#endif
    float sum_floats(const float* data, size_t count) const noexcept {
        __m512 sum_vec = _mm512_setzero_ps();
        size_t simd_end = count - (count % 16);

        for (size_t i = 0; i < simd_end; i += 16) {
            __m512 vec = _mm512_loadu_ps(&data[i]);
            sum_vec = _mm512_add_ps(sum_vec, vec);
        }

        float sum = _mm512_reduce_add_ps(sum_vec);

        for (size_t i = simd_end; i < count; ++i) {
            sum += data[i];
        }

        return sum;
    }

    [[nodiscard]]
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((target("avx512f")))
#endif
    float min_float(const float* data, size_t count) const noexcept {
        if (count == 0) return std::numeric_limits<float>::max();

        __m512 min_vec = _mm512_set1_ps(std::numeric_limits<float>::max());
        size_t simd_end = count - (count % 16);

        for (size_t i = 0; i < simd_end; i += 16) {
            __m512 vec = _mm512_loadu_ps(&data[i]);
            min_vec = _mm512_min_ps(min_vec, vec);
        }

        float min_val = _mm512_reduce_min_ps(min_vec);

        for (size_t i = simd_end; i < count; ++i) {
            if (data[i] < min_val) min_val = data[i];
        }

        return min_val;
    }

    [[nodiscard]]
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((target("avx512f")))
#endif
    float max_float(const float* data, size_t count) const noexcept {
        if (count == 0) return std::numeric_limits<float>::lowest();

        __m512 max_vec = _mm512_set1_ps(std::numeric_limits<float>::lowest());
        size_t simd_end = count - (count % 16);

        for (size_t i = 0; i < simd_end; i += 16) {
            __m512 vec = _mm512_loadu_ps(&data[i]);
            max_vec = _mm512_max_ps(max_vec, vec);
        }

        float max_val = _mm512_reduce_max_ps(max_vec);

        for (size_t i = simd_end; i < count; ++i) {
            if (data[i] > max_val) max_val = data[i];
        }

        return max_val;
    }

    [[nodiscard]]
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((target("avx512f")))
#endif
    double sum_doubles(const double* data, size_t count) const noexcept {
        __m512d sum_vec = _mm512_setzero_pd();
        size_t simd_end = count - (count % 8);

        for (size_t i = 0; i < simd_end; i += 8) {
            __m512d vec = _mm512_loadu_pd(&data[i]);
            sum_vec = _mm512_add_pd(sum_vec, vec);
        }

        double sum = _mm512_reduce_add_pd(sum_vec);

        for (size_t i = simd_end; i < count; ++i) {
            sum += data[i];
        }

        return sum;
    }
};
#endif

// ============================================================================
// NEON Policy (ARM)
// ============================================================================

#if defined(CONTAINER_HAS_ARM_NEON)
/**
 * @brief ARM NEON implementation of SIMD operations (128-bit).
 */
struct neon_simd_policy {
    static constexpr std::string_view name() noexcept { return "neon"; }
    static constexpr size_t simd_width = 4;

    [[nodiscard]] float sum_floats(const float* data, size_t count) const noexcept {
        float32x4_t sum_vec = vdupq_n_f32(0.0f);
        size_t simd_end = count - (count % 4);

        for (size_t i = 0; i < simd_end; i += 4) {
            float32x4_t vec = vld1q_f32(&data[i]);
            sum_vec = vaddq_f32(sum_vec, vec);
        }

        // Horizontal sum
        float32x2_t sum_low = vget_low_f32(sum_vec);
        float32x2_t sum_high = vget_high_f32(sum_vec);
        float32x2_t sum_pair = vadd_f32(sum_low, sum_high);
        float sum = vget_lane_f32(sum_pair, 0) + vget_lane_f32(sum_pair, 1);

        for (size_t i = simd_end; i < count; ++i) {
            sum += data[i];
        }

        return sum;
    }

    [[nodiscard]] float min_float(const float* data, size_t count) const noexcept {
        if (count == 0) return std::numeric_limits<float>::max();

        float32x4_t min_vec = vdupq_n_f32(std::numeric_limits<float>::max());
        size_t simd_end = count - (count % 4);

        for (size_t i = 0; i < simd_end; i += 4) {
            float32x4_t vec = vld1q_f32(&data[i]);
            min_vec = vminq_f32(min_vec, vec);
        }

        // Extract minimum from vector
        float result[4];
        vst1q_f32(result, min_vec);
        float min_val = result[0];
        for (int i = 1; i < 4; ++i) {
            if (result[i] < min_val) min_val = result[i];
        }

        for (size_t i = simd_end; i < count; ++i) {
            if (data[i] < min_val) min_val = data[i];
        }

        return min_val;
    }

    [[nodiscard]] float max_float(const float* data, size_t count) const noexcept {
        if (count == 0) return std::numeric_limits<float>::lowest();

        float32x4_t max_vec = vdupq_n_f32(std::numeric_limits<float>::lowest());
        size_t simd_end = count - (count % 4);

        for (size_t i = 0; i < simd_end; i += 4) {
            float32x4_t vec = vld1q_f32(&data[i]);
            max_vec = vmaxq_f32(max_vec, vec);
        }

        // Extract maximum from vector
        float result[4];
        vst1q_f32(result, max_vec);
        float max_val = result[0];
        for (int i = 1; i < 4; ++i) {
            if (result[i] > max_val) max_val = result[i];
        }

        for (size_t i = simd_end; i < count; ++i) {
            if (data[i] > max_val) max_val = data[i];
        }

        return max_val;
    }

    [[nodiscard]] double sum_doubles(const double* data, size_t count) const noexcept {
        double sum = 0.0;
        for (size_t i = 0; i < count; ++i) {
            sum += data[i];
        }
        return sum;
    }
};
#endif

// ============================================================================
// Compile-Time Policy Selection
// ============================================================================

/**
 * @brief Default SIMD policy selected at compile time based on platform.
 *
 * Selection priority (highest to lowest):
 * 1. AVX-512 (x86-64 with AVX-512F support)
 * 2. AVX2 (x86-64 with AVX2 support)
 * 3. SSE (x86 with SSE4.2/SSE2 support)
 * 4. NEON (ARM with NEON support)
 * 5. Scalar (fallback for all platforms)
 */
#if defined(CONTAINER_HAS_AVX512)
    using default_simd_policy = avx512_simd_policy;
#elif defined(CONTAINER_HAS_AVX2)
    using default_simd_policy = avx2_simd_policy;
#elif defined(CONTAINER_HAS_SSE42) || defined(CONTAINER_HAS_SSE2)
    using default_simd_policy = sse_simd_policy;
#elif defined(CONTAINER_HAS_ARM_NEON)
    using default_simd_policy = neon_simd_policy;
#else
    using default_simd_policy = scalar_simd_policy;
#endif

/**
 * @brief SIMD operations wrapper with compile-time policy selection.
 *
 * @tparam Policy SIMD policy class (must satisfy SimdPolicy concept)
 *
 * Usage:
 * @code
 * // Use default (best) policy for platform
 * simd_ops<> ops;
 *
 * // Use explicit scalar policy for testing
 * simd_ops<scalar_simd_policy> scalar_ops;
 * @endcode
 */
template<SimdPolicy Policy = default_simd_policy>
class simd_ops {
public:
    explicit simd_ops(Policy policy = Policy{}) noexcept
        : policy_(std::move(policy)) {}

    /**
     * @brief Get the name of the active SIMD policy
     */
    static constexpr std::string_view policy_name() noexcept {
        return Policy::name();
    }

    /**
     * @brief Get the SIMD width (number of floats per operation)
     */
    static constexpr size_t simd_width() noexcept {
        return Policy::simd_width;
    }

    /**
     * @brief Sum all floats in an array
     */
    [[nodiscard]] float sum_floats(const float* data, size_t count) const noexcept {
        return policy_.sum_floats(data, count);
    }

    /**
     * @brief Find minimum float in an array
     */
    [[nodiscard]] float min_float(const float* data, size_t count) const noexcept {
        return policy_.min_float(data, count);
    }

    /**
     * @brief Find maximum float in an array
     */
    [[nodiscard]] float max_float(const float* data, size_t count) const noexcept {
        return policy_.max_float(data, count);
    }

    /**
     * @brief Sum all doubles in an array
     */
    [[nodiscard]] double sum_doubles(const double* data, size_t count) const noexcept {
        return policy_.sum_doubles(data, count);
    }

private:
    [[no_unique_address]] Policy policy_;
};

// Static assertions to verify policy compliance
static_assert(SimdPolicy<scalar_simd_policy>, "scalar_simd_policy must satisfy SimdPolicy");
#if defined(CONTAINER_HAS_SSE42) || defined(CONTAINER_HAS_SSE2)
static_assert(SimdPolicy<sse_simd_policy>, "sse_simd_policy must satisfy SimdPolicy");
#endif
#if defined(CONTAINER_HAS_AVX2)
static_assert(SimdPolicy<avx2_simd_policy>, "avx2_simd_policy must satisfy SimdPolicy");
#endif
#if defined(CONTAINER_HAS_AVX512)
static_assert(SimdPolicy<avx512_simd_policy>, "avx512_simd_policy must satisfy SimdPolicy");
#endif
#if defined(CONTAINER_HAS_ARM_NEON)
static_assert(SimdPolicy<neon_simd_policy>, "neon_simd_policy must satisfy SimdPolicy");
#endif

} // namespace container_module::simd
