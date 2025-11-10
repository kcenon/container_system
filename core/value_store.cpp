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

#include "container/core/value_store.h"
#include <stdexcept>

namespace container_module {

void value_store::add(const std::string& key, value val) {
    if (thread_safe_enabled_.load(std::memory_order_relaxed)) {
        std::unique_lock lock(mutex_);
        values_[key] = std::move(val);
    } else {
        values_[key] = std::move(val);
    }
    write_count_.fetch_add(1, std::memory_order_relaxed);
}

std::optional<value> value_store::get(const std::string& key) const {
    if (thread_safe_enabled_.load(std::memory_order_relaxed)) {
        std::shared_lock lock(mutex_);
        auto it = values_.find(key);
        if (it != values_.end()) {
            read_count_.fetch_add(1, std::memory_order_relaxed);
            return it->second;
        }
    } else {
        auto it = values_.find(key);
        if (it != values_.end()) {
            read_count_.fetch_add(1, std::memory_order_relaxed);
            return it->second;
        }
    }
    return std::nullopt;
}

bool value_store::contains(const std::string& key) const {
    if (thread_safe_enabled_.load(std::memory_order_relaxed)) {
        std::shared_lock lock(mutex_);
        return values_.find(key) != values_.end();
    }
    return values_.find(key) != values_.end();
}

bool value_store::remove(const std::string& key) {
    if (thread_safe_enabled_.load(std::memory_order_relaxed)) {
        std::unique_lock lock(mutex_);
        auto it = values_.find(key);
        if (it != values_.end()) {
            values_.erase(it);
            return true;
        }
        return false;
    }

    auto it = values_.find(key);
    if (it != values_.end()) {
        values_.erase(it);
        return true;
    }
    return false;
}

void value_store::clear() {
    if (thread_safe_enabled_.load(std::memory_order_relaxed)) {
        std::unique_lock lock(mutex_);
        values_.clear();
    } else {
        values_.clear();
    }
}

size_t value_store::size() const {
    if (thread_safe_enabled_.load(std::memory_order_relaxed)) {
        std::shared_lock lock(mutex_);
        return values_.size();
    }
    return values_.size();
}

bool value_store::empty() const {
    return size() == 0;
}

std::string value_store::serialize() const {
    // TODO: Implement JSON serialization in Sprint 3 Phase 2
    // For now, return placeholder
    throw std::runtime_error("value_store::serialize() not yet implemented - Sprint 3 Phase 2");
}

std::vector<uint8_t> value_store::serialize_binary() const {
    // TODO: Implement binary serialization in Sprint 3 Phase 2
    throw std::runtime_error("value_store::serialize_binary() not yet implemented - Sprint 3 Phase 2");
}

value_store value_store::deserialize(std::string_view json_data) {
    // TODO: Implement JSON deserialization in Sprint 3 Phase 2
    throw std::runtime_error("value_store::deserialize() not yet implemented - Sprint 3 Phase 2");
}

value_store value_store::deserialize_binary(const std::vector<uint8_t>& binary_data) {
    // TODO: Implement binary deserialization in Sprint 3 Phase 2
    throw std::runtime_error("value_store::deserialize_binary() not yet implemented - Sprint 3 Phase 2");
}

void value_store::enable_thread_safety() {
    thread_safe_enabled_.store(true, std::memory_order_release);
}

void value_store::disable_thread_safety() {
    thread_safe_enabled_.store(false, std::memory_order_release);
}

bool value_store::is_thread_safe() const {
    return thread_safe_enabled_.load(std::memory_order_acquire);
}

size_t value_store::get_read_count() const {
    return read_count_.load(std::memory_order_relaxed);
}

size_t value_store::get_write_count() const {
    return write_count_.load(std::memory_order_relaxed);
}

void value_store::reset_statistics() {
    read_count_.store(0, std::memory_order_relaxed);
    write_count_.store(0, std::memory_order_relaxed);
}

} // namespace container_module
