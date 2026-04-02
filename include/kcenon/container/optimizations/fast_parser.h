// BSD 3-Clause License
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

#pragma once

/**
 * @file fast_parser.h
 * @brief Fast-path parsing utilities and configuration for the container system.
 *
 * @code
 * // Configure the fast parser and reserve container capacity
 * kcenon::container::parser_config config;
 * config.use_fast_path = true;
 * config.initial_capacity = 1024;
 *
 * std::vector<int> data;
 * kcenon::container::reserve_if_possible(data, config.initial_capacity);
 * @endcode
 */

namespace kcenon::container {

/**
 * @brief Reserve capacity on a container if the reserve() method is available.
 * @tparam Container A container type that may or may not support reserve().
 * @param c The container to reserve capacity on.
 * @param size The number of elements to reserve space for.
 */
template<typename Container>
inline void reserve_if_possible(Container& c, size_t size) {
    if constexpr (requires { c.reserve(size); }) {
        c.reserve(size);
    }
}

/**
 * @struct parser_config
 * @brief Configuration parameters for the fast parser.
 */
struct parser_config {
    bool use_fast_path{true};       ///< Whether to use the optimized fast parsing path.
    size_t initial_capacity{256};   ///< Initial buffer capacity in bytes.
};

} // namespace kcenon::container
