// BSD 3-Clause License
// Copyright (c) 2021-2026, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

/**
 * @file allocator_adapter_tests.cpp
 * @brief Unit tests for pool_allocator_adapter STL allocator compatibility
 *
 * Covers:
 * - std::allocator_traits completeness
 * - Rebind mechanism for node-based containers
 * - Direct allocate/deallocate round trip
 * - Integration with std::vector, std::list, std::unordered_map
 * - Equality / is_always_equal invariants
 * - Counting allocator using rebound pool_allocator_adapter internally
 *
 * Closes #525
 */

#include <gtest/gtest.h>

#include <internal/pool_allocator_adapter.h>

#include <atomic>
#include <list>
#include <memory>
#include <numeric>
#include <scoped_allocator>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

using kcenon::container::internal::pool_allocator;
using kcenon::container::internal::pool_allocator_adapter;

// ============================================================================
// Static compile-time checks for allocator_traits
// ============================================================================

static_assert(std::is_same_v<
        std::allocator_traits<pool_allocator_adapter<int>>::value_type, int>,
    "allocator_traits<pool_allocator_adapter<int>>::value_type must be int");

static_assert(std::is_same_v<
        std::allocator_traits<pool_allocator_adapter<int>>::pointer, int*>,
    "allocator_traits<pool_allocator_adapter<int>>::pointer must be int*");

static_assert(std::is_same_v<
        std::allocator_traits<pool_allocator_adapter<int>>::size_type, std::size_t>,
    "allocator_traits size_type mismatch");

static_assert(std::allocator_traits<pool_allocator_adapter<int>>::is_always_equal::value,
    "pool_allocator_adapter must advertise is_always_equal = true");

static_assert(!std::allocator_traits<pool_allocator_adapter<int>>
        ::propagate_on_container_copy_assignment::value,
    "stateless adapter should not propagate on copy assignment");

static_assert(!std::allocator_traits<pool_allocator_adapter<int>>
        ::propagate_on_container_move_assignment::value,
    "stateless adapter should not propagate on move assignment");

static_assert(!std::allocator_traits<pool_allocator_adapter<int>>
        ::propagate_on_container_swap::value,
    "stateless adapter should not propagate on swap");

static_assert(std::is_same_v<
        std::allocator_traits<pool_allocator_adapter<int>>::rebind_alloc<double>,
        pool_allocator_adapter<double>>,
    "rebind<double>::other must be pool_allocator_adapter<double>");

// std::allocator is nothrow default constructible; the adapter follows suit.
static_assert(std::is_nothrow_default_constructible_v<pool_allocator_adapter<int>>,
    "pool_allocator_adapter must be nothrow default constructible");

static_assert(std::is_nothrow_copy_constructible_v<pool_allocator_adapter<int>>,
    "pool_allocator_adapter must be nothrow copy constructible");

// ============================================================================
// Direct allocate/deallocate
// ============================================================================

TEST(PoolAllocatorAdapter, AllocateDeallocateSingleObject) {
    pool_allocator_adapter<int> alloc;
    int* p = alloc.allocate(1);
    ASSERT_NE(p, nullptr);
    *p = 42;
    EXPECT_EQ(*p, 42);
    alloc.deallocate(p, 1);
}

TEST(PoolAllocatorAdapter, AllocateZeroReturnsNullptr) {
    pool_allocator_adapter<int> alloc;
    EXPECT_EQ(alloc.allocate(0), nullptr);
    // Deallocate of nullptr must be safe.
    alloc.deallocate(nullptr, 0);
}

TEST(PoolAllocatorAdapter, AllocateArrayOfObjects) {
    pool_allocator_adapter<int> alloc;
    constexpr std::size_t n = 8;
    int* p = alloc.allocate(n);
    ASSERT_NE(p, nullptr);
    for (std::size_t i = 0; i < n; ++i) {
        p[i] = static_cast<int>(i);
    }
    for (std::size_t i = 0; i < n; ++i) {
        EXPECT_EQ(p[i], static_cast<int>(i));
    }
    alloc.deallocate(p, n);
}

TEST(PoolAllocatorAdapter, MaxSizeIsSensible) {
    pool_allocator_adapter<int> alloc;
    EXPECT_GT(alloc.max_size(), std::size_t{0});
    // Cannot exceed numeric_limits<size_t>::max() / sizeof(int).
    EXPECT_LE(alloc.max_size(),
              std::numeric_limits<std::size_t>::max() / sizeof(int));
}

TEST(PoolAllocatorAdapter, AllocateOverflowThrowsBadAlloc) {
    pool_allocator_adapter<int> alloc;
    const std::size_t too_many = alloc.max_size() + 1;
    EXPECT_THROW(alloc.allocate(too_many), std::bad_alloc);
}

// ============================================================================
// Equality and statelessness
// ============================================================================

TEST(PoolAllocatorAdapter, InstancesAreAlwaysEqual) {
    pool_allocator_adapter<int> a;
    pool_allocator_adapter<int> b;
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(PoolAllocatorAdapter, DifferentValueTypesCompareEqual) {
    pool_allocator_adapter<int> a;
    pool_allocator_adapter<double> b;
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(PoolAllocatorAdapter, CrossTypeConstructorCompiles) {
    pool_allocator_adapter<int> a;
    pool_allocator_adapter<double> b(a);
    (void)b;
    SUCCEED();
}

TEST(PoolAllocatorAdapter, RebindThroughTraitsProducesCorrectType) {
    using traits_int = std::allocator_traits<pool_allocator_adapter<int>>;
    using rebound = traits_int::rebind_alloc<std::string>;
    static_assert(std::is_same_v<rebound, pool_allocator_adapter<std::string>>,
                  "rebind_alloc<string> must produce the string specialisation");
    rebound r;
    std::string* s = r.allocate(1);
    ASSERT_NE(s, nullptr);
    std::allocator_traits<rebound>::construct(r, s, "hello");
    EXPECT_EQ(*s, "hello");
    std::allocator_traits<rebound>::destroy(r, s);
    r.deallocate(s, 1);
}

// ============================================================================
// STL container integration
// ============================================================================

TEST(PoolAllocatorAdapter, StdVectorBasicUsage) {
    std::vector<int, pool_allocator_adapter<int>> v;
    v.reserve(16);
    for (int i = 0; i < 100; ++i) {
        v.push_back(i);
    }
    ASSERT_EQ(v.size(), 100u);
    EXPECT_EQ(v.front(), 0);
    EXPECT_EQ(v.back(), 99);

    long long sum = std::accumulate(v.begin(), v.end(), 0LL);
    EXPECT_EQ(sum, 99LL * 100 / 2);
}

TEST(PoolAllocatorAdapter, StdVectorCopyAndMove) {
    using vec = std::vector<int, pool_allocator_adapter<int>>;
    vec a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i * 2);
    }

    vec b(a);            // copy
    EXPECT_EQ(b.size(), a.size());
    EXPECT_EQ(b.back(), 18);

    vec c(std::move(a)); // move
    EXPECT_EQ(c.size(), 10u);
    EXPECT_EQ(c.back(), 18);
}

TEST(PoolAllocatorAdapter, StdListRequiresRebind) {
    // std::list internally allocates node types, exercising rebind.
    std::list<int, pool_allocator_adapter<int>> l;
    for (int i = 0; i < 32; ++i) {
        l.push_back(i);
    }
    EXPECT_EQ(l.size(), 32u);

    int expected = 0;
    for (int v : l) {
        EXPECT_EQ(v, expected++);
    }
}

TEST(PoolAllocatorAdapter, StdUnorderedMapWithStringKeys) {
    using pair_t = std::pair<const std::string, int>;
    std::unordered_map<std::string, int, std::hash<std::string>,
                       std::equal_to<std::string>,
                       pool_allocator_adapter<pair_t>> m;

    m.emplace("alpha", 1);
    m.emplace("beta", 2);
    m.emplace("gamma", 3);

    EXPECT_EQ(m.size(), 3u);
    EXPECT_EQ(m.at("alpha"), 1);
    EXPECT_EQ(m.at("beta"), 2);
    EXPECT_EQ(m.at("gamma"), 3);
}

TEST(PoolAllocatorAdapter, ScopedAllocatorAdaptorSmoke) {
    // Outer container uses pool_allocator_adapter; inner containers (strings)
    // will also use it thanks to scoped_allocator_adaptor.
    using inner_alloc = pool_allocator_adapter<char>;
    using inner_string = std::basic_string<char, std::char_traits<char>, inner_alloc>;
    using outer_alloc = std::scoped_allocator_adaptor<pool_allocator_adapter<inner_string>>;

    std::vector<inner_string, outer_alloc> v;
    v.emplace_back("short");
    v.emplace_back("a somewhat longer string that may heap-allocate");
    ASSERT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], "short");
    EXPECT_EQ(v[1].substr(0, 8), "a somewh");
}

// ============================================================================
// Pool routing verification
// ============================================================================

TEST(PoolAllocatorAdapter, SmallAllocationRoutesThroughPool) {
    // Small allocation (<= 64 bytes) should increment small pool counter
    // when CONTAINER_USE_MEMORY_POOL is enabled.
    pool_allocator::instance().reset_stats();

    {
        pool_allocator_adapter<int> alloc;
        int* p = alloc.allocate(4);      // 16 bytes -> small pool
        ASSERT_NE(p, nullptr);
        alloc.deallocate(p, 4);
    }

    auto stats = pool_allocator::instance().get_stats();
#if CONTAINER_USE_MEMORY_POOL
    EXPECT_GE(stats.small_pool_allocs, 1u);
    EXPECT_GE(stats.pool_hits, 1u);
#endif
    EXPECT_GE(stats.deallocations, 1u);
}

// ============================================================================
// Counting adapter wrapping pool_allocator_adapter
// ============================================================================

namespace {

// Test-only counting allocator that forwards to pool_allocator_adapter. Demonstrates
// the adapter composes cleanly with other std-compliant allocators.
template<typename T>
struct counting_allocator {
    using value_type = T;

    std::atomic<std::size_t>* allocs;
    std::atomic<std::size_t>* deallocs;

    counting_allocator(std::atomic<std::size_t>* a,
                       std::atomic<std::size_t>* d) noexcept
        : allocs(a), deallocs(d) {}

    template<typename U>
    counting_allocator(const counting_allocator<U>& other) noexcept
        : allocs(other.allocs), deallocs(other.deallocs) {}

    T* allocate(std::size_t n) {
        allocs->fetch_add(1, std::memory_order_relaxed);
        pool_allocator_adapter<T> inner;
        return inner.allocate(n);
    }

    void deallocate(T* p, std::size_t n) noexcept {
        deallocs->fetch_add(1, std::memory_order_relaxed);
        pool_allocator_adapter<T> inner;
        inner.deallocate(p, n);
    }

    template<typename U>
    bool operator==(const counting_allocator<U>& other) const noexcept {
        return allocs == other.allocs && deallocs == other.deallocs;
    }
    template<typename U>
    bool operator!=(const counting_allocator<U>& other) const noexcept {
        return !(*this == other);
    }
};

} // namespace

TEST(PoolAllocatorAdapter, CountingAllocatorObservesContainerActivity) {
    std::atomic<std::size_t> allocs{0};
    std::atomic<std::size_t> deallocs{0};

    {
        std::vector<int, counting_allocator<int>> v{
            counting_allocator<int>(&allocs, &deallocs)};
        v.reserve(8);
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
    }

    EXPECT_GE(allocs.load(), 1u);
    EXPECT_GE(deallocs.load(), 1u);
    EXPECT_EQ(allocs.load(), deallocs.load());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
