// BSD 3-Clause License
//
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
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

#pragma once

/**
 * @file internal/pool_allocator.h
 * @brief Thread-local memory pool allocator for value_container
 *
 * Provides pool-based allocation for small objects to reduce malloc overhead
 * and improve cache locality. Uses thread-local pools for lock-free fast path.
 *
 * Size classes:
 * - Small pool: <= 64 bytes
 * - Medium pool: <= 256 bytes
 * - Large: Direct heap allocation (bypasses pool)
 */

#include "container/internal/memory_pool.h"

#include <cstddef>
#include <atomic>
#include <new>
#include <type_traits>
#include <utility>

namespace container_module::internal {

/**
 * @brief Size class thresholds for pool allocation
 */
struct pool_size_class {
    static constexpr std::size_t small_threshold = 64;    // <= 64 bytes
    static constexpr std::size_t medium_threshold = 256;  // <= 256 bytes
    static constexpr std::size_t blocks_per_chunk = 1024;
};

/**
 * @brief Extended pool statistics with hit/miss tracking
 */
struct pool_allocator_stats {
    std::size_t pool_hits{0};           // Allocations satisfied from pool
    std::size_t pool_misses{0};         // Allocations that went to heap
    std::size_t small_pool_allocs{0};   // Small pool allocations
    std::size_t medium_pool_allocs{0};  // Medium pool allocations
    std::size_t deallocations{0};       // Total deallocations

    double hit_rate() const noexcept {
        auto total = pool_hits + pool_misses;
        return total > 0 ? static_cast<double>(pool_hits) / total : 0.0;
    }
};

/**
 * @brief Thread-local pool manager for value_container allocations
 *
 * Manages two size-class pools (small and medium) with thread-local
 * instances for lock-free allocation on the fast path.
 */
class pool_allocator {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to thread-local pool allocator
     */
    static pool_allocator& instance() noexcept {
        thread_local pool_allocator allocator;
        return allocator;
    }

    /**
     * @brief Allocate memory from the appropriate pool
     * @param size Number of bytes to allocate
     * @return Pointer to allocated memory, or nullptr on failure
     */
    void* allocate(std::size_t size) noexcept {
        if (size == 0) {
            return nullptr;
        }

#if CONTAINER_USE_MEMORY_POOL
        try {
            if (size <= pool_size_class::small_threshold) {
                void* ptr = small_pool_.allocate();
                stats_.pool_hits++;
                stats_.small_pool_allocs++;
                return ptr;
            } else if (size <= pool_size_class::medium_threshold) {
                void* ptr = medium_pool_.allocate();
                stats_.pool_hits++;
                stats_.medium_pool_allocs++;
                return ptr;
            }
        } catch (...) {
            // Pool allocation failed, fall through to heap
        }
#endif
        // Large allocation or pool disabled: use heap
        stats_.pool_misses++;
        return ::operator new(size, std::nothrow);
    }

    /**
     * @brief Deallocate memory to the appropriate pool
     * @param ptr Pointer to deallocate
     * @param size Size of the allocation
     */
    void deallocate(void* ptr, std::size_t size) noexcept {
        if (!ptr) {
            return;
        }

        stats_.deallocations++;

#if CONTAINER_USE_MEMORY_POOL
        if (size <= pool_size_class::small_threshold) {
            small_pool_.deallocate(ptr);
            return;
        } else if (size <= pool_size_class::medium_threshold) {
            medium_pool_.deallocate(ptr);
            return;
        }
#endif
        // Large allocation: use heap
        ::operator delete(ptr);
    }

    /**
     * @brief Get allocation statistics
     * @return Current statistics
     */
    pool_allocator_stats get_stats() const noexcept {
        return stats_;
    }

    /**
     * @brief Get statistics from the small pool
     * @return Small pool statistics
     */
    fixed_block_pool::statistics get_small_pool_stats() const {
        return small_pool_.get_statistics();
    }

    /**
     * @brief Get statistics from the medium pool
     * @return Medium pool statistics
     */
    fixed_block_pool::statistics get_medium_pool_stats() const {
        return medium_pool_.get_statistics();
    }

    /**
     * @brief Reset statistics (for benchmarking)
     */
    void reset_stats() noexcept {
        stats_ = pool_allocator_stats{};
    }

private:
    pool_allocator()
        : small_pool_(pool_size_class::small_threshold, pool_size_class::blocks_per_chunk)
        , medium_pool_(pool_size_class::medium_threshold, pool_size_class::blocks_per_chunk)
    {}

    // Non-copyable, non-movable
    pool_allocator(const pool_allocator&) = delete;
    pool_allocator& operator=(const pool_allocator&) = delete;
    pool_allocator(pool_allocator&&) = delete;
    pool_allocator& operator=(pool_allocator&&) = delete;

    fixed_block_pool small_pool_;
    fixed_block_pool medium_pool_;
    pool_allocator_stats stats_;
};

/**
 * @brief Allocate and construct an object using pool allocation
 * @tparam T Type to construct
 * @tparam Args Constructor argument types
 * @param args Constructor arguments
 * @return Pointer to constructed object, or nullptr on failure
 */
template<typename T, typename... Args>
T* pool_allocate(Args&&... args) noexcept {
    static_assert(std::is_nothrow_destructible_v<T>,
                  "pool_allocate requires nothrow destructible types");

    void* ptr = pool_allocator::instance().allocate(sizeof(T));
    if (!ptr) {
        return nullptr;
    }

    try {
        return new (ptr) T(std::forward<Args>(args)...);
    } catch (...) {
        pool_allocator::instance().deallocate(ptr, sizeof(T));
        return nullptr;
    }
}

/**
 * @brief Destroy and deallocate an object allocated with pool_allocate
 * @tparam T Type of object
 * @param ptr Pointer to object
 */
template<typename T>
void pool_deallocate(T* ptr) noexcept {
    if (!ptr) {
        return;
    }

    ptr->~T();
    pool_allocator::instance().deallocate(ptr, sizeof(T));
}

/**
 * @brief Check if a type is suitable for pool allocation
 * @tparam T Type to check
 * @return true if type fits in pool (size <= medium_threshold)
 */
template<typename T>
constexpr bool is_pool_allocatable() noexcept {
    return sizeof(T) <= pool_size_class::medium_threshold;
}

/**
 * @brief Get the size class for a given size
 * @param size Size in bytes
 * @return 0 for small, 1 for medium, 2 for large
 */
inline constexpr int get_size_class(std::size_t size) noexcept {
    if (size <= pool_size_class::small_threshold) {
        return 0;
    } else if (size <= pool_size_class::medium_threshold) {
        return 1;
    }
    return 2;
}

} // namespace container_module::internal
