// BSD 3-Clause License
//
// High-level typed container helper for SIMD-friendly batches.

#pragma once

#include "container/core/concepts.h"
#include <type_traits>
#include <vector>
#include <cstddef>
#include <utility>
#include <concepts>

namespace container_module::core {

/**
 * @brief Lightweight container enforcing trivially copyable payloads.
 *
 * Designed for SIMD optimized serialization paths where deterministic layout is required.
 *
 * @tparam TValue Type that must satisfy the TriviallyCopyable concept
 */
template<container_module::concepts::TriviallyCopyable TValue>
class typed_container {

public:
    explicit typed_container(std::size_t reserve = 0) {
        values_.reserve(reserve);
    }

    void push(const TValue& value) {
        values_.push_back(value);
    }

    void push(TValue&& value) {
        values_.push_back(std::move(value));
    }

    [[nodiscard]] const std::vector<TValue>& values() const noexcept {
        return values_;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return values_.size();
    }

    void clear() {
        values_.clear();
    }

private:
    std::vector<TValue> values_;
};

} // namespace container_module::core
