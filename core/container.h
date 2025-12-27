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
 * @file core/container.h
 * @brief Core container implementation
 *
 * @deprecated Direct inclusion of this header is deprecated.
 *             Please use the canonical path:
 *             #include <kcenon/container/container.h>
 *             This direct include path will be removed in v0.5.0.0.
 */

#pragma once

// =============================================================================
// DEPRECATION WARNING
// =============================================================================
// Direct inclusion of core/container.h is deprecated.
// Please migrate to the canonical include path:
//     #include <kcenon/container/container.h>
// This path will be removed in v0.5.0.0.
// =============================================================================
#ifndef CONTAINER_INTERNAL_INCLUDE
    #if defined(__GNUC__) || defined(__clang__)
        #pragma GCC warning "Deprecated: Use <kcenon/container/container.h> instead. Direct core/ includes will be removed in v0.5.0.0."
    #elif defined(_MSC_VER)
        #pragma message("warning: Deprecated: Use <kcenon/container/container.h> instead. Direct core/ includes will be removed in v0.5.0.0.")
    #endif
#endif

#include "container/core/value_types.h"
#include "container/core/typed_container.h"
#include "container/internal/value.h"

#include <variant>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <cstdio>

namespace container_module
{
	// Forward declaration
	class value_container;

	/**
	 * @brief Small Object Optimization (SOO) for value storage
	 *
	 * This variant-based storage allows small primitive values to be stored
	 * on the stack rather than heap-allocated, significantly reducing memory
	 * overhead and improving cache locality.
	 */
	using value_variant = std::variant<
		std::monostate,                          // null_value (0 bytes)
		bool,                                    // bool_value (1 byte)
		short,                                   // short_value (2 bytes)
		unsigned short,                          // ushort_value (2 bytes)
		int,                                     // int_value (4 bytes)
		unsigned int,                            // uint_value (4 bytes)
		long,                                    // long_value (4/8 bytes)
		unsigned long,                           // ulong_value (4/8 bytes)
		long long,                               // llong_value (8 bytes)
		unsigned long long,                      // ullong_value (8 bytes)
		float,                                   // float_value (4 bytes)
		double,                                  // double_value (8 bytes)
		std::string,                             // string_value (dynamic)
		std::vector<uint8_t>,                    // bytes_value (dynamic)
		std::shared_ptr<value_container>         // container_value (pointer only)
	>;

	/**
	 * @brief Optimized value storage with Small Object Optimization
	 */
	struct optimized_value
	{
		std::string name;              ///< Value identifier
		value_types type;              ///< Type enumeration
		value_variant data;            ///< Variant storage (stack-allocated for primitives)

		optimized_value()
			: name("")
			, type(value_types::null_value)
			, data(std::monostate{})
		{}

		optimized_value(const std::string& n, value_types t)
			: name(n)
			, type(t)
			, data(std::monostate{})
		{}

		size_t memory_footprint() const
		{
			size_t base = sizeof(optimized_value);
			base += name.capacity();
			if (std::holds_alternative<std::string>(data)) {
				base += std::get<std::string>(data).capacity();
			} else if (std::holds_alternative<std::vector<uint8_t>>(data)) {
				base += std::get<std::vector<uint8_t>>(data).capacity();
			}
			return base;
		}

		bool is_stack_allocated() const
		{
			return type != value_types::string_value &&
				   type != value_types::bytes_value &&
				   type != value_types::container_value;
		}
	};

	/**
	 * @brief Pool statistics structure (stub for API compatibility)
	 */
	struct pool_stats
	{
		size_t hits{0};
		size_t misses{0};
		size_t available{0};
		double hit_rate{0.0};

		pool_stats() = default;
		pool_stats(size_t h, size_t m, size_t a)
			: hits(h), misses(m), available(a)
			, hit_rate(h + m > 0 ? static_cast<double>(h) / (h + m) : 0.0)
		{}
	};

	/**
	 * @brief Helper functions for variant value manipulation
	 */
	namespace variant_helpers
	{
		/**
		 * @brief Escape a string for JSON output per RFC 8259
		 * @param input Raw string
		 * @return JSON-escaped string (without surrounding quotes)
		 */
		inline std::string json_escape(std::string_view input)
		{
			std::string result;
			result.reserve(input.size() + input.size() / 8);

			for (char c : input)
			{
				switch (c)
				{
				case '"':
					result += "\\\"";
					break;
				case '\\':
					result += "\\\\";
					break;
				case '\b':
					result += "\\b";
					break;
				case '\f':
					result += "\\f";
					break;
				case '\n':
					result += "\\n";
					break;
				case '\r':
					result += "\\r";
					break;
				case '\t':
					result += "\\t";
					break;
				default:
					if (static_cast<unsigned char>(c) < 0x20)
					{
						// Control character - use \uXXXX format
						char buf[8];
						std::snprintf(buf, sizeof(buf), "\\u%04x",
									  static_cast<unsigned char>(c));
						result += buf;
					}
					else
					{
						result += c;
					}
					break;
				}
			}

			return result;
		}

		/**
		 * @brief Convert value_variant to string representation
		 */
		inline std::string to_string(const value_variant& var, value_types type)
		{
			switch (type)
			{
			case value_types::null_value:
				return "";
			case value_types::bool_value:
				return std::get<bool>(var) ? "true" : "false";
			case value_types::short_value:
				return std::to_string(std::get<short>(var));
			case value_types::ushort_value:
				return std::to_string(std::get<unsigned short>(var));
			case value_types::int_value:
				return std::to_string(std::get<int>(var));
			case value_types::uint_value:
				return std::to_string(std::get<unsigned int>(var));
			case value_types::long_value:
				return std::to_string(std::get<long>(var));
			case value_types::ulong_value:
				return std::to_string(std::get<unsigned long>(var));
			case value_types::llong_value:
				return std::to_string(std::get<long long>(var));
			case value_types::ullong_value:
				return std::to_string(std::get<unsigned long long>(var));
			case value_types::float_value:
				return std::to_string(std::get<float>(var));
			case value_types::double_value:
				return std::to_string(std::get<double>(var));
			case value_types::string_value:
				return std::get<std::string>(var);
			default:
				return "";
			}
		}

		/**
		 * @brief Get size in bytes of variant data
		 */
		inline size_t data_size(const value_variant& var, value_types type)
		{
			switch (type)
			{
			case value_types::null_value:
				return 0;
			case value_types::bool_value:
				return sizeof(bool);
			case value_types::short_value:
				return sizeof(short);
			case value_types::ushort_value:
				return sizeof(unsigned short);
			case value_types::int_value:
				return sizeof(int);
			case value_types::uint_value:
				return sizeof(unsigned int);
			case value_types::long_value:
				return sizeof(long);
			case value_types::ulong_value:
				return sizeof(unsigned long);
			case value_types::llong_value:
				return sizeof(long long);
			case value_types::ullong_value:
				return sizeof(unsigned long long);
			case value_types::float_value:
				return sizeof(float);
			case value_types::double_value:
				return sizeof(double);
			case value_types::string_value:
				return std::get<std::string>(var).size();
			case value_types::bytes_value:
				return std::get<std::vector<uint8_t>>(var).size();
			default:
				return 0;
			}
		}
	}

} // namespace container_module

// Include feature flags for unified macro detection
#if __has_include(<kcenon/common/config/feature_flags.h>)
    #include <kcenon/common/config/feature_flags.h>
#endif

// Optional common system integration (using unified KCENON_HAS_COMMON_SYSTEM flag)
#if KCENON_HAS_COMMON_SYSTEM
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

#if KCENON_HAS_COMMON_SYSTEM
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
// Import common system error codes as namespace alias (not 'using namespace')
namespace error_codes = ::common::error_codes;
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

		// Legacy constructors with value vectors removed - use optimized_value instead

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

		// Legacy set_units method removed - use optimized_units_ directly via iterators

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
		/**
		 * @exception_safety No-throw guarantee
		 */
		std::string source_id(void) const noexcept;
		std::string source_sub_id(void) const noexcept;
		std::string target_id(void) const noexcept;
		std::string target_sub_id(void) const noexcept;
		std::string message_type(void) const noexcept;

		// Value management methods

		/**
		 * @brief Add a value to the container
		 * @param name Value name/key
		 * @param type Value type enumeration
		 * @param data Value data as variant
		 * @exception_safety Strong guarantee - no changes on exception
		 * @throws std::bad_alloc if memory allocation fails
		 */
		void add_value(const std::string& name, value_types type, value_variant data);

		/**
		 * @brief Add a value to the container (template version for type deduction)
		 * @param name Value name/key
		 * @param value Value of any supported type in value_variant
		 * @exception_safety Strong guarantee - no changes on exception
		 * @throws std::bad_alloc if memory allocation fails
		 */
		template<typename T>
		void add_value(const std::string& name, T&& data_val) {
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
		 * @brief Legacy compatibility: Add a value object to the container
		 * @param val Shared pointer to value object
		 * @deprecated Use set_value() or add_value() instead
		 */
		[[deprecated("Use set_value() or add_value() instead. "
		             "See docs/advanced/VARIANT_VALUE_V2_MIGRATION_GUIDE.md")]]
		void add(std::shared_ptr<value> val);

		// =======================================================================
		// MIGRATE-002: variant_value_v2 Support API
		// =======================================================================

		/**
		 * @brief Set a single optimized_value, updating if key exists
		 * @param val The optimized_value to set
		 * @exception_safety Strong guarantee - no changes on exception
		 */
		void set_unit(const optimized_value& val);

		/**
		 * @brief Set multiple optimized_values, updating existing keys
		 * @param vals Vector of optimized_values to set
		 * @exception_safety Strong guarantee - no changes on exception
		 */
		void set_units(const std::vector<optimized_value>& vals);

		/**
		 * @brief Convenience method to set a typed value by key
		 * @param key The value key/name
		 * @param data_val The value to store
		 * @exception_safety Strong guarantee - no changes on exception
		 */
		template<typename T>
		void set_value(const std::string& key, T&& data_val) {
			optimized_value val;
			val.name = key;
			val.data = std::forward<T>(data_val);
			val.type = static_cast<value_types>(val.data.index());
			set_unit(val);
		}

		/**
		 * @brief Get a value as optimized_value (alias for get_value)
		 * @param key Value name/key to search for
		 * @return Optional containing the optimized_value if found
		 */
		std::optional<optimized_value> get_variant_value(const std::string& key) const noexcept;

		/**
		 * @brief Get all values as optimized_value vector
		 * @return Vector of all optimized_values in the container
		 */
		std::vector<optimized_value> get_variant_values() const;

		/**
		 * @brief Check if container is in variant mode
		 * @return Always true - container uses variant-based storage
		 */
		bool is_variant_mode() const noexcept { return true; }

		/**
		 * @brief Enable variant mode (no-op, always enabled)
		 * @param enable Ignored parameter for API compatibility
		 */
		void enable_variant_mode(bool enable = true) { (void)enable; }

		// =======================================================================

		/**
		 * @brief Get a value by name
		 * @param name Value name/key to search for
		 * @return Optional containing the optimized_value if found, nullopt otherwise
		 * @exception_safety No-throw guarantee
		 */
		std::optional<optimized_value> get_value(const std::string& name) const noexcept;

		/**
		 * @brief Remove a value by name
		 * @param target_name Name of value to remove
		 * @param update_immediately Whether to update serialized data immediately
		 */
		void remove(std::string_view target_name,
					bool update_immediately = false);

		/**
		 * @brief Reinitialize the entire container to defaults.
		 */
		void initialize(void);

		/**
		 * @brief Serialize this container (header + data).
		 * @return The string form.
		 * @exception_safety Strong guarantee - no changes on exception
		 * @throws std::bad_alloc if memory allocation fails
		 * @throws std::runtime_error if value cannot be serialized
		 */
		std::string serialize(void) const;

		/**
		 * @brief Serialize to a raw byte array.
		 * @exception_safety Strong guarantee - no changes on exception
		 * @throws std::bad_alloc if memory allocation fails
		 * @throws std::runtime_error if value cannot be serialized
		 */
		std::vector<uint8_t> serialize_array(void) const;

		/**
		 * @brief Deserialize from string. If parse_only_header is true, child
		 * values are not fully parsed yet.
		 * @exception_safety Basic guarantee - container may be partially modified
		 * @return true on success, false on parse error (no exceptions thrown)
		 */
		bool deserialize(const std::string& data_string,
					 bool parse_only_header = true);

		/**
		 * @brief Deserialize from a raw byte array. If parse_only_header is
		 * true, child values are not fully parsed.
		 * @exception_safety Basic guarantee - container may be partially modified
		 * @return true on success, false on parse error (no exceptions thrown)
		 */
		bool deserialize(const std::vector<uint8_t>& data_array,
					 bool parse_only_header = true);

#if KCENON_HAS_COMMON_SYSTEM
		/**
		 * @brief Deserialize returning common_system result to carry error context.
		 * @exception_safety Strong guarantee - no changes on error
		 * @return Result object containing success/error (no exceptions thrown)
		 */
		kcenon::common::VoidResult deserialize_result(const std::string& data_string,
					 bool parse_only_header = true) noexcept;

		/**
		 * @brief Deserialize from raw bytes returning common_system result.
		 * @exception_safety Strong guarantee - no changes on error
		 * @return Result object containing success/error (no exceptions thrown)
		 */
		kcenon::common::VoidResult deserialize_result(const std::vector<uint8_t>& data_array,
					 bool parse_only_header = true) noexcept;
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

		/**
		 * @brief Get memory pool statistics
		 * @return Pool statistics structure
		 */
		static pool_stats get_pool_stats();

		/**
		 * @brief Clear memory pool (for testing/benchmarking)
		 */
		static void clear_pool();

		// Legacy value-based operators removed - use iterators for value access

		// Stream insertion operators for debug printing
		friend std::ostream& operator<<(std::ostream& out,
										value_container& other);
		friend std::ostream& operator<<(std::ostream& out,
										std::shared_ptr<value_container> other);

		friend std::string& operator<<(std::string& out,
									   value_container& other);
		friend std::string& operator<<(std::string& out,
									   std::shared_ptr<value_container> other);

	/**
	 * @name STL Iterator Support
	 * @brief Provides Standard Library-compatible iteration over container values
	 *
	 * Enables range-based for loops and STL algorithm usage:
	 * @code
	 *   value_container container;
	 *   for (const auto& val : container) {
	 *       // Process each optimized_value
	 *   }
	 *
	 *   auto it = std::find_if(container.begin(), container.end(),
	 *       [](const auto& v) { return v.type == value_types::int_value; });
	 * @endcode
	 * @{
	 */

	/** @brief Iterator type for non-const traversal */
	using iterator = std::vector<optimized_value>::iterator;

	/** @brief Iterator type for const traversal */
	using const_iterator = std::vector<optimized_value>::const_iterator;

	/** @brief Returns iterator to beginning of values */
	iterator begin() { return optimized_units_.begin(); }

	/** @brief Returns iterator to end of values */
	iterator end() { return optimized_units_.end(); }

	/** @brief Returns const iterator to beginning of values */
	const_iterator begin() const { return optimized_units_.begin(); }

	/** @brief Returns const iterator to end of values */
	const_iterator end() const { return optimized_units_.end(); }

	/** @brief Returns const iterator to beginning of values */
	const_iterator cbegin() const { return optimized_units_.cbegin(); }

	/** @brief Returns const iterator to end of values */
	const_iterator cend() const { return optimized_units_.cend(); }

	/** @brief Returns number of values in container */
	size_t size() const { return optimized_units_.size(); }

	/** @brief Returns whether container is empty */
	bool empty() const { return optimized_units_.empty(); }

	/** @} */

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
		// Legacy parsed_values_cache_ removed - using optimized_units_ only
		// Flag to enable zero-copy mode
		bool zero_copy_mode_{false};

		// header
		std::string source_id_;
		std::string source_sub_id_;
		std::string target_id_;
		std::string target_sub_id_;
		std::string message_type_;
		std::string version_;

		// Optimized value storage using variant (stack allocation for primitives)
		// This provides 30-40% memory savings compared to heap-allocated polymorphic values
		std::vector<optimized_value> optimized_units_;
		bool use_soo_{true}; ///< Enable/disable Small Object Optimization

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
