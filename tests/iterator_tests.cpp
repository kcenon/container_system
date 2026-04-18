// BSD 3-Clause License
// Copyright (c) 2021-2026, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file iterator_tests.cpp
 * @brief Unit tests for the STL iterator interface on value_container
 *
 * Covers:
 * - iterator_traits completeness and iterator categories
 * - Range-based for loops (const and non-const)
 * - std::distance and std::advance
 * - STL algorithms (find_if, count_if, copy, accumulate, all_of)
 * - Const correctness (const_iterator, cbegin/cend)
 * - Bidirectional traversal via reverse_iterator/rbegin/rend
 * - Equality/inequality invariants
 *
 * Closes #526
 */

#include <gtest/gtest.h>
#include "test_compat.h"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <type_traits>
#include <vector>

#if defined(__has_include)
#  if __has_include(<concepts>)
#    include <concepts>
#  endif
#endif

#if defined(__has_include)
#  if __has_include(<ranges>)
#    include <ranges>
#  endif
#endif

using namespace kcenon::container;

namespace {

// Helper: populate container in-place with N int values named "k<i>".
// (The move constructor of value_container does not transfer the value list,
//  so we avoid return-by-value here.)
void populate(value_container& c, int n) {
    for (int i = 0; i < n; ++i) {
        c.set("k" + std::to_string(i), i);
    }
}

} // namespace

// ============================================================================
// Iterator Trait Checks (compile-time)
// ============================================================================

TEST(ContainerIteratorTraits, TypeAliasesExist) {
    using I = value_container::iterator;
    using CI = value_container::const_iterator;
    using RI = value_container::reverse_iterator;
    using CRI = value_container::const_reverse_iterator;

    static_assert(std::is_same_v<value_container::value_type, optimized_value>);
    static_assert(std::is_same_v<value_container::reference,
                                 optimized_value&>);
    static_assert(std::is_same_v<value_container::const_reference,
                                 const optimized_value&>);
    static_assert(std::is_same_v<value_container::pointer,
                                 optimized_value*>);
    static_assert(std::is_same_v<value_container::const_pointer,
                                 const optimized_value*>);

    using traits = std::iterator_traits<I>;
    static_assert(std::is_same_v<traits::value_type, optimized_value>);
    static_assert(std::is_same_v<traits::reference, optimized_value&>);
    static_assert(std::is_same_v<traits::pointer, optimized_value*>);
    static_assert(std::is_signed_v<traits::difference_type>);

    using ctraits = std::iterator_traits<CI>;
    static_assert(std::is_same_v<ctraits::reference,
                                 const optimized_value&>);
    static_assert(std::is_same_v<ctraits::pointer,
                                 const optimized_value*>);

    // Compiles if reverse iterators expose a reverse_iterator category.
    static_assert(std::is_same_v<
        std::iterator_traits<RI>::value_type, optimized_value>);
    static_assert(std::is_same_v<
        std::iterator_traits<CRI>::value_type, optimized_value>);

    SUCCEED();
}

TEST(ContainerIteratorTraits, ForwardAndBidirectionalCategories) {
    using I = value_container::iterator;
    using CI = value_container::const_iterator;

    // iterator_category must exist and be derived from forward_iterator_tag.
    using cat = std::iterator_traits<I>::iterator_category;
    using ccat = std::iterator_traits<CI>::iterator_category;

    static_assert(std::is_base_of_v<std::forward_iterator_tag, cat>);
    static_assert(std::is_base_of_v<std::bidirectional_iterator_tag, cat>);
    static_assert(std::is_base_of_v<std::forward_iterator_tag, ccat>);
    static_assert(
        std::is_base_of_v<std::bidirectional_iterator_tag, ccat>);

#if defined(__cpp_lib_concepts) && __cpp_lib_concepts >= 202002L
    static_assert(std::forward_iterator<I>);
    static_assert(std::forward_iterator<CI>);
    static_assert(std::bidirectional_iterator<I>);
    static_assert(std::bidirectional_iterator<CI>);
#endif

    SUCCEED();
}

// ============================================================================
// Range-based for loops
// ============================================================================

TEST(ContainerIterator, RangeForTraversalVisitsAllValues) {
    value_container c;
    populate(c, 5);

    int visited = 0;
    for (const auto& v : c) {
        EXPECT_FALSE(v.name.empty());
        ++visited;
    }
    EXPECT_EQ(visited, 5);
}

TEST(ContainerIterator, RangeForOnConstReference) {
    value_container tmp;
    populate(tmp, 3);
    const value_container& c = tmp;

    int visited = 0;
    for (const auto& v : c) {
        (void)v;
        ++visited;
    }
    EXPECT_EQ(visited, 3);
}

// ============================================================================
// std::distance, std::advance
// ============================================================================

TEST(ContainerIterator, DistanceMatchesSize) {
    value_container c;
    populate(c, 7);

    auto d = std::distance(c.begin(), c.end());
    EXPECT_EQ(static_cast<value_container::size_type>(d), c.size());
    EXPECT_EQ(d, 7);
}

TEST(ContainerIterator, DistanceOnEmptyContainerIsZero) {
    value_container c;
    EXPECT_EQ(std::distance(c.begin(), c.end()), 0);
    EXPECT_TRUE(c.empty());
}

TEST(ContainerIterator, AdvanceMovesForwardAndBackward) {
    value_container c;
    populate(c, 5);

    auto it = c.begin();
    std::advance(it, 3);
    ASSERT_NE(it, c.end());
    EXPECT_EQ(it->name, "k3");

    // Bidirectional: advance backward
    std::advance(it, -2);
    EXPECT_EQ(it->name, "k1");
}

// ============================================================================
// STL algorithms
// ============================================================================

TEST(ContainerIterator, StdFindIfLocatesMatchingValue) {
    value_container c;
    populate(c, 10);

    auto it = std::find_if(c.begin(), c.end(),
        [](const optimized_value& v) { return v.name == "k4"; });

    ASSERT_NE(it, c.end());
    EXPECT_EQ(it->name, "k4");
}

TEST(ContainerIterator, StdFindIfReturnsEndWhenMissing) {
    value_container c;
    populate(c, 3);

    auto it = std::find_if(c.begin(), c.end(),
        [](const optimized_value& v) { return v.name == "missing"; });
    EXPECT_EQ(it, c.end());
}

TEST(ContainerIterator, StdCountIfCountsMatches) {
    value_container c;
    populate(c, 6);

    auto n = std::count_if(c.cbegin(), c.cend(),
        [](const optimized_value& v) {
            return !v.name.empty() && v.name[0] == 'k';
        });
    EXPECT_EQ(n, 6);
}

TEST(ContainerIterator, StdAllOfOverConstIterators) {
    value_container tmp;
    populate(tmp, 4);
    const value_container& c = tmp;

    bool ok = std::all_of(c.begin(), c.end(),
        [](const optimized_value& v) { return !v.name.empty(); });
    EXPECT_TRUE(ok);
}

TEST(ContainerIterator, StdCopyIntoVectorProducesEqualSequence) {
    value_container c;
    populate(c, 4);

    std::vector<optimized_value> out;
    out.reserve(c.size());
    std::copy(c.begin(), c.end(), std::back_inserter(out));

    ASSERT_EQ(out.size(), c.size());
    for (std::size_t i = 0; i < out.size(); ++i) {
        EXPECT_EQ(out[i].name, "k" + std::to_string(i));
    }
}

TEST(ContainerIterator, StdAccumulateOnNameLengths) {
    value_container c;
    populate(c, 4);

    auto total = std::accumulate(c.begin(), c.end(), std::size_t{0},
        [](std::size_t acc, const optimized_value& v) {
            return acc + v.name.size();
        });

    // "k0","k1","k2","k3" -> 8 characters total.
    EXPECT_EQ(total, 8u);
}

// ============================================================================
// Const correctness
// ============================================================================

TEST(ContainerIterator, CbeginCendReturnConstIterators) {
    value_container c;
    c.set("a", 1);

    auto it = c.cbegin();
    auto end = c.cend();

    static_assert(
        std::is_same_v<decltype(it), value_container::const_iterator>);
    static_assert(
        std::is_same_v<decltype(end), value_container::const_iterator>);

    ASSERT_NE(it, end);
    EXPECT_EQ(it->name, "a");
}

TEST(ContainerIterator, ConstContainerYieldsConstIterators) {
    value_container tmp;
    populate(tmp, 2);
    const value_container& c = tmp;

    auto it = c.begin();
    static_assert(
        std::is_same_v<decltype(it), value_container::const_iterator>);
    (void)it;
    SUCCEED();
}

// ============================================================================
// Bidirectional / reverse traversal
// ============================================================================

TEST(ContainerIterator, ReverseIteratorTraversesInReverse) {
    value_container c;
    populate(c, 4);

    std::vector<std::string> names;
    for (auto it = c.rbegin(); it != c.rend(); ++it) {
        names.push_back(it->name);
    }

    ASSERT_EQ(names.size(), 4u);
    EXPECT_EQ(names[0], "k3");
    EXPECT_EQ(names[1], "k2");
    EXPECT_EQ(names[2], "k1");
    EXPECT_EQ(names[3], "k0");
}

TEST(ContainerIterator, CrbeginCrendAreConstReverseIterators) {
    value_container tmp;
    populate(tmp, 2);
    const value_container& c = tmp;

    auto it = c.crbegin();
    auto end = c.crend();
    static_assert(std::is_same_v<decltype(it),
                                 value_container::const_reverse_iterator>);
    static_assert(std::is_same_v<decltype(end),
                                 value_container::const_reverse_iterator>);

    ASSERT_NE(it, end);
    EXPECT_EQ(it->name, "k1");
}

TEST(ContainerIterator, BidirectionalDecrementReturnsToPrevious) {
    value_container c;
    populate(c, 3);

    auto it = c.begin();
    ++it;
    ++it;
    EXPECT_EQ(it->name, "k2");
    --it;
    EXPECT_EQ(it->name, "k1");
    --it;
    EXPECT_EQ(it->name, "k0");
    EXPECT_EQ(it, c.begin());
}

// ============================================================================
// Equality / inequality invariants
// ============================================================================

TEST(ContainerIterator, BeginEqualsEndOnEmpty) {
    value_container c;
    EXPECT_EQ(c.begin(), c.end());
    EXPECT_EQ(c.cbegin(), c.cend());
    EXPECT_EQ(c.rbegin(), c.rend());
    EXPECT_EQ(c.crbegin(), c.crend());
}

TEST(ContainerIterator, BeginDiffersFromEndOnNonEmpty) {
    value_container c;
    populate(c, 1);
    EXPECT_NE(c.begin(), c.end());
    EXPECT_NE(c.cbegin(), c.cend());
}

TEST(ContainerIterator, IncrementEqualityAfterSize) {
    value_container c;
    populate(c, 3);

    auto it = c.begin();
    ++it;
    ++it;
    ++it;
    EXPECT_EQ(it, c.end());
}

TEST(ContainerIterator, ConstAndNonConstIteratorsCompareEqual) {
    value_container c;
    populate(c, 2);
    const value_container& cref = c;

    EXPECT_EQ(c.begin(), cref.begin());
    EXPECT_EQ(c.end(), cref.end());
}

// ============================================================================
// Range concept (C++20 ranges)
// ============================================================================

#if defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L
TEST(ContainerIterator, SatisfiesStdRangesRange) {
    static_assert(std::ranges::range<value_container>);
    static_assert(std::ranges::range<const value_container>);
    static_assert(std::ranges::forward_range<value_container>);
    static_assert(std::ranges::bidirectional_range<value_container>);
    SUCCEED();
}
#endif
