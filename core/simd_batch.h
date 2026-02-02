// BSD 3-Clause License
//
// High-level SIMD-friendly batch container for trivially copyable types.
// Renamed from typed_container.h for clarity (Issue #328).

#pragma once

#include "concepts.h"
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
 * This class was renamed from `typed_container` for clarity - the name `simd_batch`
 * better reflects its purpose as a batch container for SIMD operations.
 *
 * @tparam TValue Type that must satisfy the TriviallyCopyable concept
 *
 * @code
 * simd_batch<float> batch;
 * batch.push(1.0f);
 * batch.push(2.0f);
 * // Use batch.values() for SIMD processing
 * @endcode
 *
 * @see Issue #320, #328 for the rename rationale
 */
template<container_module::concepts::TriviallyCopyable TValue>
class simd_batch {

public:
    using value_type = TValue;

    explicit simd_batch(std::size_t reserve = 0) {
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

/**
 * @brief Deprecated alias for simd_batch
 * @deprecated Use simd_batch instead. This alias will be removed in the next major version.
 */
template<container_module::concepts::TriviallyCopyable TValue>
using typed_container [[deprecated("Use simd_batch instead. See Issue #328.")]] = simd_batch<TValue>;

} // namespace container_module::core
