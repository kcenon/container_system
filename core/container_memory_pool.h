// BSD 3-Clause License
//
// Container memory pool backed by common_system object pool utilities.

#pragma once

#include "container/core/value_pool.h"
#include "container/core/optimized_value.h"

namespace container_module::core {

class container_memory_pool {
public:
    static container_memory_pool& instance() {
        static container_memory_pool pool;
        return pool;
    }

    template<typename... Args>
    std::shared_ptr<optimized_value> acquire(Args&&... args) {
        return value_pool<optimized_value>::instance().allocate(std::forward<Args>(args)...);
    }

    void reserve(std::size_t capacity) {
        value_pool<optimized_value>::instance().reserve(capacity);
    }

    pool_stats stats() const {
        auto [hits, misses, available] = value_pool<optimized_value>::instance().stats();
        return pool_stats(hits, misses, available);
    }

private:
    container_memory_pool() = default;
};

} // namespace container_module::core
