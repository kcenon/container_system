/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#pragma once

// Internal implementation file - suppress deprecation warnings
#define CONTAINER_INTERNAL_INCLUDE
#include "core/container.h"
#undef CONTAINER_INTERNAL_INCLUDE
#include "core/concepts.h"
#include <functional>
#include <memory>
#include <concepts>

#ifdef HAS_PERFORMANCE_METRICS
#include <chrono>
#include <atomic>
#include <mutex>
#include <unordered_map>
#endif

namespace container_module::integration {

/**
 * @brief Container integration manager for messaging systems
 *
 * Provides enhanced functionality for container operations in messaging environments,
 * including performance monitoring, optimization hints, and external system integration.
 */
class messaging_integration {
public:
    /**
     * @brief Container creation with messaging optimization
     */
    static std::shared_ptr<value_container> create_optimized_container(
        const std::string& message_type = "data_container"
    );

    /**
     * @brief High-performance serialization for messaging
     */
    static std::string serialize_for_messaging(
        const std::shared_ptr<value_container>& container,
        bool compress = false
    );

    /**
     * @brief Optimized deserialization for messaging
     */
    static std::shared_ptr<value_container> deserialize_from_messaging(
        const std::string& data,
        bool decompress = false
    );

#ifdef HAS_PERFORMANCE_METRICS
    /**
     * @brief Performance metrics collection
     */
    struct metrics {
        std::atomic<uint64_t> containers_created{0};
        std::atomic<uint64_t> serializations_performed{0};
        std::atomic<uint64_t> deserializations_performed{0};
        std::atomic<uint64_t> total_serialize_time_us{0};
        std::atomic<uint64_t> total_deserialize_time_us{0};
    };

    static metrics& get_metrics();
    static void reset_metrics();
    static std::string get_metrics_summary();
#endif

#ifdef HAS_EXTERNAL_INTEGRATION
    /**
     * @brief External system callback registration
     */
    using container_callback_t = std::function<void(const std::shared_ptr<value_container>&)>;

    static void register_creation_callback(container_callback_t callback);
    static void register_serialization_callback(container_callback_t callback);
    static void unregister_callbacks();
#endif

private:
#ifdef HAS_PERFORMANCE_METRICS
    static metrics metrics_;
#endif

#ifdef HAS_EXTERNAL_INTEGRATION
    static std::vector<container_callback_t> creation_callbacks_;
    static std::vector<container_callback_t> serialization_callbacks_;
    static std::mutex callback_mutex_;
#endif
};

/**
 * @brief Builder pattern for messaging containers
 */
class messaging_container_builder {
public:
    messaging_container_builder();

    messaging_container_builder& source(const std::string& id, const std::string& sub_id = "");
    messaging_container_builder& target(const std::string& id, const std::string& sub_id = "");
    messaging_container_builder& message_type(const std::string& type);

    /**
     * @brief Set a value by key (unified API)
     * @param key The value key/name
     * @param value The value to store
     * @return Reference to this builder for method chaining
     */
    template<typename T>
    messaging_container_builder& set(const std::string& key, T&& value);

    messaging_container_builder& optimize_for_size();
    messaging_container_builder& optimize_for_speed();

    std::shared_ptr<value_container> build();

private:
    std::shared_ptr<value_container> container_;
    bool size_optimized_ = false;
    bool speed_optimized_ = false;
};

#ifdef HAS_PERFORMANCE_METRICS
/**
 * @brief RAII container performance monitor
 */
class container_performance_monitor {
public:
    explicit container_performance_monitor(const std::string& operation_name);
    ~container_performance_monitor();

    void set_container_size(size_t size);
    void set_result_size(size_t size);

private:
    std::string operation_name_;
    std::chrono::high_resolution_clock::time_point start_time_;
    size_t container_size_ = 0;
    size_t result_size_ = 0;
};
#endif // HAS_PERFORMANCE_METRICS

// Template implementation for set (unified API)
// Uses concepts to provide clearer error messages for unsupported types
template<typename T>
messaging_container_builder& messaging_container_builder::set(const std::string& key, T&& value) {
    using DecayedT = std::decay_t<T>;

    if constexpr (std::is_same_v<DecayedT, std::shared_ptr<value_container>>) {
        // Handle nested containers by serializing and storing as bytes
        auto result = value->serialize_string(value_container::serialization_format::binary);
        if (result.is_ok()) {
            std::string serialized_data = result.value();
            std::vector<uint8_t> bytes(serialized_data.begin(), serialized_data.end());
            container_->set(key, bytes);
        }
    } else if constexpr (std::is_same_v<DecayedT, bool>) {
        container_->set(key, value);
    } else if constexpr (concepts::IntegralType<DecayedT>) {
        if constexpr (sizeof(DecayedT) <= 4) {
            container_->set(key, static_cast<int32_t>(value));
        } else {
            container_->set(key, static_cast<int64_t>(value));
        }
    } else if constexpr (concepts::FloatingPointType<DecayedT>) {
        if constexpr (std::is_same_v<DecayedT, float>) {
            container_->set(key, value);
        } else {
            container_->set(key, value);
        }
    } else if constexpr (std::is_same_v<DecayedT, std::string>) {
        container_->set(key, std::string(value));
    } else if constexpr (concepts::StringLike<DecayedT>) {
        container_->set(key, std::string(value));
    }

    return *this;
}

} // namespace container_module::integration

// Convenience macros for performance monitoring
#ifdef HAS_PERFORMANCE_METRICS
#define CONTAINER_PERF_MONITOR(name) \
    container_module::integration::container_performance_monitor _monitor(name)

#define CONTAINER_PERF_SET_SIZE(size) \
    _monitor.set_container_size(size)

#define CONTAINER_PERF_SET_RESULT(size) \
    _monitor.set_result_size(size)
#else
#define CONTAINER_PERF_MONITOR(name)
#define CONTAINER_PERF_SET_SIZE(size)
#define CONTAINER_PERF_SET_RESULT(size)
#endif