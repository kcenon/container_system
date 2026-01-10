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
