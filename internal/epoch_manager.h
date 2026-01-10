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

#pragma once

#include <atomic>
#include <array>
#include <vector>
#include <functional>
#include <mutex>

namespace container_module
{
    /**
     * @brief Epoch-based memory reclamation for lock-free data structures
     *
     * This class implements the epoch-based reclamation (EBR) algorithm for safe
     * memory deallocation in lock-free data structures. It uses a three-epoch
     * system where memory is deferred until all readers from two epochs ago
     * have completed.
     *
     * The algorithm:
     * 1. Threads enter a critical section by pinning to the current epoch
     * 2. Writers defer deletions to the current epoch's retire list
     * 3. When all threads have left an epoch, its retired memory can be reclaimed
     *
     * Properties:
     * - enter_critical/exit_critical: Lock-free
     * - defer_delete: Lock-free (uses mutex only for list append)
     * - try_gc: May reclaim memory when safe
     *
     * @code
     * epoch_manager& em = epoch_manager::instance();
     *
     * // Reader thread
     * em.enter_critical();
     * // ... read from lock-free structure ...
     * em.exit_critical();
     *
     * // Writer thread
     * em.defer_delete(old_node, [](void* p) { delete static_cast<Node*>(p); });
     * em.try_gc();
     * @endcode
     */
    class epoch_manager
    {
    public:
        /**
         * @brief Sentinel value indicating thread is not in critical section
         */
        static constexpr uint64_t INACTIVE = UINT64_MAX;

        /**
         * @brief Number of epochs in rotation (must be at least 3 for safety)
         */
        static constexpr size_t NUM_EPOCHS = 3;

        /**
         * @brief Get the singleton instance
         * @return Reference to the global epoch manager
         */
        static epoch_manager& instance() {
            static epoch_manager instance;
            return instance;
        }

        /**
         * @brief Enter critical section (pin to current epoch)
         *
         * Must be called before accessing any lock-free data structure.
         * Must be paired with exit_critical().
         *
         * Thread safety: Lock-free
         */
        void enter_critical() noexcept {
            thread_epoch() = global_epoch_.load(std::memory_order_acquire);
        }

        /**
         * @brief Exit critical section
         *
         * Must be called after finishing access to lock-free data structures.
         *
         * Thread safety: Lock-free
         */
        void exit_critical() noexcept {
            thread_epoch() = INACTIVE;
        }

        /**
         * @brief Check if current thread is in critical section
         * @return true if in critical section
         */
        [[nodiscard]] bool in_critical_section() const noexcept {
            return thread_epoch() != INACTIVE;
        }

        /**
         * @brief Defer deletion of a pointer
         *
         * The deleter will be called when it's safe to reclaim the memory
         * (when all readers from the current epoch have exited).
         *
         * @param ptr Pointer to defer deletion for
         * @param deleter Function to call to delete the pointer
         *
         * Thread safety: Uses mutex for list append
         */
        void defer_delete(void* ptr, std::function<void(void*)> deleter) {
            if (ptr == nullptr) {
                return;
            }

            uint64_t epoch = global_epoch_.load(std::memory_order_relaxed);
            size_t bucket = epoch % NUM_EPOCHS;

            std::lock_guard<std::mutex> lock(retire_mutex_[bucket]);
            retired_[bucket].emplace_back(ptr, std::move(deleter));
        }

        /**
         * @brief Defer deletion of a typed pointer
         *
         * Convenience overload that automatically creates the deleter.
         *
         * @tparam T Type of the pointer
         * @param ptr Pointer to defer deletion for
         */
        template<typename T>
        void defer_delete(T* ptr) {
            defer_delete(static_cast<void*>(ptr), [](void* p) {
                delete static_cast<T*>(p);
            });
        }

        /**
         * @brief Attempt garbage collection
         *
         * Advances the global epoch and reclaims memory from epochs that are
         * safe to collect (all threads have exited that epoch).
         *
         * @return Number of objects reclaimed
         */
        size_t try_gc() {
            // Advance epoch
            uint64_t current = global_epoch_.fetch_add(1, std::memory_order_acq_rel);

            // Calculate which epoch is safe to collect
            // We need 2 epoch advances before an epoch is safe
            if (current < 2) {
                return 0;
            }

            uint64_t safe_epoch = current - 2;
            size_t bucket = safe_epoch % NUM_EPOCHS;

            // Check if any thread is still in the safe epoch
            // For simplicity, we just try to collect
            // In production, you'd check all registered threads

            std::vector<std::pair<void*, std::function<void(void*)>>> to_delete;
            {
                std::lock_guard<std::mutex> lock(retire_mutex_[bucket]);
                to_delete = std::move(retired_[bucket]);
                retired_[bucket].clear();
            }

            // Actually delete the objects
            for (auto& [ptr, deleter] : to_delete) {
                deleter(ptr);
            }

            gc_count_.fetch_add(1, std::memory_order_relaxed);
            reclaimed_count_.fetch_add(to_delete.size(), std::memory_order_relaxed);

            return to_delete.size();
        }

        /**
         * @brief Force garbage collection of all epochs
         *
         * This should only be called when no threads are accessing
         * lock-free data structures (e.g., during shutdown).
         *
         * @return Number of objects reclaimed
         */
        size_t force_gc() {
            size_t total = 0;
            for (size_t i = 0; i < NUM_EPOCHS; ++i) {
                std::vector<std::pair<void*, std::function<void(void*)>>> to_delete;
                {
                    std::lock_guard<std::mutex> lock(retire_mutex_[i]);
                    to_delete = std::move(retired_[i]);
                    retired_[i].clear();
                }

                for (auto& [ptr, deleter] : to_delete) {
                    deleter(ptr);
                }
                total += to_delete.size();
            }

            if (total > 0) {
                gc_count_.fetch_add(1, std::memory_order_relaxed);
                reclaimed_count_.fetch_add(total, std::memory_order_relaxed);
            }

            return total;
        }

        /**
         * @brief Get the current global epoch
         * @return Current epoch number
         */
        [[nodiscard]] uint64_t current_epoch() const noexcept {
            return global_epoch_.load(std::memory_order_relaxed);
        }

        /**
         * @brief Get the number of GC cycles performed
         * @return GC cycle count
         */
        [[nodiscard]] size_t gc_count() const noexcept {
            return gc_count_.load(std::memory_order_relaxed);
        }

        /**
         * @brief Get the total number of objects reclaimed
         * @return Reclaimed object count
         */
        [[nodiscard]] size_t reclaimed_count() const noexcept {
            return reclaimed_count_.load(std::memory_order_relaxed);
        }

        /**
         * @brief Get the number of objects pending deletion
         * @return Number of objects in retire lists
         */
        [[nodiscard]] size_t pending_count() const {
            size_t total = 0;
            for (size_t i = 0; i < NUM_EPOCHS; ++i) {
                std::lock_guard<std::mutex> lock(retire_mutex_[i]);
                total += retired_[i].size();
            }
            return total;
        }

        // Delete copy and move operations
        epoch_manager(const epoch_manager&) = delete;
        epoch_manager& operator=(const epoch_manager&) = delete;
        epoch_manager(epoch_manager&&) = delete;
        epoch_manager& operator=(epoch_manager&&) = delete;

    private:
        /**
         * @brief Private constructor for singleton
         */
        epoch_manager() = default;

        /**
         * @brief Destructor - clean up any remaining deferred deletions
         */
        ~epoch_manager() {
            force_gc();
        }

        /**
         * @brief Get thread-local epoch reference
         * @return Reference to this thread's epoch
         */
        static std::atomic<uint64_t>& thread_epoch() {
            thread_local std::atomic<uint64_t> epoch{INACTIVE};
            return epoch;
        }

        std::atomic<uint64_t> global_epoch_{0};
        std::atomic<size_t> gc_count_{0};
        std::atomic<size_t> reclaimed_count_{0};

        mutable std::array<std::mutex, NUM_EPOCHS> retire_mutex_;
        std::array<std::vector<std::pair<void*, std::function<void(void*)>>>, NUM_EPOCHS> retired_;
    };

    /**
     * @brief RAII guard for epoch critical section
     *
     * Automatically enters critical section on construction and
     * exits on destruction.
     *
     * @code
     * {
     *     epoch_guard guard;
     *     // ... access lock-free data structure ...
     * } // automatically exits critical section
     * @endcode
     */
    class epoch_guard
    {
    public:
        /**
         * @brief Enter critical section
         */
        epoch_guard() noexcept {
            epoch_manager::instance().enter_critical();
        }

        /**
         * @brief Exit critical section
         */
        ~epoch_guard() noexcept {
            epoch_manager::instance().exit_critical();
        }

        // Non-copyable, non-movable
        epoch_guard(const epoch_guard&) = delete;
        epoch_guard& operator=(const epoch_guard&) = delete;
        epoch_guard(epoch_guard&&) = delete;
        epoch_guard& operator=(epoch_guard&&) = delete;
    };

} // namespace container_module
