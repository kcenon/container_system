/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>

// Include async header first to get CONTAINER_HAS_COROUTINES definition
#include "container/internal/async/async.h"

#if CONTAINER_HAS_COROUTINES

#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <string>
#include <stdexcept>

using namespace container_module::async;
using namespace std::chrono_literals;

class AsyncTaskTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test data
    }

    void TearDown() override {
        // Cleanup
    }
};

// =============================================================================
// Basic task<T> tests
// =============================================================================

TEST_F(AsyncTaskTest, TaskReturnsValue) {
    auto simple_task = []() -> task<int> {
        co_return 42;
    };

    auto t = simple_task();
    EXPECT_TRUE(t.valid());
    EXPECT_TRUE(t.done());
    EXPECT_EQ(t.get(), 42);
}

TEST_F(AsyncTaskTest, TaskReturnsString) {
    auto string_task = []() -> task<std::string> {
        co_return "hello world";
    };

    auto t = string_task();
    EXPECT_TRUE(t.valid());
    EXPECT_TRUE(t.done());
    EXPECT_EQ(t.get(), "hello world");
}

TEST_F(AsyncTaskTest, TaskVoid) {
    int counter = 0;

    auto void_task = [&counter]() -> task<void> {
        counter = 42;
        co_return;
    };

    auto t = void_task();
    EXPECT_TRUE(t.valid());
    EXPECT_TRUE(t.done());
    EXPECT_EQ(counter, 42);
}

TEST_F(AsyncTaskTest, TaskChaining) {
    auto inner_task = []() -> task<int> {
        co_return 21;
    };

    auto outer_task = [&inner_task]() -> task<int> {
        int value = co_await inner_task();
        co_return value * 2;
    };

    auto t = outer_task();
    EXPECT_TRUE(t.done());
    EXPECT_EQ(t.get(), 42);
}

TEST_F(AsyncTaskTest, TaskExceptionPropagation) {
    auto throwing_task = []() -> task<int> {
        throw std::runtime_error("test error");
        co_return 0;  // Never reached
    };

    auto t = throwing_task();
    EXPECT_TRUE(t.done());
    EXPECT_THROW(t.get(), std::runtime_error);
}

TEST_F(AsyncTaskTest, TaskMoveSemantics) {
    auto simple_task = []() -> task<int> {
        co_return 42;
    };

    auto t1 = simple_task();
    EXPECT_TRUE(t1.valid());

    auto t2 = std::move(t1);
    EXPECT_FALSE(t1.valid());
    EXPECT_TRUE(t2.valid());
    EXPECT_EQ(t2.get(), 42);
}

TEST_F(AsyncTaskTest, TaskMoveAssignment) {
    auto task1 = []() -> task<int> { co_return 1; };
    auto task2 = []() -> task<int> { co_return 2; };

    auto t = task1();
    EXPECT_EQ(t.get(), 1);

    t = task2();
    EXPECT_EQ(t.get(), 2);
}

TEST_F(AsyncTaskTest, MakeReadyTask) {
    auto t = make_ready_task(42);
    EXPECT_TRUE(t.done());
    EXPECT_EQ(t.get(), 42);
}

TEST_F(AsyncTaskTest, MakeReadyTaskVoid) {
    auto t = make_ready_task();
    EXPECT_TRUE(t.done());
    // Just verify it doesn't throw
    t.get();
}

TEST_F(AsyncTaskTest, MakeExceptionalTask) {
    auto ex = std::make_exception_ptr(std::runtime_error("test"));
    auto t = make_exceptional_task<int>(ex);
    EXPECT_TRUE(t.done());
    EXPECT_THROW(t.get(), std::runtime_error);
}

TEST_F(AsyncTaskTest, NestedAwait) {
    auto level3 = []() -> task<int> {
        co_return 10;
    };

    auto level2 = [&level3]() -> task<int> {
        int v = co_await level3();
        co_return v + 20;
    };

    auto level1 = [&level2]() -> task<int> {
        int v = co_await level2();
        co_return v + 30;
    };

    auto t = level1();
    EXPECT_EQ(t.get(), 60);
}

// =============================================================================
// Generator tests
// =============================================================================

class AsyncGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(AsyncGeneratorTest, SimpleGenerator) {
    auto range = [](int start, int end) -> generator<int> {
        for (int i = start; i < end; ++i) {
            co_yield i;
        }
    };

    auto gen = range(0, 5);
    std::vector<int> results;

    for (int value : gen) {
        results.push_back(value);
    }

    EXPECT_EQ(results, (std::vector<int>{0, 1, 2, 3, 4}));
}

TEST_F(AsyncGeneratorTest, EmptyGenerator) {
    auto empty = []() -> generator<int> {
        co_return;
    };

    auto gen = empty();
    std::vector<int> results;

    for (int value : gen) {
        results.push_back(value);
    }

    EXPECT_TRUE(results.empty());
}

TEST_F(AsyncGeneratorTest, SingleValueGenerator) {
    auto single = []() -> generator<std::string> {
        co_yield "hello";
    };

    auto gen = single();
    std::vector<std::string> results;

    for (auto& value : gen) {
        results.push_back(value);
    }

    EXPECT_EQ(results, (std::vector<std::string>{"hello"}));
}

TEST_F(AsyncGeneratorTest, GeneratorMoveSemantics) {
    auto range = [](int n) -> generator<int> {
        for (int i = 0; i < n; ++i) {
            co_yield i;
        }
    };

    auto gen1 = range(3);
    EXPECT_TRUE(gen1.valid());

    auto gen2 = std::move(gen1);
    EXPECT_FALSE(gen1.valid());
    EXPECT_TRUE(gen2.valid());

    std::vector<int> results;
    for (int value : gen2) {
        results.push_back(value);
    }
    EXPECT_EQ(results, (std::vector<int>{0, 1, 2}));
}

TEST_F(AsyncGeneratorTest, GeneratorExceptionPropagation) {
    auto throwing_gen = []() -> generator<int> {
        co_yield 1;
        co_yield 2;
        throw std::runtime_error("generator error");
        co_yield 3;  // Never reached
    };

    auto gen = throwing_gen();
    std::vector<int> results;

    EXPECT_THROW({
        for (int value : gen) {
            results.push_back(value);
        }
    }, std::runtime_error);

    EXPECT_EQ(results, (std::vector<int>{1, 2}));
}

TEST_F(AsyncGeneratorTest, GeneratorLargeSequence) {
    auto large_range = [](size_t n) -> generator<size_t> {
        for (size_t i = 0; i < n; ++i) {
            co_yield i;
        }
    };

    const size_t count = 10000;
    auto gen = large_range(count);

    size_t sum = 0;
    size_t actual_count = 0;
    for (size_t value : gen) {
        sum += value;
        ++actual_count;
    }

    EXPECT_EQ(actual_count, count);
    EXPECT_EQ(sum, (count - 1) * count / 2);  // Sum of 0 to n-1
}

TEST_F(AsyncGeneratorTest, TakeGenerator) {
    auto infinite = []() -> generator<int> {
        int i = 0;
        while (true) {
            co_yield i++;
        }
    };

    auto limited = take(infinite(), 5);
    std::vector<int> results;

    for (int value : limited) {
        results.push_back(value);
    }

    EXPECT_EQ(results, (std::vector<int>{0, 1, 2, 3, 4}));
}

// =============================================================================
// Feature detection test
// =============================================================================

TEST(AsyncFeatureTest, CoroutinesSupportDetected) {
    EXPECT_TRUE(has_coroutine_support);
}

#else

// Placeholder test when coroutines are not available
TEST(AsyncFeatureTest, CoroutinesNotAvailable) {
    EXPECT_FALSE(container_module::async::has_coroutine_support);
}

#endif // CONTAINER_HAS_COROUTINES
