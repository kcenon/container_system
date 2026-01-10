/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, kcenon
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
 * @file internal/async/thread_pool_executor.h
 * @brief Thread pool executor integration for async operations
 *
 * Provides integration between container_system async operations and
 * common_system's thread pool executor interface.
 *
 * @code
 * // Example usage with custom executor:
 * auto executor = get_custom_executor();
 * async_container cont(container);
 * cont.set_executor(executor);
 *
 * auto result = co_await cont.serialize_async();
 * @endcode
 *
 * @see container_module::async::executor_awaitable
 * @see kcenon::common::interfaces::IExecutor
 */

#pragma once

#include "task.h"

#include <atomic>
#include <coroutine>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <thread>
#include <type_traits>

#ifdef KCENON_HAS_COMMON_SYSTEM
#include <kcenon/common/interfaces/executor_interface.h>
#include <kcenon/common/interfaces/thread_pool_interface.h>
#endif

namespace container_module::async
{
    /**
     * @brief Executor type for async operations
     *
     * When common_system is available, uses IExecutor interface.
     * Otherwise, falls back to simple std::thread-based execution.
     */
#ifdef KCENON_HAS_COMMON_SYSTEM
    using executor_ptr = std::shared_ptr<kcenon::common::interfaces::IExecutor>;
#else
    using executor_ptr = std::nullptr_t;
#endif

    namespace detail
    {
        /**
         * @brief Shared state for executor-based async operations
         *
         * This state is shared between the awaitable and the executor
         * using shared_ptr, ensuring thread-safe access.
         */
        template<typename T>
        struct executor_state
        {
            std::function<T()> work_;
            std::optional<T> result_;
            std::exception_ptr exception_;
            std::atomic<bool> ready_{false};
            std::coroutine_handle<> continuation_;

            explicit executor_state(std::function<T()> work)
                : work_(std::move(work)) {}

            executor_state(const executor_state&) = delete;
            executor_state& operator=(const executor_state&) = delete;
            executor_state(executor_state&&) = delete;
            executor_state& operator=(executor_state&&) = delete;
        };

        /**
         * @brief Awaitable that runs work using an executor or fallback thread
         *
         * When an executor is provided, submits work to the executor.
         * Otherwise, falls back to std::thread-based execution.
         */
        template<typename T>
        struct executor_awaitable
        {
            std::shared_ptr<executor_state<T>> state_;
            executor_ptr executor_;

            executor_awaitable(std::function<T()> work, executor_ptr executor)
                : state_(std::make_shared<executor_state<T>>(std::move(work)))
                , executor_(std::move(executor))
            {
            }

            executor_awaitable(executor_awaitable&&) noexcept = default;
            executor_awaitable& operator=(executor_awaitable&&) noexcept = default;

            executor_awaitable(const executor_awaitable&) = delete;
            executor_awaitable& operator=(const executor_awaitable&) = delete;

            [[nodiscard]] bool await_ready() const noexcept
            {
                return false;
            }

            void await_suspend(std::coroutine_handle<> handle)
            {
                auto state = state_;
                state->continuation_ = handle;

#ifdef KCENON_HAS_COMMON_SYSTEM
                if (executor_ && executor_->is_running())
                {
                    // Create a job that executes the work
                    class work_job : public kcenon::common::interfaces::IJob
                    {
                    public:
                        work_job(std::shared_ptr<executor_state<T>> s)
                            : state_(std::move(s)) {}

                        kcenon::common::VoidResult execute() override
                        {
                            try {
                                state_->result_.emplace(state_->work_());
                            } catch (...) {
                                state_->exception_ = std::current_exception();
                            }
                            state_->ready_.store(true, std::memory_order_release);
                            if (state_->continuation_) {
                                state_->continuation_.resume();
                            }
                            return kcenon::common::ok();
                        }

                        std::string get_name() const override {
                            return "async_container_work";
                        }

                    private:
                        std::shared_ptr<executor_state<T>> state_;
                    };

                    auto job = std::make_unique<work_job>(state);
                    auto result = executor_->execute(std::move(job));
                    if (!result.is_ok()) {
                        // Fallback to thread if executor fails
                        run_in_thread(state, handle);
                    }
                    return;
                }
#endif
                // Fallback: run in a detached thread
                run_in_thread(state, handle);
            }

            T await_resume()
            {
                while (!state_->ready_.load(std::memory_order_acquire)) {
                    // Spin until ready
                }
                if (state_->exception_) {
                    std::rethrow_exception(state_->exception_);
                }
                return std::move(*state_->result_);
            }

        private:
            static void run_in_thread(
                std::shared_ptr<executor_state<T>> state,
                std::coroutine_handle<> handle)
            {
                std::thread([state, handle]() mutable {
                    try {
                        state->result_.emplace(state->work_());
                    } catch (...) {
                        state->exception_ = std::current_exception();
                    }
                    state->ready_.store(true, std::memory_order_release);
                    handle.resume();
                }).detach();
            }
        };

        /**
         * @brief Helper to create executor awaitable
         */
        template<typename F>
        auto make_executor_awaitable(F&& func, executor_ptr executor)
            -> executor_awaitable<std::invoke_result_t<F>>
        {
            using result_type = std::invoke_result_t<F>;
            return executor_awaitable<result_type>(
                std::forward<F>(func), std::move(executor));
        }

    } // namespace detail

    /**
     * @brief Global executor for async operations
     *
     * When set, async operations will use this executor instead of
     * creating individual threads. This improves performance by
     * reusing threads from a pool.
     */
    class async_executor_context
    {
    public:
        /**
         * @brief Get the singleton instance
         */
        static async_executor_context& instance()
        {
            static async_executor_context instance;
            return instance;
        }

        /**
         * @brief Set the global executor
         * @param executor The executor to use for async operations
         */
        void set_executor(executor_ptr executor)
        {
            executor_ = std::move(executor);
        }

        /**
         * @brief Get the global executor
         * @return The current executor, or nullptr if not set
         */
        [[nodiscard]] executor_ptr get_executor() const
        {
            return executor_;
        }

        /**
         * @brief Check if an executor is configured
         */
        [[nodiscard]] bool has_executor() const
        {
#ifdef KCENON_HAS_COMMON_SYSTEM
            return executor_ != nullptr;
#else
            return false;
#endif
        }

        /**
         * @brief Clear the global executor
         */
        void clear_executor()
        {
#ifdef KCENON_HAS_COMMON_SYSTEM
            executor_ = nullptr;
#endif
        }

    private:
        async_executor_context() = default;
        executor_ptr executor_{};
    };

    /**
     * @brief RAII guard for setting executor context
     *
     * Sets the executor on construction and restores the previous
     * executor on destruction.
     *
     * @code
     * {
     *     executor_context_guard guard(my_executor);
     *     // All async operations in this scope use my_executor
     *     auto result = co_await container.serialize_async();
     * }
     * // Previous executor restored
     * @endcode
     */
    class executor_context_guard
    {
    public:
        explicit executor_context_guard(executor_ptr executor)
            : previous_(async_executor_context::instance().get_executor())
        {
            async_executor_context::instance().set_executor(std::move(executor));
        }

        ~executor_context_guard()
        {
            async_executor_context::instance().set_executor(std::move(previous_));
        }

        executor_context_guard(const executor_context_guard&) = delete;
        executor_context_guard& operator=(const executor_context_guard&) = delete;

    private:
        executor_ptr previous_;
    };

} // namespace container_module::async
