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
// async_container tests
// =============================================================================

class AsyncContainerTest : public ::testing::Test {
protected:
    void SetUp() override {
        container_ = std::make_shared<container_module::value_container>();
        container_->set("string_key", std::string("hello"));
        container_->set("int_key", static_cast<int64_t>(42));
        container_->set("double_key", 3.14);
    }

    void TearDown() override {
        container_.reset();
    }

    std::shared_ptr<container_module::value_container> container_;
};

TEST_F(AsyncContainerTest, SerializeAsyncReturnsValidData) {
    async_container async_cont(container_);

    auto serialize_task = async_cont.serialize_async();
    EXPECT_TRUE(serialize_task.valid());

    // Wait for completion
    while (!serialize_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto result = serialize_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    EXPECT_TRUE(result.is_ok());
    EXPECT_FALSE(result.value().empty());
#else
    EXPECT_FALSE(result.empty());
#endif
}

TEST_F(AsyncContainerTest, SerializeStringAsyncReturnsValidData) {
    async_container async_cont(container_);

    auto serialize_task = async_cont.serialize_string_async();
    EXPECT_TRUE(serialize_task.valid());

    while (!serialize_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto result = serialize_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    EXPECT_TRUE(result.is_ok());
    EXPECT_FALSE(result.value().empty());
#else
    EXPECT_FALSE(result.empty());
#endif
}

TEST_F(AsyncContainerTest, DeserializeAsyncRestoresData) {
    // First serialize
    auto serialized = container_->serialize_array();
    EXPECT_FALSE(serialized.empty());

    // Then deserialize async
    auto deserialize_task = async_container::deserialize_async(serialized);
    EXPECT_TRUE(deserialize_task.valid());

    while (!deserialize_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto result = deserialize_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    EXPECT_TRUE(result.is_ok());
    auto restored = result.value();
    EXPECT_NE(restored, nullptr);

    // Verify data integrity
    auto string_val = restored->get_value("string_key");
    ASSERT_TRUE(string_val.has_value());
    auto* str_ptr = std::get_if<std::string>(&string_val->data);
    ASSERT_NE(str_ptr, nullptr);
    EXPECT_EQ(*str_ptr, "hello");

    auto int_val = restored->get_value("int_key");
    ASSERT_TRUE(int_val.has_value());
    auto* int_ptr = std::get_if<int64_t>(&int_val->data);
    ASSERT_NE(int_ptr, nullptr);
    EXPECT_EQ(*int_ptr, 42);
#else
    EXPECT_NE(result, nullptr);
#endif
}

TEST_F(AsyncContainerTest, DeserializeStringAsyncRestoresData) {
    // First serialize
    auto serialized = container_->serialize();
    EXPECT_FALSE(serialized.empty());

    // Then deserialize async
    auto deserialize_task = async_container::deserialize_string_async(serialized);
    EXPECT_TRUE(deserialize_task.valid());

    while (!deserialize_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto result = deserialize_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    EXPECT_TRUE(result.is_ok());
    auto restored = result.value();
    EXPECT_NE(restored, nullptr);
#else
    EXPECT_NE(result, nullptr);
#endif
}

TEST_F(AsyncContainerTest, AsyncContainerSetAndGet) {
    async_container async_cont;

    async_cont.set("name", std::string("test"))
              .set("count", static_cast<int64_t>(100));

    EXPECT_TRUE(async_cont.contains("name"));
    EXPECT_TRUE(async_cont.contains("count"));

    auto name = async_cont.get<std::string>("name");
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(*name, "test");

    auto count = async_cont.get<int64_t>("count");
    ASSERT_TRUE(count.has_value());
    EXPECT_EQ(*count, 100);
}

TEST_F(AsyncContainerTest, AsyncContainerMoveSemantics) {
    async_container async_cont1(container_);
    EXPECT_NE(async_cont1.get_container(), nullptr);

    async_container async_cont2 = std::move(async_cont1);
    EXPECT_NE(async_cont2.get_container(), nullptr);
    EXPECT_TRUE(async_cont2.contains("string_key"));
}

TEST_F(AsyncContainerTest, RoundTripSerializationAsync) {
    async_container async_cont(container_);

    // Serialize async
    auto serialize_task = async_cont.serialize_async();
    while (!serialize_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto serialize_result = serialize_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    ASSERT_TRUE(serialize_result.is_ok());
    auto serialized = serialize_result.value();
#else
    auto serialized = serialize_result;
#endif

    // Deserialize async
    auto deserialize_task = async_container::deserialize_async(serialized);
    while (!deserialize_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto deserialize_result = deserialize_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    ASSERT_TRUE(deserialize_result.is_ok());
    auto restored = deserialize_result.value();
#else
    auto restored = deserialize_result;
#endif

    EXPECT_NE(restored, nullptr);

    // Verify all values survived round-trip
    auto string_val = restored->get_value("string_key");
    ASSERT_TRUE(string_val.has_value());
    auto* str_ptr = std::get_if<std::string>(&string_val->data);
    ASSERT_NE(str_ptr, nullptr);
    EXPECT_EQ(*str_ptr, "hello");
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
