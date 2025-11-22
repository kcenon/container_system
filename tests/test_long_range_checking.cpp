// BSD 3-Clause License
//
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this
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
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

/**
 * @file test_long_range_checking.cpp
 * @brief Unit tests for value type range support
 *
 * Tests the unified value implementation:
 * - long_value (type 6): mapped to int64_t (64-bit)
 * - ulong_value (type 7): mapped to uint64_t (64-bit)
 * - Verifies that large values are accepted and correctly stored
 */

#include "container/internal/value.h"
#include <cstdint>
#include <gtest/gtest.h>
#include <limits>

using namespace container_module;

// ============================================================================
// Test Fixture
// ============================================================================

class LongRangeCheckingTest : public ::testing::Test {
protected:
  // 32-bit boundary values
  static constexpr int64_t kInt32Min = -2147483648LL;
  static constexpr int64_t kInt32Max = 2147483647LL;
  static constexpr uint64_t kUInt32Max = 4294967295ULL;

  // 64-bit boundary values
  static constexpr int64_t kInt64Min = std::numeric_limits<int64_t>::min();
  static constexpr int64_t kInt64Max = std::numeric_limits<int64_t>::max();
  static constexpr uint64_t kUInt64Max = std::numeric_limits<uint64_t>::max();
};

// ============================================================================
// long_value (type 6) Tests - 64-bit Range
// ============================================================================

TEST_F(LongRangeCheckingTest, LongValueAcceptsValidPositiveValue) {
  EXPECT_NO_THROW({
    value lv("test", 1000000L);
    auto val = lv.get<int64_t>();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 1000000L);
  });
}

TEST_F(LongRangeCheckingTest, LongValueAcceptsValidNegativeValue) {
  EXPECT_NO_THROW({
    value lv("test", -1000000L);
    auto val = lv.get<int64_t>();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, -1000000L);
  });
}

TEST_F(LongRangeCheckingTest, LongValueAcceptsZero) {
  EXPECT_NO_THROW({
    value lv("test", 0L);
    auto val = lv.get<int64_t>();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 0L);
  });
}

TEST_F(LongRangeCheckingTest, LongValueAcceptsInt32Max) {
  EXPECT_NO_THROW({
    value lv("test", static_cast<long>(kInt32Max));
    auto val = lv.get<int64_t>();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, kInt32Max);
  });
}

TEST_F(LongRangeCheckingTest, LongValueAcceptsInt32Min) {
  EXPECT_NO_THROW({
    value lv("test", static_cast<long>(kInt32Min));
    auto val = lv.get<int64_t>();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, kInt32Min);
  });
}

// Test 64-bit support
TEST_F(LongRangeCheckingTest, LongValueAcceptsLargePositiveValue) {
  EXPECT_NO_THROW({
    value lv("test", 5000000000L); // 5 billion
    auto val = lv.get<int64_t>();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 5000000000L);
  });
}

TEST_F(LongRangeCheckingTest, LongValueAcceptsLargeNegativeValue) {
  EXPECT_NO_THROW({
    value lv("test", -5000000000L); // -5 billion
    auto val = lv.get<int64_t>();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, -5000000000L);
  });
}

// ============================================================================
// ulong_value (type 7) Tests - 64-bit Range
// ============================================================================

TEST_F(LongRangeCheckingTest, ULongValueAcceptsValidValue) {
  EXPECT_NO_THROW({
    value ulv("test", 1000000UL);
    auto val = ulv.get<uint64_t>();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 1000000UL);
  });
}

TEST_F(LongRangeCheckingTest, ULongValueAcceptsZero) {
  EXPECT_NO_THROW({
    value ulv("test", 0UL);
    auto val = ulv.get<uint64_t>();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 0UL);
  });
}

TEST_F(LongRangeCheckingTest, ULongValueAcceptsUInt32Max) {
  EXPECT_NO_THROW({
    value ulv("test", static_cast<unsigned long>(kUInt32Max));
    auto val = ulv.get<uint64_t>();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, kUInt32Max);
  });
}

TEST_F(LongRangeCheckingTest, ULongValueAcceptsLargeValue) {
  EXPECT_NO_THROW({
    value ulv("test", 10000000000UL); // 10 billion
    auto val = ulv.get<uint64_t>();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 10000000000UL);
  });
}

// ============================================================================
// Serialization Tests
// ============================================================================

TEST_F(LongRangeCheckingTest, LongValueSerializesCorrectly) {
  value lv("test", 12345L);
  std::vector<uint8_t> serialized = lv.serialize();
  EXPECT_FALSE(serialized.empty());

  auto deserialized = value::deserialize(serialized);
  ASSERT_TRUE(deserialized.has_value());
  EXPECT_EQ(deserialized->get<int64_t>(), 12345L);
}

TEST_F(LongRangeCheckingTest, ULongValueSerializesCorrectly) {
  value ulv("test", 12345UL);
  std::vector<uint8_t> serialized = ulv.serialize();
  EXPECT_FALSE(serialized.empty());

  auto deserialized = value::deserialize(serialized);
  ASSERT_TRUE(deserialized.has_value());
  EXPECT_EQ(deserialized->get<uint64_t>(), 12345UL);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
