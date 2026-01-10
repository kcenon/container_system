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
#include <memory>
#include <type_traits>

namespace container_module
{
    /**
     * @brief Lock-free value wrapper using Read-Copy-Update (RCU) pattern
     *
     * This template provides truly lock-free reads using atomic shared_ptr operations.
     * Writers create new versions, and readers access the current version atomically.
     * Old versions are automatically reclaimed when the last reader releases them.
     *
     * Properties:
     * - Read: Wait-free O(1) - no blocking, no spinning
     * - Update: Lock-free - may require retry under contention
     * - Memory: Automatic reclamation via shared_ptr reference counting
     *
     * @tparam T The value type to store (must be copy-constructible or move-constructible)
     *
     * Example usage:
     * @code
     * rcu_value<int> counter{0};
     *
     * // Lock-free read from any thread
     * auto snapshot = counter.read();
     * int value = *snapshot;
     *
     * // Update from any thread
     * counter.update(42);
     * @endcode
     */
    template<typename T>
    class rcu_value
    {
    public:
        static_assert(std::is_copy_constructible_v<T> || std::is_move_constructible_v<T>,
                     "T must be copy or move constructible");

        /**
         * @brief Default constructor - initializes with default-constructed T
         */
        rcu_value() requires std::is_default_constructible_v<T>
            : current_(std::make_shared<const T>()) {}

        /**
         * @brief Construct with initial value
         * @param initial The initial value
         */
        explicit rcu_value(T initial)
            : current_(std::make_shared<const T>(std::move(initial))) {}

        /**
         * @brief Copy constructor
         * @param other The rcu_value to copy from
         */
        rcu_value(const rcu_value& other)
            : current_(std::atomic_load_explicit(&other.current_, std::memory_order_acquire)) {}

        /**
         * @brief Move constructor
         * @param other The rcu_value to move from
         */
        rcu_value(rcu_value&& other) noexcept
            : current_(std::atomic_load_explicit(&other.current_, std::memory_order_acquire)) {}

        /**
         * @brief Copy assignment operator
         * @param other The rcu_value to copy from
         * @return Reference to this
         */
        rcu_value& operator=(const rcu_value& other) {
            if (this != &other) {
                auto new_ptr = std::atomic_load_explicit(&other.current_, std::memory_order_acquire);
                std::atomic_store_explicit(&current_, new_ptr, std::memory_order_release);
            }
            return *this;
        }

        /**
         * @brief Move assignment operator
         * @param other The rcu_value to move from
         * @return Reference to this
         */
        rcu_value& operator=(rcu_value&& other) noexcept {
            if (this != &other) {
                auto new_ptr = std::atomic_load_explicit(&other.current_, std::memory_order_acquire);
                std::atomic_store_explicit(&current_, new_ptr, std::memory_order_release);
            }
            return *this;
        }

        /**
         * @brief Destructor
         */
        ~rcu_value() = default;

        /**
         * @brief Lock-free read - returns current snapshot
         *
         * This operation is wait-free: it completes in O(1) time regardless of
         * what other threads are doing. The returned shared_ptr keeps the
         * snapshot alive even if the value is updated by another thread.
         *
         * @return Shared pointer to immutable snapshot of current value
         *
         * Thread safety: Safe to call concurrently with any other operation
         */
        [[nodiscard]] std::shared_ptr<const T> read() const noexcept {
            return std::atomic_load_explicit(&current_, std::memory_order_acquire);
        }

        /**
         * @brief Update the value atomically
         *
         * Creates a new immutable version and publishes it atomically.
         * The old version is automatically reclaimed when the last reader releases it.
         *
         * @param new_value The new value to store
         *
         * Thread safety: Safe to call concurrently with any other operation
         */
        void update(T new_value) {
            auto new_ptr = std::make_shared<const T>(std::move(new_value));
            std::atomic_store_explicit(&current_, new_ptr, std::memory_order_release);
            update_count_.fetch_add(1, std::memory_order_relaxed);
        }

        /**
         * @brief Compare-and-swap update
         *
         * Atomically updates the value only if the current value equals expected.
         * This is useful for implementing lock-free algorithms that need to detect
         * concurrent modifications.
         *
         * @param expected Shared pointer to expected current value
         * @param new_value The new value to store if current matches expected
         * @return true if update succeeded, false if current value changed
         *
         * Thread safety: Safe to call concurrently with any other operation
         */
        bool compare_and_update(const std::shared_ptr<const T>& expected, T new_value) {
            auto new_ptr = std::make_shared<const T>(std::move(new_value));
            auto expected_copy = expected;
            bool success = std::atomic_compare_exchange_strong_explicit(
                &current_,
                &expected_copy,
                new_ptr,
                std::memory_order_acq_rel,
                std::memory_order_acquire
            );
            if (success) {
                update_count_.fetch_add(1, std::memory_order_relaxed);
            }
            return success;
        }

        /**
         * @brief Get the number of updates performed
         * @return Update count since construction
         */
        [[nodiscard]] size_t update_count() const noexcept {
            return update_count_.load(std::memory_order_relaxed);
        }

        /**
         * @brief Check if the value has been initialized
         * @return true if a value is stored, false if null
         */
        [[nodiscard]] bool has_value() const noexcept {
            return std::atomic_load_explicit(&current_, std::memory_order_acquire) != nullptr;
        }

    private:
        std::shared_ptr<const T> current_;
        std::atomic<size_t> update_count_{0};
    };

} // namespace container_module
