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

// Internal implementation file - suppress deprecation warnings
#define CONTAINER_INTERNAL_INCLUDE
#include "container/core/container.h"
#undef CONTAINER_INTERNAL_INCLUDE

#include "utilities/core/formatter.h"
#include "utilities/core/convert_string.h"

#include "container/core/value_types.h"
#include "container/internal/value.h"
#include "container/internal/pool_allocator.h"
// Legacy value includes removed - using variant-based storage only

#include <fcntl.h>
#include <wchar.h>

#include <regex>
#include <sstream>
#include <fstream>
#include <unordered_set>

namespace container_module
{
	// Use integer constants instead of string_view to ensure cross-platform compatibility
	// with fmt library formatting. Linux fmt has issues with string_view arguments.
	inline constexpr int TARGET_ID = 1;
	inline constexpr int TARGET_SUB_ID = 2;
	inline constexpr int SOURCE_ID = 3;
	inline constexpr int SOURCE_SUB_ID = 4;
	inline constexpr int MESSAGE_TYPE = 5;
	inline constexpr int MESSAGE_VERSION = 6;

	using namespace utility_module;

	value_container::value_container()
		: parsed_data_(true)
		, changed_data_(false)
		, data_string_("@data={{}};")
		, source_id_("")
		, source_sub_id_("")
		, target_id_("")
		, target_sub_id_("")
		, message_type_("data_container")
		, version_("1.0.0.0")
	{
	}

	value_container::value_container(const std::string& data_str,
									 bool parse_only_header)
		: value_container()
	{
		// Enable zero-copy mode: store shared pointer to original data
		// This allows lazy parsing and reduces memory copies
		if (parse_only_header)
		{
			raw_data_ptr_ = std::make_shared<const std::string>(data_str);
			zero_copy_mode_ = true;
		}
		deserialize(data_str, parse_only_header);
	}

	value_container::value_container(const std::vector<uint8_t>& data_array,
									 bool parse_only_header)
		: value_container()
	{
		// Enable zero-copy mode: convert to string and store shared pointer
		if (parse_only_header)
		{
			std::string data_str(data_array.begin(), data_array.end());
			raw_data_ptr_ = std::make_shared<const std::string>(std::move(data_str));
			zero_copy_mode_ = true;
		}
		deserialize(data_array, parse_only_header);
	}

	value_container::value_container(const value_container& other,
									 bool parse_only_header)
		: value_container()
	{
		if (metrics_manager::is_enabled())
		{
			metrics_manager::get().operations.copies.fetch_add(1, std::memory_order_relaxed);
		}
		deserialize(other.serialize(), parse_only_header);
	}

	value_container::value_container(std::shared_ptr<value_container> other,
									 bool parse_only_header)
		: value_container()
	{
		if (other)
		{
			deserialize(other->serialize(), parse_only_header);
		}
	}

	// Legacy constructors with value vector parameters removed

	value_container::value_container(value_container&& other) noexcept
		: parsed_data_(other.parsed_data_)
		, changed_data_(other.changed_data_)
		, data_string_(std::move(other.data_string_))
		, source_id_(std::move(other.source_id_))
		, source_sub_id_(std::move(other.source_sub_id_))
		, target_id_(std::move(other.target_id_))
		, target_sub_id_(std::move(other.target_sub_id_))
		, message_type_(std::move(other.message_type_))
		, version_(std::move(other.version_))
		// Legacy units_ and data_type_map_ removed
	{
		if (metrics_manager::is_enabled())
		{
			metrics_manager::get().operations.moves.fetch_add(1, std::memory_order_relaxed);
		}
		// Reset other to a valid state
		other.parsed_data_ = true;
		other.changed_data_ = false;
	}
	
	value_container& value_container::operator=(value_container&& other) noexcept
	{
		if (this != &other)
		{
			source_id_ = std::move(other.source_id_);
			source_sub_id_ = std::move(other.source_sub_id_);
			target_id_ = std::move(other.target_id_);
			target_sub_id_ = std::move(other.target_sub_id_);
			message_type_ = std::move(other.message_type_);
			version_ = std::move(other.version_);
			parsed_data_ = other.parsed_data_;
			changed_data_ = other.changed_data_;
			data_string_ = std::move(other.data_string_);
		optimized_units_ = std::move(other.optimized_units_);
		use_soo_ = other.use_soo_;
			
			// Reset other to a valid state
			other.parsed_data_ = true;
			other.changed_data_ = false;
		}
		return *this;
	}

	value_container::~value_container() {}

	std::shared_ptr<value_container> value_container::get_ptr()
	{
		return shared_from_this();
	}

	void value_container::set_source(std::string_view sid,
									 std::string_view ssubid)
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);
		source_id_ = std::string(sid);
		source_sub_id_ = std::string(ssubid);
	}

	void value_container::set_target(std::string_view tid,
									 std::string_view tsubid)
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);
		target_id_ = std::string(tid);
		target_sub_id_ = std::string(tsubid);
	}

	void value_container::set_message_type(std::string_view msg_type)
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);
		message_type_ = std::string(msg_type);
	}


	void value_container::swap_header(void)
	{
		std::swap(source_id_, target_id_);
		std::swap(source_sub_id_, target_sub_id_);
	}

	void value_container::clear_value(void)
	{
		parsed_data_ = true;
		changed_data_ = false;
		data_string_ = "@data={{}};";
		optimized_units_.clear();
	}

	std::shared_ptr<value_container> value_container::copy(
		bool containing_values)
	{
		auto newC = std::make_shared<value_container>(serialize(),
													  !containing_values);
		if (!containing_values && newC)
		{
			newC->clear_value();
		}
		return newC;
	}

	std::string value_container::source_id(void) const noexcept {
		std::shared_lock<std::shared_mutex> lock(mutex_);
		return source_id_;
	}

	std::string value_container::source_sub_id(void) const noexcept
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);
		return source_sub_id_;
	}

	std::string value_container::target_id(void) const noexcept {
		std::shared_lock<std::shared_mutex> lock(mutex_);
		return target_id_;
	}

	std::string value_container::target_sub_id(void) const noexcept
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);
		return target_sub_id_;
	}

	std::string value_container::message_type(void) const noexcept
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);
		return message_type_;
	}

	void value_container::add_value(const std::string& name, value_types type, value_variant data)
	{
		write_lock_guard lock(this);

		optimized_value val;
		val.name = name;
		val.type = type;
		val.data = std::move(data);

		optimized_units_.push_back(std::move(val));
		changed_data_ = true;

		if (use_soo_ && val.is_stack_allocated()) {
			stack_allocations_.fetch_add(1, std::memory_order_relaxed);
		} else {
			heap_allocations_.fetch_add(1, std::memory_order_relaxed);
		}
	}

	void value_container::add(std::shared_ptr<value> val)
	{
		if (!val) {
			return;
		}

		std::string name_str = std::string(val->name());

		// Handle only types compatible with value_variant
		val->visit([this, &name_str](const auto& data) {
			using T = std::decay_t<decltype(data)>;
			if constexpr (std::is_same_v<T, std::monostate>) {
				// Skip monostate
			} else if constexpr (std::is_same_v<T, bool> ||
								 std::is_same_v<T, int16_t> ||
								 std::is_same_v<T, uint16_t> ||
								 std::is_same_v<T, int32_t> ||
								 std::is_same_v<T, uint32_t> ||
								 std::is_same_v<T, int64_t> ||
								 std::is_same_v<T, uint64_t> ||
								 std::is_same_v<T, float> ||
								 std::is_same_v<T, double> ||
								 std::is_same_v<T, std::string> ||
								 std::is_same_v<T, std::vector<uint8_t>>) {
				this->add_value(name_str, data);
			}
			// Skip incompatible types: thread_safe_container, array_variant
		});
	}

	std::optional<optimized_value> value_container::get_value(const std::string& name) const noexcept
	{
		// Record metrics if enabled
		auto timer = metrics_manager::make_timer(
			metrics_manager::get().read_latency,
			&metrics_manager::get().timing.total_read_ns);
		if (metrics_manager::is_enabled())
		{
			metrics_manager::get().operations.reads.fetch_add(1, std::memory_order_relaxed);
		}

		read_lock_guard lock(this);

		for (const auto& val : optimized_units_) {
			if (val.name == name) {
				return val;
			}
		}
		return std::nullopt;
	}

	// =======================================================================
	// MIGRATE-002: variant_value_v2 Support API Implementation
	// =======================================================================

	// =======================================================================
	// Internal implementation method
	// =======================================================================

	void value_container::set_unit_impl(const optimized_value& val)
	{
		// Record metrics if enabled
		auto timer = metrics_manager::make_timer(
			metrics_manager::get().write_latency,
			&metrics_manager::get().timing.total_write_ns);
		if (metrics_manager::is_enabled())
		{
			metrics_manager::get().operations.writes.fetch_add(1, std::memory_order_relaxed);
		}

		write_lock_guard lock(this);

		// Check if key already exists
		for (auto& existing : optimized_units_) {
			if (existing.name == val.name) {
				existing = val;
				changed_data_ = true;
				return;
			}
		}

		// Key doesn't exist, add new value
		optimized_units_.push_back(val);
		changed_data_ = true;

		if (use_soo_ && val.is_stack_allocated()) {
			stack_allocations_.fetch_add(1, std::memory_order_relaxed);
		} else {
			heap_allocations_.fetch_add(1, std::memory_order_relaxed);
		}
	}

	// =======================================================================
	// Deprecated methods (delegating to implementation)
	// =======================================================================

	void value_container::set_unit(const optimized_value& val)
	{
		set_unit_impl(val);
	}

	void value_container::set_units(const std::vector<optimized_value>& vals)
	{
		for (const auto& val : vals) {
			set_unit_impl(val);
		}
	}

	// =======================================================================
	// Unified Value Setter API (Issue #207)
	// =======================================================================

	value_container& value_container::set(const optimized_value& val)
	{
		set_unit_impl(val);
		return *this;
	}

	value_container& value_container::set_all(std::span<const optimized_value> vals)
	{
		for (const auto& val : vals) {
			set_unit_impl(val);
		}
		return *this;
	}

	bool value_container::contains(std::string_view key) const noexcept
	{
		read_lock_guard lock(this);

		for (const auto& val : optimized_units_) {
			if (val.name == key) {
				return true;
			}
		}
		return false;
	}

	// =======================================================================
	// Batch Operation APIs (Issue #229)
	// =======================================================================

	value_container& value_container::bulk_insert(std::vector<optimized_value>&& values)
	{
		if (values.empty()) {
			return *this;
		}

		write_lock_guard lock(this);

		// Pre-allocate for efficiency
		optimized_units_.reserve(optimized_units_.size() + values.size());

		// Move all values
		for (auto& val : values) {
			if (use_soo_ && val.is_stack_allocated()) {
				stack_allocations_.fetch_add(1, std::memory_order_relaxed);
			} else {
				heap_allocations_.fetch_add(1, std::memory_order_relaxed);
			}
			optimized_units_.push_back(std::move(val));
		}

		changed_data_ = true;
		return *this;
	}

	value_container& value_container::bulk_insert(
		std::span<const optimized_value> values,
		size_t reserve_hint)
	{
		if (values.empty()) {
			return *this;
		}

		write_lock_guard lock(this);

		// Apply reserve hint or use values.size()
		size_t reserve_size = reserve_hint > 0 ? reserve_hint : values.size();
		optimized_units_.reserve(optimized_units_.size() + reserve_size);

		// Copy all values
		for (const auto& val : values) {
			if (use_soo_ && val.is_stack_allocated()) {
				stack_allocations_.fetch_add(1, std::memory_order_relaxed);
			} else {
				heap_allocations_.fetch_add(1, std::memory_order_relaxed);
			}
			optimized_units_.push_back(val);
		}

		changed_data_ = true;
		return *this;
	}

	std::vector<std::optional<optimized_value>> value_container::get_batch(
		std::span<const std::string_view> keys) const noexcept
	{
		std::vector<std::optional<optimized_value>> results;
		results.reserve(keys.size());

		read_lock_guard lock(this);

		for (const auto& key : keys) {
			std::optional<optimized_value> found = std::nullopt;
			for (const auto& val : optimized_units_) {
				if (val.name == key) {
					found = val;
					break;
				}
			}
			results.push_back(std::move(found));
		}

		return results;
	}

	std::unordered_map<std::string, optimized_value> value_container::get_batch_map(
		std::span<const std::string_view> keys) const
	{
		std::unordered_map<std::string, optimized_value> results;
		results.reserve(keys.size());

		read_lock_guard lock(this);

		for (const auto& key : keys) {
			for (const auto& val : optimized_units_) {
				if (val.name == key) {
					results[std::string(key)] = val;
					break;
				}
			}
		}

		return results;
	}

	std::vector<bool> value_container::contains_batch(
		std::span<const std::string_view> keys) const noexcept
	{
		std::vector<bool> results;
		results.reserve(keys.size());

		read_lock_guard lock(this);

		for (const auto& key : keys) {
			bool found = false;
			for (const auto& val : optimized_units_) {
				if (val.name == key) {
					found = true;
					break;
				}
			}
			results.push_back(found);
		}

		return results;
	}

	size_t value_container::remove_batch(std::span<const std::string_view> keys)
	{
		if (keys.empty()) {
			return 0;
		}

		write_lock_guard lock(this);

		if (!parsed_data_) {
			deserialize_values(data_string_, false);
		}

		// Build a set of keys for O(1) lookup
		std::unordered_set<std::string_view> key_set(keys.begin(), keys.end());

		// Count and remove matching elements
		size_t original_size = optimized_units_.size();
		optimized_units_.erase(
			std::remove_if(optimized_units_.begin(), optimized_units_.end(),
						   [&key_set](const optimized_value& ov) {
							   return key_set.find(ov.name) != key_set.end();
						   }),
			optimized_units_.end());

		size_t removed = original_size - optimized_units_.size();
		if (removed > 0) {
			changed_data_ = true;
		}

		return removed;
	}

	bool value_container::update_if(
		std::string_view key,
		const value_variant& expected,
		value_variant&& new_value)
	{
		write_lock_guard lock(this);

		for (auto& val : optimized_units_) {
			if (val.name == key) {
				// Compare current value with expected
				if (val.data == expected) {
					val.data = std::move(new_value);
					val.type = static_cast<value_types>(val.data.index());
					changed_data_ = true;
					return true;
				}
				return false;
			}
		}

		return false;
	}

	std::vector<bool> value_container::update_batch_if(
		std::span<const update_spec> updates)
	{
		std::vector<bool> results;
		results.reserve(updates.size());

		if (updates.empty()) {
			return results;
		}

		write_lock_guard lock(this);

		for (const auto& update : updates) {
			bool updated = false;
			for (auto& val : optimized_units_) {
				if (val.name == update.key) {
					if (val.data == update.expected) {
						val.data = update.new_value;
						val.type = static_cast<value_types>(val.data.index());
						changed_data_ = true;
						updated = true;
					}
					break;
				}
			}
			results.push_back(updated);
		}

		return results;
	}

	std::optional<optimized_value> value_container::get_variant_value(const std::string& key) const noexcept
	{
		return get_value(key);
	}

	std::vector<optimized_value> value_container::get_variant_values() const
	{
		read_lock_guard lock(this);
		return optimized_units_;
	}

	// =======================================================================
	// Zero-Copy Deserialization Implementation (Issue #226)
	// =======================================================================

	std::optional<value_view> value_container::get_view(std::string_view key) const noexcept
	{
		read_lock_guard lock(this);

		// Zero-copy mode requires raw_data_ptr_ to be valid
		if (!zero_copy_mode_ || !raw_data_ptr_) {
			return std::nullopt;
		}

		// Build index if not already built
		if (!index_built_) {
			build_index();
		}

		// Search in the index
		if (value_index_) {
			for (const auto& entry : *value_index_) {
				if (entry.name == key) {
					const char* data_ptr = raw_data_ptr_->data();
					return value_view(
						entry.name.data(), entry.name.size(),
						data_ptr + entry.value_offset, entry.value_length,
						entry.type
					);
				}
			}
		}

		return std::nullopt;
	}

	void value_container::ensure_index_built() const
	{
		read_lock_guard lock(this);
		if (!index_built_) {
			build_index();
		}
	}

	void value_container::build_index() const
	{
		if (index_built_ || !raw_data_ptr_) {
			return;
		}

		value_index_ = std::vector<value_index_entry>();
		const std::string& data = *raw_data_ptr_;

		// Find the @data section
		// Match both single and double braces for @data section
		std::regex reData("@data=\\s*\\{\\{?\\s*(.*?)\\s*\\}\\}?;");
		std::smatch dataMatch;
		if (!std::regex_search(data, dataMatch, reData)) {
			index_built_ = true;
			return;
		}

		// Get the position of the data section in the original string
		size_t dataStart = static_cast<size_t>(dataMatch.position(1));
		std::string dataInside = dataMatch[1].str();

		// Parse items: [name,type,data];
		std::regex reItems("\\[(\\w+),\\s*(\\w+),\\s*(.*?)\\];");
		auto it = std::sregex_iterator(dataInside.begin(), dataInside.end(), reItems);
		auto end = std::sregex_iterator();

		for (; it != end; ++it) {
			value_index_entry entry;

			// Calculate offsets into the original raw data
			size_t matchPos = static_cast<size_t>(it->position());
			size_t nameStart = dataStart + matchPos + 1; // +1 for '['
			size_t nameLen = (*it)[1].length();

			// Store name as string_view into raw data
			entry.name = std::string_view(data.data() + nameStart, nameLen);

			// Parse type
			std::string typeStr = (*it)[2].str();
			entry.type = convert_value_type(typeStr);

			// Calculate value offset and length
			// Value starts after [name,type,
			size_t valueStart = dataStart + matchPos + 1 + nameLen + 1 + (*it)[2].length() + 1;
			// +1 for '[', +1 for first ',', +1 for second ','
			entry.value_offset = valueStart;
			entry.value_length = (*it)[3].length();

			value_index_->push_back(entry);
		}

		index_built_ = true;
	}

	// =======================================================================

	void value_container::remove(std::string_view target_name,
								 bool update_immediately)
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);
		
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}
		bool found = true;
		while (found)
		{
			found = false;
		auto it = std::find_if(optimized_units_.begin(), optimized_units_.end(),
							   [&target_name](const optimized_value& ov)
							   { return (ov.name == target_name); });
		if (it != optimized_units_.end())
		{
			optimized_units_.erase(it);
				found = true;
			}
		}
		changed_data_ = !update_immediately;
		if (update_immediately)
		{
			data_string_ = datas();
		}
	}




	void value_container::initialize(void)
	{
		source_id_.clear();
		source_sub_id_.clear();
		target_id_.clear();
		target_sub_id_.clear();
		message_type_ = "data_container";
		version_ = "1.0";

		clear_value();
	}

	std::string value_container::serialize(void) const
	{
		// Record metrics if enabled
		auto timer = metrics_manager::make_timer(
			metrics_manager::get().serialize_latency,
			&metrics_manager::get().timing.total_serialize_ns);
		if (metrics_manager::is_enabled())
		{
			metrics_manager::get().operations.serializations.fetch_add(1, std::memory_order_relaxed);
		}

		std::shared_lock<std::shared_mutex> lock(mutex_);

		// If everything parsed, just rebuild data
		std::string ds = (parsed_data_ ? datas() : data_string_);

		// Compose header with pre-allocated buffer to reduce reallocations
		// Estimate: header is typically ~150 bytes, reserve more to be safe
		std::string result;
		result.reserve(200 + ds.size());

		// Note: In fmt library, {{}} produces single {}, so {{{{}}}} produces {{}}
		formatter::format_to(std::back_inserter(result), "@header={{{{");

		if (message_type_ != "data_container")
		{
			formatter::format_to(std::back_inserter(result), "[{},{}];",
								 TARGET_ID, target_id_);
			formatter::format_to(std::back_inserter(result), "[{},{}];",
								 TARGET_SUB_ID, target_sub_id_);
			formatter::format_to(std::back_inserter(result), "[{},{}];",
								 SOURCE_ID, source_id_);
			formatter::format_to(std::back_inserter(result), "[{},{}];",
								 SOURCE_SUB_ID, source_sub_id_);
		}
		formatter::format_to(std::back_inserter(result), "[{},{}];",
							 MESSAGE_TYPE, message_type_);
		formatter::format_to(std::back_inserter(result), "[{},{}];",
							 MESSAGE_VERSION, version_);
		formatter::format_to(std::back_inserter(result), "}}}};");

		// Append data section directly (avoids string concatenation copy)
		result.append(ds);

		return result;
	}

	std::vector<uint8_t> value_container::serialize_array(void) const
	{
		auto [arr, err] = convert_string::to_array(serialize());
		if (!err.empty())
		{
			return {};
		}
		return arr;
	}

	bool value_container::deserialize(const std::string& data_str,
									  bool parse_only_header)
	{
		// Record metrics if enabled
		auto timer = metrics_manager::make_timer(
			metrics_manager::get().deserialize_latency,
			&metrics_manager::get().timing.total_deserialize_ns);
		if (metrics_manager::is_enabled())
		{
			metrics_manager::get().operations.deserializations.fetch_add(1, std::memory_order_relaxed);
		}

		initialize();
		if (data_str.empty())
			return false;

		// Remove newlines
		std::regex newlineRe("\\r\\n?|\\n");
		std::string clean = std::regex_replace(data_str, newlineRe, "");

		// parse header portion - matches both single and double braces
		std::regex fullRe("@header=\\s*\\{\\{?\\s*(.*?)\\s*\\}\\}?;");
		std::smatch match;
		if (!std::regex_search(clean, match, fullRe))
		{
			// No header => parse as data only
			return deserialize_values(clean, parse_only_header);
		}
		// match[1] => inside the header
		std::string headerInside = match[1].str();
		std::regex pairRe("\\[(\\w+),(.*?)\\];");
		auto it = std::sregex_iterator(headerInside.begin(), headerInside.end(),
									   pairRe);
		auto end = std::sregex_iterator();
		for (; it != end; ++it)
		{
			parsing((*it)[1].str(), std::to_string(TARGET_ID), (*it)[2].str(), target_id_);
			parsing((*it)[1].str(), std::to_string(TARGET_SUB_ID), (*it)[2].str(), target_sub_id_);
			parsing((*it)[1].str(), std::to_string(SOURCE_ID), (*it)[2].str(), source_id_);
			parsing((*it)[1].str(), std::to_string(SOURCE_SUB_ID), (*it)[2].str(), source_sub_id_);
			parsing((*it)[1].str(), std::to_string(MESSAGE_TYPE), (*it)[2].str(), message_type_);
			parsing((*it)[1].str(), std::to_string(MESSAGE_VERSION), (*it)[2].str(), version_);
		}

		return deserialize_values(clean, parse_only_header);
	}

bool value_container::deserialize(const std::vector<uint8_t>& data_array,
					  bool parse_only_header)
{
	auto [strVal, err] = convert_string::to_string(data_array);
	if (!err.empty())
	{
		return false;
	}
	return deserialize(strVal, parse_only_header);
}

#if KCENON_HAS_COMMON_SYSTEM
	kcenon::common::VoidResult value_container::deserialize_result(
		const std::string& data_str,
		bool parse_only_header) noexcept
	{
		if (deserialize(data_str, parse_only_header))
		{
			return kcenon::common::ok();
		}
		return kcenon::common::VoidResult(
			kcenon::common::error_info{
				error_codes::deserialization_failed,
				error_codes::make_message(error_codes::deserialization_failed, "string data"),
				"container_system"});
	}

	kcenon::common::VoidResult value_container::deserialize_result(
		const std::vector<uint8_t>& data_array,
		bool parse_only_header) noexcept
	{
		if (deserialize(data_array, parse_only_header))
		{
			return kcenon::common::ok();
		}
		return kcenon::common::VoidResult(
			kcenon::common::error_info{
				error_codes::deserialization_failed,
				error_codes::make_message(error_codes::deserialization_failed, "byte array data"),
				"container_system"});
	}

	kcenon::common::Result<optimized_value> value_container::get_result(
		std::string_view key) const noexcept
	{
		read_lock_guard lock(this);

		for (const auto& val : optimized_units_)
		{
			if (val.name == key)
			{
				return kcenon::common::ok(val);
			}
		}

		return kcenon::common::Result<optimized_value>(
			kcenon::common::error_info{
				error_codes::key_not_found,
				error_codes::make_message(error_codes::key_not_found, key),
				"container_system"});
	}

	kcenon::common::VoidResult value_container::set_result(
		const optimized_value& val) noexcept
	{
		try
		{
			if (val.name.empty())
			{
				return kcenon::common::VoidResult(
					kcenon::common::error_info{
						error_codes::empty_key,
						error_codes::make_message(error_codes::empty_key),
						"container_system"});
			}

			set_unit_impl(val);
			return kcenon::common::ok();
		}
		catch (const std::bad_alloc&)
		{
			return kcenon::common::VoidResult(
				kcenon::common::error_info{
					error_codes::memory_allocation_failed,
					error_codes::make_message(error_codes::memory_allocation_failed, val.name),
					"container_system"});
		}
		catch (const std::exception& e)
		{
			return kcenon::common::VoidResult(
				kcenon::common::error_info{
					error_codes::invalid_value,
					std::string("Failed to set value: ") + e.what(),
					"container_system"});
		}
	}

	kcenon::common::VoidResult value_container::set_all_result(
		std::span<const optimized_value> vals) noexcept
	{
		try
		{
			for (const auto& val : vals)
			{
				if (val.name.empty())
				{
					return kcenon::common::VoidResult(
						kcenon::common::error_info{
							error_codes::empty_key,
							error_codes::make_message(error_codes::empty_key),
							"container_system"});
				}
				set_unit_impl(val);
			}
			return kcenon::common::ok();
		}
		catch (const std::bad_alloc&)
		{
			return kcenon::common::VoidResult(
				kcenon::common::error_info{
					error_codes::memory_allocation_failed,
					error_codes::make_message(error_codes::memory_allocation_failed),
					"container_system"});
		}
		catch (const std::exception& e)
		{
			return kcenon::common::VoidResult(
				kcenon::common::error_info{
					error_codes::invalid_value,
					std::string("Failed to set values: ") + e.what(),
					"container_system"});
		}
	}

	kcenon::common::VoidResult value_container::remove_result(
		std::string_view target_name) noexcept
	{
		try
		{
			write_lock_guard lock(this);

			if (!parsed_data_)
			{
				deserialize_values(data_string_, false);
			}

			auto it = std::find_if(optimized_units_.begin(), optimized_units_.end(),
								   [&target_name](const optimized_value& ov)
								   { return (ov.name == target_name); });

			if (it == optimized_units_.end())
			{
				return kcenon::common::VoidResult(
					kcenon::common::error_info{
						error_codes::key_not_found,
						error_codes::make_message(error_codes::key_not_found, target_name),
						"container_system"});
			}

			// Remove all occurrences
			optimized_units_.erase(
				std::remove_if(optimized_units_.begin(), optimized_units_.end(),
							   [&target_name](const optimized_value& ov)
							   { return (ov.name == target_name); }),
				optimized_units_.end());

			changed_data_ = true;
			return kcenon::common::ok();
		}
		catch (const std::exception& e)
		{
			return kcenon::common::VoidResult(
				kcenon::common::error_info{
					error_codes::invalid_value,
					std::string("Failed to remove value: ") + e.what(),
					"container_system"});
		}
	}

	kcenon::common::Result<std::string> value_container::serialize_result() const noexcept
	{
		try
		{
			return kcenon::common::ok(serialize());
		}
		catch (const std::bad_alloc&)
		{
			return kcenon::common::Result<std::string>(
				kcenon::common::error_info{
					error_codes::memory_allocation_failed,
					error_codes::make_message(error_codes::memory_allocation_failed),
					"container_system"});
		}
		catch (const std::exception& e)
		{
			return kcenon::common::Result<std::string>(
				kcenon::common::error_info{
					error_codes::serialization_failed,
					std::string("Serialization failed: ") + e.what(),
					"container_system"});
		}
	}

	kcenon::common::Result<std::vector<uint8_t>> value_container::serialize_array_result() const noexcept
	{
		try
		{
			auto [arr, err] = convert_string::to_array(serialize());
			if (!err.empty())
			{
				return kcenon::common::Result<std::vector<uint8_t>>(
					kcenon::common::error_info{
						error_codes::encoding_error,
						std::string("Encoding error: ") + err,
						"container_system"});
			}
			return kcenon::common::ok(std::move(arr));
		}
		catch (const std::bad_alloc&)
		{
			return kcenon::common::Result<std::vector<uint8_t>>(
				kcenon::common::error_info{
					error_codes::memory_allocation_failed,
					error_codes::make_message(error_codes::memory_allocation_failed),
					"container_system"});
		}
		catch (const std::exception& e)
		{
			return kcenon::common::Result<std::vector<uint8_t>>(
				kcenon::common::error_info{
					error_codes::serialization_failed,
					std::string("Serialization failed: ") + e.what(),
					"container_system"});
		}
	}

	kcenon::common::Result<std::string> value_container::to_json_result() noexcept
	{
		try
		{
			return kcenon::common::ok(to_json());
		}
		catch (const std::bad_alloc&)
		{
			return kcenon::common::Result<std::string>(
				kcenon::common::error_info{
					error_codes::memory_allocation_failed,
					error_codes::make_message(error_codes::memory_allocation_failed),
					"container_system"});
		}
		catch (const std::exception& e)
		{
			return kcenon::common::Result<std::string>(
				kcenon::common::error_info{
					error_codes::serialization_failed,
					std::string("JSON serialization failed: ") + e.what(),
					"container_system"});
		}
	}

	kcenon::common::Result<std::string> value_container::to_xml_result() noexcept
	{
		try
		{
			return kcenon::common::ok(to_xml());
		}
		catch (const std::bad_alloc&)
		{
			return kcenon::common::Result<std::string>(
				kcenon::common::error_info{
					error_codes::memory_allocation_failed,
					error_codes::make_message(error_codes::memory_allocation_failed),
					"container_system"});
		}
		catch (const std::exception& e)
		{
			return kcenon::common::Result<std::string>(
				kcenon::common::error_info{
					error_codes::serialization_failed,
					std::string("XML serialization failed: ") + e.what(),
					"container_system"});
		}
	}

	kcenon::common::VoidResult value_container::load_packet_result(
		const std::string& file_path) noexcept
	{
		try
		{
			std::ifstream file(file_path, std::ios::binary);
			if (!file)
			{
				return kcenon::common::VoidResult(
					kcenon::common::error_info{
						error_codes::file_not_found,
						error_codes::make_message(error_codes::file_not_found, file_path),
						"container_system"});
			}

			std::string content((std::istreambuf_iterator<char>(file)),
								std::istreambuf_iterator<char>());

			if (file.bad())
			{
				return kcenon::common::VoidResult(
					kcenon::common::error_info{
						error_codes::file_read_error,
						error_codes::make_message(error_codes::file_read_error, file_path),
						"container_system"});
			}

			return deserialize_result(content, false);
		}
		catch (const std::bad_alloc&)
		{
			return kcenon::common::VoidResult(
				kcenon::common::error_info{
					error_codes::memory_allocation_failed,
					error_codes::make_message(error_codes::memory_allocation_failed, file_path),
					"container_system"});
		}
		catch (const std::exception& e)
		{
			return kcenon::common::VoidResult(
				kcenon::common::error_info{
					error_codes::file_read_error,
					std::string("File read error: ") + e.what(),
					"container_system"});
		}
	}

	kcenon::common::VoidResult value_container::save_packet_result(
		const std::string& file_path) noexcept
	{
		try
		{
			auto serialize_res = serialize_result();
			if (!serialize_res.is_ok())
			{
				return kcenon::common::VoidResult(serialize_res.error());
			}

			std::ofstream file(file_path, std::ios::binary | std::ios::trunc);
			if (!file)
			{
				return kcenon::common::VoidResult(
					kcenon::common::error_info{
						error_codes::file_write_error,
						error_codes::make_message(error_codes::file_write_error, file_path),
						"container_system"});
			}

			const auto& content = serialize_res.value();
			file.write(content.data(), static_cast<std::streamsize>(content.size()));

			if (file.bad())
			{
				return kcenon::common::VoidResult(
					kcenon::common::error_info{
						error_codes::file_write_error,
						error_codes::make_message(error_codes::file_write_error, file_path),
						"container_system"});
			}

			return kcenon::common::ok();
		}
		catch (const std::bad_alloc&)
		{
			return kcenon::common::VoidResult(
				kcenon::common::error_info{
					error_codes::memory_allocation_failed,
					error_codes::make_message(error_codes::memory_allocation_failed, file_path),
					"container_system"});
		}
		catch (const std::exception& e)
		{
			return kcenon::common::VoidResult(
				kcenon::common::error_info{
					error_codes::file_write_error,
					std::string("File write error: ") + e.what(),
					"container_system"});
		}
	}

	// =======================================================================
	// Batch Operation APIs with Result return type (Issue #229)
	// =======================================================================

	kcenon::common::VoidResult value_container::bulk_insert_result(
		std::vector<optimized_value>&& values) noexcept
	{
		try
		{
			bulk_insert(std::move(values));
			return kcenon::common::ok();
		}
		catch (const std::bad_alloc&)
		{
			return kcenon::common::VoidResult(
				kcenon::common::error_info{
					error_codes::memory_allocation_failed,
					error_codes::make_message(error_codes::memory_allocation_failed),
					"container_system"});
		}
		catch (const std::exception& e)
		{
			return kcenon::common::VoidResult(
				kcenon::common::error_info{
					error_codes::invalid_value,
					std::string("Bulk insert failed: ") + e.what(),
					"container_system"});
		}
	}

	kcenon::common::Result<std::vector<std::optional<optimized_value>>>
		value_container::get_batch_result(
			std::span<const std::string_view> keys) const noexcept
	{
		try
		{
			return kcenon::common::ok(get_batch(keys));
		}
		catch (const std::bad_alloc&)
		{
			return kcenon::common::Result<std::vector<std::optional<optimized_value>>>(
				kcenon::common::error_info{
					error_codes::memory_allocation_failed,
					error_codes::make_message(error_codes::memory_allocation_failed),
					"container_system"});
		}
		catch (const std::exception& e)
		{
			return kcenon::common::Result<std::vector<std::optional<optimized_value>>>(
				kcenon::common::error_info{
					error_codes::invalid_value,
					std::string("Batch get failed: ") + e.what(),
					"container_system"});
		}
	}

	kcenon::common::Result<size_t> value_container::remove_batch_result(
		std::span<const std::string_view> keys) noexcept
	{
		try
		{
			return kcenon::common::ok(remove_batch(keys));
		}
		catch (const std::bad_alloc&)
		{
			return kcenon::common::Result<size_t>(
				kcenon::common::error_info{
					error_codes::memory_allocation_failed,
					error_codes::make_message(error_codes::memory_allocation_failed),
					"container_system"});
		}
		catch (const std::exception& e)
		{
			return kcenon::common::Result<size_t>(
				kcenon::common::error_info{
					error_codes::invalid_value,
					std::string("Batch remove failed: ") + e.what(),
					"container_system"});
		}
	}
#endif

	const std::string value_container::to_xml(void)
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);

		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}
		std::string result;
		formatter::format_to(std::back_inserter(result), "<container>");
		formatter::format_to(std::back_inserter(result), "<header>");
		if (message_type_ != "data_container")
		{
			formatter::format_to(std::back_inserter(result),
								 "<target_id>{}</target_id>",
								 variant_helpers::xml_encode(target_id_));
			formatter::format_to(std::back_inserter(result),
								 "<target_sub_id>{}</target_sub_id>",
								 variant_helpers::xml_encode(target_sub_id_));
			formatter::format_to(std::back_inserter(result),
								 "<source_id>{}</source_id>",
								 variant_helpers::xml_encode(source_id_));
			formatter::format_to(std::back_inserter(result),
								 "<source_sub_id>{}</source_sub_id>",
								 variant_helpers::xml_encode(source_sub_id_));
		}
		formatter::format_to(std::back_inserter(result),
							 "<message_type>{}</message_type>",
							 variant_helpers::xml_encode(message_type_));
		formatter::format_to(std::back_inserter(result),
							 "<version>{}</version>",
							 variant_helpers::xml_encode(version_));
		formatter::format_to(std::back_inserter(result), "</header>");

		formatter::format_to(std::back_inserter(result), "<values>");
		for (auto& u : optimized_units_)
		{
			std::string value_str = variant_helpers::to_string(u.data, u.type);
			formatter::format_to(std::back_inserter(result), "<{}>{}</{}>",
								 u.name, variant_helpers::xml_encode(value_str),
								 u.name);
		}
		formatter::format_to(std::back_inserter(result), "</values>");
		formatter::format_to(std::back_inserter(result), "</container>");
		return result;
	}

	const std::string value_container::to_json(void)
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);

		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}
		std::string result;
		formatter::format_to(std::back_inserter(result), "{{");
		// header
		formatter::format_to(std::back_inserter(result), "\"header\":{{");
		if (message_type_ != "data_container")
		{
			formatter::format_to(std::back_inserter(result),
								 "\"target_id\":\"{}\",",
								 variant_helpers::json_escape(target_id_));
			formatter::format_to(std::back_inserter(result),
								 "\"target_sub_id\":\"{}\",",
								 variant_helpers::json_escape(target_sub_id_));
			formatter::format_to(std::back_inserter(result),
								 "\"source_id\":\"{}\",",
								 variant_helpers::json_escape(source_id_));
			formatter::format_to(std::back_inserter(result),
								 "\"source_sub_id\":\"{}\",",
								 variant_helpers::json_escape(source_sub_id_));
		}
		formatter::format_to(std::back_inserter(result),
							 "\"message_type\":\"{}\"",
							 variant_helpers::json_escape(message_type_));
		formatter::format_to(std::back_inserter(result), ",\"version\":\"{}\"",
							 variant_helpers::json_escape(version_));
		formatter::format_to(std::back_inserter(result),
							 "}},"); // end header

		// values
		formatter::format_to(std::back_inserter(result), "\"values\":{{");
		bool first = true;
	for (auto& u : optimized_units_)
	{
		if (!first)
		{
			formatter::format_to(std::back_inserter(result), ",");
		}
		std::string value_str = variant_helpers::to_string(u.data, u.type);
		std::string escaped_name = variant_helpers::json_escape(u.name);

		// String and bytes values need quotes
		if (u.type == value_types::string_value || u.type == value_types::bytes_value)
		{
			std::string escaped_value = variant_helpers::json_escape(value_str);
			formatter::format_to(std::back_inserter(result), "\"{}\":\"{}\"",
								 escaped_name, escaped_value);
		}
		else
		{
			formatter::format_to(std::back_inserter(result), "\"{}\":{}",
								 escaped_name, value_str);
		}
		first = false;
	}
		formatter::format_to(std::back_inserter(result),
							 "}}"); // end values
		formatter::format_to(std::back_inserter(result), "}}");
		return result;
	}

	// =======================================================================
	// MessagePack Serialization Implementation (Issue #234)
	// =======================================================================

	std::vector<uint8_t> value_container::to_msgpack() const
	{
		// Record metrics if enabled
		auto timer = metrics_manager::make_timer(
			metrics_manager::get().serialize_latency,
			&metrics_manager::get().timing.total_serialize_ns);
		if (metrics_manager::is_enabled())
		{
			metrics_manager::get().operations.serializations.fetch_add(1, std::memory_order_relaxed);
		}

		std::shared_lock<std::shared_mutex> lock(mutex_);

		msgpack_encoder encoder;

		// Estimate buffer size: header (~100 bytes) + values
		encoder.reserve(200 + optimized_units_.size() * 32);

		// MessagePack structure:
		// {
		//   "header": { ... },
		//   "values": { ... }
		// }

		// Write outer map with 2 entries (header + values)
		encoder.write_map_header(2);

		// Write "header" key
		encoder.write_string("header");

		// Calculate header entries count
		size_t header_count = 2; // message_type and version are always present
		if (message_type_ != "data_container")
		{
			header_count += 4; // target_id, target_sub_id, source_id, source_sub_id
		}

		// Write header map
		encoder.write_map_header(header_count);

		if (message_type_ != "data_container")
		{
			encoder.write_string("target_id");
			encoder.write_string(target_id_);

			encoder.write_string("target_sub_id");
			encoder.write_string(target_sub_id_);

			encoder.write_string("source_id");
			encoder.write_string(source_id_);

			encoder.write_string("source_sub_id");
			encoder.write_string(source_sub_id_);
		}

		encoder.write_string("message_type");
		encoder.write_string(message_type_);

		encoder.write_string("version");
		encoder.write_string(version_);

		// Write "values" key
		encoder.write_string("values");

		// Write values map
		encoder.write_map_header(optimized_units_.size());

		for (const auto& unit : optimized_units_)
		{
			// Write key
			encoder.write_string(unit.name);

			// Write value based on type
			switch (unit.type)
			{
			case value_types::null_value:
				encoder.write_nil();
				break;

			case value_types::bool_value:
				encoder.write_bool(std::get<bool>(unit.data));
				break;

			case value_types::short_value:
				encoder.write_int(std::get<short>(unit.data));
				break;

			case value_types::ushort_value:
				encoder.write_uint(std::get<unsigned short>(unit.data));
				break;

			case value_types::int_value:
				encoder.write_int(std::get<int>(unit.data));
				break;

			case value_types::uint_value:
				encoder.write_uint(std::get<unsigned int>(unit.data));
				break;

			case value_types::long_value:
				encoder.write_int(std::get<long>(unit.data));
				break;

			case value_types::ulong_value:
				encoder.write_uint(std::get<unsigned long>(unit.data));
				break;

			case value_types::llong_value:
				encoder.write_int(std::get<long long>(unit.data));
				break;

			case value_types::ullong_value:
				encoder.write_uint(std::get<unsigned long long>(unit.data));
				break;

			case value_types::float_value:
				encoder.write_float(std::get<float>(unit.data));
				break;

			case value_types::double_value:
				encoder.write_double(std::get<double>(unit.data));
				break;

			case value_types::string_value:
				encoder.write_string(std::get<std::string>(unit.data));
				break;

			case value_types::bytes_value:
				encoder.write_binary(std::get<std::vector<uint8_t>>(unit.data));
				break;

			case value_types::container_value:
				{
					// Nested container: serialize recursively
					auto nested = std::get<std::shared_ptr<value_container>>(unit.data);
					if (nested)
					{
						auto nested_data = nested->to_msgpack();
						encoder.write_binary(nested_data);
					}
					else
					{
						encoder.write_nil();
					}
				}
				break;

			case value_types::array_value:
				// Array values are stored as containers
				encoder.write_nil();
				break;

			default:
				encoder.write_nil();
				break;
			}
		}

		return encoder.finish();
	}

	bool value_container::from_msgpack(const std::vector<uint8_t>& data)
	{
		// Record metrics if enabled
		auto timer = metrics_manager::make_timer(
			metrics_manager::get().deserialize_latency,
			&metrics_manager::get().timing.total_deserialize_ns);
		if (metrics_manager::is_enabled())
		{
			metrics_manager::get().operations.deserializations.fetch_add(1, std::memory_order_relaxed);
		}

		if (data.empty())
		{
			return false;
		}

		initialize();

		msgpack_decoder decoder(data);

		// Expect outer map
		auto outer_count = decoder.read_map_header();
		if (!outer_count)
		{
			return false;
		}

		std::unique_lock<std::shared_mutex> lock(mutex_);

		for (size_t i = 0; i < *outer_count; ++i)
		{
			auto key = decoder.read_string();
			if (!key)
			{
				return false;
			}

			if (*key == "header")
			{
				auto header_count = decoder.read_map_header();
				if (!header_count)
				{
					return false;
				}

				for (size_t j = 0; j < *header_count; ++j)
				{
					auto hkey = decoder.read_string();
					auto hval = decoder.read_string();
					if (!hkey || !hval)
					{
						return false;
					}

					if (*hkey == "target_id")
					{
						target_id_ = *hval;
					}
					else if (*hkey == "target_sub_id")
					{
						target_sub_id_ = *hval;
					}
					else if (*hkey == "source_id")
					{
						source_id_ = *hval;
					}
					else if (*hkey == "source_sub_id")
					{
						source_sub_id_ = *hval;
					}
					else if (*hkey == "message_type")
					{
						message_type_ = *hval;
					}
					else if (*hkey == "version")
					{
						version_ = *hval;
					}
				}
			}
			else if (*key == "values")
			{
				auto values_count = decoder.read_map_header();
				if (!values_count)
				{
					return false;
				}

				for (size_t j = 0; j < *values_count; ++j)
				{
					auto vkey = decoder.read_string();
					if (!vkey)
					{
						return false;
					}

					optimized_value val;
					val.name = *vkey;

					// Read value based on type
					auto type = decoder.peek_type();
					switch (type)
					{
					case msgpack_type::nil:
						decoder.read_nil();
						val.type = value_types::null_value;
						val.data = std::monostate{};
						break;

					case msgpack_type::boolean:
						{
							auto b = decoder.read_bool();
							if (!b)
							{
								return false;
							}
							val.type = value_types::bool_value;
							val.data = *b;
						}
						break;

					case msgpack_type::positive_int:
					case msgpack_type::negative_int:
						{
							auto n = decoder.read_int();
							if (!n)
							{
								return false;
							}
							// Determine best type based on value
							if (*n >= 0 && *n <= INT32_MAX)
							{
								val.type = value_types::int_value;
								val.data = static_cast<int>(*n);
							}
							else if (*n >= INT32_MIN && *n < 0)
							{
								val.type = value_types::int_value;
								val.data = static_cast<int>(*n);
							}
							else
							{
								val.type = value_types::llong_value;
								val.data = static_cast<long long>(*n);
							}
						}
						break;

					case msgpack_type::float32:
						{
							auto f = decoder.read_float();
							if (!f)
							{
								return false;
							}
							val.type = value_types::float_value;
							val.data = *f;
						}
						break;

					case msgpack_type::float64:
						{
							auto d = decoder.read_double();
							if (!d)
							{
								return false;
							}
							val.type = value_types::double_value;
							val.data = *d;
						}
						break;

					case msgpack_type::str:
						{
							auto s = decoder.read_string();
							if (!s)
							{
								return false;
							}
							val.type = value_types::string_value;
							val.data = *s;
						}
						break;

					case msgpack_type::bin:
						{
							auto b = decoder.read_binary();
							if (!b)
							{
								return false;
							}
							val.type = value_types::bytes_value;
							val.data = *b;
						}
						break;

					default:
						// Skip unknown types
						val.type = value_types::null_value;
						val.data = std::monostate{};
						break;
					}

					optimized_units_.push_back(std::move(val));
				}
			}
		}

		parsed_data_ = true;
		changed_data_ = false;

		return true;
	}

	std::shared_ptr<value_container> value_container::create_from_msgpack(
		const std::vector<uint8_t>& data)
	{
		auto container = std::make_shared<value_container>();
		if (container->from_msgpack(data))
		{
			return container;
		}
		return nullptr;
	}

	value_container::serialization_format value_container::detect_format(
		const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return serialization_format::unknown;
		}

		// Check for MessagePack format
		// MessagePack maps start with 0x80-0x8f (fixmap) or 0xde (map16) or 0xdf (map32)
		uint8_t first_byte = data[0];
		if ((first_byte >= 0x80 && first_byte <= 0x8f) ||  // fixmap
			first_byte == 0xde ||                           // map16
			first_byte == 0xdf)                             // map32
		{
			return serialization_format::msgpack;
		}

		// Check for text-based formats (convert to string for analysis)
		std::string_view str_view(reinterpret_cast<const char*>(data.data()), data.size());
		return detect_format(str_view);
	}

	value_container::serialization_format value_container::detect_format(
		std::string_view data)
	{
		if (data.empty())
		{
			return serialization_format::unknown;
		}

		// Skip leading whitespace
		size_t pos = 0;
		while (pos < data.size() && std::isspace(static_cast<unsigned char>(data[pos])))
		{
			++pos;
		}

		if (pos >= data.size())
		{
			return serialization_format::unknown;
		}

		// Check first non-whitespace character
		char first_char = data[pos];

		// JSON starts with '{' or '['
		if (first_char == '{' || first_char == '[')
		{
			return serialization_format::json;
		}

		// XML starts with '<'
		if (first_char == '<')
		{
			return serialization_format::xml;
		}

		// Binary format starts with "@header"
		if (data.substr(pos, 7) == "@header")
		{
			return serialization_format::binary;
		}

		// Binary format may also start with "@data"
		if (data.substr(pos, 5) == "@data")
		{
			return serialization_format::binary;
		}

		return serialization_format::unknown;
	}

#if CONTAINER_HAS_COMMON_RESULT
	kcenon::common::Result<std::vector<uint8_t>> value_container::to_msgpack_result() const noexcept
	{
		try
		{
			return kcenon::common::ok(to_msgpack());
		}
		catch (const std::bad_alloc&)
		{
			return kcenon::common::Result<std::vector<uint8_t>>(
				kcenon::common::error_info{
					error_codes::memory_allocation_failed,
					error_codes::make_message(error_codes::memory_allocation_failed),
					"container_system"});
		}
		catch (const std::exception& e)
		{
			return kcenon::common::Result<std::vector<uint8_t>>(
				kcenon::common::error_info{
					error_codes::serialization_failed,
					std::string("MessagePack serialization failed: ") + e.what(),
					"container_system"});
		}
	}

	kcenon::common::VoidResult value_container::from_msgpack_result(
		const std::vector<uint8_t>& data) noexcept
	{
		try
		{
			if (from_msgpack(data))
			{
				return kcenon::common::ok();
			}
			return kcenon::common::VoidResult(
				kcenon::common::error_info{
					error_codes::deserialization_failed,
					error_codes::make_message(error_codes::deserialization_failed, "Invalid MessagePack data"),
					"container_system"});
		}
		catch (const std::bad_alloc&)
		{
			return kcenon::common::VoidResult(
				kcenon::common::error_info{
					error_codes::memory_allocation_failed,
					error_codes::make_message(error_codes::memory_allocation_failed),
					"container_system"});
		}
		catch (const std::exception& e)
		{
			return kcenon::common::VoidResult(
				kcenon::common::error_info{
					error_codes::deserialization_failed,
					std::string("MessagePack deserialization failed: ") + e.what(),
					"container_system"});
		}
	}
#endif

	// =======================================================================

	std::string value_container::datas(void) const
	{
		if (!parsed_data_)
		{
			return data_string_;
		}
		// Rebuild from top-level units
		std::string result;
		formatter::format_to(std::back_inserter(result), "@data={{{{");
	for (auto& u : optimized_units_)
	{
		std::string value_str = variant_helpers::to_string(u.data, u.type);
		std::string type_str = convert_value_type(u.type);
		formatter::format_to(std::back_inserter(result), "[{},{},{}];",
							 u.name, type_str, value_str);
	}
		formatter::format_to(std::back_inserter(result), "}}}};");
		return result;
	}

	void value_container::load_packet(const std::string& file_path)
	{
		// TODO: Implement file loading without file_handler
		// For now, this functionality is disabled
		(void)file_path;
		throw std::runtime_error("File loading not implemented - file_handler not available");
	}

	void value_container::save_packet(const std::string& file_path)
	{
		// TODO: Implement file saving without file_handler
		// For now, this functionality is disabled
		(void)file_path;
		throw std::runtime_error("File saving not implemented - file_handler not available");
	}

	size_t value_container::memory_footprint() const
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);

		size_t total = sizeof(value_container);

		// Add header strings
		total += source_id_.capacity();
		total += source_sub_id_.capacity();
		total += target_id_.capacity();
		total += target_sub_id_.capacity();
		total += message_type_.capacity();
		total += version_.capacity();

		// Add data string
		total += data_string_.capacity();

		// Add traditional value storage (heap-allocated)

		// Add optimized value storage (stack-allocated)
		total += optimized_units_.capacity() * sizeof(optimized_value);
		for (const auto& ov : optimized_units_)
		{
			total += ov.memory_footprint();
		}

		// Add parsed values cache

		return total;
	}

	pool_stats value_container::get_pool_stats()
	{
#if CONTAINER_USE_MEMORY_POOL
		auto& allocator = internal::pool_allocator::instance();
		auto stats = allocator.get_stats();
		auto small_stats = allocator.get_small_pool_stats();
		auto medium_stats = allocator.get_medium_pool_stats();

		return pool_stats(
			stats.pool_hits,
			stats.pool_misses,
			stats.small_pool_allocs,
			stats.medium_pool_allocs,
			stats.deallocations,
			small_stats.free_blocks + medium_stats.free_blocks
		);
#else
		return pool_stats(0, 0, 0);
#endif
	}

	void value_container::clear_pool()
	{
#if CONTAINER_USE_MEMORY_POOL
		internal::pool_allocator::instance().reset_stats();
#endif
	}






	std::ostream& operator<<(std::ostream& out, value_container& other)
	{
		out << other.serialize();
		return out;
	}

	std::ostream& operator<<(std::ostream& out,
							 std::shared_ptr<value_container> other)
	{
		if (other)
			out << other->serialize();
		return out;
	}

	std::string& operator<<(std::string& out, value_container& other)
	{
		out = other.serialize();
		return out;
	}

	std::string& operator<<(std::string& out,
							std::shared_ptr<value_container> other)
	{
		if (other)
			out = other->serialize();
		else
			out.clear();
		return out;
	}

	// =======================================================================
	// Metrics Export Implementation (Issue #230)
	// =======================================================================

	std::string value_container::metrics_to_json() const
	{
		auto& m = metrics_manager::get();

		std::string result;
		result.reserve(2048);  // Pre-allocate for performance

		result += "{\n";
		result += "  \"operations\": {\n";
		result += "    \"reads\": " + std::to_string(m.operations.reads.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"writes\": " + std::to_string(m.operations.writes.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"serializations\": " + std::to_string(m.operations.serializations.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"deserializations\": " + std::to_string(m.operations.deserializations.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"copies\": " + std::to_string(m.operations.copies.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"moves\": " + std::to_string(m.operations.moves.load(std::memory_order_relaxed)) + "\n";
		result += "  },\n";

		result += "  \"timing\": {\n";
		result += "    \"total_serialize_ns\": " + std::to_string(m.timing.total_serialize_ns.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"total_deserialize_ns\": " + std::to_string(m.timing.total_deserialize_ns.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"total_read_ns\": " + std::to_string(m.timing.total_read_ns.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"total_write_ns\": " + std::to_string(m.timing.total_write_ns.load(std::memory_order_relaxed)) + "\n";
		result += "  },\n";

		result += "  \"latency\": {\n";
		result += "    \"serialize\": {\n";
		result += "      \"p50_ns\": " + std::to_string(m.serialize_latency.p50()) + ",\n";
		result += "      \"p95_ns\": " + std::to_string(m.serialize_latency.p95()) + ",\n";
		result += "      \"p99_ns\": " + std::to_string(m.serialize_latency.p99()) + ",\n";
		result += "      \"p999_ns\": " + std::to_string(m.serialize_latency.p999()) + ",\n";
		result += "      \"max_ns\": " + std::to_string(m.serialize_latency.max_ns.load(std::memory_order_relaxed)) + ",\n";
		result += "      \"avg_ns\": " + std::to_string(m.serialize_latency.avg()) + "\n";
		result += "    },\n";
		result += "    \"deserialize\": {\n";
		result += "      \"p50_ns\": " + std::to_string(m.deserialize_latency.p50()) + ",\n";
		result += "      \"p95_ns\": " + std::to_string(m.deserialize_latency.p95()) + ",\n";
		result += "      \"p99_ns\": " + std::to_string(m.deserialize_latency.p99()) + ",\n";
		result += "      \"p999_ns\": " + std::to_string(m.deserialize_latency.p999()) + ",\n";
		result += "      \"max_ns\": " + std::to_string(m.deserialize_latency.max_ns.load(std::memory_order_relaxed)) + ",\n";
		result += "      \"avg_ns\": " + std::to_string(m.deserialize_latency.avg()) + "\n";
		result += "    },\n";
		result += "    \"read\": {\n";
		result += "      \"p50_ns\": " + std::to_string(m.read_latency.p50()) + ",\n";
		result += "      \"p95_ns\": " + std::to_string(m.read_latency.p95()) + ",\n";
		result += "      \"p99_ns\": " + std::to_string(m.read_latency.p99()) + ",\n";
		result += "      \"p999_ns\": " + std::to_string(m.read_latency.p999()) + ",\n";
		result += "      \"max_ns\": " + std::to_string(m.read_latency.max_ns.load(std::memory_order_relaxed)) + ",\n";
		result += "      \"avg_ns\": " + std::to_string(m.read_latency.avg()) + "\n";
		result += "    },\n";
		result += "    \"write\": {\n";
		result += "      \"p50_ns\": " + std::to_string(m.write_latency.p50()) + ",\n";
		result += "      \"p95_ns\": " + std::to_string(m.write_latency.p95()) + ",\n";
		result += "      \"p99_ns\": " + std::to_string(m.write_latency.p99()) + ",\n";
		result += "      \"p999_ns\": " + std::to_string(m.write_latency.p999()) + ",\n";
		result += "      \"max_ns\": " + std::to_string(m.write_latency.max_ns.load(std::memory_order_relaxed)) + ",\n";
		result += "      \"avg_ns\": " + std::to_string(m.write_latency.avg()) + "\n";
		result += "    }\n";
		result += "  },\n";

		result += "  \"simd\": {\n";
		result += "    \"simd_operations\": " + std::to_string(m.simd.simd_operations.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"scalar_fallbacks\": " + std::to_string(m.simd.scalar_fallbacks.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"bytes_processed_simd\": " + std::to_string(m.simd.bytes_processed_simd.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"utilization_percent\": " + std::to_string(m.simd.utilization()) + "\n";
		result += "  },\n";

		result += "  \"cache\": {\n";
		result += "    \"key_index_hits\": " + std::to_string(m.cache.key_index_hits.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"key_index_misses\": " + std::to_string(m.cache.key_index_misses.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"value_cache_hits\": " + std::to_string(m.cache.value_cache_hits.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"value_cache_misses\": " + std::to_string(m.cache.value_cache_misses.load(std::memory_order_relaxed)) + ",\n";
		result += "    \"key_index_hit_rate_percent\": " + std::to_string(m.cache.key_index_hit_rate()) + ",\n";
		result += "    \"value_cache_hit_rate_percent\": " + std::to_string(m.cache.value_cache_hit_rate()) + "\n";
		result += "  }\n";

		result += "}";

		return result;
	}

	std::string value_container::metrics_to_prometheus() const
	{
		auto& m = metrics_manager::get();

		std::string result;
		result.reserve(4096);  // Pre-allocate for performance

		// Operation counters
		result += "# HELP container_operations_total Total number of container operations\n";
		result += "# TYPE container_operations_total counter\n";
		result += "container_operations_total{operation=\"read\"} " + std::to_string(m.operations.reads.load(std::memory_order_relaxed)) + "\n";
		result += "container_operations_total{operation=\"write\"} " + std::to_string(m.operations.writes.load(std::memory_order_relaxed)) + "\n";
		result += "container_operations_total{operation=\"serialize\"} " + std::to_string(m.operations.serializations.load(std::memory_order_relaxed)) + "\n";
		result += "container_operations_total{operation=\"deserialize\"} " + std::to_string(m.operations.deserializations.load(std::memory_order_relaxed)) + "\n";
		result += "container_operations_total{operation=\"copy\"} " + std::to_string(m.operations.copies.load(std::memory_order_relaxed)) + "\n";
		result += "container_operations_total{operation=\"move\"} " + std::to_string(m.operations.moves.load(std::memory_order_relaxed)) + "\n";

		// Timing totals
		result += "# HELP container_operation_duration_nanoseconds_total Total time spent in operations\n";
		result += "# TYPE container_operation_duration_nanoseconds_total counter\n";
		result += "container_operation_duration_nanoseconds_total{operation=\"serialize\"} " + std::to_string(m.timing.total_serialize_ns.load(std::memory_order_relaxed)) + "\n";
		result += "container_operation_duration_nanoseconds_total{operation=\"deserialize\"} " + std::to_string(m.timing.total_deserialize_ns.load(std::memory_order_relaxed)) + "\n";
		result += "container_operation_duration_nanoseconds_total{operation=\"read\"} " + std::to_string(m.timing.total_read_ns.load(std::memory_order_relaxed)) + "\n";
		result += "container_operation_duration_nanoseconds_total{operation=\"write\"} " + std::to_string(m.timing.total_write_ns.load(std::memory_order_relaxed)) + "\n";

		// Latency percentiles - serialize
		result += "# HELP container_serialize_latency_nanoseconds Serialize operation latency percentiles\n";
		result += "# TYPE container_serialize_latency_nanoseconds summary\n";
		result += "container_serialize_latency_nanoseconds{quantile=\"0.5\"} " + std::to_string(m.serialize_latency.p50()) + "\n";
		result += "container_serialize_latency_nanoseconds{quantile=\"0.95\"} " + std::to_string(m.serialize_latency.p95()) + "\n";
		result += "container_serialize_latency_nanoseconds{quantile=\"0.99\"} " + std::to_string(m.serialize_latency.p99()) + "\n";
		result += "container_serialize_latency_nanoseconds{quantile=\"0.999\"} " + std::to_string(m.serialize_latency.p999()) + "\n";
		result += "container_serialize_latency_nanoseconds_max " + std::to_string(m.serialize_latency.max_ns.load(std::memory_order_relaxed)) + "\n";
		result += "container_serialize_latency_nanoseconds_count " + std::to_string(m.serialize_latency.sample_count.load(std::memory_order_relaxed)) + "\n";

		// Latency percentiles - deserialize
		result += "# HELP container_deserialize_latency_nanoseconds Deserialize operation latency percentiles\n";
		result += "# TYPE container_deserialize_latency_nanoseconds summary\n";
		result += "container_deserialize_latency_nanoseconds{quantile=\"0.5\"} " + std::to_string(m.deserialize_latency.p50()) + "\n";
		result += "container_deserialize_latency_nanoseconds{quantile=\"0.95\"} " + std::to_string(m.deserialize_latency.p95()) + "\n";
		result += "container_deserialize_latency_nanoseconds{quantile=\"0.99\"} " + std::to_string(m.deserialize_latency.p99()) + "\n";
		result += "container_deserialize_latency_nanoseconds{quantile=\"0.999\"} " + std::to_string(m.deserialize_latency.p999()) + "\n";
		result += "container_deserialize_latency_nanoseconds_max " + std::to_string(m.deserialize_latency.max_ns.load(std::memory_order_relaxed)) + "\n";
		result += "container_deserialize_latency_nanoseconds_count " + std::to_string(m.deserialize_latency.sample_count.load(std::memory_order_relaxed)) + "\n";

		// Latency percentiles - read
		result += "# HELP container_read_latency_nanoseconds Read operation latency percentiles\n";
		result += "# TYPE container_read_latency_nanoseconds summary\n";
		result += "container_read_latency_nanoseconds{quantile=\"0.5\"} " + std::to_string(m.read_latency.p50()) + "\n";
		result += "container_read_latency_nanoseconds{quantile=\"0.95\"} " + std::to_string(m.read_latency.p95()) + "\n";
		result += "container_read_latency_nanoseconds{quantile=\"0.99\"} " + std::to_string(m.read_latency.p99()) + "\n";
		result += "container_read_latency_nanoseconds{quantile=\"0.999\"} " + std::to_string(m.read_latency.p999()) + "\n";
		result += "container_read_latency_nanoseconds_max " + std::to_string(m.read_latency.max_ns.load(std::memory_order_relaxed)) + "\n";
		result += "container_read_latency_nanoseconds_count " + std::to_string(m.read_latency.sample_count.load(std::memory_order_relaxed)) + "\n";

		// Latency percentiles - write
		result += "# HELP container_write_latency_nanoseconds Write operation latency percentiles\n";
		result += "# TYPE container_write_latency_nanoseconds summary\n";
		result += "container_write_latency_nanoseconds{quantile=\"0.5\"} " + std::to_string(m.write_latency.p50()) + "\n";
		result += "container_write_latency_nanoseconds{quantile=\"0.95\"} " + std::to_string(m.write_latency.p95()) + "\n";
		result += "container_write_latency_nanoseconds{quantile=\"0.99\"} " + std::to_string(m.write_latency.p99()) + "\n";
		result += "container_write_latency_nanoseconds{quantile=\"0.999\"} " + std::to_string(m.write_latency.p999()) + "\n";
		result += "container_write_latency_nanoseconds_max " + std::to_string(m.write_latency.max_ns.load(std::memory_order_relaxed)) + "\n";
		result += "container_write_latency_nanoseconds_count " + std::to_string(m.write_latency.sample_count.load(std::memory_order_relaxed)) + "\n";

		// SIMD metrics
		result += "# HELP container_simd_operations_total Total SIMD operations performed\n";
		result += "# TYPE container_simd_operations_total counter\n";
		result += "container_simd_operations_total " + std::to_string(m.simd.simd_operations.load(std::memory_order_relaxed)) + "\n";
		result += "# HELP container_scalar_fallbacks_total Total scalar fallback operations\n";
		result += "# TYPE container_scalar_fallbacks_total counter\n";
		result += "container_scalar_fallbacks_total " + std::to_string(m.simd.scalar_fallbacks.load(std::memory_order_relaxed)) + "\n";
		result += "# HELP container_simd_bytes_processed_total Total bytes processed via SIMD\n";
		result += "# TYPE container_simd_bytes_processed_total counter\n";
		result += "container_simd_bytes_processed_total " + std::to_string(m.simd.bytes_processed_simd.load(std::memory_order_relaxed)) + "\n";
		result += "# HELP container_simd_utilization_ratio SIMD utilization ratio\n";
		result += "# TYPE container_simd_utilization_ratio gauge\n";
		result += "container_simd_utilization_ratio " + std::to_string(m.simd.utilization() / 100.0) + "\n";

		// Cache metrics
		result += "# HELP container_cache_hits_total Total cache hits\n";
		result += "# TYPE container_cache_hits_total counter\n";
		result += "container_cache_hits_total{cache=\"key_index\"} " + std::to_string(m.cache.key_index_hits.load(std::memory_order_relaxed)) + "\n";
		result += "container_cache_hits_total{cache=\"value\"} " + std::to_string(m.cache.value_cache_hits.load(std::memory_order_relaxed)) + "\n";
		result += "# HELP container_cache_misses_total Total cache misses\n";
		result += "# TYPE container_cache_misses_total counter\n";
		result += "container_cache_misses_total{cache=\"key_index\"} " + std::to_string(m.cache.key_index_misses.load(std::memory_order_relaxed)) + "\n";
		result += "container_cache_misses_total{cache=\"value\"} " + std::to_string(m.cache.value_cache_misses.load(std::memory_order_relaxed)) + "\n";
		result += "# HELP container_cache_hit_ratio Cache hit ratio\n";
		result += "# TYPE container_cache_hit_ratio gauge\n";
		result += "container_cache_hit_ratio{cache=\"key_index\"} " + std::to_string(m.cache.key_index_hit_rate() / 100.0) + "\n";
		result += "container_cache_hit_ratio{cache=\"value\"} " + std::to_string(m.cache.value_cache_hit_rate() / 100.0) + "\n";

		return result;
	}

	bool value_container::deserialize_values(const std::string& data,
											 bool parse_only_header)
	{
		if (!optimized_units_.empty())
		{
			optimized_units_.clear();
		}
		changed_data_ = false;

		// Match both single and double braces for @data section
		std::regex reData("@data=\\s*\\{\\{?\\s*(.*?)\\s*\\}\\}?;");
		std::smatch match;
		if (!std::regex_search(data, match, reData))
		{
			data_string_ = "@data={{}}";
			parsed_data_ = true;
			return false;
		}
		data_string_ = match[0];

		if (parse_only_header)
		{
			parsed_data_ = false;
			return true;
		}
		parsed_data_ = true;

	// Parse items: [name,type,data];
	std::regex reItems("\\[(\\w+),\\s*(\\w+),\\s*(.*?)\\];");
	auto it = std::sregex_iterator(data_string_.begin(), data_string_.end(),
								   reItems);
	auto end = std::sregex_iterator();

	for (; it != end; ++it)
	{
		auto nameStr = (*it)[1].str();
		auto typeStr = (*it)[2].str();
		auto dataStr = (*it)[3].str();

		// Convert string -> value_types
		auto vt = convert_value_type(typeStr);

		// Create optimized_value and populate variant based on type
		optimized_value ov;
		ov.name = nameStr;
		ov.type = vt;

		// Parse data string into appropriate variant type
		switch (vt)
		{
		case value_types::null_value:
			ov.data = std::monostate{};
			break;
		case value_types::bool_value:
			ov.data = (dataStr == "true" || dataStr == "1");
			break;
		case value_types::short_value:
			ov.data = static_cast<short>(std::stoi(dataStr));
			break;
		case value_types::ushort_value:
			ov.data = static_cast<unsigned short>(std::stoul(dataStr));
			break;
		case value_types::int_value:
			ov.data = std::stoi(dataStr);
			break;
		case value_types::uint_value:
			ov.data = static_cast<unsigned int>(std::stoul(dataStr));
			break;
		case value_types::long_value:
			ov.data = std::stol(dataStr);
			break;
		case value_types::ulong_value:
			ov.data = std::stoul(dataStr);
			break;
		case value_types::llong_value:
			ov.data = std::stoll(dataStr);
			break;
		case value_types::ullong_value:
			ov.data = std::stoull(dataStr);
			break;
		case value_types::float_value:
			ov.data = std::stof(dataStr);
			break;
		case value_types::double_value:
			ov.data = std::stod(dataStr);
			break;
		case value_types::string_value:
			ov.data = dataStr;
			break;
		case value_types::bytes_value:
		{
			// For bytes, dataStr should be the string representation
			std::vector<uint8_t> bytes(dataStr.begin(), dataStr.end());
			ov.data = bytes;
			break;
		}
		case value_types::container_value:
			// TODO: Handle nested containers
			ov.data = std::monostate{};
			break;
		default:
			ov.data = std::monostate{};
			break;
		}

		optimized_units_.push_back(std::move(ov));
	}

	return true;
	}

	void value_container::parsing(std::string_view source_name,
								  std::string_view target_name,
								  std::string_view target_value,
								  std::string& target_variable)
	{
		if (source_name == target_name)
		{
			// Optimized trim using string_view operations
			// This avoids creating intermediate strings and multiple erase operations
			if (target_value.empty())
			{
				target_variable.clear();
				return;
			}

			// Find first non-space character
			size_t start = target_value.find_first_not_of(' ');
			if (start == std::string_view::npos)
			{
				// All spaces
				target_variable.clear();
				return;
			}

			// Find last non-space character
			size_t end = target_value.find_last_not_of(' ');

			// Create trimmed string in one operation (reduced memory allocations)
			target_variable = std::string(target_value.substr(start, end - start + 1));
		}
	}
} // namespace container_module
