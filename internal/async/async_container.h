/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
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
 * @file internal/async/async_container.h
 * @brief Async wrapper for value_container operations
 *
 * Provides coroutine-based async API for container serialization and
 * deserialization operations.
 *
 * @code
 * // Example usage:
 * task<void> process() {
 *     auto container = std::make_shared<value_container>();
 *     container->set("key", "value");
 *
 *     async_container async_cont(container);
 *     auto data = co_await async_cont.serialize_async();
 *     if (data.is_ok()) {
 *         // Use serialized data
 *     }
 * }
 * @endcode
 *
 * @see container_module::async::async_container
 */

#pragma once

#include "task.h"
#include "container/core/container.h"
#include "container/core/container/error_codes.h"

#include <memory>
#include <vector>
#include <string>
#include <span>
#include <thread>
#include <functional>
#include <atomic>
#include <fstream>
#include <optional>

namespace container_module::async
{
    /**
     * @brief Progress callback type for async file operations
     * @param bytes_processed Number of bytes processed so far
     * @param total_bytes Total number of bytes (0 if unknown)
     */
    using progress_callback = std::function<void(size_t bytes_processed, size_t total_bytes)>;

    namespace detail
    {
        /**
         * @brief Shared state for async operations
         *
         * This state is shared between the awaitable and the worker thread
         * using shared_ptr, ensuring thread-safe access even when the
         * awaitable is moved or destroyed.
         */
        template<typename T>
        struct async_state
        {
            std::function<T()> work_;
            std::optional<T> result_;
            std::exception_ptr exception_;
            std::atomic<bool> ready_{false};

            explicit async_state(std::function<T()> work)
                : work_(std::move(work)) {}

            // Non-copyable, non-movable
            async_state(const async_state&) = delete;
            async_state& operator=(const async_state&) = delete;
            async_state(async_state&&) = delete;
            async_state& operator=(async_state&&) = delete;
        };

        /**
         * @brief Awaitable that runs work in a separate thread
         *
         * Uses shared_ptr to manage state, ensuring the worker thread
         * can safely access state even if the awaitable is moved.
         */
        template<typename T>
        struct async_awaitable
        {
            std::shared_ptr<async_state<T>> state_;

            explicit async_awaitable(std::function<T()> work)
                : state_(std::make_shared<async_state<T>>(std::move(work))) {}

            async_awaitable(async_awaitable&&) noexcept = default;
            async_awaitable& operator=(async_awaitable&&) noexcept = default;

            async_awaitable(const async_awaitable&) = delete;
            async_awaitable& operator=(const async_awaitable&) = delete;

            [[nodiscard]] bool await_ready() const noexcept
            {
                return false;
            }

            void await_suspend(std::coroutine_handle<> handle)
            {
                // Copy shared_ptr for thread to ensure state lifetime
                auto state = state_;
                std::thread([state, handle]() mutable {
                    try {
                        state->result_.emplace(state->work_());
                    } catch (...) {
                        state->exception_ = std::current_exception();
                    }
                    // Release barrier ensures all writes above are visible
                    // to the thread that reads with acquire semantics
                    state->ready_.store(true, std::memory_order_release);
                    handle.resume();
                }).detach();
            }

            T await_resume()
            {
                // Acquire barrier synchronizes with the release store above,
                // ensuring all writes in the worker thread are visible here
                while (!state_->ready_.load(std::memory_order_acquire)) {
                    // Spin until ready - in practice, handle.resume()
                    // ensures we only get here after ready_ is set
                }
                if (state_->exception_) {
                    std::rethrow_exception(state_->exception_);
                }
                return std::move(*state_->result_);
            }
        };

        template<typename F>
        auto make_async_awaitable(F&& func) -> async_awaitable<std::invoke_result_t<F>>
        {
            using result_type = std::invoke_result_t<F>;
            return async_awaitable<result_type>(std::forward<F>(func));
        }

    } // namespace detail

    /**
     * @brief Async wrapper for value_container operations
     *
     * This class wraps a value_container and provides async versions of
     * serialization and deserialization operations using C++20 coroutines.
     *
     * The async operations offload CPU-bound work to a separate thread,
     * allowing the calling coroutine to yield and resume when the operation
     * completes.
     *
     * Properties:
     * - Thread-safe wrapper for value_container
     * - Supports both Result-based and exception-based APIs
     * - Movable but not copyable
     */
    class async_container
    {
    public:
        /**
         * @brief Construct async_container with existing container
         * @param container Shared pointer to value_container
         */
        explicit async_container(std::shared_ptr<value_container> container)
            : container_(std::move(container))
        {
        }

        /**
         * @brief Construct async_container with new empty container
         */
        async_container()
            : container_(std::make_shared<value_container>())
        {
        }

        /**
         * @brief Move constructor
         */
        async_container(async_container&& other) noexcept = default;

        /**
         * @brief Move assignment operator
         */
        async_container& operator=(async_container&& other) noexcept = default;

        /**
         * @brief Destructor
         */
        ~async_container() = default;

        // Non-copyable
        async_container(const async_container&) = delete;
        async_container& operator=(const async_container&) = delete;

        /**
         * @brief Get the underlying container
         * @return Shared pointer to the wrapped value_container
         */
        [[nodiscard]] std::shared_ptr<value_container> get_container() const noexcept
        {
            return container_;
        }

        /**
         * @brief Set the underlying container
         * @param container New container to wrap
         */
        void set_container(std::shared_ptr<value_container> container) noexcept
        {
            container_ = std::move(container);
        }

        // =======================================================================
        // Async Serialization APIs
        // =======================================================================

        /**
         * @brief Serialize container to byte array asynchronously
         *
         * Offloads serialization to a worker thread and returns a task
         * that completes when serialization is done.
         *
         * @return task containing Result with serialized bytes or error
         *
         * @code
         * async_container cont(my_container);
         * auto result = co_await cont.serialize_async();
         * if (result.is_ok()) {
         *     auto& bytes = result.value();
         *     // Use serialized bytes
         * } else {
         *     // Handle error
         * }
         * @endcode
         */
#if CONTAINER_HAS_COMMON_RESULT
        [[nodiscard]] task<kcenon::common::Result<std::vector<uint8_t>>> serialize_async() const;
#else
        [[nodiscard]] task<std::vector<uint8_t>> serialize_async() const;
#endif

        /**
         * @brief Serialize container to string asynchronously
         *
         * @return task containing Result with serialized string or error
         */
#if CONTAINER_HAS_COMMON_RESULT
        [[nodiscard]] task<kcenon::common::Result<std::string>> serialize_string_async() const;
#else
        [[nodiscard]] task<std::string> serialize_string_async() const;
#endif

        // =======================================================================
        // Async Deserialization APIs
        // =======================================================================

        /**
         * @brief Deserialize from byte array asynchronously
         *
         * Creates a new container from serialized data.
         *
         * @param data Span of bytes containing serialized container data
         * @return task containing Result with deserialized container or error
         *
         * @code
         * std::vector<uint8_t> data = receive_data();
         * auto result = co_await async_container::deserialize_async(data);
         * if (result.is_ok()) {
         *     auto container = result.value();
         *     // Use deserialized container
         * }
         * @endcode
         */
#if CONTAINER_HAS_COMMON_RESULT
        [[nodiscard]] static task<kcenon::common::Result<std::shared_ptr<value_container>>>
            deserialize_async(std::span<const uint8_t> data);
#else
        [[nodiscard]] static task<std::shared_ptr<value_container>>
            deserialize_async(std::span<const uint8_t> data);
#endif

        /**
         * @brief Deserialize from string asynchronously
         *
         * @param data String containing serialized container data
         * @return task containing Result with deserialized container or error
         */
#if CONTAINER_HAS_COMMON_RESULT
        [[nodiscard]] static task<kcenon::common::Result<std::shared_ptr<value_container>>>
            deserialize_string_async(std::string_view data);
#else
        [[nodiscard]] static task<std::shared_ptr<value_container>>
            deserialize_string_async(std::string_view data);
#endif

        // =======================================================================
        // Convenience Methods (forwarding to underlying container)
        // =======================================================================

        /**
         * @brief Set a value in the container
         * @tparam T Value type
         * @param key Key name
         * @param value Value to set
         * @return Reference to this async_container for chaining
         */
        template<typename T>
        async_container& set(std::string_view key, T&& value)
        {
            container_->set(key, std::forward<T>(value));
            return *this;
        }

        /**
         * @brief Get a value from the container
         * @tparam T Expected value type
         * @param key Key name
         * @return Optional containing the value if found and type matches
         */
        template<typename T>
        [[nodiscard]] std::optional<T> get(std::string_view key) const
        {
            auto val = container_->get_value(std::string(key));
            if (!val) {
                return std::nullopt;
            }
            if (auto* ptr = std::get_if<T>(&val->data)) {
                return *ptr;
            }
            return std::nullopt;
        }

        /**
         * @brief Check if container contains a key
         * @param key Key name to check
         * @return true if key exists
         */
        [[nodiscard]] bool contains(std::string_view key) const noexcept
        {
            return container_->contains(key);
        }

        // =======================================================================
        // Async File I/O APIs (Issue #267 - Phase 3)
        // =======================================================================

        /**
         * @brief Load container from file asynchronously
         *
         * Reads the file in a worker thread and deserializes the content
         * into the container.
         *
         * @param path File path to load from
         * @param callback Optional progress callback
         * @return task containing VoidResult indicating success or error
         *
         * @code
         * async_container cont;
         * auto result = co_await cont.load_async("data.bin");
         * if (result.is_ok()) {
         *     // Container now contains loaded data
         * }
         * @endcode
         */
#if CONTAINER_HAS_COMMON_RESULT
        [[nodiscard]] task<kcenon::common::VoidResult> load_async(
            std::string_view path,
            progress_callback callback = nullptr);
#else
        [[nodiscard]] task<bool> load_async(
            std::string_view path,
            progress_callback callback = nullptr);
#endif

        /**
         * @brief Save container to file asynchronously
         *
         * Serializes the container and writes to file in a worker thread.
         *
         * @param path File path to save to
         * @param callback Optional progress callback
         * @return task containing VoidResult indicating success or error
         *
         * @code
         * async_container cont;
         * cont.set("key", "value");
         * auto result = co_await cont.save_async("output.bin");
         * if (result.is_ok()) {
         *     // Data saved successfully
         * }
         * @endcode
         */
#if CONTAINER_HAS_COMMON_RESULT
        [[nodiscard]] task<kcenon::common::VoidResult> save_async(
            std::string_view path,
            progress_callback callback = nullptr);
#else
        [[nodiscard]] task<bool> save_async(
            std::string_view path,
            progress_callback callback = nullptr);
#endif

    private:
        std::shared_ptr<value_container> container_;
    };

    // ==========================================================================
    // Async File I/O Utility Functions (Issue #267 - Phase 3)
    // ==========================================================================

    /**
     * @brief Read file contents asynchronously
     *
     * Reads entire file content in a worker thread.
     *
     * @param path File path to read from
     * @param callback Optional progress callback
     * @return task containing Result with file bytes or error
     *
     * @code
     * auto result = co_await read_file_async("data.bin");
     * if (result.is_ok()) {
     *     auto& bytes = result.value();
     *     // Process bytes
     * }
     * @endcode
     */
#if CONTAINER_HAS_COMMON_RESULT
    [[nodiscard]] task<kcenon::common::Result<std::vector<uint8_t>>> read_file_async(
        std::string_view path,
        progress_callback callback = nullptr);
#else
    [[nodiscard]] task<std::vector<uint8_t>> read_file_async(
        std::string_view path,
        progress_callback callback = nullptr);
#endif

    /**
     * @brief Write data to file asynchronously
     *
     * Writes data to file in a worker thread.
     *
     * @param path File path to write to
     * @param data Data to write
     * @param callback Optional progress callback
     * @return task containing VoidResult indicating success or error
     *
     * @code
     * std::vector<uint8_t> data = {1, 2, 3, 4, 5};
     * auto result = co_await write_file_async("output.bin", data);
     * if (result.is_ok()) {
     *     // File written successfully
     * }
     * @endcode
     */
#if CONTAINER_HAS_COMMON_RESULT
    [[nodiscard]] task<kcenon::common::VoidResult> write_file_async(
        std::string_view path,
        std::span<const uint8_t> data,
        progress_callback callback = nullptr);
#else
    [[nodiscard]] task<bool> write_file_async(
        std::string_view path,
        std::span<const uint8_t> data,
        progress_callback callback = nullptr);
#endif

    // ==========================================================================
    // Implementation of async methods (separate to avoid compiler issues)
    // ==========================================================================

#if CONTAINER_HAS_COMMON_RESULT
    inline task<kcenon::common::Result<std::vector<uint8_t>>>
        async_container::serialize_async() const
    {
        auto container = container_;
        auto result = co_await detail::make_async_awaitable([container]() {
            return container->serialize_array_result();
        });
        co_return result;
    }

    inline task<kcenon::common::Result<std::string>>
        async_container::serialize_string_async() const
    {
        auto container = container_;
        auto result = co_await detail::make_async_awaitable([container]() {
            return container->serialize_result();
        });
        co_return result;
    }

    inline task<kcenon::common::Result<std::shared_ptr<value_container>>>
        async_container::deserialize_async(std::span<const uint8_t> data)
    {
        std::vector<uint8_t> data_copy(data.begin(), data.end());
        auto result = co_await detail::make_async_awaitable(
            [data_copy = std::move(data_copy)]() {
                auto container = std::make_shared<value_container>();
                auto deser_result = container->deserialize_result(data_copy, false);
                if (!deser_result.is_ok()) {
                    return kcenon::common::Result<std::shared_ptr<value_container>>(
                        deser_result.error());
                }
                return kcenon::common::ok(container);
            });
        co_return result;
    }

    inline task<kcenon::common::Result<std::shared_ptr<value_container>>>
        async_container::deserialize_string_async(std::string_view data)
    {
        std::string data_copy(data);
        auto result = co_await detail::make_async_awaitable(
            [data_copy = std::move(data_copy)]() {
                auto container = std::make_shared<value_container>();
                auto deser_result = container->deserialize_result(data_copy, false);
                if (!deser_result.is_ok()) {
                    return kcenon::common::Result<std::shared_ptr<value_container>>(
                        deser_result.error());
                }
                return kcenon::common::ok(container);
            });
        co_return result;
    }

    inline task<kcenon::common::VoidResult>
        async_container::load_async(std::string_view path, progress_callback callback)
    {
        using namespace container_module;
        auto container = container_;
        std::string path_str(path);
        auto result = co_await detail::make_async_awaitable(
            [container, path_str, callback]() -> kcenon::common::VoidResult {
                std::ifstream file(path_str, std::ios::binary | std::ios::ate);
                if (!file) {
                    return kcenon::common::VoidResult(
                        kcenon::common::error_info{
                            container_module::error_codes::file_not_found,
                            container_module::error_codes::make_message(
                                container_module::error_codes::file_not_found, path_str),
                            "container_system"});
                }

                auto size = file.tellg();
                if (size < 0) {
                    return kcenon::common::VoidResult(
                        kcenon::common::error_info{
                            container_module::error_codes::file_read_error,
                            container_module::error_codes::make_message(
                                container_module::error_codes::file_read_error, path_str),
                            "container_system"});
                }
                file.seekg(0, std::ios::beg);

                std::vector<uint8_t> buffer(static_cast<size_t>(size));
                const size_t chunk_size = 64 * 1024;
                size_t bytes_read = 0;

                while (bytes_read < static_cast<size_t>(size)) {
                    size_t to_read = std::min(chunk_size,
                        static_cast<size_t>(size) - bytes_read);
                    file.read(reinterpret_cast<char*>(buffer.data() + bytes_read),
                        static_cast<std::streamsize>(to_read));
                    if (!file) {
                        return kcenon::common::VoidResult(
                            kcenon::common::error_info{
                                container_module::error_codes::file_read_error,
                                container_module::error_codes::make_message(
                                    container_module::error_codes::file_read_error, path_str),
                                "container_system"});
                    }
                    bytes_read += to_read;
                    if (callback) {
                        callback(bytes_read, static_cast<size_t>(size));
                    }
                }

                auto deser_result = container->deserialize_result(buffer, false);
                if (!deser_result.is_ok()) {
                    return deser_result;
                }
                return kcenon::common::ok();
            });
        co_return result;
    }

    inline task<kcenon::common::VoidResult>
        async_container::save_async(std::string_view path, progress_callback callback)
    {
        auto container = container_;
        std::string path_str(path);
        auto result = co_await detail::make_async_awaitable(
            [container, path_str, callback]() -> kcenon::common::VoidResult {
                auto data_result = container->serialize_array_result();
                if (!data_result.is_ok()) {
                    return kcenon::common::VoidResult(data_result.error());
                }
                const auto& data = data_result.value();

                std::ofstream file(path_str, std::ios::binary);
                if (!file) {
                    return kcenon::common::VoidResult(
                        kcenon::common::error_info{
                            container_module::error_codes::file_write_error,
                            container_module::error_codes::make_message(
                                container_module::error_codes::file_write_error, path_str),
                            "container_system"});
                }

                const size_t chunk_size = 64 * 1024;
                size_t bytes_written = 0;
                const size_t total_size = data.size();

                while (bytes_written < total_size) {
                    size_t to_write = std::min(chunk_size, total_size - bytes_written);
                    file.write(reinterpret_cast<const char*>(data.data() + bytes_written),
                        static_cast<std::streamsize>(to_write));
                    if (!file) {
                        return kcenon::common::VoidResult(
                            kcenon::common::error_info{
                                container_module::error_codes::file_write_error,
                                container_module::error_codes::make_message(
                                    container_module::error_codes::file_write_error, path_str),
                                "container_system"});
                    }
                    bytes_written += to_write;
                    if (callback) {
                        callback(bytes_written, total_size);
                    }
                }

                return kcenon::common::ok();
            });
        co_return result;
    }

    inline task<kcenon::common::Result<std::vector<uint8_t>>>
        read_file_async(std::string_view path, progress_callback callback)
    {
        std::string path_str(path);
        auto result = co_await detail::make_async_awaitable(
            [path_str, callback]() -> kcenon::common::Result<std::vector<uint8_t>> {
                std::ifstream file(path_str, std::ios::binary | std::ios::ate);
                if (!file) {
                    return kcenon::common::Result<std::vector<uint8_t>>(
                        kcenon::common::error_info{
                            container_module::error_codes::file_not_found,
                            container_module::error_codes::make_message(
                                container_module::error_codes::file_not_found, path_str),
                            "container_system"});
                }

                auto size = file.tellg();
                if (size < 0) {
                    return kcenon::common::Result<std::vector<uint8_t>>(
                        kcenon::common::error_info{
                            container_module::error_codes::file_read_error,
                            container_module::error_codes::make_message(
                                container_module::error_codes::file_read_error, path_str),
                            "container_system"});
                }
                file.seekg(0, std::ios::beg);

                std::vector<uint8_t> buffer(static_cast<size_t>(size));
                const size_t chunk_size = 64 * 1024;
                size_t bytes_read = 0;

                while (bytes_read < static_cast<size_t>(size)) {
                    size_t to_read = std::min(chunk_size,
                        static_cast<size_t>(size) - bytes_read);
                    file.read(reinterpret_cast<char*>(buffer.data() + bytes_read),
                        static_cast<std::streamsize>(to_read));
                    if (!file) {
                        return kcenon::common::Result<std::vector<uint8_t>>(
                            kcenon::common::error_info{
                                container_module::error_codes::file_read_error,
                                container_module::error_codes::make_message(
                                    container_module::error_codes::file_read_error, path_str),
                                "container_system"});
                    }
                    bytes_read += to_read;
                    if (callback) {
                        callback(bytes_read, static_cast<size_t>(size));
                    }
                }

                return kcenon::common::ok(std::move(buffer));
            });
        co_return result;
    }

    inline task<kcenon::common::VoidResult>
        write_file_async(std::string_view path, std::span<const uint8_t> data,
            progress_callback callback)
    {
        std::string path_str(path);
        std::vector<uint8_t> data_copy(data.begin(), data.end());
        auto result = co_await detail::make_async_awaitable(
            [path_str, data_copy = std::move(data_copy), callback]()
                -> kcenon::common::VoidResult {
                std::ofstream file(path_str, std::ios::binary);
                if (!file) {
                    return kcenon::common::VoidResult(
                        kcenon::common::error_info{
                            container_module::error_codes::file_write_error,
                            container_module::error_codes::make_message(
                                container_module::error_codes::file_write_error, path_str),
                            "container_system"});
                }

                const size_t chunk_size = 64 * 1024;
                size_t bytes_written = 0;
                const size_t total_size = data_copy.size();

                while (bytes_written < total_size) {
                    size_t to_write = std::min(chunk_size, total_size - bytes_written);
                    file.write(reinterpret_cast<const char*>(data_copy.data() + bytes_written),
                        static_cast<std::streamsize>(to_write));
                    if (!file) {
                        return kcenon::common::VoidResult(
                            kcenon::common::error_info{
                                container_module::error_codes::file_write_error,
                                container_module::error_codes::make_message(
                                    container_module::error_codes::file_write_error, path_str),
                                "container_system"});
                    }
                    bytes_written += to_write;
                    if (callback) {
                        callback(bytes_written, total_size);
                    }
                }

                return kcenon::common::ok();
            });
        co_return result;
    }

#else

    inline task<std::vector<uint8_t>>
        async_container::serialize_async() const
    {
        auto container = container_;
        auto result = co_await detail::make_async_awaitable([container]() {
            return container->serialize_array();
        });
        co_return result;
    }

    inline task<std::string>
        async_container::serialize_string_async() const
    {
        auto container = container_;
        auto result = co_await detail::make_async_awaitable([container]() {
            return container->serialize();
        });
        co_return result;
    }

    inline task<std::shared_ptr<value_container>>
        async_container::deserialize_async(std::span<const uint8_t> data)
    {
        std::vector<uint8_t> data_copy(data.begin(), data.end());
        auto result = co_await detail::make_async_awaitable(
            [data_copy = std::move(data_copy)]() {
                auto container = std::make_shared<value_container>(data_copy, false);
                return container;
            });
        co_return result;
    }

    inline task<std::shared_ptr<value_container>>
        async_container::deserialize_string_async(std::string_view data)
    {
        std::string data_copy(data);
        auto result = co_await detail::make_async_awaitable(
            [data_copy = std::move(data_copy)]() {
                auto container = std::make_shared<value_container>(data_copy, false);
                return container;
            });
        co_return result;
    }

    inline task<bool>
        async_container::load_async(std::string_view path, progress_callback callback)
    {
        auto container = container_;
        std::string path_str(path);
        auto result = co_await detail::make_async_awaitable(
            [container, path_str, callback]() {
                std::ifstream file(path_str, std::ios::binary | std::ios::ate);
                if (!file) {
                    return false;
                }

                auto size = file.tellg();
                if (size < 0) {
                    return false;
                }
                file.seekg(0, std::ios::beg);

                std::vector<uint8_t> buffer(static_cast<size_t>(size));
                const size_t chunk_size = 64 * 1024;  // 64KB chunks
                size_t bytes_read = 0;

                while (bytes_read < static_cast<size_t>(size)) {
                    size_t to_read = std::min(chunk_size,
                        static_cast<size_t>(size) - bytes_read);
                    file.read(reinterpret_cast<char*>(buffer.data() + bytes_read),
                        static_cast<std::streamsize>(to_read));
                    if (!file) {
                        return false;
                    }
                    bytes_read += to_read;
                    if (callback) {
                        callback(bytes_read, static_cast<size_t>(size));
                    }
                }

                container->deserialize(buffer, false);
                return true;
            });
        co_return result;
    }

    inline task<bool>
        async_container::save_async(std::string_view path, progress_callback callback)
    {
        auto container = container_;
        std::string path_str(path);
        auto result = co_await detail::make_async_awaitable(
            [container, path_str, callback]() {
                auto data = container->serialize_array();
                if (data.empty()) {
                    return false;
                }

                std::ofstream file(path_str, std::ios::binary);
                if (!file) {
                    return false;
                }

                const size_t chunk_size = 64 * 1024;  // 64KB chunks
                size_t bytes_written = 0;
                const size_t total_size = data.size();

                while (bytes_written < total_size) {
                    size_t to_write = std::min(chunk_size, total_size - bytes_written);
                    file.write(reinterpret_cast<const char*>(data.data() + bytes_written),
                        static_cast<std::streamsize>(to_write));
                    if (!file) {
                        return false;
                    }
                    bytes_written += to_write;
                    if (callback) {
                        callback(bytes_written, total_size);
                    }
                }

                return true;
            });
        co_return result;
    }

    inline task<std::vector<uint8_t>>
        read_file_async(std::string_view path, progress_callback callback)
    {
        std::string path_str(path);
        auto result = co_await detail::make_async_awaitable(
            [path_str, callback]() {
                std::ifstream file(path_str, std::ios::binary | std::ios::ate);
                if (!file) {
                    return std::vector<uint8_t>{};
                }

                auto size = file.tellg();
                if (size < 0) {
                    return std::vector<uint8_t>{};
                }
                file.seekg(0, std::ios::beg);

                std::vector<uint8_t> buffer(static_cast<size_t>(size));
                const size_t chunk_size = 64 * 1024;  // 64KB chunks
                size_t bytes_read = 0;

                while (bytes_read < static_cast<size_t>(size)) {
                    size_t to_read = std::min(chunk_size,
                        static_cast<size_t>(size) - bytes_read);
                    file.read(reinterpret_cast<char*>(buffer.data() + bytes_read),
                        static_cast<std::streamsize>(to_read));
                    if (!file) {
                        return std::vector<uint8_t>{};
                    }
                    bytes_read += to_read;
                    if (callback) {
                        callback(bytes_read, static_cast<size_t>(size));
                    }
                }

                return buffer;
            });
        co_return result;
    }

    inline task<bool>
        write_file_async(std::string_view path, std::span<const uint8_t> data,
            progress_callback callback)
    {
        std::string path_str(path);
        std::vector<uint8_t> data_copy(data.begin(), data.end());
        auto result = co_await detail::make_async_awaitable(
            [path_str, data_copy = std::move(data_copy), callback]() {
                std::ofstream file(path_str, std::ios::binary);
                if (!file) {
                    return false;
                }

                const size_t chunk_size = 64 * 1024;  // 64KB chunks
                size_t bytes_written = 0;
                const size_t total_size = data_copy.size();

                while (bytes_written < total_size) {
                    size_t to_write = std::min(chunk_size, total_size - bytes_written);
                    file.write(reinterpret_cast<const char*>(data_copy.data() + bytes_written),
                        static_cast<std::streamsize>(to_write));
                    if (!file) {
                        return false;
                    }
                    bytes_written += to_write;
                    if (callback) {
                        callback(bytes_written, total_size);
                    }
                }

                return true;
            });
        co_return result;
    }

#endif

} // namespace container_module::async
