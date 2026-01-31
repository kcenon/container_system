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
 * @file fast_parser_integration_tests.cpp
 * @brief Integration tests for fast_parser.h with external common_system
 *
 * Verifies that fast_parser.h works correctly when common_system is fetched
 * as an external dependency instead of a local copy.
 *
 * Tests cover:
 * - Include paths resolve correctly
 * - Dependencies compile without errors
 * - Functionality preserved after refactoring
 *
 * Related: Issue #352, #350 (CMake FetchContent setup)
 */

#include <gtest/gtest.h>
#include <include/container/optimizations/fast_parser.h>
#include <vector>
#include <string>
#include <list>
#include <chrono>

namespace {

using namespace container_module;

/**
 * @brief Verifies that fast_parser.h includes resolve correctly
 *
 * This test ensures that the header can be included without compilation errors
 * when using external common_system via CMake FetchContent.
 */
TEST(FastParserIntegrationTest, HeaderIncludesResolveCorrectly) {
    // If this test compiles, include paths are working correctly
    SUCCEED();
}

/**
 * @brief Tests reserve_if_possible with vector (has reserve method)
 */
TEST(FastParserIntegrationTest, ReserveIfPossibleWithVector) {
    std::vector<int> vec;

    // Reserve space for 100 elements
    reserve_if_possible(vec, 100);

    // Verify capacity was reserved
    EXPECT_GE(vec.capacity(), 100);
    EXPECT_EQ(vec.size(), 0);  // Size should remain 0
}

/**
 * @brief Tests reserve_if_possible with string (has reserve method)
 */
TEST(FastParserIntegrationTest, ReserveIfPossibleWithString) {
    std::string str;

    // Reserve space for 256 characters
    reserve_if_possible(str, 256);

    // Verify capacity was reserved
    EXPECT_GE(str.capacity(), 256);
    EXPECT_EQ(str.size(), 0);  // Size should remain 0
}

/**
 * @brief Tests reserve_if_possible with container without reserve (e.g., std::list)
 *
 * This test verifies that the constexpr if correctly detects containers
 * without reserve() method and safely does nothing.
 */
TEST(FastParserIntegrationTest, ReserveIfPossibleWithListDoesNothing) {
    // This test ensures the template compiles correctly even for containers
    // without reserve() method
    std::list<int> lst;

    // Should compile and do nothing (constexpr if branch)
    reserve_if_possible(lst, 100);

    // List doesn't have capacity concept, just verify size is 0
    EXPECT_EQ(lst.size(), 0);
    SUCCEED();
}

/**
 * @brief Tests parser_config default values
 */
TEST(FastParserIntegrationTest, ParserConfigDefaults) {
    parser_config config;

    // Verify default configuration
    EXPECT_TRUE(config.use_fast_path);
    EXPECT_EQ(config.initial_capacity, 256);
}

/**
 * @brief Tests parser_config with custom values
 */
TEST(FastParserIntegrationTest, ParserConfigCustomValues) {
    parser_config config;
    config.use_fast_path = false;
    config.initial_capacity = 512;

    // Verify custom configuration
    EXPECT_FALSE(config.use_fast_path);
    EXPECT_EQ(config.initial_capacity, 512);
}

/**
 * @brief Performance test: verify reserve_if_possible improves allocation performance
 */
TEST(FastParserIntegrationTest, ReserveImprovesPerformance) {
    constexpr size_t num_elements = 10000;

    // Measure time with reserve
    auto start_with_reserve = std::chrono::high_resolution_clock::now();
    std::vector<int> vec_with_reserve;
    reserve_if_possible(vec_with_reserve, num_elements);
    for (size_t i = 0; i < num_elements; ++i) {
        vec_with_reserve.push_back(static_cast<int>(i));
    }
    auto end_with_reserve = std::chrono::high_resolution_clock::now();

    // Measure time without reserve
    auto start_without_reserve = std::chrono::high_resolution_clock::now();
    std::vector<int> vec_without_reserve;
    for (size_t i = 0; i < num_elements; ++i) {
        vec_without_reserve.push_back(static_cast<int>(i));
    }
    auto end_without_reserve = std::chrono::high_resolution_clock::now();

    // Verify both vectors have same content
    EXPECT_EQ(vec_with_reserve.size(), num_elements);
    EXPECT_EQ(vec_without_reserve.size(), num_elements);
    EXPECT_EQ(vec_with_reserve, vec_without_reserve);

    // Note: We don't assert performance improvement as it may vary by platform,
    // but this test exercises the functionality under realistic conditions
}

/**
 * @brief Integration test: combine parser_config with reserve_if_possible
 */
TEST(FastParserIntegrationTest, ConfigWithReserveIntegration) {
    parser_config config;
    std::vector<std::string> tokens;

    // Use config to reserve initial capacity
    reserve_if_possible(tokens, config.initial_capacity);

    // Verify capacity was reserved
    EXPECT_GE(tokens.capacity(), config.initial_capacity);

    // Simulate parsing with fast path
    if (config.use_fast_path) {
        // Fast path: use reserved capacity
        for (size_t i = 0; i < 10; ++i) {
            tokens.push_back("token_" + std::to_string(i));
        }
    }

    EXPECT_EQ(tokens.size(), 10);
}

} // namespace
