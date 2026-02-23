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

#include <gtest/gtest.h>

#include "internal/async/async.h"

#if CONTAINER_HAS_COROUTINES

#include <thread>
#include <atomic>
#include <chrono>
#include <stdexcept>

using namespace kcenon::container::async;
using namespace std::chrono_literals;

class ThreadPoolExecutorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure clean state before each test
        async_executor_context::instance().clear_executor();
    }

    void TearDown() override {
        async_executor_context::instance().clear_executor();
    }
};

// =============================================================================
// async_executor_context singleton tests
// =============================================================================

TEST_F(ThreadPoolExecutorTest, SingletonReturnsSameInstance) {
    auto& inst1 = async_executor_context::instance();
    auto& inst2 = async_executor_context::instance();
    EXPECT_EQ(&inst1, &inst2);
}

TEST_F(ThreadPoolExecutorTest, InitialStateHasNoExecutor) {
    EXPECT_FALSE(async_executor_context::instance().has_executor());
}

TEST_F(ThreadPoolExecutorTest, GetExecutorReturnsNullWhenNotSet) {
    auto executor = async_executor_context::instance().get_executor();
#ifdef KCENON_HAS_COMMON_SYSTEM
    EXPECT_EQ(executor, nullptr);
#else
    // Without common_system, executor_ptr is std::nullptr_t
    (void)executor;
    SUCCEED();
#endif
}

TEST_F(ThreadPoolExecutorTest, ClearExecutorIsIdempotent) {
    async_executor_context::instance().clear_executor();
    EXPECT_FALSE(async_executor_context::instance().has_executor());

    async_executor_context::instance().clear_executor();
    EXPECT_FALSE(async_executor_context::instance().has_executor());
}

// =============================================================================
// executor_context_guard RAII tests
// =============================================================================

TEST_F(ThreadPoolExecutorTest, GuardRestoresPreviousExecutor) {
    EXPECT_FALSE(async_executor_context::instance().has_executor());

    {
        executor_context_guard guard(executor_ptr{});
        // Within guard scope, executor is set (even if to nullptr_t)
        // On destruction, previous state should be restored
    }

    EXPECT_FALSE(async_executor_context::instance().has_executor());
}

TEST_F(ThreadPoolExecutorTest, NestedGuardsRestoreCorrectly) {
    EXPECT_FALSE(async_executor_context::instance().has_executor());

    {
        executor_context_guard outer(executor_ptr{});
        {
            executor_context_guard inner(executor_ptr{});
        }
        // After inner guard destruction, state from outer guard remains
        EXPECT_FALSE(async_executor_context::instance().has_executor());
    }

    // After outer guard destruction, original state restored
    EXPECT_FALSE(async_executor_context::instance().has_executor());
}

// =============================================================================
// executor_awaitable tests (fallback thread path)
// =============================================================================

TEST_F(ThreadPoolExecutorTest, AwaitableAlwaysNotReady) {
    auto work = []() -> int { return 42; };
    detail::executor_awaitable<int> awaitable(work, executor_ptr{});
    EXPECT_FALSE(awaitable.await_ready());
}

TEST_F(ThreadPoolExecutorTest, FallbackThreadExecutesWork) {
    auto coroutine_task = []() -> task<int> {
        auto result = co_await detail::make_executor_awaitable(
            []() -> int { return 42; }, executor_ptr{});
        co_return result;
    };

    auto t = coroutine_task();
    // Wait for async completion with timeout
    auto deadline = std::chrono::steady_clock::now() + 5s;
    while (!t.done() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(10ms);
    }

    ASSERT_TRUE(t.done());
    EXPECT_EQ(t.get(), 42);
}

TEST_F(ThreadPoolExecutorTest, FallbackThreadHandlesStringResult) {
    auto coroutine_task = []() -> task<std::string> {
        auto result = co_await detail::make_executor_awaitable(
            []() -> std::string { return "hello from thread"; },
            executor_ptr{});
        co_return result;
    };

    auto t = coroutine_task();
    auto deadline = std::chrono::steady_clock::now() + 5s;
    while (!t.done() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(10ms);
    }

    ASSERT_TRUE(t.done());
    EXPECT_EQ(t.get(), "hello from thread");
}

TEST_F(ThreadPoolExecutorTest, FallbackThreadPropagatesException) {
    auto coroutine_task = []() -> task<int> {
        auto result = co_await detail::make_executor_awaitable(
            []() -> int { throw std::runtime_error("test error"); },
            executor_ptr{});
        co_return result;
    };

    auto t = coroutine_task();
    auto deadline = std::chrono::steady_clock::now() + 5s;
    while (!t.done() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(10ms);
    }

    ASSERT_TRUE(t.done());
    EXPECT_THROW(t.get(), std::runtime_error);
}

TEST_F(ThreadPoolExecutorTest, FallbackThreadRunsOnDifferentThread) {
    auto main_thread_id = std::this_thread::get_id();

    auto coroutine_task = [&main_thread_id]() -> task<bool> {
        auto runs_on_different = co_await detail::make_executor_awaitable(
            [&main_thread_id]() -> bool {
                return std::this_thread::get_id() != main_thread_id;
            },
            executor_ptr{});
        co_return runs_on_different;
    };

    auto t = coroutine_task();
    auto deadline = std::chrono::steady_clock::now() + 5s;
    while (!t.done() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(10ms);
    }

    ASSERT_TRUE(t.done());
    EXPECT_TRUE(t.get());
}

TEST_F(ThreadPoolExecutorTest, MakeExecutorAwaitableDeducesType) {
    // Verify that make_executor_awaitable correctly deduces the return type
    auto awaitable_int = detail::make_executor_awaitable(
        []() -> int { return 0; }, executor_ptr{});
    auto awaitable_str = detail::make_executor_awaitable(
        []() -> std::string { return ""; }, executor_ptr{});

    EXPECT_FALSE(awaitable_int.await_ready());
    EXPECT_FALSE(awaitable_str.await_ready());
}

#else // !CONTAINER_HAS_COROUTINES

TEST(ThreadPoolExecutorTest, CoroutinesNotAvailable) {
    GTEST_SKIP() << "C++20 coroutines not available";
}

#endif // CONTAINER_HAS_COROUTINES
