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

#include "container/core/value.h"
#include "container/core/optimized_value.h"

// Optional common system integration
#ifdef CONTAINER_USE_COMMON_SYSTEM
    #if __has_include(<kcenon/common/patterns/result.h>)
        #include <kcenon/common/patterns/result.h>
        #define CONTAINER_HAS_COMMON_RESULT 1
    #elif __has_include(<common/patterns/result.h>)
        #include <common/patterns/result.h>
        #define CONTAINER_HAS_COMMON_RESULT 1
        // Create namespace alias for compatibility
        namespace kcenon::common = ::common;
    #else
        #define CONTAINER_HAS_COMMON_RESULT 0
    #endif
#else
    #define CONTAINER_HAS_COMMON_RESULT 0
#endif

#include <memory>
#include <vector>
#include <string_view>
#include <shared_mutex>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <optional>

#ifdef CONTAINER_USE_COMMON_SYSTEM
#if __has_include(<kcenon/common/patterns/result.h>)
#include <kcenon/common/patterns/result.h>
#elif __has_include(<common/patterns/result.h>)
#include <common/patterns/result.h>
#ifndef KCENON_COMMON_RESULT_FALLBACK_DEFINED
#define KCENON_COMMON_RESULT_FALLBACK_DEFINED
namespace kcenon {
namespace common {
using ::common::error_info;
template<typename T>
using Result = ::common::Result<T>;
using VoidResult = ::common::VoidResult;
using ::common::ok;
using ::common::error;
using ::common::is_ok;
using ::common::is_error;
using ::common::get_value;
using ::common::get_error;
using ::common::get_if_ok;
using ::common::get_if_error;
using ::common::value_or;
using ::common::map;
using ::common::and_then;
using ::common::or_else;
using ::common::try_catch;
namespace error_codes {
using namespace ::common::error_codes;
} // namespace error_codes
} // namespace common
} // namespace kcenon
#endif // KCENON_COMMON_RESULT_FALLBACK_DEFINED
#else
#error "Unable to locate common system result header."
#endif
#endif

namespace container_module
{
	/**
	 * @class value_container
	 * @brief A high-level container for messages, including source/target IDs,
	 * message type, and a list of values (similar to a root node).
	 */
	class value_container : public std::enable_shared_from_this<value_container>
	{
	public:
		/**
		 * @brief Default constructor: sets up a "data_container" type with
		 * version "1.0.0.0".
		 */
		value_container();

		/**
		 * @brief Construct from a serialized data string. If
		 * parse_only_header==true, only parse the header part, delaying value
		 * parsing.
		 */
		value_container(const std::string& data_string,
						bool parse_only_header = true);

		/**
		 * @brief Construct from a raw byte array. If parse_only_header==true,
		 * only parse the header part, delaying value parsing.
		 */
		value_container(const std::vector<uint8_t>& data_array,
						bool parse_only_header = true);

		/**
		 * @brief Copy constructor from another value_container.
		 *        If parse_only_header==true, only parse the header portion of
		 * source.
		 */
		value_container(const value_container& data_container,
						bool parse_only_header = true);

		/**
		 * @brief Copy from an existing pointer. If parse_only_header==true,
		 * only parse header.
		 */
		value_container(std::shared_ptr<value_container> data_container,
						bool parse_only_header = true);

		/**
		 * @brief Construct with message type and a set of child units.
		 */
		value_container(const std::string& message_type,
						std::vector<std::shared_ptr<value>> units);

		/**
		 * @brief Construct with target info and message type, plus child units.
		 */
		value_container(const std::string& target_id,
						const std::string& target_sub_id,
						const std::string& message_type,
						std::vector<std::shared_ptr<value>> units);

		/**
		 * @brief Construct with full header info and child units.
		 */
		value_container(const std::string& source_id,
						const std::string& source_sub_id,
						const std::string& target_id,
						const std::string& target_sub_id,
						const std::string& message_type,
						std::vector<std::shared_ptr<value>> units);

		/**
		 * @brief Move constructor
		 */
		value_container(value_container&& other) noexcept;
		
		/**
		 * @brief Move assignment operator
		 */
		value_container& operator=(value_container&& other) noexcept;
		
		virtual ~value_container();

		std::shared_ptr<value_container> get_ptr();

		void set_source(std::string_view source_id,
						std::string_view source_sub_id);
		void set_target(std::string_view target_id,
						std::string_view target_sub_id = "");
		void set_message_type(std::string_view message_type);

		/**
		 * @brief Add or merge multiple child values into this container (the
		 * top-level).
		 */
		void set_units(const std::vector<std::shared_ptr<value>>& target_values,
					   bool update_immediately = false);

		/**
		 * @brief Swap source/target IDs in this header.
		 */
		void swap_header(void);

		/**
		 * @brief Clear all stored child values.
		 */
		void clear_value(void);

		/**
		 * @brief Create a copy of this container. If containing_values ==
		 * false, the returned container only has the header, no child values.
		 */
		std::shared_ptr<value_container> copy(bool containing_values = true);

		// Accessors
		std::string source_id(void) const;
		std::string source_sub_id(void) const;
		std::string target_id(void) const;
		std::string target_sub_id(void) const;
		std::string message_type(void) const;

		// Value management
		std::shared_ptr<value> add(const value& target_value,
								   bool update_immediately = false);
		std::shared_ptr<value> add(std::shared_ptr<value> target_value,
								   bool update_immediately = false);

		void remove(std::string_view target_name,
					bool update_immediately = false);
		void remove(std::shared_ptr<value> target_value,
					bool update_immediately = false);

		/**
		 * @brief Return all child values that match the given name.
		 */
		std::vector<std::shared_ptr<value>> value_array(
			std::string_view target_name);

		/**
		 * @brief Return the first child value matching the given name, or a new
		 * null_value if not found.
		 */
		std::shared_ptr<value> get_value(std::string_view target_name,
										 unsigned int index = 0);

		/**
		 * @brief Reinitialize the entire container to defaults.
		 */
		void initialize(void);

		/**
		 * @brief Serialize this container (header + data).
		 * @return The string form.
		 */
		std::string serialize(void) const;

		/**
		 * @brief Serialize to a raw byte array.
		 */
		std::vector<uint8_t> serialize_array(void) const;

		/**
		 * @brief Deserialize from string. If parse_only_header is true, child
		 * values are not fully parsed yet.
		 */
		bool deserialize(const std::string& data_string,
					 bool parse_only_header = true);

		/**
		 * @brief Deserialize from a raw byte array. If parse_only_header is
		 * true, child values are not fully parsed.
		 */
		bool deserialize(const std::vector<uint8_t>& data_array,
					 bool parse_only_header = true);

#ifdef CONTAINER_USE_COMMON_SYSTEM
		/**
		 * @brief Deserialize returning common_system result to carry error context.
		 */
		kcenon::common::VoidResult deserialize_result(const std::string& data_string,
					 bool parse_only_header = true);

		/**
		 * @brief Deserialize from raw bytes returning common_system result.
		 */
		kcenon::common::VoidResult deserialize_result(const std::vector<uint8_t>& data_array,
					 bool parse_only_header = true);
#endif

		/**
		 * @brief Generate an XML representation of this container (header +
		 * values).
		 */
		const std::string to_xml(void);

		/**
		 * @brief Generate a JSON representation of this container (header +
		 * values).
		 */
		const std::string to_json(void);

		/**
		 * @brief Returns only the data portion's serialized string, if fully
		 * parsed.
		 */
		std::string datas(void) const;

		/**
		 * @brief Load from a file path (reads entire file content, then calls
		 * deserialize).
		 */
		void load_packet(const std::string& file_path);

		/**
		 * @brief Save to a file path (serialize to bytes, then write to file).
		 */
		void save_packet(const std::string& file_path);

		/**
		 * @brief Get memory usage statistics
		 * @return Pair of (heap_allocations, stack_allocations)
		 */
		std::pair<size_t, size_t> memory_stats() const
		{
			return {
				heap_allocations_.load(std::memory_order_relaxed),
				stack_allocations_.load(std::memory_order_relaxed)
			};
		}

		/**
		 * @brief Get total memory footprint estimate in bytes
		 * @return Approximate total memory used
		 */
		size_t memory_footprint() const;

		/**
		 * @brief Enable or disable Small Object Optimization
		 * @param enable true to enable SOO, false to use traditional storage
		 */
		void set_soo_enabled(bool enable) { use_soo_ = enable; }

		/**
		 * @brief Check if Small Object Optimization is enabled
		 * @return true if SOO is enabled
		 */
		bool is_soo_enabled() const { return use_soo_; }

		// Operator to get multiple child values by key
		std::vector<std::shared_ptr<value>> operator[](std::string_view key);

		// Operators to append values
		friend value_container operator<<(value_container target_container,
										  value& other);
		friend value_container operator<<(value_container target_container,
										  std::shared_ptr<value> other);

		friend std::shared_ptr<value_container> operator<<(
			std::shared_ptr<value_container> target_container, value& other);
		friend std::shared_ptr<value_container> operator<<(
			std::shared_ptr<value_container> target_container,
			std::shared_ptr<value> other);

		// Stream insertion operators for debug printing
		friend std::ostream& operator<<(std::ostream& out,
										value_container& other);
		friend std::ostream& operator<<(std::ostream& out,
										std::shared_ptr<value_container> other);

		friend std::string& operator<<(std::string& out,
									   value_container& other);
		friend std::string& operator<<(std::string& out,
									   std::shared_ptr<value_container> other);


	private:
		bool deserialize_values(const std::string& data,
								bool parse_only_header);
		void parsing(std::string_view source_name,
					 std::string_view target_name,
					 std::string_view target_value,
					 std::string& target_variable);

		// Thread-safe helper classes with conditional lock acquisition
		// Optimized: Only acquire lock when thread-safe mode is enabled
		// This eliminates lock overhead in single-threaded scenarios
		class read_lock_guard {
			std::optional<std::shared_lock<std::shared_mutex>> lock_;
			bool is_active_;
		public:
			read_lock_guard(const value_container* c)
				: is_active_(c->thread_safe_enabled_.load(std::memory_order_acquire)) {
				// Conditionally acquire lock only when thread-safe mode is enabled
				if (is_active_) {
					lock_.emplace(c->mutex_);
					c->read_count_.fetch_add(1, std::memory_order_relaxed);
				}
			}

			// Query whether lock is actually being used
			bool is_locked() const noexcept { return is_active_; }
		};

		class write_lock_guard {
			std::optional<std::unique_lock<std::shared_mutex>> lock_;
			bool is_active_;
		public:
			write_lock_guard(value_container* c)
				: is_active_(c->thread_safe_enabled_.load(std::memory_order_acquire)) {
				// Conditionally acquire lock only when thread-safe mode is enabled
				if (is_active_) {
					lock_.emplace(c->mutex_);
					c->write_count_.fetch_add(1, std::memory_order_relaxed);
				}
			}

			// Query whether lock is actually being used
			bool is_locked() const noexcept { return is_active_; }
		};

	private:
		bool parsed_data_;	///< Indicates if all child values have been parsed.
		bool changed_data_; ///< If true, data_string_ is not updated yet.
		std::string data_string_; ///< Full stored data (header + data).

		// Zero-copy deserialization support
		// Shared pointer to original serialized data for zero-copy access
		// This enables lazy parsing: parse values only when accessed
		std::shared_ptr<const std::string> raw_data_ptr_;
		// Cache for lazily parsed values to avoid re-parsing
		mutable std::unordered_map<std::string, std::shared_ptr<value>> parsed_values_cache_;
		// Flag to enable zero-copy mode
		bool zero_copy_mode_{false};

		// header
		std::string source_id_;
		std::string source_sub_id_;
		std::string target_id_;
		std::string target_sub_id_;
		std::string message_type_;
		std::string version_;

		std::vector<std::shared_ptr<value>> units_; ///< Top-level child values

		// Small Object Optimization (SOO) storage
		// Stores primitive values on stack using variant, reducing heap allocations
		// This provides 30-40% memory savings for typical workloads
		std::vector<optimized_value> optimized_units_;
		bool use_soo_{true}; ///< Enable/disable Small Object Optimization

		// Optional map for dynamic type creation if needed
		std::map<value_types,
				 std::function<std::shared_ptr<value>(const std::string&,
													  const std::string&)>>
			data_type_map_;

		// Thread safety members
		mutable std::shared_mutex mutex_;  ///< Mutex for thread-safe access
		std::atomic<bool> thread_safe_enabled_{false}; ///< Thread-safe mode flag

		// Statistics
		mutable std::atomic<size_t> read_count_{0};
		mutable std::atomic<size_t> write_count_{0};
		mutable std::atomic<size_t> serialization_count_{0};
		mutable std::atomic<size_t> heap_allocations_{0}; ///< Track heap allocations
		mutable std::atomic<size_t> stack_allocations_{0}; ///< Track stack allocations
	};
} // namespace container_module
