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

/**
 * @file core/container/legacy_api.h
 * @brief Legacy API declarations and inline implementations
 *
 * This header contains deprecated value management and serialization methods
 * that are maintained for backward compatibility. New code should use the
 * unified API instead.
 *
 * ## Migration Guide
 *
 * ### Value Management
 *
 * | Deprecated Method | Replacement |
 * |-------------------|-------------|
 * | `add_value(name, type, data)` | `set(key, value)` |
 * | `add_value<T>(name, data)` | `set(key, value)` |
 * | `add(shared_ptr<value>)` | `set(optimized_value)` |
 * | `set_unit(val)` | `set(val)` |
 * | `set_units(vals)` | `set_all(vals)` |
 * | `set_value<T>(key, val)` | `set(key, val)` |
 * | `remove(name)` | `remove_result(name)` |
 *
 * ### Serialization
 *
 * | Deprecated Method | Replacement |
 * |-------------------|-------------|
 * | `serialize()` | `serialize_result()` or `serialize(format)` |
 * | `serialize_array()` | `serialize_array_result()` or `serialize(format)` |
 * | `deserialize(string, bool)` | `deserialize_result()` or `deserialize(span)` |
 * | `deserialize(vector, bool)` | `deserialize_result()` or `deserialize(span)` |
 * | `to_xml()` | `to_xml_result()` or `serialize_string(xml)` |
 * | `to_json()` | `to_json_result()` or `serialize_string(json)` |
 * | `to_msgpack()` | `to_msgpack_result()` or `serialize(msgpack)` |
 * | `from_msgpack(data)` | `from_msgpack_result()` or `deserialize(data, msgpack)` |
 *
 * ### File I/O
 *
 * | Deprecated Method | Replacement |
 * |-------------------|-------------|
 * | `load_packet(path)` | `load_packet_result(path)` |
 * | `save_packet(path)` | `save_packet_result(path)` |
 *
 * ### Schema-Validated Deserialization
 *
 * | Deprecated Method | Replacement |
 * |-------------------|-------------|
 * | `deserialize(string, schema, bool)` | `deserialize_result(string, schema, bool)` |
 * | `deserialize(vector, schema, bool)` | `deserialize_result(vector, schema, bool)` |
 *
 * ## Disabling Legacy API
 *
 * Define `CONTAINER_NO_LEGACY_API` before including container headers to
 * exclude deprecated methods:
 *
 * @code
 * #define CONTAINER_NO_LEGACY_API
 * #include <container/container.h>
 * @endcode
 *
 * This will cause compile-time errors for any usage of deprecated methods,
 * helping identify code that needs migration.
 *
 * ## Deprecation Timeline
 *
 * - **v2.x**: Deprecated methods available by default, warnings on use
 * - **v3.0**: Deprecated methods require explicit include of legacy_api.h
 * - **v4.0**: Deprecated methods removed entirely
 *
 * @see value_container::set() for the unified value setter API
 * @see value_container::set_all() for bulk value operations
 * @see value_container::serialize(serialization_format) for unified serialization
 * @see value_container::deserialize(std::span<const uint8_t>) for unified deserialization
 */

#pragma once

// This header is included by container.h when CONTAINER_NO_LEGACY_API is not defined
// It provides documentation and can hold future extracted implementations

#ifndef CONTAINER_NO_LEGACY_API

namespace container_module
{

/**
 * @defgroup LegacyValueManagement Legacy Value Management API
 * @brief Deprecated methods for value management
 *
 * These methods are deprecated and will be removed in a future version.
 * Use the unified set()/set_all() API instead.
 * @{
 */

/**
 * @}
 */

/**
 * @defgroup LegacySerialization Legacy Serialization API
 * @brief Deprecated methods for serialization and format conversion
 *
 * These methods are deprecated and will be removed in a future version.
 * Use the Result-based API or unified serialization API instead.
 *
 * ## Recommended Alternatives
 *
 * ### For Result-based error handling:
 * - `serialize()` -> `serialize_result()`
 * - `serialize_array()` -> `serialize_array_result()`
 * - `deserialize()` -> `deserialize_result()`
 * - `to_xml()` -> `to_xml_result()`
 * - `to_json()` -> `to_json_result()`
 * - `to_msgpack()` -> `to_msgpack_result()`
 * - `from_msgpack()` -> `from_msgpack_result()`
 * - `load_packet()` -> `load_packet_result()`
 * - `save_packet()` -> `save_packet_result()`
 *
 * ### For unified format-agnostic API:
 * - `serialize(serialization_format::json)` - serialize to any format
 * - `serialize_string(serialization_format::json)` - serialize to string
 * - `deserialize(data)` - auto-detect format
 * - `deserialize(data, serialization_format::msgpack)` - explicit format
 *
 * @{
 */

/**
 * @}
 */

/**
 * @defgroup LegacyFileIO Legacy File I/O API
 * @brief Deprecated methods for file operations
 *
 * These methods are deprecated and will be removed in a future version.
 * Use the Result-based API instead:
 * - `load_packet()` -> `load_packet_result()`
 * - `save_packet()` -> `save_packet_result()`
 * @{
 */

/**
 * @}
 */

} // namespace container_module

// =============================================================================
// Inline Template Implementations for Legacy API
// =============================================================================
// These implementations are separated from the main container.h for clarity.
// They are automatically included when CONTAINER_NO_LEGACY_API is not defined.

namespace container_module
{

/**
 * @brief Add a value to the container (template version for type deduction)
 * @param name Value name/key
 * @param data_val Value of any supported type in value_variant
 * @exception_safety Strong guarantee - no changes on exception
 * @throws std::bad_alloc if memory allocation fails
 * @deprecated Use set() instead for unified API
 */
template<typename T>
void value_container::add_value(const std::string& name, T&& data_val) {
	write_lock_guard lock(this);

	optimized_value val;
	val.name = name;
	val.data = std::forward<T>(data_val);
	val.type = static_cast<value_types>(val.data.index());

	optimized_units_.push_back(std::move(val));
	changed_data_ = true;

	if (use_soo_ && val.is_stack_allocated()) {
		stack_allocations_.fetch_add(1, std::memory_order_relaxed);
	} else {
		heap_allocations_.fetch_add(1, std::memory_order_relaxed);
	}
}

/**
 * @brief Convenience method to set a typed value by key
 * @param key The value key/name
 * @param data_val The value to store
 * @exception_safety Strong guarantee - no changes on exception
 * @deprecated Use set() instead for unified API
 */
template<typename T>
void value_container::set_value(const std::string& key, T&& data_val) {
	optimized_value val;
	val.name = key;
	val.data = std::forward<T>(data_val);
	val.type = static_cast<value_types>(val.data.index());
	set_unit_impl(val);
}

} // namespace container_module

#endif // CONTAINER_NO_LEGACY_API
