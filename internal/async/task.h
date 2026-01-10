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

/**
 * @file internal/async/task.h
 * @brief C++20 coroutine task type for async operations
 *
 * Provides a lightweight coroutine task type that enables async/await style
 * programming for container operations.
 *
 * @code
 * // Example usage:
 * task<int> compute_async() {
 *     co_return 42;
 * }
 *
 * task<void> process() {
 *     int result = co_await compute_async();
 * }
 * @endcode
 *
 * @see container_module::async::task
 */

#pragma once

#include <coroutine>
#include <exception>
#include <optional>
#include <utility>
#include <type_traits>
#include <variant>
#include <atomic>

namespace container_module::async
{
    /**
     * @brief Forward declaration of task
     */
    template<typename T>
    class task;

    namespace detail
    {
        /**
         * @brief Base promise type with common functionality
         */
        struct promise_base
        {
            std::exception_ptr exception_;
            std::coroutine_handle<> continuation_;
            std::atomic<bool> completed_{false};

            /**
             * @brief Never suspend at start - start executing immediately
             */
            [[nodiscard]] std::suspend_never initial_suspend() noexcept
            {
                return {};
            }

            /**
             * @brief Handle unhandled exceptions
             */
            void unhandled_exception() noexcept
            {
                exception_ = std::current_exception();
            }

            /**
             * @brief Final awaiter that resumes the continuation
             */
            struct final_awaiter
            {
                [[nodiscard]] bool await_ready() const noexcept
                {
                    return false;
                }

                template<typename Promise>
                std::coroutine_handle<> await_suspend(
                    std::coroutine_handle<Promise> handle) noexcept
                {
                    auto& promise = handle.promise();
                    // Read continuation BEFORE marking as complete to avoid
                    // data race with destructor. Once completed_ is true,
                    // the task owner may destroy the coroutine frame.
                    auto continuation = promise.continuation_;
                    // Mark as completed with release semantics to ensure
                    // all prior writes are visible to threads checking done()
                    promise.completed_.store(true, std::memory_order_release);
                    if (continuation)
                    {
                        return continuation;
                    }
                    return std::noop_coroutine();
                }

                void await_resume() noexcept {}
            };

            /**
             * @brief Suspend at final point to allow result retrieval
             */
            [[nodiscard]] final_awaiter final_suspend() noexcept
            {
                return {};
            }
        };

        /**
         * @brief Promise type for value-returning tasks
         */
        template<typename T>
        struct promise_type : promise_base
        {
            std::optional<T> result_;

            /**
             * @brief Get return object (the task)
             */
            [[nodiscard]] task<T> get_return_object() noexcept;

            /**
             * @brief Store the return value
             */
            template<typename U>
            void return_value(U&& value)
                requires std::convertible_to<U, T>
            {
                result_.emplace(std::forward<U>(value));
            }

            /**
             * @brief Get the stored result, rethrowing any exception
             */
            [[nodiscard]] T& result() &
            {
                if (exception_)
                {
                    std::rethrow_exception(exception_);
                }
                return *result_;
            }

            /**
             * @brief Get the stored result (rvalue), rethrowing any exception
             */
            [[nodiscard]] T&& result() &&
            {
                if (exception_)
                {
                    std::rethrow_exception(exception_);
                }
                return std::move(*result_);
            }
        };

        /**
         * @brief Promise type for void-returning tasks
         */
        template<>
        struct promise_type<void> : promise_base
        {
            /**
             * @brief Get return object (the task)
             */
            [[nodiscard]] task<void> get_return_object() noexcept;

            /**
             * @brief Handle void return
             */
            void return_void() noexcept {}

            /**
             * @brief Check for exceptions
             */
            void result()
            {
                if (exception_)
                {
                    std::rethrow_exception(exception_);
                }
            }
        };

    } // namespace detail

    /**
     * @brief Coroutine task type for async operations
     *
     * This is a lazy coroutine type that starts executing when awaited.
     * It supports:
     * - Value-returning coroutines (task<T>)
     * - Void-returning coroutines (task<void>)
     * - Exception propagation
     * - Proper RAII resource management
     *
     * Properties:
     * - Move-only (non-copyable)
     * - Lazy execution (starts when awaited)
     * - Symmetric transfer for efficient chaining
     *
     * @tparam T The return type of the coroutine
     */
    template<typename T>
    class task
    {
    public:
        using promise_type = detail::promise_type<T>;
        using handle_type = std::coroutine_handle<promise_type>;

        /**
         * @brief Default constructor - creates empty task
         */
        task() noexcept : handle_(nullptr) {}

        /**
         * @brief Construct from coroutine handle
         */
        explicit task(handle_type handle) noexcept : handle_(handle) {}

        /**
         * @brief Move constructor
         */
        task(task&& other) noexcept : handle_(std::exchange(other.handle_, nullptr)) {}

        /**
         * @brief Move assignment operator
         */
        task& operator=(task&& other) noexcept
        {
            if (this != &other)
            {
                if (handle_)
                {
                    handle_.destroy();
                }
                handle_ = std::exchange(other.handle_, nullptr);
            }
            return *this;
        }

        /**
         * @brief Destructor - destroys the coroutine if owned
         */
        ~task()
        {
            if (handle_)
            {
                handle_.destroy();
            }
        }

        // Non-copyable
        task(const task&) = delete;
        task& operator=(const task&) = delete;

        /**
         * @brief Check if task is valid (has a coroutine)
         */
        [[nodiscard]] bool valid() const noexcept
        {
            return handle_ != nullptr;
        }

        /**
         * @brief Check if task is valid (has a coroutine)
         */
        [[nodiscard]] explicit operator bool() const noexcept
        {
            return valid();
        }

        /**
         * @brief Check if the coroutine is done
         *
         * Uses atomic flag for thread-safe completion checking.
         * This avoids data races when polling from a different thread
         * than the one executing the coroutine.
         */
        [[nodiscard]] bool done() const noexcept
        {
            return handle_ && handle_.promise().completed_.load(std::memory_order_acquire);
        }

        /**
         * @brief Get the result (blocking, for testing)
         *
         * @warning This assumes the coroutine has already completed
         */
        [[nodiscard]] decltype(auto) get() &
        {
            return handle_.promise().result();
        }

        /**
         * @brief Get the result (rvalue, blocking, for testing)
         */
        [[nodiscard]] decltype(auto) get() &&
        {
            return std::move(handle_.promise()).result();
        }

        /**
         * @brief Awaitable interface
         */
        struct awaiter
        {
            handle_type handle_;

            /**
             * @brief Check if the coroutine is already done
             */
            [[nodiscard]] bool await_ready() const noexcept
            {
                return handle_.done();
            }

            /**
             * @brief Suspend and set continuation
             */
            std::coroutine_handle<> await_suspend(
                std::coroutine_handle<> awaiting) noexcept
            {
                handle_.promise().continuation_ = awaiting;
                return handle_;
            }

            /**
             * @brief Get the result when resumed
             */
            [[nodiscard]] decltype(auto) await_resume()
            {
                return handle_.promise().result();
            }
        };

        /**
         * @brief Get awaiter for co_await
         */
        [[nodiscard]] awaiter operator co_await() noexcept
        {
            return awaiter{handle_};
        }

        /**
         * @brief Resume the coroutine (for manual execution)
         */
        void resume() const
        {
            if (handle_ && !handle_.done())
            {
                handle_.resume();
            }
        }

    private:
        handle_type handle_;
    };

    // Implementation of get_return_object for value type
    template<typename T>
    task<T> detail::promise_type<T>::get_return_object() noexcept
    {
        return task<T>{std::coroutine_handle<detail::promise_type<T>>::from_promise(*this)};
    }

    // Implementation of get_return_object for void type
    inline task<void> detail::promise_type<void>::get_return_object() noexcept
    {
        return task<void>{std::coroutine_handle<detail::promise_type<void>>::from_promise(*this)};
    }

    /**
     * @brief Create a task that returns a value immediately
     *
     * @tparam T The value type
     * @param value The value to return
     * @return A task that immediately returns the value
     */
    template<typename T>
    task<std::decay_t<T>> make_ready_task(T&& value)
    {
        co_return std::forward<T>(value);
    }

    /**
     * @brief Create a task that completes immediately with no value
     *
     * @return A task that immediately completes
     */
    inline task<void> make_ready_task()
    {
        co_return;
    }

    /**
     * @brief Create a task that throws an exception
     *
     * @tparam T The return type of the task
     * @param ex The exception pointer to throw
     * @return A task that throws the exception when awaited
     */
    template<typename T>
    task<T> make_exceptional_task(std::exception_ptr ex)
    {
        std::rethrow_exception(ex);
        // Unreachable, but needed for coroutine
        co_return T{};
    }

    /**
     * @brief Specialization for void
     */
    template<>
    inline task<void> make_exceptional_task<void>(std::exception_ptr ex)
    {
        std::rethrow_exception(ex);
        co_return;
    }

} // namespace container_module::async
