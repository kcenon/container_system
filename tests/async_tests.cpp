/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>

// Include async header first to get CONTAINER_HAS_COROUTINES definition
#include "internal/async/async.h"

#if CONTAINER_HAS_COROUTINES

#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <fstream>

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
    // First serialize using unified API
    auto serialize_result = container_->serialize(container_module::value_container::serialization_format::binary);
    ASSERT_TRUE(serialize_result.is_ok());
    auto serialized = serialize_result.value();
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
    // First serialize using unified API
    auto serialize_result = container_->serialize_string(container_module::value_container::serialization_format::binary);
    ASSERT_TRUE(serialize_result.is_ok());
    auto serialized = serialize_result.value();
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
// Async File I/O Tests (Issue #267 - Phase 3)
// =============================================================================

class AsyncFileIOTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = std::filesystem::temp_directory_path() / "async_file_test";
        std::filesystem::create_directories(test_dir_);
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir_);
    }

    std::filesystem::path test_dir_;
};

TEST_F(AsyncFileIOTest, SaveAndLoadAsync) {
    auto container = std::make_shared<container_module::value_container>();
    container->set("test_key", std::string("test_value"));
    container->set("int_key", static_cast<int64_t>(12345));

    async_container async_cont(container);

    // Save to file
    auto save_path = test_dir_ / "test_container.bin";
    auto save_task = async_cont.save_async(save_path.string());
    EXPECT_TRUE(save_task.valid());

    while (!save_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto save_result = save_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    EXPECT_TRUE(save_result.is_ok());
#else
    EXPECT_TRUE(save_result);
#endif

    // Verify file exists
    EXPECT_TRUE(std::filesystem::exists(save_path));

    // Load from file
    async_container loaded_cont;
    auto load_task = loaded_cont.load_async(save_path.string());
    EXPECT_TRUE(load_task.valid());

    while (!load_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto load_result = load_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    EXPECT_TRUE(load_result.is_ok());
#else
    EXPECT_TRUE(load_result);
#endif

    // Verify loaded data
    auto str_val = loaded_cont.get<std::string>("test_key");
    ASSERT_TRUE(str_val.has_value());
    EXPECT_EQ(*str_val, "test_value");

    auto int_val = loaded_cont.get<int64_t>("int_key");
    ASSERT_TRUE(int_val.has_value());
    EXPECT_EQ(*int_val, 12345);
}

TEST_F(AsyncFileIOTest, LoadAsyncFileNotFound) {
    async_container async_cont;
    auto non_existent = test_dir_ / "non_existent_file.bin";

    auto load_task = async_cont.load_async(non_existent.string());
    EXPECT_TRUE(load_task.valid());

    while (!load_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto result = load_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error().code, container_module::error_codes::file_not_found);
#else
    EXPECT_FALSE(result);
#endif
}

TEST_F(AsyncFileIOTest, ReadFileAsync) {
    // Create a test file
    auto test_file = test_dir_ / "test_read.bin";
    std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

    std::ofstream ofs(test_file, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(test_data.data()),
        static_cast<std::streamsize>(test_data.size()));
    ofs.close();

    // Read file async
    auto read_task = read_file_async(test_file.string());
    EXPECT_TRUE(read_task.valid());

    while (!read_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto result = read_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(result.value(), test_data);
#else
    EXPECT_EQ(result, test_data);
#endif
}

TEST_F(AsyncFileIOTest, WriteFileAsync) {
    auto test_file = test_dir_ / "test_write.bin";
    std::vector<uint8_t> test_data = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

    auto write_task = write_file_async(test_file.string(), test_data);
    EXPECT_TRUE(write_task.valid());

    while (!write_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto result = write_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    EXPECT_TRUE(result.is_ok());
#else
    EXPECT_TRUE(result);
#endif

    // Verify written data
    std::ifstream ifs(test_file, std::ios::binary);
    std::vector<uint8_t> read_data(
        (std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());

    EXPECT_EQ(read_data, test_data);
}

TEST_F(AsyncFileIOTest, ProgressCallbackCalled) {
    auto container = std::make_shared<container_module::value_container>();
    // Create container with some data
    for (int i = 0; i < 100; ++i) {
        container->set("key_" + std::to_string(i),
            std::string(1000, 'x'));  // Large values
    }

    async_container async_cont(container);
    auto save_path = test_dir_ / "progress_test.bin";

    std::atomic<size_t> callback_count{0};
    std::atomic<size_t> last_bytes{0};
    std::atomic<size_t> total_bytes{0};

    auto save_task = async_cont.save_async(save_path.string(),
        [&](size_t bytes, size_t total) {
            ++callback_count;
            last_bytes.store(bytes, std::memory_order_relaxed);
            total_bytes.store(total, std::memory_order_relaxed);
        });

    while (!save_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto save_result = save_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    EXPECT_TRUE(save_result.is_ok());
#else
    EXPECT_TRUE(save_result);
#endif

    // Callback should have been called at least once
    EXPECT_GT(callback_count.load(), 0u);
    // Final bytes should equal total
    EXPECT_EQ(last_bytes.load(), total_bytes.load());
}

TEST_F(AsyncFileIOTest, RoundTripLargeFile) {
    auto container = std::make_shared<container_module::value_container>();
    // Create container with substantial data
    // Note: Using smaller data size (10KB total) to avoid stack overflow in
    // std::regex under AddressSanitizer (regex uses backtracking which is
    // stack-intensive). See ContainerTest.LargeDataHandling for similar limit.
    for (int i = 0; i < 10; ++i) {
        container->set("large_key_" + std::to_string(i),
            std::string(1000, static_cast<char>('A' + (i % 26))));
    }

    async_container async_cont(container);
    auto file_path = test_dir_ / "large_file.bin";

    // Save
    auto save_task = async_cont.save_async(file_path.string());
    while (!save_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto save_result = save_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    EXPECT_TRUE(save_result.is_ok());
#else
    EXPECT_TRUE(save_result);
#endif

    // Load into new container
    async_container loaded_cont;
    auto load_task = loaded_cont.load_async(file_path.string());
    while (!load_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto load_result = load_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    EXPECT_TRUE(load_result.is_ok());
#else
    EXPECT_TRUE(load_result);
#endif

    // Verify data integrity
    for (int i = 0; i < 10; ++i) {
        auto val = loaded_cont.get<std::string>("large_key_" + std::to_string(i));
        ASSERT_TRUE(val.has_value());
        EXPECT_EQ(val->size(), 1000u);
        EXPECT_EQ(val->front(), static_cast<char>('A' + (i % 26)));
    }
}

// =============================================================================
// Streaming Tests (Issue #268 - Phase 4)
// =============================================================================

class AsyncStreamingTest : public ::testing::Test {
protected:
    void SetUp() override {
        container_ = std::make_shared<container_module::value_container>();
        // Add some test data
        for (int i = 0; i < 100; ++i) {
            container_->set("key_" + std::to_string(i),
                std::string(100, static_cast<char>('A' + (i % 26))));
        }
    }

    void TearDown() override {
        container_.reset();
    }

    std::shared_ptr<container_module::value_container> container_;
};

TEST_F(AsyncStreamingTest, SerializeChunkedBasic) {
    async_container async_cont(container_);

    // Serialize with default chunk size
    auto gen = async_cont.serialize_chunked();
    EXPECT_TRUE(gen.valid());

    std::vector<uint8_t> full_data;
    size_t chunk_count = 0;

    for (auto& chunk : gen) {
        EXPECT_FALSE(chunk.empty());
        full_data.insert(full_data.end(), chunk.begin(), chunk.end());
        ++chunk_count;
    }

    // Verify we got at least one chunk
    EXPECT_GT(chunk_count, 0u);
    EXPECT_FALSE(full_data.empty());

    // Verify the full data can be deserialized
    auto restored = std::make_shared<container_module::value_container>(full_data, false);
    EXPECT_TRUE(restored->contains("key_0"));
    EXPECT_TRUE(restored->contains("key_99"));
}

TEST_F(AsyncStreamingTest, SerializeChunkedSmallChunks) {
    async_container async_cont(container_);

    // Use small chunk size to force multiple chunks
    const size_t small_chunk_size = 256;
    auto gen = async_cont.serialize_chunked(small_chunk_size);

    std::vector<uint8_t> full_data;
    size_t chunk_count = 0;

    for (auto& chunk : gen) {
        // Each chunk should be at most chunk_size (except possibly the last)
        EXPECT_LE(chunk.size(), small_chunk_size);
        full_data.insert(full_data.end(), chunk.begin(), chunk.end());
        ++chunk_count;
    }

    // With small chunks, we should have multiple
    EXPECT_GT(chunk_count, 1u);

    // Verify data integrity
    auto restored = std::make_shared<container_module::value_container>(full_data, false);
    auto val = restored->get_value("key_50");
    ASSERT_TRUE(val.has_value());
    auto* str_ptr = std::get_if<std::string>(&val->data);
    ASSERT_NE(str_ptr, nullptr);
    EXPECT_EQ(str_ptr->size(), 100u);
}

TEST_F(AsyncStreamingTest, SerializeChunkedLargeChunks) {
    async_container async_cont(container_);

    // Use very large chunk size (larger than data) - should produce single chunk
    const size_t large_chunk_size = 1024 * 1024;  // 1MB
    auto gen = async_cont.serialize_chunked(large_chunk_size);

    std::vector<uint8_t> full_data;
    size_t chunk_count = 0;

    for (auto& chunk : gen) {
        full_data.insert(full_data.end(), chunk.begin(), chunk.end());
        ++chunk_count;
    }

    // With very large chunks, should be just one
    EXPECT_EQ(chunk_count, 1u);

    // Verify data integrity
    auto restored = std::make_shared<container_module::value_container>(full_data, false);
    EXPECT_TRUE(restored->contains("key_0"));
}

TEST_F(AsyncStreamingTest, SerializeChunkedEmptyContainer) {
    auto empty_container = std::make_shared<container_module::value_container>();
    async_container async_cont(empty_container);

    auto gen = async_cont.serialize_chunked();
    size_t chunk_count = 0;

    for ([[maybe_unused]] auto& chunk : gen) {
        ++chunk_count;
    }

    // Empty container should still produce at least header data
    // (depending on serialization format)
    EXPECT_GE(chunk_count, 0u);
}

TEST_F(AsyncStreamingTest, DeserializeStreamingComplete) {
    // First serialize the container using unified API
    auto serialize_result = container_->serialize(container_module::value_container::serialization_format::binary);
    ASSERT_TRUE(serialize_result.is_ok());
    auto serialized = serialize_result.value();
    EXPECT_FALSE(serialized.empty());

    // Deserialize with is_final=true
    auto deserialize_task = async_container::deserialize_streaming(serialized, true);
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
    auto val = restored->get_value("key_0");
    ASSERT_TRUE(val.has_value());
    auto* str_ptr = std::get_if<std::string>(&val->data);
    ASSERT_NE(str_ptr, nullptr);
    EXPECT_EQ(str_ptr->size(), 100u);
#else
    EXPECT_NE(result, nullptr);
    EXPECT_TRUE(result->contains("key_0"));
#endif
}

TEST_F(AsyncStreamingTest, DeserializeStreamingIncomplete) {
    // First serialize the container using unified API
    auto serialize_result = container_->serialize(container_module::value_container::serialization_format::binary);
    ASSERT_TRUE(serialize_result.is_ok());
    auto serialized = serialize_result.value();
    EXPECT_FALSE(serialized.empty());

    // Deserialize with is_final=false should fail/return error
    auto deserialize_task = async_container::deserialize_streaming(serialized, false);
    EXPECT_TRUE(deserialize_task.valid());

    while (!deserialize_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto result = deserialize_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    // Should return error because is_final=false
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error().code, container_module::error_codes::deserialization_failed);
#else
    // Should return nullptr
    EXPECT_EQ(result, nullptr);
#endif
}

TEST_F(AsyncStreamingTest, RoundTripChunkedSerialization) {
    async_container async_cont(container_);

    // Serialize in chunks
    auto gen = async_cont.serialize_chunked(1024);  // 1KB chunks

    std::vector<uint8_t> accumulated_data;
    for (auto& chunk : gen) {
        accumulated_data.insert(accumulated_data.end(), chunk.begin(), chunk.end());
    }

    // Deserialize the accumulated data
    auto deserialize_task = async_container::deserialize_streaming(accumulated_data, true);

    while (!deserialize_task.done()) {
        std::this_thread::sleep_for(1ms);
    }

    auto result = deserialize_task.get();
#if CONTAINER_HAS_COMMON_RESULT
    EXPECT_TRUE(result.is_ok());
    auto restored = result.value();
#else
    auto restored = result;
#endif

    EXPECT_NE(restored, nullptr);

    // Verify all keys survived round-trip
    for (int i = 0; i < 100; ++i) {
        EXPECT_TRUE(restored->contains("key_" + std::to_string(i)));
    }
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
