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
 * @file memory_pool_benchmark.cpp
 * @brief Comprehensive performance benchmarks for memory pool
 *
 * Benchmarks cover:
 * - Allocation/deallocation throughput across various block sizes
 * - Concurrent allocation performance with varying thread counts
 * - Comparison with standard allocator (new/delete)
 * - Memory fragmentation impact
 * - Cache efficiency measurements
 * - Real-world allocation patterns
 */

#include <benchmark/benchmark.h>
#include <container/internal/memory_pool.h>
#include <vector>
#include <thread>
#include <random>
#include <algorithm>
#include <cstring>

using namespace container_module::internal;

// ============================================================================
// Basic Allocation Benchmarks
// ============================================================================

static void BM_PoolAllocation_64B(benchmark::State& state) {
    fixed_block_pool pool(64, 1024);

    for (auto _ : state) {
        void* ptr = pool.allocate();
        benchmark::DoNotOptimize(ptr);
        pool.deallocate(ptr);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 64);
}
BENCHMARK(BM_PoolAllocation_64B);

static void BM_PoolAllocation_256B(benchmark::State& state) {
    fixed_block_pool pool(256, 1024);

    for (auto _ : state) {
        void* ptr = pool.allocate();
        benchmark::DoNotOptimize(ptr);
        pool.deallocate(ptr);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 256);
}
BENCHMARK(BM_PoolAllocation_256B);

static void BM_PoolAllocation_1KB(benchmark::State& state) {
    fixed_block_pool pool(1024, 1024);

    for (auto _ : state) {
        void* ptr = pool.allocate();
        benchmark::DoNotOptimize(ptr);
        pool.deallocate(ptr);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 1024);
}
BENCHMARK(BM_PoolAllocation_1KB);

static void BM_PoolAllocation_4KB(benchmark::State& state) {
    fixed_block_pool pool(4096, 256);

    for (auto _ : state) {
        void* ptr = pool.allocate();
        benchmark::DoNotOptimize(ptr);
        pool.deallocate(ptr);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 4096);
}
BENCHMARK(BM_PoolAllocation_4KB);

// ============================================================================
// Comparison with Standard Allocator
// ============================================================================

static void BM_StandardAllocation_64B(benchmark::State& state) {
    for (auto _ : state) {
        void* ptr = ::operator new(64);
        benchmark::DoNotOptimize(ptr);
        ::operator delete(ptr);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 64);
}
BENCHMARK(BM_StandardAllocation_64B);

static void BM_StandardAllocation_256B(benchmark::State& state) {
    for (auto _ : state) {
        void* ptr = ::operator new(256);
        benchmark::DoNotOptimize(ptr);
        ::operator delete(ptr);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 256);
}
BENCHMARK(BM_StandardAllocation_256B);

static void BM_StandardAllocation_1KB(benchmark::State& state) {
    for (auto _ : state) {
        void* ptr = ::operator new(1024);
        benchmark::DoNotOptimize(ptr);
        ::operator delete(ptr);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 1024);
}
BENCHMARK(BM_StandardAllocation_1KB);

static void BM_StandardAllocation_4KB(benchmark::State& state) {
    for (auto _ : state) {
        void* ptr = ::operator new(4096);
        benchmark::DoNotOptimize(ptr);
        ::operator delete(ptr);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * 4096);
}
BENCHMARK(BM_StandardAllocation_4KB);

// ============================================================================
// Batch Allocation Benchmarks
// ============================================================================

static void BM_PoolBatchAllocation(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    fixed_block_pool pool(64, 1024);

    for (auto _ : state) {
        std::vector<void*> ptrs;
        ptrs.reserve(batch_size);

        // Allocate batch
        for (size_t i = 0; i < batch_size; ++i) {
            ptrs.push_back(pool.allocate());
        }

        benchmark::DoNotOptimize(ptrs.data());

        // Deallocate batch
        for (void* ptr : ptrs) {
            pool.deallocate(ptr);
        }
    }

    state.SetItemsProcessed(state.iterations() * batch_size);
    state.SetBytesProcessed(state.iterations() * batch_size * 64);
}
BENCHMARK(BM_PoolBatchAllocation)->Range(8, 1024);

static void BM_StandardBatchAllocation(benchmark::State& state) {
    const size_t batch_size = state.range(0);

    for (auto _ : state) {
        std::vector<void*> ptrs;
        ptrs.reserve(batch_size);

        // Allocate batch
        for (size_t i = 0; i < batch_size; ++i) {
            ptrs.push_back(::operator new(64));
        }

        benchmark::DoNotOptimize(ptrs.data());

        // Deallocate batch
        for (void* ptr : ptrs) {
            ::operator delete(ptr);
        }
    }

    state.SetItemsProcessed(state.iterations() * batch_size);
    state.SetBytesProcessed(state.iterations() * batch_size * 64);
}
BENCHMARK(BM_StandardBatchAllocation)->Range(8, 1024);

// ============================================================================
// Allocation Pattern Benchmarks
// ============================================================================

static void BM_PoolSequentialPattern(benchmark::State& state) {
    fixed_block_pool pool(64, 1024);
    std::vector<void*> ptrs(100);

    for (auto _ : state) {
        // Sequential allocation
        for (size_t i = 0; i < 100; ++i) {
            ptrs[i] = pool.allocate();
        }

        benchmark::DoNotOptimize(ptrs.data());

        // Sequential deallocation
        for (size_t i = 0; i < 100; ++i) {
            pool.deallocate(ptrs[i]);
        }
    }

    state.SetItemsProcessed(state.iterations() * 100);
}
BENCHMARK(BM_PoolSequentialPattern);

static void BM_PoolReversePattern(benchmark::State& state) {
    fixed_block_pool pool(64, 1024);
    std::vector<void*> ptrs(100);

    for (auto _ : state) {
        // Sequential allocation
        for (size_t i = 0; i < 100; ++i) {
            ptrs[i] = pool.allocate();
        }

        benchmark::DoNotOptimize(ptrs.data());

        // Reverse deallocation (LIFO)
        for (size_t i = 100; i > 0; --i) {
            pool.deallocate(ptrs[i - 1]);
        }
    }

    state.SetItemsProcessed(state.iterations() * 100);
}
BENCHMARK(BM_PoolReversePattern);

static void BM_PoolRandomPattern(benchmark::State& state) {
    fixed_block_pool pool(64, 1024);
    std::vector<void*> ptrs(100);
    std::vector<size_t> indices(100);

    for (auto _ : state) {
        state.PauseTiming();
        std::iota(indices.begin(), indices.end(), 0);
        std::mt19937 gen(42);
        std::shuffle(indices.begin(), indices.end(), gen);
        state.ResumeTiming();

        // Sequential allocation
        for (size_t i = 0; i < 100; ++i) {
            ptrs[i] = pool.allocate();
        }

        benchmark::DoNotOptimize(ptrs.data());

        // Random deallocation
        for (size_t idx : indices) {
            pool.deallocate(ptrs[idx]);
        }
    }

    state.SetItemsProcessed(state.iterations() * 100);
}
BENCHMARK(BM_PoolRandomPattern);

// ============================================================================
// Concurrent Allocation Benchmarks
// ============================================================================

static void BM_PoolConcurrentAllocation(benchmark::State& state) {
    const int num_threads = state.range(0);
    fixed_block_pool pool(64, 1024);

    for (auto _ : state) {
        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&pool]() {
                for (int i = 0; i < 100; ++i) {
                    void* ptr = pool.allocate();
                    benchmark::DoNotOptimize(ptr);
                    pool.deallocate(ptr);
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }

    state.SetItemsProcessed(state.iterations() * num_threads * 100);
}
BENCHMARK(BM_PoolConcurrentAllocation)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->UseRealTime();

// ============================================================================
// Memory Reuse Efficiency Benchmarks
// ============================================================================

static void BM_PoolReuseEfficiency(benchmark::State& state) {
    fixed_block_pool pool(64, 16);  // Small chunk size to force reuse

    // Warm up pool
    std::vector<void*> warmup(16);
    for (size_t i = 0; i < 16; ++i) {
        warmup[i] = pool.allocate();
    }
    for (void* ptr : warmup) {
        pool.deallocate(ptr);
    }

    for (auto _ : state) {
        void* ptr = pool.allocate();  // Should hit pool
        benchmark::DoNotOptimize(ptr);
        pool.deallocate(ptr);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_PoolReuseEfficiency);

// ============================================================================
// Cache Efficiency Benchmarks
// ============================================================================

static void BM_PoolCacheEfficiency_Sequential(benchmark::State& state) {
    const size_t count = state.range(0);
    fixed_block_pool pool(64, count * 2);
    std::vector<void*> ptrs(count);

    for (auto _ : state) {
        // Allocate sequentially (cache-friendly)
        for (size_t i = 0; i < count; ++i) {
            ptrs[i] = pool.allocate();
            // Write to memory (simulating real usage)
            *static_cast<uint64_t*>(ptrs[i]) = i;
        }

        benchmark::DoNotOptimize(ptrs.data());

        // Access sequentially (cache-friendly)
        uint64_t sum = 0;
        for (size_t i = 0; i < count; ++i) {
            sum += *static_cast<uint64_t*>(ptrs[i]);
        }
        benchmark::DoNotOptimize(sum);

        // Deallocate
        for (size_t i = 0; i < count; ++i) {
            pool.deallocate(ptrs[i]);
        }
    }

    state.SetItemsProcessed(state.iterations() * count);
}
BENCHMARK(BM_PoolCacheEfficiency_Sequential)->Range(8, 512);

static void BM_PoolCacheEfficiency_Random(benchmark::State& state) {
    const size_t count = state.range(0);
    fixed_block_pool pool(64, count * 2);
    std::vector<void*> ptrs(count);
    std::vector<size_t> access_pattern(count);

    for (auto _ : state) {
        state.PauseTiming();
        std::iota(access_pattern.begin(), access_pattern.end(), 0);
        std::mt19937 gen(42);
        std::shuffle(access_pattern.begin(), access_pattern.end(), gen);
        state.ResumeTiming();

        // Allocate sequentially
        for (size_t i = 0; i < count; ++i) {
            ptrs[i] = pool.allocate();
            *static_cast<uint64_t*>(ptrs[i]) = i;
        }

        benchmark::DoNotOptimize(ptrs.data());

        // Access randomly (cache-unfriendly)
        uint64_t sum = 0;
        for (size_t idx : access_pattern) {
            sum += *static_cast<uint64_t*>(ptrs[idx]);
        }
        benchmark::DoNotOptimize(sum);

        // Deallocate
        for (size_t i = 0; i < count; ++i) {
            pool.deallocate(ptrs[i]);
        }
    }

    state.SetItemsProcessed(state.iterations() * count);
}
BENCHMARK(BM_PoolCacheEfficiency_Random)->Range(8, 512);

// ============================================================================
// Fragmentation Benchmarks
// ============================================================================

static void BM_PoolFragmentation(benchmark::State& state) {
    const size_t total_allocs = 1000;
    fixed_block_pool pool(64, 1024);
    std::vector<void*> ptrs(total_allocs);

    for (auto _ : state) {
        state.PauseTiming();

        // Allocate all blocks
        for (size_t i = 0; i < total_allocs; ++i) {
            ptrs[i] = pool.allocate();
        }

        // Create fragmentation by deallocating every other block
        for (size_t i = 0; i < total_allocs; i += 2) {
            pool.deallocate(ptrs[i]);
        }

        state.ResumeTiming();

        // Measure allocation performance with fragmentation
        for (size_t i = 0; i < total_allocs / 2; ++i) {
            void* ptr = pool.allocate();
            benchmark::DoNotOptimize(ptr);
            ptrs[i * 2] = ptr;
        }

        state.PauseTiming();
        // Cleanup
        for (void* ptr : ptrs) {
            pool.deallocate(ptr);
        }
        state.ResumeTiming();
    }

    state.SetItemsProcessed(state.iterations() * (total_allocs / 2));
}
BENCHMARK(BM_PoolFragmentation);

// ============================================================================
// Real-World Usage Patterns
// ============================================================================

static void BM_PoolWebServerPattern(benchmark::State& state) {
    // Simulates web server request handling:
    // - Allocate request buffer
    // - Allocate response buffer
    // - Process (simulate with small delay)
    // - Deallocate in reverse order

    fixed_block_pool request_pool(1024, 256);  // 1KB request buffers
    fixed_block_pool response_pool(4096, 128); // 4KB response buffers

    for (auto _ : state) {
        void* request = request_pool.allocate();
        void* response = response_pool.allocate();

        // Simulate processing (write to buffers)
        std::memset(request, 0, 1024);
        std::memset(response, 0, 4096);

        benchmark::DoNotOptimize(request);
        benchmark::DoNotOptimize(response);

        // Deallocate in reverse order (LIFO)
        response_pool.deallocate(response);
        request_pool.deallocate(request);
    }

    state.SetItemsProcessed(state.iterations() * 2);
}
BENCHMARK(BM_PoolWebServerPattern);

static void BM_PoolParserPattern(benchmark::State& state) {
    // Simulates parser allocating many small tokens
    const size_t tokens_per_parse = 50;
    fixed_block_pool pool(32, 1024);
    std::vector<void*> tokens(tokens_per_parse);

    for (auto _ : state) {
        // Allocate tokens during parsing
        for (size_t i = 0; i < tokens_per_parse; ++i) {
            tokens[i] = pool.allocate();
            benchmark::DoNotOptimize(tokens[i]);
        }

        // Process tokens (simulate)
        for (void* token : tokens) {
            benchmark::ClobberMemory();
        }

        // Free all tokens after parsing
        for (void* token : tokens) {
            pool.deallocate(token);
        }
    }

    state.SetItemsProcessed(state.iterations() * tokens_per_parse);
}
BENCHMARK(BM_PoolParserPattern);

// ============================================================================
// Statistics Collection Overhead
// ============================================================================

static void BM_PoolStatisticsOverhead(benchmark::State& state) {
    fixed_block_pool pool(64, 1024);

    for (auto _ : state) {
        void* ptr = pool.allocate();
        auto stats = pool.get_statistics();  // Measure overhead
        benchmark::DoNotOptimize(stats.total_chunks);
        pool.deallocate(ptr);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_PoolStatisticsOverhead);

// ============================================================================
// Chunk Growth Impact
// ============================================================================

static void BM_PoolChunkGrowth(benchmark::State& state) {
    const size_t blocks_per_chunk = 16;  // Small chunk to force growth

    for (auto _ : state) {
        state.PauseTiming();
        fixed_block_pool pool(64, blocks_per_chunk);
        state.ResumeTiming();

        std::vector<void*> ptrs;

        // Allocate enough to trigger multiple chunk growths
        for (size_t i = 0; i < blocks_per_chunk * 5; ++i) {
            void* ptr = pool.allocate();
            benchmark::DoNotOptimize(ptr);
            ptrs.push_back(ptr);
        }

        state.PauseTiming();
        for (void* ptr : ptrs) {
            pool.deallocate(ptr);
        }
        state.ResumeTiming();
    }

    state.SetItemsProcessed(state.iterations() * 16 * 5);
}
BENCHMARK(BM_PoolChunkGrowth);

// ============================================================================
// Memory Bandwidth Tests
// ============================================================================

static void BM_PoolMemoryBandwidth(benchmark::State& state) {
    const size_t count = state.range(0);
    const size_t block_size = 1024;
    fixed_block_pool pool(block_size, count * 2);

    std::vector<void*> ptrs(count);

    for (auto _ : state) {
        // Allocate
        for (size_t i = 0; i < count; ++i) {
            ptrs[i] = pool.allocate();
        }

        // Write to all blocks (measure write bandwidth)
        for (size_t i = 0; i < count; ++i) {
            std::memset(ptrs[i], i & 0xFF, block_size);
        }

        benchmark::DoNotOptimize(ptrs.data());

        // Read from all blocks (measure read bandwidth)
        uint64_t sum = 0;
        for (size_t i = 0; i < count; ++i) {
            const uint8_t* data = static_cast<const uint8_t*>(ptrs[i]);
            for (size_t j = 0; j < block_size; j += 8) {
                sum += *reinterpret_cast<const uint64_t*>(data + j);
            }
        }
        benchmark::DoNotOptimize(sum);

        // Deallocate
        for (size_t i = 0; i < count; ++i) {
            pool.deallocate(ptrs[i]);
        }
    }

    state.SetBytesProcessed(state.iterations() * count * block_size * 2); // Read + Write
}
BENCHMARK(BM_PoolMemoryBandwidth)->Range(8, 256);

// ============================================================================
// Main
// ============================================================================

BENCHMARK_MAIN();
