// BSD 3-Clause License
//
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * @file storage_policy_tests.cpp
 * @brief Unit tests for storage policy interfaces (Issue #322)
 *
 * Tests cover:
 * - StoragePolicy concept validation
 * - dynamic_storage_policy CRUD operations
 * - indexed_storage_policy CRUD operations
 * - Iterator support
 * - Edge cases and error handling
 */

#include <gtest/gtest.h>
#include "core/storage_policy.h"

#include <string>
#include <algorithm>

using namespace container_module;
using namespace container_module::policy;

// ============================================================================
// Concept Verification Tests
// ============================================================================

TEST(StoragePolicyConceptTest, DynamicStoragePolicySatisfiesConcept) {
    static_assert(StoragePolicy<dynamic_storage_policy>,
        "dynamic_storage_policy must satisfy StoragePolicy concept");
    SUCCEED();
}

TEST(StoragePolicyConceptTest, IndexedStoragePolicySatisfiesConcept) {
    static_assert(StoragePolicy<indexed_storage_policy>,
        "indexed_storage_policy must satisfy StoragePolicy concept");
    SUCCEED();
}

// ============================================================================
// Dynamic Storage Policy Tests
// ============================================================================

class DynamicStoragePolicyTest : public ::testing::Test {
protected:
    dynamic_storage_policy storage;

    void SetUp() override {
        storage.clear();
    }

    optimized_value make_value(const std::string& name, int data) {
        optimized_value val;
        val.name = name;
        val.type = value_types::int_value;
        val.data = data;
        return val;
    }

    optimized_value make_string_value(const std::string& name, const std::string& data) {
        optimized_value val;
        val.name = name;
        val.type = value_types::string_value;
        val.data = data;
        return val;
    }
};

TEST_F(DynamicStoragePolicyTest, InitiallyEmpty) {
    EXPECT_TRUE(storage.empty());
    EXPECT_EQ(storage.size(), 0u);
}

TEST_F(DynamicStoragePolicyTest, SetAndGet) {
    auto val = make_value("key1", 42);
    storage.set("key1", val);

    EXPECT_FALSE(storage.empty());
    EXPECT_EQ(storage.size(), 1u);

    auto result = storage.get("key1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "key1");
    EXPECT_EQ(std::get<int>(result->data), 42);
}

TEST_F(DynamicStoragePolicyTest, SetUpdatesExistingValue) {
    storage.set("key1", make_value("key1", 10));
    storage.set("key1", make_value("key1", 20));

    EXPECT_EQ(storage.size(), 1u);
    auto result = storage.get("key1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<int>(result->data), 20);
}

TEST_F(DynamicStoragePolicyTest, GetNonExistentKey) {
    auto result = storage.get("nonexistent");
    EXPECT_FALSE(result.has_value());
}

TEST_F(DynamicStoragePolicyTest, Contains) {
    storage.set("key1", make_value("key1", 42));

    EXPECT_TRUE(storage.contains("key1"));
    EXPECT_FALSE(storage.contains("key2"));
}

TEST_F(DynamicStoragePolicyTest, Remove) {
    storage.set("key1", make_value("key1", 42));
    storage.set("key2", make_value("key2", 100));

    EXPECT_TRUE(storage.remove("key1"));
    EXPECT_FALSE(storage.contains("key1"));
    EXPECT_TRUE(storage.contains("key2"));
    EXPECT_EQ(storage.size(), 1u);
}

TEST_F(DynamicStoragePolicyTest, RemoveNonExistent) {
    EXPECT_FALSE(storage.remove("nonexistent"));
}

TEST_F(DynamicStoragePolicyTest, Clear) {
    storage.set("key1", make_value("key1", 1));
    storage.set("key2", make_value("key2", 2));
    storage.set("key3", make_value("key3", 3));

    EXPECT_EQ(storage.size(), 3u);

    storage.clear();

    EXPECT_TRUE(storage.empty());
    EXPECT_EQ(storage.size(), 0u);
}

TEST_F(DynamicStoragePolicyTest, IteratorTraversal) {
    storage.set("key1", make_value("key1", 1));
    storage.set("key2", make_value("key2", 2));
    storage.set("key3", make_value("key3", 3));

    std::vector<std::string> keys;
    for (const auto& val : storage) {
        keys.push_back(val.name);
    }

    EXPECT_EQ(keys.size(), 3u);
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key1") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key2") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key3") != keys.end());
}

TEST_F(DynamicStoragePolicyTest, PreservesInsertionOrder) {
    storage.set("c", make_value("c", 3));
    storage.set("a", make_value("a", 1));
    storage.set("b", make_value("b", 2));

    std::vector<std::string> keys;
    for (const auto& val : storage) {
        keys.push_back(val.name);
    }

    ASSERT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], "c");
    EXPECT_EQ(keys[1], "a");
    EXPECT_EQ(keys[2], "b");
}

TEST_F(DynamicStoragePolicyTest, MoveSemantics) {
    auto val = make_string_value("key1", "test_string_data");
    storage.set("key1", std::move(val));

    auto result = storage.get("key1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<std::string>(result->data), "test_string_data");
}

TEST_F(DynamicStoragePolicyTest, CopyConstructor) {
    storage.set("key1", make_value("key1", 42));
    storage.set("key2", make_value("key2", 100));

    dynamic_storage_policy copy(storage);

    EXPECT_EQ(copy.size(), 2u);
    EXPECT_TRUE(copy.contains("key1"));
    EXPECT_TRUE(copy.contains("key2"));
}

TEST_F(DynamicStoragePolicyTest, MoveConstructor) {
    storage.set("key1", make_value("key1", 42));

    dynamic_storage_policy moved(std::move(storage));

    EXPECT_EQ(moved.size(), 1u);
    EXPECT_TRUE(moved.contains("key1"));
}

TEST_F(DynamicStoragePolicyTest, Reserve) {
    storage.reserve(100);
    // Just verify no crash - capacity is implementation detail
    SUCCEED();
}

// ============================================================================
// Indexed Storage Policy Tests
// ============================================================================

class IndexedStoragePolicyTest : public ::testing::Test {
protected:
    indexed_storage_policy storage;

    void SetUp() override {
        storage.clear();
    }

    optimized_value make_value(const std::string& name, int data) {
        optimized_value val;
        val.name = name;
        val.type = value_types::int_value;
        val.data = data;
        return val;
    }

    optimized_value make_string_value(const std::string& name, const std::string& data) {
        optimized_value val;
        val.name = name;
        val.type = value_types::string_value;
        val.data = data;
        return val;
    }
};

TEST_F(IndexedStoragePolicyTest, InitiallyEmpty) {
    EXPECT_TRUE(storage.empty());
    EXPECT_EQ(storage.size(), 0u);
}

TEST_F(IndexedStoragePolicyTest, SetAndGet) {
    auto val = make_value("key1", 42);
    storage.set("key1", val);

    EXPECT_FALSE(storage.empty());
    EXPECT_EQ(storage.size(), 1u);

    auto result = storage.get("key1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "key1");
    EXPECT_EQ(std::get<int>(result->data), 42);
}

TEST_F(IndexedStoragePolicyTest, SetUpdatesExistingValue) {
    storage.set("key1", make_value("key1", 10));
    storage.set("key1", make_value("key1", 20));

    EXPECT_EQ(storage.size(), 1u);
    auto result = storage.get("key1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<int>(result->data), 20);
}

TEST_F(IndexedStoragePolicyTest, GetNonExistentKey) {
    auto result = storage.get("nonexistent");
    EXPECT_FALSE(result.has_value());
}

TEST_F(IndexedStoragePolicyTest, Contains) {
    storage.set("key1", make_value("key1", 42));

    EXPECT_TRUE(storage.contains("key1"));
    EXPECT_FALSE(storage.contains("key2"));
}

TEST_F(IndexedStoragePolicyTest, Remove) {
    storage.set("key1", make_value("key1", 42));
    storage.set("key2", make_value("key2", 100));

    EXPECT_TRUE(storage.remove("key1"));
    EXPECT_FALSE(storage.contains("key1"));
    EXPECT_TRUE(storage.contains("key2"));
    EXPECT_EQ(storage.size(), 1u);

    // Verify the remaining value is still accessible
    auto result = storage.get("key2");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<int>(result->data), 100);
}

TEST_F(IndexedStoragePolicyTest, RemoveMiddleElement) {
    storage.set("key1", make_value("key1", 1));
    storage.set("key2", make_value("key2", 2));
    storage.set("key3", make_value("key3", 3));

    EXPECT_TRUE(storage.remove("key2"));
    EXPECT_EQ(storage.size(), 2u);

    // Verify index is correctly updated
    auto result1 = storage.get("key1");
    auto result3 = storage.get("key3");
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(std::get<int>(result1->data), 1);
    EXPECT_EQ(std::get<int>(result3->data), 3);
}

TEST_F(IndexedStoragePolicyTest, RemoveNonExistent) {
    EXPECT_FALSE(storage.remove("nonexistent"));
}

TEST_F(IndexedStoragePolicyTest, Clear) {
    storage.set("key1", make_value("key1", 1));
    storage.set("key2", make_value("key2", 2));
    storage.set("key3", make_value("key3", 3));

    EXPECT_EQ(storage.size(), 3u);

    storage.clear();

    EXPECT_TRUE(storage.empty());
    EXPECT_EQ(storage.size(), 0u);
    EXPECT_FALSE(storage.contains("key1"));
}

TEST_F(IndexedStoragePolicyTest, IteratorTraversal) {
    storage.set("key1", make_value("key1", 1));
    storage.set("key2", make_value("key2", 2));
    storage.set("key3", make_value("key3", 3));

    std::vector<std::string> keys;
    for (const auto& val : storage) {
        keys.push_back(val.name);
    }

    EXPECT_EQ(keys.size(), 3u);
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key1") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key2") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key3") != keys.end());
}

TEST_F(IndexedStoragePolicyTest, MoveSemantics) {
    auto val = make_string_value("key1", "test_string_data");
    storage.set("key1", std::move(val));

    auto result = storage.get("key1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<std::string>(result->data), "test_string_data");
}

TEST_F(IndexedStoragePolicyTest, CopyConstructor) {
    storage.set("key1", make_value("key1", 42));
    storage.set("key2", make_value("key2", 100));

    indexed_storage_policy copy(storage);

    EXPECT_EQ(copy.size(), 2u);
    EXPECT_TRUE(copy.contains("key1"));
    EXPECT_TRUE(copy.contains("key2"));
}

TEST_F(IndexedStoragePolicyTest, MoveConstructor) {
    storage.set("key1", make_value("key1", 42));

    indexed_storage_policy moved(std::move(storage));

    EXPECT_EQ(moved.size(), 1u);
    EXPECT_TRUE(moved.contains("key1"));
}

TEST_F(IndexedStoragePolicyTest, RebuildIndex) {
    storage.set("key1", make_value("key1", 1));
    storage.set("key2", make_value("key2", 2));

    // Directly modify underlying data (simulating external modification)
    storage.data()[0].data = 100;

    storage.rebuild_index();

    // Verify index still works after rebuild
    EXPECT_TRUE(storage.contains("key1"));
    EXPECT_TRUE(storage.contains("key2"));
}

TEST_F(IndexedStoragePolicyTest, Reserve) {
    storage.reserve(100);
    // Just verify no crash - capacity is implementation detail
    SUCCEED();
}

TEST_F(IndexedStoragePolicyTest, LargeNumberOfEntries) {
    const int NUM_ENTRIES = 1000;

    for (int i = 0; i < NUM_ENTRIES; ++i) {
        std::string key = "key_" + std::to_string(i);
        storage.set(key, make_value(key, i));
    }

    EXPECT_EQ(storage.size(), static_cast<std::size_t>(NUM_ENTRIES));

    // Verify random access
    auto result500 = storage.get("key_500");
    ASSERT_TRUE(result500.has_value());
    EXPECT_EQ(std::get<int>(result500->data), 500);

    auto result999 = storage.get("key_999");
    ASSERT_TRUE(result999.has_value());
    EXPECT_EQ(std::get<int>(result999->data), 999);
}

// ============================================================================
// Edge Cases and Error Handling Tests
// ============================================================================

TEST(StoragePolicyEdgeCases, EmptyKeyHandling) {
    dynamic_storage_policy storage;
    optimized_value val;
    val.name = "";
    val.type = value_types::int_value;
    val.data = 42;

    storage.set("", val);

    // Empty key should be valid
    EXPECT_TRUE(storage.contains(""));
    auto result = storage.get("");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<int>(result->data), 42);
}

TEST(StoragePolicyEdgeCases, SpecialCharactersInKey) {
    indexed_storage_policy storage;
    optimized_value val;
    val.name = "key/with:special!chars@#$%";
    val.type = value_types::string_value;
    val.data = std::string("test");

    storage.set(val.name, val);

    EXPECT_TRUE(storage.contains("key/with:special!chars@#$%"));
    auto result = storage.get("key/with:special!chars@#$%");
    ASSERT_TRUE(result.has_value());
}

TEST(StoragePolicyEdgeCases, UnicodeKeyHandling) {
    dynamic_storage_policy storage;
    optimized_value val;
    val.name = "키_한글_测试";
    val.type = value_types::int_value;
    val.data = 42;

    storage.set(val.name, val);

    EXPECT_TRUE(storage.contains("키_한글_测试"));
    auto result = storage.get("키_한글_测试");
    ASSERT_TRUE(result.has_value());
}

TEST(StoragePolicyEdgeCases, DirectDataAccess) {
    dynamic_storage_policy storage;
    optimized_value val;
    val.name = "key1";
    val.type = value_types::int_value;
    val.data = 10;
    storage.set("key1", val);

    auto& data = storage.data();
    EXPECT_EQ(data.size(), 1u);
    EXPECT_EQ(data[0].name, "key1");

    const dynamic_storage_policy& const_storage = storage;
    const auto& const_data = const_storage.data();
    EXPECT_EQ(const_data.size(), 1u);
}

// ============================================================================
// Static Storage Policy Tests (Issue #325)
// ============================================================================

// Type alias for commonly used static storage policy
using IntDoubleStringPolicy = static_storage_policy<int, double, std::string>;

class StaticStoragePolicyTest : public ::testing::Test {
protected:
    IntDoubleStringPolicy storage;

    void SetUp() override {
        storage.clear();
    }

    optimized_value make_int_value(const std::string& name, int data) {
        optimized_value val;
        val.name = name;
        val.type = value_types::int_value;
        val.data = data;
        return val;
    }

    optimized_value make_double_value(const std::string& name, double data) {
        optimized_value val;
        val.name = name;
        val.type = value_types::double_value;
        val.data = data;
        return val;
    }

    optimized_value make_string_value(const std::string& name, const std::string& data) {
        optimized_value val;
        val.name = name;
        val.type = value_types::string_value;
        val.data = data;
        return val;
    }

    optimized_value make_bool_value(const std::string& name, bool data) {
        optimized_value val;
        val.name = name;
        val.type = value_types::bool_value;
        val.data = data;
        return val;
    }
};

TEST_F(StaticStoragePolicyTest, ConceptSatisfied) {
    static_assert(StoragePolicy<IntDoubleStringPolicy>,
        "static_storage_policy<int, double, string> must satisfy StoragePolicy concept");
    SUCCEED();
}

TEST_F(StaticStoragePolicyTest, InitiallyEmpty) {
    EXPECT_TRUE(storage.empty());
    EXPECT_EQ(storage.size(), 0u);
}

TEST_F(StaticStoragePolicyTest, SetAndGetAllowedTypes) {
    storage.set("int_key", make_int_value("int_key", 42));
    storage.set("double_key", make_double_value("double_key", 3.14));
    storage.set("string_key", make_string_value("string_key", "hello"));

    EXPECT_EQ(storage.size(), 3u);

    auto int_result = storage.get("int_key");
    ASSERT_TRUE(int_result.has_value());
    EXPECT_EQ(std::get<int>(int_result->data), 42);

    auto double_result = storage.get("double_key");
    ASSERT_TRUE(double_result.has_value());
    EXPECT_DOUBLE_EQ(std::get<double>(double_result->data), 3.14);

    auto string_result = storage.get("string_key");
    ASSERT_TRUE(string_result.has_value());
    EXPECT_EQ(std::get<std::string>(string_result->data), "hello");
}

TEST_F(StaticStoragePolicyTest, SetTypedCompileTimeCheck) {
    // These should compile (types are allowed)
    storage.set_typed("count", 42);
    storage.set_typed("rate", 3.14);
    storage.set_typed("name", std::string("test"));

    EXPECT_EQ(storage.size(), 3u);

    // Verify values
    auto count = storage.get_typed<int>("count");
    ASSERT_TRUE(count.has_value());
    EXPECT_EQ(*count, 42);

    auto rate = storage.get_typed<double>("rate");
    ASSERT_TRUE(rate.has_value());
    EXPECT_DOUBLE_EQ(*rate, 3.14);

    auto name = storage.get_typed<std::string>("name");
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(*name, "test");
}

TEST_F(StaticStoragePolicyTest, GetTypedWrongType) {
    storage.set_typed("count", 42);

    // Key exists but type is wrong
    auto result = storage.get_typed<double>("count");
    EXPECT_FALSE(result.has_value());
}

TEST_F(StaticStoragePolicyTest, GetTypedNonExistent) {
    auto result = storage.get_typed<int>("nonexistent");
    EXPECT_FALSE(result.has_value());
}

TEST_F(StaticStoragePolicyTest, DisallowedTypeIgnored) {
    // bool is not in AllowedTypes, so it should be silently ignored
    storage.set("bool_key", make_bool_value("bool_key", true));

    // Value should not be stored
    EXPECT_FALSE(storage.contains("bool_key"));
    EXPECT_EQ(storage.size(), 0u);
}

TEST_F(StaticStoragePolicyTest, AllowsStaticCheck) {
    // Compile-time checks for allowed types
    static_assert(IntDoubleStringPolicy::allows<int>(), "int should be allowed");
    static_assert(IntDoubleStringPolicy::allows<double>(), "double should be allowed");
    static_assert(IntDoubleStringPolicy::allows<std::string>(), "string should be allowed");
    static_assert(!IntDoubleStringPolicy::allows<bool>(), "bool should not be allowed");
    static_assert(!IntDoubleStringPolicy::allows<float>(), "float should not be allowed");
    static_assert(!IntDoubleStringPolicy::allows<long>(), "long should not be allowed");
    SUCCEED();
}

TEST_F(StaticStoragePolicyTest, SetUpdatesExistingValue) {
    storage.set_typed("key", 10);
    storage.set_typed("key", 20);

    EXPECT_EQ(storage.size(), 1u);
    auto result = storage.get_typed<int>("key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 20);
}

TEST_F(StaticStoragePolicyTest, Contains) {
    storage.set_typed("key1", 42);

    EXPECT_TRUE(storage.contains("key1"));
    EXPECT_FALSE(storage.contains("key2"));
}

TEST_F(StaticStoragePolicyTest, Remove) {
    storage.set_typed("key1", 42);
    storage.set_typed("key2", 100);

    EXPECT_TRUE(storage.remove("key1"));
    EXPECT_FALSE(storage.contains("key1"));
    EXPECT_TRUE(storage.contains("key2"));
    EXPECT_EQ(storage.size(), 1u);
}

TEST_F(StaticStoragePolicyTest, RemoveNonExistent) {
    EXPECT_FALSE(storage.remove("nonexistent"));
}

TEST_F(StaticStoragePolicyTest, Clear) {
    storage.set_typed("key1", 1);
    storage.set_typed("key2", 2.0);
    storage.set_typed("key3", std::string("three"));

    EXPECT_EQ(storage.size(), 3u);

    storage.clear();

    EXPECT_TRUE(storage.empty());
    EXPECT_EQ(storage.size(), 0u);
}

TEST_F(StaticStoragePolicyTest, IteratorTraversal) {
    storage.set_typed("key1", 1);
    storage.set_typed("key2", 2.0);
    storage.set_typed("key3", std::string("three"));

    std::vector<std::string> keys;
    for (const auto& val : storage) {
        keys.push_back(val.name);
    }

    EXPECT_EQ(keys.size(), 3u);
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key1") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key2") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key3") != keys.end());
}

TEST_F(StaticStoragePolicyTest, CopyConstructor) {
    storage.set_typed("key1", 42);
    storage.set_typed("key2", 3.14);

    IntDoubleStringPolicy copy(storage);

    EXPECT_EQ(copy.size(), 2u);
    EXPECT_TRUE(copy.contains("key1"));
    EXPECT_TRUE(copy.contains("key2"));
}

TEST_F(StaticStoragePolicyTest, MoveConstructor) {
    storage.set_typed("key1", 42);

    IntDoubleStringPolicy moved(std::move(storage));

    EXPECT_EQ(moved.size(), 1u);
    EXPECT_TRUE(moved.contains("key1"));
}

TEST_F(StaticStoragePolicyTest, Reserve) {
    storage.reserve(100);
    SUCCEED();
}

TEST_F(StaticStoragePolicyTest, DirectDataAccess) {
    storage.set_typed("key1", 10);

    auto& data = storage.data();
    EXPECT_EQ(data.size(), 1u);
    EXPECT_EQ(data[0].name, "key1");

    const IntDoubleStringPolicy& const_storage = storage;
    const auto& const_data = const_storage.data();
    EXPECT_EQ(const_data.size(), 1u);
}

// Test with different type combinations
TEST(StaticStoragePolicyVariants, SingleTypePolicy) {
    static_storage_policy<int> int_only;

    static_assert(static_storage_policy<int>::allows<int>(), "int should be allowed");
    static_assert(!static_storage_policy<int>::allows<double>(), "double should not be allowed");

    int_only.set_typed("count", 42);
    EXPECT_EQ(int_only.size(), 1u);

    auto result = int_only.get_typed<int>("count");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST(StaticStoragePolicyVariants, NumericTypesOnly) {
    static_storage_policy<int, float, double> numeric;

    numeric.set_typed("int_val", 42);
    numeric.set_typed("float_val", 3.14f);
    numeric.set_typed("double_val", 2.718);

    EXPECT_EQ(numeric.size(), 3u);

    // String should be ignored
    optimized_value str_val;
    str_val.name = "str_val";
    str_val.type = value_types::string_value;
    str_val.data = std::string("ignored");
    numeric.set("str_val", str_val);

    EXPECT_EQ(numeric.size(), 3u);
    EXPECT_FALSE(numeric.contains("str_val"));
}

TEST(StaticStoragePolicyVariants, MixedTypePreservation) {
    // Test that types are correctly preserved through storage
    static_storage_policy<int, std::string> mixed;

    mixed.set_typed("int_key", 42);
    mixed.set_typed("str_key", std::string("hello"));

    auto int_result = mixed.get("int_key");
    ASSERT_TRUE(int_result.has_value());
    EXPECT_TRUE(std::holds_alternative<int>(int_result->data));

    auto str_result = mixed.get("str_key");
    ASSERT_TRUE(str_result.has_value());
    EXPECT_TRUE(std::holds_alternative<std::string>(str_result->data));
}
