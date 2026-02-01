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

#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <algorithm>
#include <concepts>
#include <thread>
#include <chrono>
#include <condition_variable>
#include "internal/value.h"
#include "core/concepts.h"

namespace container_module
{
    // Forward declarations
    class lockfree_container_reader;
    class auto_refresh_reader;

    /**
     * @brief Thread-safe container with lock optimization
     * 
     * This container provides thread-safe access to variant values with optimized
     * locking strategies for different access patterns.
     */
    class thread_safe_container : public std::enable_shared_from_this<thread_safe_container>
    {
    public:
        using value_map = std::unordered_map<std::string, value>;
        using const_iterator = value_map::const_iterator;
        using iterator = value_map::iterator;

        /**
         * @brief Default constructor
         */
        thread_safe_container() = default;

        /**
         * @brief Construct with initial values
         */
        thread_safe_container(std::initializer_list<std::pair<std::string, value>> init);

        /**
         * @brief Copy constructor (thread-safe)
         */
        thread_safe_container(const thread_safe_container& other);

        /**
         * @brief Move constructor
         */
        thread_safe_container(thread_safe_container&& other) noexcept;

        /**
         * @brief Copy assignment (thread-safe)
         */
        thread_safe_container& operator=(const thread_safe_container& other);

        /**
         * @brief Move assignment
         */
        thread_safe_container& operator=(thread_safe_container&& other) noexcept;

        /**
         * @brief Get value by key (thread-safe read)
         * @param key The key to look up
         * @return Optional containing the value if found
         */
        std::optional<value> get(std::string_view key) const;

        /**
         * @brief Get typed value by key
         * @tparam T The expected type (must be a valid variant type)
         * @param key The key to look up
         * @return Optional containing the value if found and type matches
         */
        template<concepts::ValueVariantType T>
        std::optional<T> get_typed(std::string_view key) const {
            std::shared_lock lock(mutex_);
            auto it = values_.find(std::string(key));
            if (it != values_.end()) {
                return it->second.get<T>();
            }
            return std::nullopt;
        }

        /**
         * @brief Set value for key (thread-safe write)
         * @param key The key to set
         * @param value The value to store
         */
        void set(std::string_view key, value value);

        /**
         * @brief Set typed value for key
         * @tparam T The value type (must be a valid variant type)
         * @param key The key to set
         * @param value The value to store
         */
        template<concepts::ValueVariantType T>
        void set_typed(std::string_view key, T&& val) {
            set(key, value(key, std::forward<T>(val)));
        }

        /**
         * @brief Remove value by key
         * @param key The key to remove
         * @return true if removed, false if not found
         */
        bool remove(std::string_view key);

        /**
         * @brief Clear all values
         */
        void clear();

        /**
         * @brief Get the number of values
         */
        size_t size() const;

        /**
         * @brief Check if container is empty
         */
        bool empty() const;

        /**
         * @brief Check if key exists
         */
        bool contains(std::string_view key) const;

        /**
         * @brief Get all keys
         */
        std::vector<std::string> keys() const;

        /**
         * @brief Set a value directly using variant value
         * @param val The value to store (key is extracted from value's name)
         */
        void set_variant(const value& val);

        /**
         * @brief Get a value as variant value
         * @param key The key to look up
         * @return Optional containing the value if found
         */
        std::optional<value> get_variant(const std::string& key) const;

        /**
         * @brief Set a nested container
         * @param key The key for the nested container
         * @param container The container to store
         */
        void set_container(const std::string& key,
                          std::shared_ptr<thread_safe_container> container);

        /**
         * @brief Get a nested container
         * @param key The key to look up
         * @return Shared pointer to container, nullptr if not found or not a container
         */
        std::shared_ptr<thread_safe_container> get_container(const std::string& key) const;

        /**
         * @brief Apply a function to all values (read-only)
         * @tparam Func Function type satisfying KeyValueCallback concept
         * @param func Function to apply to each key-value pair
         */
        template<concepts::KeyValueCallback Func>
        void for_each(Func&& func) const {
            std::shared_lock lock(mutex_);
            for (const auto& [key, val] : values_) {
                func(key, val);
            }
        }

        /**
         * @brief Apply a function to all values (mutable)
         * @tparam Func Function type satisfying MutableKeyValueCallback concept
         * @param func Function to apply to each key-value pair
         */
        template<concepts::MutableKeyValueCallback Func>
        void for_each_mut(Func&& func) {
            std::unique_lock lock(mutex_);
            for (auto& [key, val] : values_) {
                func(key, val);
            }
        }

        /**
         * @brief Bulk update operation with minimal lock contention
         * @tparam Func Function type satisfying ValueMapCallback concept
         * @param updater Function to perform bulk updates
         */
        template<concepts::ValueMapCallback<value_map> Func>
        void bulk_update(Func&& updater) {
            std::unique_lock lock(mutex_);
            updater(values_);
            bulk_write_count_.fetch_add(1, std::memory_order_relaxed);
        }

        /**
         * @brief Bulk read operation
         * @tparam Func Function type satisfying ConstValueMapCallback concept
         * @param reader Function to perform bulk reads
         * @return Result of the reader function
         */
        template<concepts::ConstValueMapCallback<value_map> Func>
        auto bulk_read(Func&& reader) const {
            std::shared_lock lock(mutex_);
            bulk_read_count_.fetch_add(1, std::memory_order_relaxed);
            return reader(values_);
        }

        /**
         * @brief Atomic compare and swap
         * @param key The key to update
         * @param expected The expected current value
         * @param desired The desired new value
         * @return true if swap succeeded, false otherwise
         */
        bool compare_exchange(std::string_view key,
                            const value& expected,
                            const value& desired);

        /**
         * @brief Get statistics
         */
        struct Statistics {
            size_t read_count;
            size_t write_count;
            size_t bulk_read_count;
            size_t bulk_write_count;
            size_t size;
        };

        Statistics get_statistics() const;

        /**
         * @brief Serialize container to JSON
         */
        std::string to_json() const;

        /**
         * @brief Serialize container to binary
         */
        std::vector<uint8_t> serialize() const;

        /**
         * @brief Deserialize from binary
         */
        static std::shared_ptr<thread_safe_container> deserialize(
            const std::vector<uint8_t>& data);

        /**
         * @brief Array-style access (creates if not exists)
         */
        value& operator[](const std::string& key);

        /**
         * @brief Create a lock-free reader for this container
         *
         * The returned reader provides truly lock-free reads using the RCU pattern.
         * Call refresh() on the reader periodically to update its snapshot.
         *
         * @return Shared pointer to a new lock-free reader
         */
        std::shared_ptr<lockfree_container_reader> create_lockfree_reader();

        /**
         * @brief Create an auto-refreshing lock-free reader
         *
         * The returned reader automatically refreshes its snapshot at the specified interval
         * using a background thread. This is useful for scenarios where you want lock-free
         * reads with automatic updates without managing the refresh manually.
         *
         * @param refresh_interval The interval between automatic refreshes
         * @return Shared pointer to a new auto-refresh reader
         *
         * @code
         * auto container = std::make_shared<thread_safe_container>();
         * auto reader = container->create_auto_refresh_reader(std::chrono::milliseconds(100));
         *
         * // Lock-free reads are always up-to-date within refresh_interval
         * auto value = reader->get<int>("counter");
         * @endcode
         */
        std::shared_ptr<auto_refresh_reader> create_auto_refresh_reader(
            std::chrono::milliseconds refresh_interval);

    private:
        mutable std::shared_mutex mutex_;
        value_map values_;
        
        // Statistics
        mutable std::atomic<size_t> read_count_{0};
        mutable std::atomic<size_t> write_count_{0};
        mutable std::atomic<size_t> bulk_read_count_{0};
        mutable std::atomic<size_t> bulk_write_count_{0};
    };

    /**
     * @brief Snapshot-based reader for frequently accessed data
     *
     * Uses a snapshot pattern to provide read access with reduced lock contention.
     * Note: This is not truly lock-free, as it uses a shared_mutex internally.
     * The snapshot is updated explicitly, allowing multiple reads without acquiring
     * the main container lock. This is useful for read-heavy workloads where
     * slightly stale data is acceptable.
     */
    class snapshot_reader {
    public:
        using snapshot_ptr = std::shared_ptr<const thread_safe_container::value_map>;

        explicit snapshot_reader(std::shared_ptr<thread_safe_container> container)
            : container_(container) {
            update_snapshot();
        }

        /**
         * @brief Get value from snapshot (lock-protected read of snapshot)
         * @tparam T The expected type (must be a valid variant type)
         * @note The snapshot may be stale; call update_snapshot() to refresh
         */
        template<concepts::ValueVariantType T>
        std::optional<T> get(std::string_view key) const {
            std::shared_lock lock(snapshot_mutex_);
            if (!snapshot_) return std::nullopt;

            auto it = snapshot_->find(std::string(key));
            if (it != snapshot_->end()) {
                return it->second.get<T>();
            }
            return std::nullopt;
        }

        /**
         * @brief Update snapshot from container
         * @note This acquires the container's lock and should be called periodically
         */
        void update_snapshot();

    private:
        std::shared_ptr<thread_safe_container> container_;
        snapshot_ptr snapshot_;
        mutable std::shared_mutex snapshot_mutex_;
    };

    /**
     * @brief True lock-free reader using RCU (Read-Copy-Update) pattern
     *
     * Unlike snapshot_reader which still uses a shared_mutex for snapshot access,
     * this class provides genuinely lock-free reads using atomic shared_ptr operations.
     * The trade-off is that reads may return slightly stale data between refreshes.
     *
     * Properties:
     * - Read: Wait-free O(1) - no locks, no blocking, no spinning
     * - Refresh: Blocking (acquires source container lock)
     * - Memory: Automatic reclamation via shared_ptr reference counting
     *
     * Use this when:
     * - You need true lock-free reads (e.g., signal handlers, real-time systems)
     * - Read performance is critical and slight staleness is acceptable
     * - You have many concurrent readers
     *
     * @code
     * // Create lock-free reader
     * auto container = std::make_shared<thread_safe_container>();
     * lockfree_container_reader reader(container);
     *
     * // Lock-free reads from any thread
     * auto value = reader.get<int>("counter");  // No locks!
     *
     * // Periodic refresh (e.g., from background thread)
     * reader.refresh();
     * @endcode
     */
    class lockfree_container_reader {
    public:
        using snapshot_type = std::unordered_map<std::string, value>;
        using snapshot_ptr = std::shared_ptr<const snapshot_type>;

        /**
         * @brief Construct reader from container
         * @param container The thread-safe container to read from
         */
        explicit lockfree_container_reader(std::shared_ptr<thread_safe_container> container)
            : container_(std::move(container))
            , snapshot_(std::make_shared<const snapshot_type>())
        {
            refresh();
        }

        /**
         * @brief Copy constructor
         */
        lockfree_container_reader(const lockfree_container_reader& other)
            : container_(other.container_)
            , snapshot_(std::atomic_load_explicit(&other.snapshot_, std::memory_order_acquire))
        {}

        /**
         * @brief Move constructor
         */
        lockfree_container_reader(lockfree_container_reader&& other) noexcept
            : container_(std::move(other.container_))
            , snapshot_(std::atomic_load_explicit(&other.snapshot_, std::memory_order_acquire))
        {}

        /**
         * @brief Copy assignment
         */
        lockfree_container_reader& operator=(const lockfree_container_reader& other) {
            if (this != &other) {
                container_ = other.container_;
                auto new_snapshot = std::atomic_load_explicit(&other.snapshot_, std::memory_order_acquire);
                std::atomic_store_explicit(&snapshot_, new_snapshot, std::memory_order_release);
            }
            return *this;
        }

        /**
         * @brief Move assignment
         */
        lockfree_container_reader& operator=(lockfree_container_reader&& other) noexcept {
            if (this != &other) {
                container_ = std::move(other.container_);
                auto new_snapshot = std::atomic_load_explicit(&other.snapshot_, std::memory_order_acquire);
                std::atomic_store_explicit(&snapshot_, new_snapshot, std::memory_order_release);
            }
            return *this;
        }

        /**
         * @brief Destructor
         */
        ~lockfree_container_reader() = default;

        /**
         * @brief Lock-free typed value read
         *
         * This operation is wait-free: it completes in O(1) time regardless of
         * what other threads are doing. The snapshot may be stale; call refresh()
         * to update.
         *
         * @tparam T The expected value type (must satisfy ValueVariantType)
         * @param key The key to look up
         * @return Optional containing value if found and type matches
         *
         * Thread safety: Safe to call concurrently with any operation including refresh()
         */
        template<concepts::ValueVariantType T>
        [[nodiscard]] std::optional<T> get(std::string_view key) const noexcept {
            auto snap = std::atomic_load_explicit(&snapshot_, std::memory_order_acquire);
            if (!snap) {
                return std::nullopt;
            }

            auto it = snap->find(std::string(key));
            if (it != snap->end()) {
                return it->second.get<T>();
            }
            return std::nullopt;
        }

        /**
         * @brief Lock-free value existence check
         *
         * @param key The key to check
         * @return true if key exists in snapshot, false otherwise
         *
         * Thread safety: Safe to call concurrently with any operation
         */
        [[nodiscard]] bool contains(std::string_view key) const noexcept {
            auto snap = std::atomic_load_explicit(&snapshot_, std::memory_order_acquire);
            if (!snap) {
                return false;
            }
            return snap->find(std::string(key)) != snap->end();
        }

        /**
         * @brief Lock-free snapshot size check
         *
         * @return Number of values in the current snapshot
         *
         * Thread safety: Safe to call concurrently with any operation
         */
        [[nodiscard]] size_t size() const noexcept {
            auto snap = std::atomic_load_explicit(&snapshot_, std::memory_order_acquire);
            return snap ? snap->size() : 0;
        }

        /**
         * @brief Lock-free empty check
         *
         * @return true if snapshot is empty, false otherwise
         *
         * Thread safety: Safe to call concurrently with any operation
         */
        [[nodiscard]] bool empty() const noexcept {
            return size() == 0;
        }

        /**
         * @brief Get all keys from snapshot (lock-free)
         *
         * @return Vector of keys in the current snapshot
         *
         * Thread safety: Safe to call concurrently with any operation
         */
        [[nodiscard]] std::vector<std::string> keys() const {
            auto snap = std::atomic_load_explicit(&snapshot_, std::memory_order_acquire);
            std::vector<std::string> result;
            if (snap) {
                result.reserve(snap->size());
                for (const auto& [key, val] : *snap) {
                    result.push_back(key);
                }
            }
            return result;
        }

        /**
         * @brief Iterate over snapshot (lock-free for snapshot access)
         *
         * The callback is called for each key-value pair in the snapshot.
         * The iteration itself is lock-free, but the callback may perform
         * locking operations.
         *
         * @tparam Func Callback function type: void(const std::string&, const value&)
         * @param func Callback to apply to each key-value pair
         *
         * Thread safety: Safe to call concurrently with any operation
         */
        template<typename Func>
        void for_each(Func&& func) const {
            auto snap = std::atomic_load_explicit(&snapshot_, std::memory_order_acquire);
            if (snap) {
                for (const auto& [key, val] : *snap) {
                    func(key, val);
                }
            }
        }

        /**
         * @brief Refresh snapshot from source container
         *
         * This operation acquires the container's read lock to create a new snapshot.
         * After this call returns, subsequent lock-free reads will see the updated data.
         *
         * Thread safety: Safe to call concurrently with reads, but multiple
         * concurrent refreshes may waste work (only one will "win").
         */
        void refresh() {
            auto new_snapshot = std::make_shared<snapshot_type>();
            container_->for_each([&new_snapshot](const std::string& key, const value& val) {
                new_snapshot->emplace(key, val);
            });
            std::atomic_store_explicit(&snapshot_,
                                      snapshot_ptr(new_snapshot),
                                      std::memory_order_release);
            refresh_count_.fetch_add(1, std::memory_order_relaxed);
        }

        /**
         * @brief Get the number of refreshes performed
         * @return Refresh count since construction
         */
        [[nodiscard]] size_t refresh_count() const noexcept {
            return refresh_count_.load(std::memory_order_relaxed);
        }

        /**
         * @brief Get the source container
         * @return Shared pointer to the source container
         */
        [[nodiscard]] std::shared_ptr<thread_safe_container> source() const noexcept {
            return container_;
        }

    private:
        std::shared_ptr<thread_safe_container> container_;
        snapshot_ptr snapshot_;
        std::atomic<size_t> refresh_count_{0};
    };

    /**
     * @brief Auto-refreshing lock-free reader
     *
     * This class wraps a lockfree_container_reader and automatically refreshes
     * its snapshot at a configurable interval using a background thread.
     * All read operations are delegated to the underlying lock-free reader.
     *
     * Properties:
     * - Read: Wait-free O(1) - same as lockfree_container_reader
     * - Refresh: Automatic in background thread
     * - Staleness: Bounded by refresh_interval
     *
     * @code
     * auto container = std::make_shared<thread_safe_container>();
     * auto reader = std::make_shared<auto_refresh_reader>(container, std::chrono::milliseconds(100));
     *
     * // Lock-free reads automatically refreshed every 100ms
     * auto value = reader->get<int>("counter");
     *
     * // When done, stop will be called automatically in destructor
     * @endcode
     */
    class auto_refresh_reader {
    public:
        /**
         * @brief Construct auto-refresh reader
         * @param container The thread-safe container to read from
         * @param refresh_interval The interval between automatic refreshes
         */
        explicit auto_refresh_reader(std::shared_ptr<thread_safe_container> container,
                                     std::chrono::milliseconds refresh_interval)
            : reader_(std::make_shared<lockfree_container_reader>(std::move(container)))
            , refresh_interval_(refresh_interval)
            , running_(true)
        {
            refresh_thread_ = std::thread(&auto_refresh_reader::refresh_loop, this);
        }

        /**
         * @brief Destructor - stops the refresh thread
         */
        ~auto_refresh_reader() {
            stop();
        }

        // Non-copyable
        auto_refresh_reader(const auto_refresh_reader&) = delete;
        auto_refresh_reader& operator=(const auto_refresh_reader&) = delete;

        // Non-movable (due to thread)
        auto_refresh_reader(auto_refresh_reader&&) = delete;
        auto_refresh_reader& operator=(auto_refresh_reader&&) = delete;

        /**
         * @brief Stop the auto-refresh thread
         *
         * After calling stop(), no more automatic refreshes will occur.
         * Manual refresh() calls are still possible.
         */
        void stop() {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (!running_) {
                    return;
                }
                running_ = false;
            }
            cv_.notify_one();
            if (refresh_thread_.joinable()) {
                refresh_thread_.join();
            }
        }

        /**
         * @brief Check if auto-refresh is running
         * @return true if auto-refresh thread is active
         */
        [[nodiscard]] bool is_running() const noexcept {
            std::lock_guard<std::mutex> lock(mutex_);
            return running_;
        }

        /**
         * @brief Get the refresh interval
         * @return The configured refresh interval
         */
        [[nodiscard]] std::chrono::milliseconds refresh_interval() const noexcept {
            return refresh_interval_;
        }

        /**
         * @brief Lock-free typed value read
         *
         * Delegates to the underlying lockfree_container_reader.
         *
         * @tparam T The expected value type
         * @param key The key to look up
         * @return Optional containing value if found and type matches
         */
        template<concepts::ValueVariantType T>
        [[nodiscard]] std::optional<T> get(std::string_view key) const noexcept {
            return reader_->get<T>(key);
        }

        /**
         * @brief Lock-free value existence check
         * @param key The key to check
         * @return true if key exists in snapshot
         */
        [[nodiscard]] bool contains(std::string_view key) const noexcept {
            return reader_->contains(key);
        }

        /**
         * @brief Lock-free snapshot size check
         * @return Number of values in the current snapshot
         */
        [[nodiscard]] size_t size() const noexcept {
            return reader_->size();
        }

        /**
         * @brief Lock-free empty check
         * @return true if snapshot is empty
         */
        [[nodiscard]] bool empty() const noexcept {
            return reader_->empty();
        }

        /**
         * @brief Get all keys from snapshot
         * @return Vector of keys in the current snapshot
         */
        [[nodiscard]] std::vector<std::string> keys() const {
            return reader_->keys();
        }

        /**
         * @brief Iterate over snapshot
         * @tparam Func Callback function type
         * @param func Callback to apply to each key-value pair
         */
        template<typename Func>
        void for_each(Func&& func) const {
            reader_->for_each(std::forward<Func>(func));
        }

        /**
         * @brief Manual refresh (in addition to automatic)
         *
         * Forces an immediate refresh of the snapshot.
         */
        void refresh() {
            reader_->refresh();
        }

        /**
         * @brief Get the number of refreshes performed
         * @return Total refresh count (automatic + manual)
         */
        [[nodiscard]] size_t refresh_count() const noexcept {
            return reader_->refresh_count();
        }

        /**
         * @brief Get the underlying lock-free reader
         * @return Shared pointer to the underlying reader
         */
        [[nodiscard]] std::shared_ptr<lockfree_container_reader> reader() const noexcept {
            return reader_;
        }

        /**
         * @brief Get the source container
         * @return Shared pointer to the source container
         */
        [[nodiscard]] std::shared_ptr<thread_safe_container> source() const noexcept {
            return reader_->source();
        }

    private:
        void refresh_loop() {
            while (true) {
                std::unique_lock<std::mutex> lock(mutex_);
                if (cv_.wait_for(lock, refresh_interval_, [this] { return !running_; })) {
                    break;  // Stopped
                }
                lock.unlock();
                reader_->refresh();
            }
        }

        std::shared_ptr<lockfree_container_reader> reader_;
        std::chrono::milliseconds refresh_interval_;
        std::thread refresh_thread_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        bool running_;
    };

    // Type alias for backward compatibility
    using lockfree_reader = snapshot_reader;

    // Inline definition of create_lockfree_reader (must be after lockfree_container_reader is defined)
    inline std::shared_ptr<lockfree_container_reader> thread_safe_container::create_lockfree_reader() {
        return std::make_shared<lockfree_container_reader>(shared_from_this());
    }

    // Inline definition of create_auto_refresh_reader (must be after auto_refresh_reader is defined)
    inline std::shared_ptr<auto_refresh_reader> thread_safe_container::create_auto_refresh_reader(
        std::chrono::milliseconds refresh_interval) {
        return std::make_shared<auto_refresh_reader>(shared_from_this(), refresh_interval);
    }

} // namespace container_module