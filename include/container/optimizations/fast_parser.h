#pragma once
namespace container_module {
template<typename Container>
inline void reserve_if_possible(Container& c, size_t size) {
    if constexpr (requires { c.reserve(size); }) {
        c.reserve(size);
    }
}
struct parser_config {
    bool use_fast_path{true};
    size_t initial_capacity{256};
};
} // namespace
