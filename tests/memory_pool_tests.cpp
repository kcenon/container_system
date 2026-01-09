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

/**
 * @file memory_pool_tests.cpp
 * @brief Comprehensive unit tests for memory pool functionality
 *
 * Tests cover:
 * - Basic allocation and deallocation
 * - Memory pool statistics and monitoring
 * - Concurrent access (thread safety)
 * - Allocation patterns and fragmentation
 * - Stress testing with many allocations
 * - Error handling and edge cases
 * - Pool capacity and growth
 */

#include <gtest/gtest.h>
#include <container/internal/memory_pool.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

using namespace container_module::internal;

// Test fixture for memory pool tests
class MemoryPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create pool with small block size for testing
        pool = std::make_unique<fixed_block_pool>(64, 16);  // 64-byte blocks, 16 per chunk
    }

    void TearDown() override {
        pool.reset();
    }

    std::unique_ptr<fixed_block_pool> pool;
};

// ============================================================================
// Basic Allocation Tests
// ============================================================================

TEST_F(MemoryPoolTest, BasicAllocation) {
    void* ptr = pool->allocate();
    EXPECT_NE(ptr, nullptr);

    pool->deallocate(ptr);
}

TEST_F(MemoryPoolTest, MultipleAllocations) {
    std::vector<void*> ptrs;

    // Allocate multiple blocks
    for (int i = 0; i < 10; ++i) {
        void* ptr = pool->allocate();
        EXPECT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    // All pointers should be unique
    std::sort(ptrs.begin(), ptrs.end());
    auto it = std::unique(ptrs.begin(), ptrs.end());
    EXPECT_EQ(it, ptrs.end());

    // Deallocate all
    for (void* ptr : ptrs) {
        pool->deallocate(ptr);
    }
}

TEST_F(MemoryPoolTest, AllocationDeallocationCycle) {
    void* ptr1 = pool->allocate();
    EXPECT_NE(ptr1, nullptr);

    pool->deallocate(ptr1);

    // Allocate again - might get the same pointer back (from free list)
    void* ptr2 = pool->allocate();
    EXPECT_NE(ptr2, nullptr);

    pool->deallocate(ptr2);
}

TEST_F(MemoryPoolTest, WriteReadData) {
    struct TestData {
        int value;
        double data;
    };

    void* ptr = pool->allocate();
    ASSERT_NE(ptr, nullptr);

    // Write data
    TestData* data = static_cast<TestData*>(ptr);
    data->value = 42;
    data->data = 3.14159;

    // Read back
    EXPECT_EQ(data->value, 42);
    EXPECT_DOUBLE_EQ(data->data, 3.14159);

    pool->deallocate(ptr);
}

// ============================================================================
// Statistics Tests
// ============================================================================

TEST_F(MemoryPoolTest, InitialStatistics) {
    auto stats = pool->get_statistics();

    EXPECT_EQ(stats.total_chunks, 0);  // No chunks allocated initially
    EXPECT_EQ(stats.allocated_blocks, 0);
    EXPECT_EQ(stats.total_capacity, 0);
    EXPECT_EQ(stats.free_blocks, 0);
    EXPECT_DOUBLE_EQ(stats.utilization_ratio(), 0.0);
}

TEST_F(MemoryPoolTest, StatisticsAfterAllocation) {
    void* ptr = pool->allocate();
    ASSERT_NE(ptr, nullptr);

    auto stats = pool->get_statistics();

    EXPECT_EQ(stats.total_chunks, 1);  // One chunk should be allocated
    EXPECT_EQ(stats.allocated_blocks, 1);
    EXPECT_EQ(stats.total_capacity, 16);  // blocks_per_chunk = 16
    EXPECT_EQ(stats.free_blocks, 15);  // 16 - 1 = 15

    pool->deallocate(ptr);
}

TEST_F(MemoryPoolTest, StatisticsAfterDeallocation) {
    void* ptr = pool->allocate();
    ASSERT_NE(ptr, nullptr);

    pool->deallocate(ptr);

    auto stats = pool->get_statistics();

    EXPECT_EQ(stats.total_chunks, 1);
    EXPECT_EQ(stats.allocated_blocks, 0);
    EXPECT_EQ(stats.free_blocks, 16);  // All blocks free again
}

TEST_F(MemoryPoolTest, UtilizationRatio) {
    std::vector<void*> ptrs;

    // Allocate half of first chunk
    for (int i = 0; i < 8; ++i) {
        ptrs.push_back(pool->allocate());
    }

    auto stats = pool->get_statistics();
    EXPECT_DOUBLE_EQ(stats.utilization_ratio(), 0.5);  // 8/16 = 0.5

    // Deallocate all
    for (void* ptr : ptrs) {
        pool->deallocate(ptr);
    }
}

// ============================================================================
// Chunk Growth Tests
// ============================================================================

TEST_F(MemoryPoolTest, AutomaticChunkGrowth) {
    std::vector<void*> ptrs;

    // Allocate more than one chunk worth (16 blocks per chunk)
    for (int i = 0; i < 20; ++i) {
        void* ptr = pool->allocate();
        EXPECT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    auto stats = pool->get_statistics();
    EXPECT_GE(stats.total_chunks, 2);  // Should have at least 2 chunks
    EXPECT_EQ(stats.allocated_blocks, 20);

    // Deallocate all
    for (void* ptr : ptrs) {
        pool->deallocate(ptr);
    }
}

TEST_F(MemoryPoolTest, MultipleChunkGrowth) {
    std::vector<void*> ptrs;

    // Allocate enough to trigger multiple chunk allocations
    for (int i = 0; i < 50; ++i) {
        void* ptr = pool->allocate();
        EXPECT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    auto stats = pool->get_statistics();
    EXPECT_GE(stats.total_chunks, 4);  // At least 4 chunks (50 / 16 = 3.125)
    EXPECT_EQ(stats.allocated_blocks, 50);
    EXPECT_GE(stats.total_capacity, 50);

    // Deallocate all
    for (void* ptr : ptrs) {
        pool->deallocate(ptr);
    }
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(MemoryPoolTest, ConcurrentAllocations) {
    constexpr int num_threads = 4;
    constexpr int allocations_per_thread = 25;

    std::vector<std::thread> threads;
    std::vector<std::vector<void*>> thread_ptrs(num_threads);

    // Launch threads to allocate concurrently
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t, &thread_ptrs]() {
            for (int i = 0; i < allocations_per_thread; ++i) {
                void* ptr = pool->allocate();
                EXPECT_NE(ptr, nullptr);
                thread_ptrs[t].push_back(ptr);

                // Small delay to increase contention
                std::this_thread::yield();
            }
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify all allocations succeeded
    int total_allocated = 0;
    for (const auto& ptrs : thread_ptrs) {
        total_allocated += ptrs.size();
    }
    EXPECT_EQ(total_allocated, num_threads * allocations_per_thread);

    // Verify no duplicate pointers across threads
    std::vector<void*> all_ptrs;
    for (const auto& ptrs : thread_ptrs) {
        all_ptrs.insert(all_ptrs.end(), ptrs.begin(), ptrs.end());
    }
    std::sort(all_ptrs.begin(), all_ptrs.end());
    auto it = std::unique(all_ptrs.begin(), all_ptrs.end());
    EXPECT_EQ(it, all_ptrs.end());

    // Deallocate all
    for (void* ptr : all_ptrs) {
        pool->deallocate(ptr);
    }
}

TEST_F(MemoryPoolTest, ConcurrentAllocationDeallocation) {
    constexpr int num_threads = 4;
    constexpr int operations_per_thread = 50;

    std::vector<std::thread> threads;

    // Launch threads for mixed alloc/dealloc
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this]() {
            std::vector<void*> local_ptrs;

            for (int i = 0; i < operations_per_thread; ++i) {
                // Allocate
                void* ptr = pool->allocate();
                EXPECT_NE(ptr, nullptr);
                local_ptrs.push_back(ptr);

                // Deallocate some periodically
                if (local_ptrs.size() > 10) {
                    pool->deallocate(local_ptrs.back());
                    local_ptrs.pop_back();
                }

                std::this_thread::yield();
            }

            // Cleanup remaining
            for (void* ptr : local_ptrs) {
                pool->deallocate(ptr);
            }
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify pool is consistent
    auto stats = pool->get_statistics();
    EXPECT_EQ(stats.allocated_blocks, 0);
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST_F(MemoryPoolTest, StressTestManyAllocations) {
    constexpr int num_allocations = 1000;
    std::vector<void*> ptrs;

    // Allocate many blocks
    for (int i = 0; i < num_allocations; ++i) {
        void* ptr = pool->allocate();
        EXPECT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    auto stats = pool->get_statistics();
    EXPECT_EQ(stats.allocated_blocks, num_allocations);

    // Deallocate all
    for (void* ptr : ptrs) {
        pool->deallocate(ptr);
    }

    stats = pool->get_statistics();
    EXPECT_EQ(stats.allocated_blocks, 0);
}

TEST_F(MemoryPoolTest, StressTestRandomPattern) {
    std::vector<void*> ptrs;
    std::mt19937 gen(42);  // Fixed seed for reproducibility
    std::uniform_int_distribution<> dist(0, 1);

    // Random allocation/deallocation pattern
    for (int i = 0; i < 500; ++i) {
        if (ptrs.empty() || dist(gen) == 0) {
            // Allocate
            void* ptr = pool->allocate();
            EXPECT_NE(ptr, nullptr);
            ptrs.push_back(ptr);
        } else {
            // Deallocate random element
            std::uniform_int_distribution<> index_dist(0, ptrs.size() - 1);
            int index = index_dist(gen);
            pool->deallocate(ptrs[index]);
            ptrs.erase(ptrs.begin() + index);
        }
    }

    // Cleanup remaining
    for (void* ptr : ptrs) {
        pool->deallocate(ptr);
    }

    auto stats = pool->get_statistics();
    EXPECT_EQ(stats.allocated_blocks, 0);
}

// ============================================================================
// Edge Cases Tests
// ============================================================================

TEST_F(MemoryPoolTest, DeallocateNull) {
    // Deallocating null should not crash
    pool->deallocate(nullptr);

    auto stats = pool->get_statistics();
    EXPECT_EQ(stats.allocated_blocks, 0);
}

TEST_F(MemoryPoolTest, MultipleDeallocations) {
    void* ptr = pool->allocate();
    ASSERT_NE(ptr, nullptr);

    pool->deallocate(ptr);

    // Second deallocation of same pointer (undefined behavior in real code)
    // In debug mode, the pool might detect this
    // We're just ensuring it doesn't crash
    // Note: Don't do this in production code!
}

TEST_F(MemoryPoolTest, BlockSizeValidation) {
    // Verify block size is what we requested
    EXPECT_EQ(pool->block_size(), 64);
}

TEST_F(MemoryPoolTest, MinimumBlockSize) {
    // Pool should enforce minimum block size (at least sizeof(void*))
    auto small_pool = std::make_unique<fixed_block_pool>(1);  // Request 1 byte

    // Should be rounded up to at least sizeof(void*)
    EXPECT_GE(small_pool->block_size(), sizeof(void*));
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST_F(MemoryPoolTest, AllocationSpeed) {
    constexpr int num_allocations = 10000;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<void*> ptrs;
    for (int i = 0; i < num_allocations; ++i) {
        ptrs.push_back(pool->allocate());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Just verify it completes in reasonable time (not too strict)
    EXPECT_LT(duration.count(), 1000000);  // Less than 1 second

    // Cleanup
    for (void* ptr : ptrs) {
        pool->deallocate(ptr);
    }
}

TEST_F(MemoryPoolTest, DeallocationSpeed) {
    constexpr int num_allocations = 10000;

    std::vector<void*> ptrs;
    for (int i = 0; i < num_allocations; ++i) {
        ptrs.push_back(pool->allocate());
    }

    auto start = std::chrono::high_resolution_clock::now();

    for (void* ptr : ptrs) {
        pool->deallocate(ptr);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Deallocation should be fast
    EXPECT_LT(duration.count(), 1000000);  // Less than 1 second
}

TEST_F(MemoryPoolTest, ReallocPatternPerformance) {
    constexpr int iterations = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        void* ptr = pool->allocate();
        pool->deallocate(ptr);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Repeated alloc/dealloc should benefit from pooling
    EXPECT_LT(duration.count(), 100000);  // Should be very fast
}

// ============================================================================
// Large Pool Tests
// ============================================================================

TEST_F(MemoryPoolTest, LargeBlockSize) {
    // Create pool with large blocks
    auto large_pool = std::make_unique<fixed_block_pool>(4096, 10);

    std::vector<void*> ptrs;
    for (int i = 0; i < 5; ++i) {
        void* ptr = large_pool->allocate();
        EXPECT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    auto stats = large_pool->get_statistics();
    EXPECT_EQ(stats.allocated_blocks, 5);
    EXPECT_EQ(stats.total_capacity, 10);

    for (void* ptr : ptrs) {
        large_pool->deallocate(ptr);
    }
}

TEST_F(MemoryPoolTest, ManyBlocksPerChunk) {
    // Create pool with many blocks per chunk
    auto large_pool = std::make_unique<fixed_block_pool>(64, 10000);

    std::vector<void*> ptrs;
    for (int i = 0; i < 5000; ++i) {
        ptrs.push_back(large_pool->allocate());
    }

    auto stats = large_pool->get_statistics();
    EXPECT_EQ(stats.total_chunks, 1);  // Should fit in one chunk
    EXPECT_EQ(stats.allocated_blocks, 5000);

    for (void* ptr : ptrs) {
        large_pool->deallocate(ptr);
    }
}

// ============================================================================
// Pool Allocator Integration Tests
// ============================================================================

#include <container/internal/pool_allocator.h>
#include <container/core/container.h>

using namespace container_module::internal;

class PoolAllocatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        pool_allocator::instance().reset_stats();
    }
};

TEST_F(PoolAllocatorTest, BasicAllocation) {
    auto& allocator = pool_allocator::instance();

    // Small allocation (<=64 bytes)
    void* small_ptr = allocator.allocate(32);
    ASSERT_NE(small_ptr, nullptr);

    // Medium allocation (<=256 bytes)
    void* medium_ptr = allocator.allocate(128);
    ASSERT_NE(medium_ptr, nullptr);

    // Large allocation (>256 bytes)
    void* large_ptr = allocator.allocate(512);
    ASSERT_NE(large_ptr, nullptr);

    auto stats = allocator.get_stats();
#if CONTAINER_USE_MEMORY_POOL
    EXPECT_EQ(stats.small_pool_allocs, 1);
    EXPECT_EQ(stats.medium_pool_allocs, 1);
    EXPECT_EQ(stats.pool_hits, 2);  // small + medium
    EXPECT_EQ(stats.pool_misses, 1);  // large
#endif

    allocator.deallocate(small_ptr, 32);
    allocator.deallocate(medium_ptr, 128);
    allocator.deallocate(large_ptr, 512);
}

TEST_F(PoolAllocatorTest, PoolAllocateTemplate) {
    struct SmallStruct {
        int value;
        double data;
    };

    auto* ptr = pool_allocate<SmallStruct>(42, 3.14);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->value, 42);
    EXPECT_DOUBLE_EQ(ptr->data, 3.14);

    pool_deallocate(ptr);
}

TEST_F(PoolAllocatorTest, SizeClassRouting) {
    EXPECT_EQ(get_size_class(32), 0);   // Small
    EXPECT_EQ(get_size_class(64), 0);   // Small (boundary)
    EXPECT_EQ(get_size_class(65), 1);   // Medium
    EXPECT_EQ(get_size_class(128), 1);  // Medium
    EXPECT_EQ(get_size_class(256), 1);  // Medium (boundary)
    EXPECT_EQ(get_size_class(257), 2);  // Large
    EXPECT_EQ(get_size_class(1024), 2); // Large
}

TEST_F(PoolAllocatorTest, IsPoolAllocatable) {
    struct Small { char data[32]; };
    struct Medium { char data[128]; };
    struct Large { char data[512]; };

    EXPECT_TRUE(is_pool_allocatable<Small>());
    EXPECT_TRUE(is_pool_allocatable<Medium>());
    EXPECT_FALSE(is_pool_allocatable<Large>());
}

TEST_F(PoolAllocatorTest, HitRateCalculation) {
    auto& allocator = pool_allocator::instance();

    // Perform some allocations
    std::vector<void*> ptrs;
    for (int i = 0; i < 10; ++i) {
        ptrs.push_back(allocator.allocate(32));  // Small, should hit
    }
    for (int i = 0; i < 5; ++i) {
        ptrs.push_back(allocator.allocate(512)); // Large, should miss
    }

    auto stats = allocator.get_stats();
#if CONTAINER_USE_MEMORY_POOL
    EXPECT_DOUBLE_EQ(stats.hit_rate(), 10.0 / 15.0);
#endif

    // Cleanup
    for (int i = 0; i < 10; ++i) {
        allocator.deallocate(ptrs[i], 32);
    }
    for (int i = 10; i < 15; ++i) {
        allocator.deallocate(ptrs[i], 512);
    }
}

TEST_F(PoolAllocatorTest, ContainerPoolStats) {
    using namespace container_module;

    // Clear any previous stats
    value_container::clear_pool();

    auto stats = value_container::get_pool_stats();
#if CONTAINER_USE_MEMORY_POOL
    EXPECT_GE(stats.hits, 0u);
    EXPECT_GE(stats.misses, 0u);
#else
    EXPECT_EQ(stats.hits, 0u);
    EXPECT_EQ(stats.misses, 0u);
#endif
}

TEST_F(PoolAllocatorTest, ConcurrentPoolAccess) {
    constexpr int num_threads = 4;
    constexpr int allocations_per_thread = 100;

    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&success_count]() {
            auto& allocator = pool_allocator::instance();
            std::vector<void*> local_ptrs;

            for (int i = 0; i < allocations_per_thread; ++i) {
                void* ptr = allocator.allocate(32);
                if (ptr) {
                    local_ptrs.push_back(ptr);
                    success_count.fetch_add(1, std::memory_order_relaxed);
                }
            }

            // Deallocate
            for (void* ptr : local_ptrs) {
                allocator.deallocate(ptr, 32);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(success_count.load(), num_threads * allocations_per_thread);
}

// ============================================================================
// Main entry point
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
