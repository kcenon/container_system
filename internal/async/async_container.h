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

#include <memory>
#include <vector>
#include <string>
#include <span>
#include <thread>
#include <functional>

namespace container_module::async
{
    namespace detail
    {
        /**
         * @brief Awaitable that runs work in a separate thread using std::async
         */
        template<typename T>
        struct async_awaitable
        {
            std::function<T()> work_;
            std::optional<T> result_;
            std::exception_ptr exception_;
            std::thread worker_;
            bool completed_{false};

            explicit async_awaitable(std::function<T()> work)
                : work_(std::move(work)) {}

            async_awaitable(async_awaitable&& other) noexcept
                : work_(std::move(other.work_))
                , result_(std::move(other.result_))
                , exception_(std::move(other.exception_))
                , completed_(other.completed_)
            {
                // Note: worker_ thread handle not moved - it's detached
            }

            [[nodiscard]] bool await_ready() const noexcept
            {
                return false;
            }

            void await_suspend(std::coroutine_handle<> handle)
            {
                worker_ = std::thread([this, handle]() mutable {
                    try {
                        result_.emplace(work_());
                    } catch (...) {
                        exception_ = std::current_exception();
                    }
                    completed_ = true;
                    handle.resume();
                });
                worker_.detach();
            }

            T await_resume()
            {
                if (exception_) {
                    std::rethrow_exception(exception_);
                }
                return std::move(*result_);
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

    private:
        std::shared_ptr<value_container> container_;
    };

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

#endif

} // namespace container_module::async
