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
        if (!free_list_) allocate_chunk_unlocked();
        void* p = free_list_;
        free_list_ = *reinterpret_cast<void**>(free_list_);
        return p;
    }

    void deallocate(void* p) {
        if (!p) return;
        std::lock_guard<std::mutex> lock(mutex_);
        *reinterpret_cast<void**>(p) = free_list_;
        free_list_ = p;
    }

    std::size_t block_size() const noexcept { return block_size_; }

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
    std::mutex mutex_;
};

} // namespace container_module::internal

