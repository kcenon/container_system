#pragma once

/*
 * Simple fixed-block memory pool (prototype)
 * Not wired into container_system by default; intended for experimentation.
 */

#include <cstddef>
#include <cstdint>
#include <vector>
#include <mutex>
#include <memory>
#include <stdexcept>

namespace container_module::internal {

class fixed_block_pool {
public:
    explicit fixed_block_pool(std::size_t block_size,
                              std::size_t blocks_per_chunk = 1024)
        : block_size_(block_size < sizeof(void*) ? sizeof(void*) : block_size)
        , blocks_per_chunk_(blocks_per_chunk) {}

    ~fixed_block_pool() = default;

    void* allocate() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!free_list_) {
            try {
                allocate_chunk_unlocked();
            } catch (const std::bad_alloc&) {
                // Chunk allocation failed, propagate the exception
                throw;
            }
            // Verify that allocate_chunk_unlocked() actually created free blocks
            if (!free_list_) {
                throw std::runtime_error("Memory pool chunk allocation failed to create free list");
            }
        }
        void* p = free_list_;
        free_list_ = *reinterpret_cast<void**>(free_list_);
        ++allocated_count_;
        return p;
    }

    void deallocate(void* p) {
        if (!p) return;
        std::lock_guard<std::mutex> lock(mutex_);

#ifndef NDEBUG
        // Debug-mode validation: ensure pointer appears to be from our pool
        // This is a simple sanity check, not a complete validation
        bool found = false;
        for (const auto& chunk : chunks_) {
            std::uint8_t* chunk_start = chunk.get();
            std::uint8_t* chunk_end = chunk_start + (block_size_ * blocks_per_chunk_);
            if (reinterpret_cast<std::uint8_t*>(p) >= chunk_start &&
                reinterpret_cast<std::uint8_t*>(p) < chunk_end) {
                found = true;
                break;
            }
        }
        if (!found) {
            // Pointer does not belong to any of our chunks
            // In debug mode, we could log a warning or assert
            // For now, we'll just return to avoid corruption
            return;
        }
#endif

        *reinterpret_cast<void**>(p) = free_list_;
        free_list_ = p;
        --allocated_count_;
    }

    std::size_t block_size() const noexcept { return block_size_; }

    /**
     * @brief Statistics for monitoring memory pool usage
     */
    struct statistics {
        std::size_t total_chunks;       // Number of chunks allocated
        std::size_t allocated_blocks;   // Number of blocks currently in use
        std::size_t total_capacity;     // Total number of blocks across all chunks
        std::size_t free_blocks;        // Number of blocks in free list

        // Calculated metrics
        double utilization_ratio() const {
            return total_capacity > 0
                ? static_cast<double>(allocated_blocks) / total_capacity
                : 0.0;
        }

        std::size_t memory_bytes() const {
            // This would need block_size, which we don't have in this struct
            // For now, just return 0; caller can calculate if needed
            return 0;
        }
    };

    /**
     * @brief Get current pool statistics
     * @note This method is thread-safe but may impact performance if called frequently
     */
    statistics get_statistics() const {
        std::lock_guard<std::mutex> lock(mutex_);

        // Count free blocks in the free list
        std::size_t free_count = 0;
        void* current = free_list_;
        while (current) {
            ++free_count;
            current = *reinterpret_cast<void**>(current);
        }

        std::size_t total_capacity = chunks_.size() * blocks_per_chunk_;

        return statistics{
            chunks_.size(),
            allocated_count_,
            total_capacity,
            free_count
        };
    }

private:
    void allocate_chunk_unlocked() {
        std::unique_ptr<std::uint8_t[]> chunk(new std::uint8_t[block_size_ * blocks_per_chunk_]);
        std::uint8_t* base = chunk.get();
        chunks_.push_back(std::move(chunk));
        // Build free list
        for (std::size_t i = 0; i < blocks_per_chunk_; ++i) {
            void* p = base + i * block_size_;
            *reinterpret_cast<void**>(p) = free_list_;
            free_list_ = p;
        }
    }

private:
    std::size_t block_size_;
    std::size_t blocks_per_chunk_;
    std::vector<std::unique_ptr<std::uint8_t[]>> chunks_;
    void* free_list_{nullptr};
    mutable std::mutex mutex_;  // Mutable to allow locking in const methods
    std::size_t allocated_count_{0};  // Track number of currently allocated blocks
};

} // namespace container_module::internal

