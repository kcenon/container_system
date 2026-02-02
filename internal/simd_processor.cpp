/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
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

#include "simd_processor.h"
#include <algorithm>
#include <cmath>
#include <limits>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #if defined(__GNUC__) || defined(__clang__)
        #include <cpuid.h>
    #endif
#endif

namespace container_module
{
namespace simd
{
    // Scalar implementations (fallback)
    float simd_processor::sum_floats_scalar(const float* data, size_t count)
    {
        float sum = 0.0f;
        for (size_t i = 0; i < count; ++i) {
            sum += data[i];
        }
        return sum;
    }

    float simd_processor::min_float_scalar(const float* data, size_t count)
    {
        if (count == 0) return std::numeric_limits<float>::max();
        
        float min_val = data[0];
        for (size_t i = 1; i < count; ++i) {
            if (data[i] < min_val) {
                min_val = data[i];
            }
        }
        return min_val;
    }

    float simd_processor::max_float_scalar(const float* data, size_t count)
    {
        if (count == 0) return std::numeric_limits<float>::lowest();
        
        float max_val = data[0];
        for (size_t i = 1; i < count; ++i) {
            if (data[i] > max_val) {
                max_val = data[i];
            }
        }
        return max_val;
    }

#if defined(HAS_AVX512)
    __attribute__((target("avx512f")))
    float simd_processor::sum_floats_avx512(const float* data, size_t count)
    {
        __m512 sum_vec = _mm512_setzero_ps();
        size_t simd_end = count - (count % 16);

        // Process 16 floats at a time
        for (size_t i = 0; i < simd_end; i += 16) {
            __m512 vec = _mm512_loadu_ps(&data[i]);
            sum_vec = _mm512_add_ps(sum_vec, vec);
        }

        // Horizontal sum using _mm512_reduce_add_ps
        float sum = _mm512_reduce_add_ps(sum_vec);

        // Handle remaining elements
        for (size_t i = simd_end; i < count; ++i) {
            sum += data[i];
        }

        return sum;
    }

    __attribute__((target("avx512f")))
    float simd_processor::min_float_avx512(const float* data, size_t count)
    {
        if (count == 0) return std::numeric_limits<float>::max();

        __m512 min_vec = _mm512_set1_ps(std::numeric_limits<float>::max());
        size_t simd_end = count - (count % 16);

        for (size_t i = 0; i < simd_end; i += 16) {
            __m512 vec = _mm512_loadu_ps(&data[i]);
            min_vec = _mm512_min_ps(min_vec, vec);
        }

        // Horizontal minimum using _mm512_reduce_min_ps
        float min_val = _mm512_reduce_min_ps(min_vec);

        // Handle remaining elements
        for (size_t i = simd_end; i < count; ++i) {
            if (data[i] < min_val) min_val = data[i];
        }

        return min_val;
    }

    __attribute__((target("avx512f")))
    float simd_processor::max_float_avx512(const float* data, size_t count)
    {
        if (count == 0) return std::numeric_limits<float>::lowest();

        __m512 max_vec = _mm512_set1_ps(std::numeric_limits<float>::lowest());
        size_t simd_end = count - (count % 16);

        for (size_t i = 0; i < simd_end; i += 16) {
            __m512 vec = _mm512_loadu_ps(&data[i]);
            max_vec = _mm512_max_ps(max_vec, vec);
        }

        // Horizontal maximum using _mm512_reduce_max_ps
        float max_val = _mm512_reduce_max_ps(max_vec);

        // Handle remaining elements
        for (size_t i = simd_end; i < count; ++i) {
            if (data[i] > max_val) max_val = data[i];
        }

        return max_val;
    }

    __attribute__((target("avx512f")))
    double simd_processor::sum_doubles_avx512(const double* data, size_t count)
    {
        __m512d sum_vec = _mm512_setzero_pd();
        size_t simd_end = count - (count % 8);

        // Process 8 doubles at a time
        for (size_t i = 0; i < simd_end; i += 8) {
            __m512d vec = _mm512_loadu_pd(&data[i]);
            sum_vec = _mm512_add_pd(sum_vec, vec);
        }

        // Horizontal sum using _mm512_reduce_add_pd
        double sum = _mm512_reduce_add_pd(sum_vec);

        // Handle remaining elements
        for (size_t i = simd_end; i < count; ++i) {
            sum += data[i];
        }

        return sum;
    }
#endif

#if defined(HAS_AVX2)
    __attribute__((target("avx2")))
    float simd_processor::sum_floats_avx2(const float* data, size_t count)
    {
        __m256 sum_vec = _mm256_setzero_ps();
        size_t simd_end = count - (count % 8);
        
        // Process 8 floats at a time
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

    __attribute__((target("avx2")))
    float simd_processor::min_float_avx2(const float* data, size_t count)
    {
        if (count == 0) return std::numeric_limits<float>::max();
        
        __m256 min_vec = _mm256_set1_ps(std::numeric_limits<float>::max());
        size_t simd_end = count - (count % 8);
        
        for (size_t i = 0; i < simd_end; i += 8) {
            __m256 vec = _mm256_loadu_ps(&data[i]);
            min_vec = _mm256_min_ps(min_vec, vec);
        }
        
        // Extract minimum from vector
        float result[8];
        _mm256_storeu_ps(result, min_vec);
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

    __attribute__((target("avx2")))
    float simd_processor::max_float_avx2(const float* data, size_t count)
    {
        if (count == 0) return std::numeric_limits<float>::lowest();
        
        __m256 max_vec = _mm256_set1_ps(std::numeric_limits<float>::lowest());
        size_t simd_end = count - (count % 8);
        
        for (size_t i = 0; i < simd_end; i += 8) {
            __m256 vec = _mm256_loadu_ps(&data[i]);
            max_vec = _mm256_max_ps(max_vec, vec);
        }
        
        // Extract maximum from vector
        float result[8];
        _mm256_storeu_ps(result, max_vec);
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
#endif

#if defined(HAS_X86_SIMD) && (defined(HAS_SSE2) || defined(HAS_SSE42))
    __attribute__((target("sse3")))
    float simd_processor::sum_floats_sse(const float* data, size_t count)
    {
        __m128 sum_vec = _mm_setzero_ps();
        size_t simd_end = count - (count % 4);
        
        // Process 4 floats at a time
        for (size_t i = 0; i < simd_end; i += 4) {
            __m128 vec = _mm_loadu_ps(&data[i]);
            sum_vec = _mm_add_ps(sum_vec, vec);
        }
        
        // Horizontal sum
        sum_vec = _mm_hadd_ps(sum_vec, sum_vec);
        sum_vec = _mm_hadd_ps(sum_vec, sum_vec);
        
        float sum = _mm_cvtss_f32(sum_vec);
        
        // Handle remaining elements
        for (size_t i = simd_end; i < count; ++i) {
            sum += data[i];
        }
        
        return sum;
    }

    __attribute__((target("sse2")))
    float simd_processor::min_float_sse(const float* data, size_t count)
    {
        if (count == 0) return std::numeric_limits<float>::max();
        
        __m128 min_vec = _mm_set1_ps(std::numeric_limits<float>::max());
        size_t simd_end = count - (count % 4);
        
        for (size_t i = 0; i < simd_end; i += 4) {
            __m128 vec = _mm_loadu_ps(&data[i]);
            min_vec = _mm_min_ps(min_vec, vec);
        }
        
        // Extract minimum from vector
        float result[4];
        _mm_storeu_ps(result, min_vec);
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

    __attribute__((target("sse2")))
    float simd_processor::max_float_sse(const float* data, size_t count)
    {
        if (count == 0) return std::numeric_limits<float>::lowest();
        
        __m128 max_vec = _mm_set1_ps(std::numeric_limits<float>::lowest());
        size_t simd_end = count - (count % 4);
        
        for (size_t i = 0; i < simd_end; i += 4) {
            __m128 vec = _mm_loadu_ps(&data[i]);
            max_vec = _mm_max_ps(max_vec, vec);
        }
        
        // Extract maximum from vector
        float result[4];
        _mm_storeu_ps(result, max_vec);
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
#endif

#if defined(HAS_ARM_NEON)
    float simd_processor::sum_floats_neon(const float* data, size_t count)
    {
        float32x4_t sum_vec = vdupq_n_f32(0.0f);
        size_t simd_end = count - (count % 4);
        
        // Process 4 floats at a time
        for (size_t i = 0; i < simd_end; i += 4) {
            float32x4_t vec = vld1q_f32(&data[i]);
            sum_vec = vaddq_f32(sum_vec, vec);
        }
        
        // Horizontal sum
        float32x2_t sum_low = vget_low_f32(sum_vec);
        float32x2_t sum_high = vget_high_f32(sum_vec);
        float32x2_t sum_pair = vadd_f32(sum_low, sum_high);
        float sum = vget_lane_f32(sum_pair, 0) + vget_lane_f32(sum_pair, 1);
        
        // Handle remaining elements
        for (size_t i = simd_end; i < count; ++i) {
            sum += data[i];
        }
        
        return sum;
    }

    float simd_processor::min_float_neon(const float* data, size_t count)
    {
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
        
        // Handle remaining elements
        for (size_t i = simd_end; i < count; ++i) {
            if (data[i] < min_val) min_val = data[i];
        }
        
        return min_val;
    }

    float simd_processor::max_float_neon(const float* data, size_t count)
    {
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
        
        // Handle remaining elements
        for (size_t i = simd_end; i < count; ++i) {
            if (data[i] > max_val) max_val = data[i];
        }
        
        return max_val;
    }
#endif

    // Main interface implementations
    float simd_processor::sum_floats(const std::vector<ValueVariant>& values)
    {
        // Extract float values
        std::vector<float> floats;
        floats.reserve(values.size());
        
        for (const auto& val : values) {
            if (auto* f = std::get_if<float>(&val)) {
                floats.push_back(*f);
            }
        }
        
        if (floats.empty()) return 0.0f;

        #if defined(HAS_AVX512)
            return sum_floats_avx512(floats.data(), floats.size());
        #elif defined(HAS_AVX2)
            return sum_floats_avx2(floats.data(), floats.size());
        #elif defined(HAS_X86_SIMD) && (defined(HAS_SSE2) || defined(HAS_SSE42))
            return sum_floats_sse(floats.data(), floats.size());
        #elif defined(HAS_ARM_NEON)
            return sum_floats_neon(floats.data(), floats.size());
        #else
            return sum_floats_scalar(floats.data(), floats.size());
        #endif
    }

    double simd_processor::sum_doubles(const std::vector<ValueVariant>& values)
    {
        // For now, use scalar implementation for doubles
        double sum = 0.0;
        for (const auto& val : values) {
            if (auto* d = std::get_if<double>(&val)) {
                sum += *d;
            }
        }
        return sum;
    }

    std::optional<float> simd_processor::min_float(const std::vector<ValueVariant>& values)
    {
        std::vector<float> floats;
        floats.reserve(values.size());
        
        for (const auto& val : values) {
            if (auto* f = std::get_if<float>(&val)) {
                floats.push_back(*f);
            }
        }
        
        if (floats.empty()) return std::nullopt;

        #if defined(HAS_AVX512)
            return min_float_avx512(floats.data(), floats.size());
        #elif defined(HAS_AVX2)
            return min_float_avx2(floats.data(), floats.size());
        #elif defined(HAS_X86_SIMD) && (defined(HAS_SSE2) || defined(HAS_SSE42))
            return min_float_sse(floats.data(), floats.size());
        #elif defined(HAS_ARM_NEON)
            return min_float_neon(floats.data(), floats.size());
        #else
            return min_float_scalar(floats.data(), floats.size());
        #endif
    }

    std::optional<float> simd_processor::max_float(const std::vector<ValueVariant>& values)
    {
        std::vector<float> floats;
        floats.reserve(values.size());
        
        for (const auto& val : values) {
            if (auto* f = std::get_if<float>(&val)) {
                floats.push_back(*f);
            }
        }
        
        if (floats.empty()) return std::nullopt;

        #if defined(HAS_AVX512)
            return max_float_avx512(floats.data(), floats.size());
        #elif defined(HAS_AVX2)
            return max_float_avx2(floats.data(), floats.size());
        #elif defined(HAS_X86_SIMD) && (defined(HAS_SSE2) || defined(HAS_SSE42))
            return max_float_sse(floats.data(), floats.size());
        #elif defined(HAS_ARM_NEON)
            return max_float_neon(floats.data(), floats.size());
        #else
            return max_float_scalar(floats.data(), floats.size());
        #endif
    }

    std::vector<size_t> simd_processor::find_equal_floats(
        const std::vector<ValueVariant>& values, float target)
    {
        std::vector<size_t> indices;
        
        for (size_t i = 0; i < values.size(); ++i) {
            if (auto* f = std::get_if<float>(&values[i])) {
                if (*f == target) {
                    indices.push_back(i);
                }
            }
        }
        
        return indices;
    }

    void simd_processor::fast_copy(const void* src, void* dst, size_t size)
    {
        // Use standard memcpy which is often optimized with SIMD
        std::memcpy(dst, src, size);
    }

    bool simd_processor::fast_compare(const void* a, const void* b, size_t size)
    {
        return std::memcmp(a, b, size) == 0;
    }

    // SIMD support detection
    bool simd_support::has_sse2()
    {
        #if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
            #if defined(__GNUC__) || defined(__clang__)
                unsigned int eax, ebx, ecx, edx;
                if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
                    return (edx & (1 << 26)) != 0; // SSE2 bit
                }
            #endif
        #endif
        return false;
    }

    bool simd_support::has_sse42()
    {
        #if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
            #if defined(__GNUC__) || defined(__clang__)
                unsigned int eax, ebx, ecx, edx;
                if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
                    return (ecx & (1 << 20)) != 0; // SSE4.2 bit
                }
            #endif
        #endif
        return false;
    }

    bool simd_support::has_avx2()
    {
        #if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
            #if defined(__GNUC__) || defined(__clang__)
                unsigned int eax, ebx, ecx, edx;
                if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
                    return (ebx & (1 << 5)) != 0; // AVX2 bit
                }
            #endif
        #endif
        return false;
    }

    bool simd_support::has_avx512f()
    {
        #if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
            #if defined(__GNUC__) || defined(__clang__)
                unsigned int eax, ebx, ecx, edx;
                if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
                    return (ebx & (1 << 16)) != 0; // AVX-512F bit
                }
            #endif
        #endif
        return false;
    }

    bool simd_support::has_avx512dq()
    {
        #if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
            #if defined(__GNUC__) || defined(__clang__)
                unsigned int eax, ebx, ecx, edx;
                if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
                    return (ebx & (1 << 17)) != 0; // AVX-512DQ bit
                }
            #endif
        #endif
        return false;
    }

    bool simd_support::has_avx512bw()
    {
        #if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
            #if defined(__GNUC__) || defined(__clang__)
                unsigned int eax, ebx, ecx, edx;
                if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
                    return (ebx & (1 << 30)) != 0; // AVX-512BW bit
                }
            #endif
        #endif
        return false;
    }

    bool simd_support::has_avx512vl()
    {
        #if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
            #if defined(__GNUC__) || defined(__clang__)
                unsigned int eax, ebx, ecx, edx;
                if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
                    return (ebx & (1 << 31)) != 0; // AVX-512VL bit
                }
            #endif
        #endif
        return false;
    }

    simd_level simd_support::get_best_simd_level()
    {
        #if defined(HAS_ARM_NEON)
            return simd_level::neon;
        #endif
        if (has_avx512f()) return simd_level::avx512;
        if (has_avx2()) return simd_level::avx2;
        if (has_sse42()) return simd_level::sse42;
        if (has_sse2()) return simd_level::sse2;
        return simd_level::none;
    }

    bool simd_support::has_neon()
    {
        #if defined(HAS_ARM_NEON)
            return true;
        #else
            return false;
        #endif
    }

    std::string simd_support::get_simd_info()
    {
        std::string info = "SIMD Support: ";

        #if defined(HAS_AVX512)
            info += "AVX-512 ";
        #elif defined(HAS_AVX2)
            info += "AVX2 ";
        #elif defined(HAS_SSE42)
            info += "SSE4.2 ";
        #elif defined(HAS_SSE2)
            info += "SSE2 ";
        #elif defined(HAS_ARM_NEON)
            info += "NEON ";
        #else
            info += "None ";
        #endif

        // Add runtime detection info
        info += "(Compile-time), Runtime: ";
        if (has_avx512f()) {
            info += "AVX-512F ";
            if (has_avx512dq()) info += "AVX-512DQ ";
            if (has_avx512bw()) info += "AVX-512BW ";
            if (has_avx512vl()) info += "AVX-512VL ";
        } else if (has_avx2()) {
            info += "AVX2 ";
        } else if (has_sse42()) {
            info += "SSE4.2 ";
        } else if (has_sse2()) {
            info += "SSE2 ";
        } else if (has_neon()) {
            info += "NEON ";
        } else {
            info += "None ";
        }

        info += "(Width: " + std::to_string(get_optimal_width()) + ")";
        return info;
    }

} // namespace simd
} // namespace container_module