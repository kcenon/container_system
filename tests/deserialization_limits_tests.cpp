#include <gtest/gtest.h>
#include <kcenon/container/internal/value.h>
#include <kcenon/container/internal/thread_safe_container.h>

#include <atomic>
#include <cstdint>
#include <limits>
#include <thread>
#include <vector>

using kcenon::container::value;
using kcenon::container::value_types;

namespace {
void append_u32(std::vector<uint8_t>& bytes, uint32_t number) {
    const auto* data = reinterpret_cast<const uint8_t*>(&number);
    bytes.insert(bytes.end(), data, data + sizeof(number));
}

// Construct hostile wire input iteratively; the test never builds a deep C++
// object graph whose own construction or destruction could exhaust the stack.
std::vector<uint8_t> nested_arrays(size_t depth) {
    std::vector<uint8_t> bytes;
    bytes.reserve(depth * 9 + 5);
    for (size_t index = 0; index < depth; ++index) {
        append_u32(bytes, 0);
        bytes.push_back(static_cast<uint8_t>(value_types::array_value));
        append_u32(bytes, 1);
    }
    append_u32(bytes, 0);
    bytes.push_back(static_cast<uint8_t>(value_types::null_value));
    return bytes;
}

std::vector<uint8_t> container_for_value(const std::vector<uint8_t>& bytes) {
    std::vector<uint8_t> container;
    append_u32(container, 1);
    append_u32(container, 1);
    container.push_back('x');
    append_u32(container, static_cast<uint32_t>(bytes.size()));
    container.insert(container.end(), bytes.begin(), bytes.end());
    return container;
}

std::vector<uint8_t> nested_containers(size_t depth) {
    auto bytes = nested_arrays(0);
    for (size_t index = 0; index < depth; ++index) {
        const auto container = container_for_value(bytes);
        bytes.clear();
        append_u32(bytes, 0);
        bytes.push_back(static_cast<uint8_t>(value_types::container_value));
        append_u32(bytes, static_cast<uint32_t>(container.size()));
        bytes.insert(bytes.end(), container.begin(), container.end());
    }
    return bytes;
}
} // namespace

TEST(DeserializationLimits, AcceptsTheDepthBoundaryAndPreservesWireData) {
    const auto bytes = nested_arrays(value::max_deserialization_depth - 1);
    auto decoded = value::deserialize(bytes);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->serialize(), bytes);
}

TEST(DeserializationLimits, RejectsOneLevelBeyondTheBoundary) {
    EXPECT_FALSE(value::deserialize(nested_arrays(value::max_deserialization_depth)));
}

TEST(DeserializationLimits, RejectsOneHundredThousandLevelsWithoutCrashing) {
    EXPECT_FALSE(value::deserialize(nested_arrays(100000)));
}

TEST(DeserializationLimits, NestedContainersShareTheSameDepthBudget) {
    EXPECT_TRUE(value::deserialize(nested_containers(value::max_deserialization_depth - 1)));
    EXPECT_FALSE(value::deserialize(nested_containers(value::max_deserialization_depth)));
}

TEST(DeserializationLimits, TopLevelContainerRejectsAnInvalidNestedValue) {
    using kcenon::container::thread_safe_container;
    EXPECT_TRUE(thread_safe_container::deserialize(container_for_value(nested_arrays(2))));
    EXPECT_FALSE(thread_safe_container::deserialize(container_for_value(nested_arrays(100000))));
}

TEST(DeserializationLimits, LegacyConstructorCannotBypassTheDepthBudget) {
    auto bytes = nested_arrays(1000);
    bytes.erase(bytes.begin(), bytes.begin() + 5);
    value decoded("legacy", value_types::array_value, bytes);
    EXPECT_EQ(decoded.type(), value_types::null_value);
}

TEST(DeserializationLimits, RejectsImpossibleArrayCountsBeforeAllocation) {
    std::vector<uint8_t> bytes;
    append_u32(bytes, 0);
    bytes.push_back(static_cast<uint8_t>(value_types::array_value));
    append_u32(bytes, std::numeric_limits<uint32_t>::max());
    EXPECT_NO_THROW(EXPECT_FALSE(value::deserialize(bytes)));
}

TEST(DeserializationLimits, RejectedInputsDoNotPoisonLaterDecodes) {
    EXPECT_FALSE(value::deserialize(nested_arrays(1000)));
    const auto bytes = nested_arrays(2);
    auto decoded = value::deserialize(bytes);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->serialize(), bytes);
}

TEST(DeserializationLimits, ConcurrentDecodesHaveIndependentBudgets) {
    std::atomic<bool> valid{true};
    auto reject = [&] {
        for (int iteration = 0; iteration < 20; ++iteration) {
            if (value::deserialize(nested_arrays(1000))) valid = false;
        }
    };
    auto accept = [&] {
        for (int iteration = 0; iteration < 100; ++iteration) {
            if (!value::deserialize(nested_arrays(2))) valid = false;
        }
    };
    std::thread first(reject), second(accept);
    first.join();
    second.join();
    EXPECT_TRUE(valid);
}
