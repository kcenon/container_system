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
 * @file core/container/metrics.h
 * @brief Detailed observability metrics for container_module
 *
 * This header defines comprehensive metrics structures for monitoring
 * container operations, including:
 * - Operation counters (reads, writes, serializations, etc.)
 * - Timing metrics (nanoseconds for key operations)
 * - Latency histograms (P50, P95, P99, P999)
 * - SIMD and cache efficiency metrics
 *
 * @see container_module::value_container
 * @since 2.1.0
 */

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <array>
#include <algorithm>
#include <mutex>

namespace container_module
{
	/**
	 * @brief Operation counter metrics for container operations
	 *
	 * Thread-safe atomic counters for tracking operation counts.
	 * All counters use relaxed memory ordering for minimal overhead.
	 */
	struct operation_counts
	{
		std::atomic<uint64_t> reads{0};            ///< Number of read operations
		std::atomic<uint64_t> writes{0};           ///< Number of write operations
		std::atomic<uint64_t> serializations{0};   ///< Number of serialize operations
		std::atomic<uint64_t> deserializations{0}; ///< Number of deserialize operations
		std::atomic<uint64_t> copies{0};           ///< Number of copy operations
		std::atomic<uint64_t> moves{0};            ///< Number of move operations

		operation_counts() = default;
		~operation_counts() = default;

		// Copy constructor - load atomic values
		operation_counts(const operation_counts& other) noexcept
			: reads(other.reads.load(std::memory_order_relaxed))
			, writes(other.writes.load(std::memory_order_relaxed))
			, serializations(other.serializations.load(std::memory_order_relaxed))
			, deserializations(other.deserializations.load(std::memory_order_relaxed))
			, copies(other.copies.load(std::memory_order_relaxed))
			, moves(other.moves.load(std::memory_order_relaxed))
		{
		}

		// Copy assignment - load and store atomic values
		operation_counts& operator=(const operation_counts& other) noexcept
		{
			if (this != &other)
			{
				reads.store(other.reads.load(std::memory_order_relaxed), std::memory_order_relaxed);
				writes.store(other.writes.load(std::memory_order_relaxed), std::memory_order_relaxed);
				serializations.store(other.serializations.load(std::memory_order_relaxed), std::memory_order_relaxed);
				deserializations.store(other.deserializations.load(std::memory_order_relaxed), std::memory_order_relaxed);
				copies.store(other.copies.load(std::memory_order_relaxed), std::memory_order_relaxed);
				moves.store(other.moves.load(std::memory_order_relaxed), std::memory_order_relaxed);
			}
			return *this;
		}

		/**
		 * @brief Reset all counters to zero
		 */
		void reset() noexcept
		{
			reads.store(0, std::memory_order_relaxed);
			writes.store(0, std::memory_order_relaxed);
			serializations.store(0, std::memory_order_relaxed);
			deserializations.store(0, std::memory_order_relaxed);
			copies.store(0, std::memory_order_relaxed);
			moves.store(0, std::memory_order_relaxed);
		}
	};

	/**
	 * @brief Timing metrics for container operations
	 *
	 * Accumulates total time spent in each operation type.
	 * Times are stored in nanoseconds.
	 */
	struct timing_metrics
	{
		std::atomic<uint64_t> total_serialize_ns{0};    ///< Total serialize time
		std::atomic<uint64_t> total_deserialize_ns{0};  ///< Total deserialize time
		std::atomic<uint64_t> total_read_ns{0};         ///< Total read time
		std::atomic<uint64_t> total_write_ns{0};        ///< Total write time

		timing_metrics() = default;
		~timing_metrics() = default;

		timing_metrics(const timing_metrics& other) noexcept
			: total_serialize_ns(other.total_serialize_ns.load(std::memory_order_relaxed))
			, total_deserialize_ns(other.total_deserialize_ns.load(std::memory_order_relaxed))
			, total_read_ns(other.total_read_ns.load(std::memory_order_relaxed))
			, total_write_ns(other.total_write_ns.load(std::memory_order_relaxed))
		{
		}

		timing_metrics& operator=(const timing_metrics& other) noexcept
		{
			if (this != &other)
			{
				total_serialize_ns.store(other.total_serialize_ns.load(std::memory_order_relaxed), std::memory_order_relaxed);
				total_deserialize_ns.store(other.total_deserialize_ns.load(std::memory_order_relaxed), std::memory_order_relaxed);
				total_read_ns.store(other.total_read_ns.load(std::memory_order_relaxed), std::memory_order_relaxed);
				total_write_ns.store(other.total_write_ns.load(std::memory_order_relaxed), std::memory_order_relaxed);
			}
			return *this;
		}

		/**
		 * @brief Reset all timing metrics to zero
		 */
		void reset() noexcept
		{
			total_serialize_ns.store(0, std::memory_order_relaxed);
			total_deserialize_ns.store(0, std::memory_order_relaxed);
			total_read_ns.store(0, std::memory_order_relaxed);
			total_write_ns.store(0, std::memory_order_relaxed);
		}
	};

	/**
	 * @brief Approximate latency histogram using reservoir sampling
	 *
	 * Tracks percentile latencies (P50, P95, P99, P999) with minimal overhead.
	 * Uses a lock-free reservoir sampling approach for thread safety.
	 */
	struct latency_histogram
	{
		static constexpr size_t kReservoirSize = 1024;

		std::atomic<uint64_t> sample_count{0};  ///< Total samples collected
		std::atomic<uint64_t> max_ns{0};        ///< Maximum observed latency
		std::atomic<uint64_t> min_ns{UINT64_MAX}; ///< Minimum observed latency
		std::atomic<uint64_t> sum_ns{0};        ///< Sum for average calculation

		// Reservoir for percentile calculation
		std::array<std::atomic<uint64_t>, kReservoirSize> reservoir{};
		std::atomic<size_t> reservoir_count{0};

		latency_histogram() = default;
		~latency_histogram() = default;

		latency_histogram(const latency_histogram& other) noexcept
			: sample_count(other.sample_count.load(std::memory_order_relaxed))
			, max_ns(other.max_ns.load(std::memory_order_relaxed))
			, min_ns(other.min_ns.load(std::memory_order_relaxed))
			, sum_ns(other.sum_ns.load(std::memory_order_relaxed))
			, reservoir_count(other.reservoir_count.load(std::memory_order_relaxed))
		{
			size_t count = reservoir_count.load(std::memory_order_relaxed);
			for (size_t i = 0; i < count && i < kReservoirSize; ++i)
			{
				reservoir[i].store(other.reservoir[i].load(std::memory_order_relaxed),
								   std::memory_order_relaxed);
			}
		}

		latency_histogram& operator=(const latency_histogram& other) noexcept
		{
			if (this != &other)
			{
				sample_count.store(other.sample_count.load(std::memory_order_relaxed), std::memory_order_relaxed);
				max_ns.store(other.max_ns.load(std::memory_order_relaxed), std::memory_order_relaxed);
				min_ns.store(other.min_ns.load(std::memory_order_relaxed), std::memory_order_relaxed);
				sum_ns.store(other.sum_ns.load(std::memory_order_relaxed), std::memory_order_relaxed);
				reservoir_count.store(other.reservoir_count.load(std::memory_order_relaxed), std::memory_order_relaxed);

				size_t count = reservoir_count.load(std::memory_order_relaxed);
				for (size_t i = 0; i < count && i < kReservoirSize; ++i)
				{
					reservoir[i].store(other.reservoir[i].load(std::memory_order_relaxed),
									   std::memory_order_relaxed);
				}
			}
			return *this;
		}

		/**
		 * @brief Record a latency sample
		 * @param latency_ns Latency in nanoseconds
		 */
		void record(uint64_t latency_ns) noexcept
		{
			// Update sample count
			uint64_t count = sample_count.fetch_add(1, std::memory_order_relaxed) + 1;

			// Update max
			uint64_t current_max = max_ns.load(std::memory_order_relaxed);
			while (latency_ns > current_max &&
				   !max_ns.compare_exchange_weak(current_max, latency_ns,
												 std::memory_order_relaxed))
			{
			}

			// Update min
			uint64_t current_min = min_ns.load(std::memory_order_relaxed);
			while (latency_ns < current_min &&
				   !min_ns.compare_exchange_weak(current_min, latency_ns,
												 std::memory_order_relaxed))
			{
			}

			// Update sum
			sum_ns.fetch_add(latency_ns, std::memory_order_relaxed);

			// Reservoir sampling
			size_t idx = reservoir_count.load(std::memory_order_relaxed);
			if (idx < kReservoirSize)
			{
				// Fill the reservoir initially
				reservoir[idx].store(latency_ns, std::memory_order_relaxed);
				reservoir_count.fetch_add(1, std::memory_order_relaxed);
			}
			else
			{
				// Probabilistic replacement for uniform sampling
				// Use simple hash of count as pseudo-random
				size_t replace_idx = (count * 2654435769ULL) % kReservoirSize;
				if ((count * 2654435769ULL >> 32) % count < kReservoirSize)
				{
					reservoir[replace_idx].store(latency_ns, std::memory_order_relaxed);
				}
			}
		}

		/**
		 * @brief Get a percentile value
		 * @param percentile The percentile (0.0 to 1.0, e.g., 0.99 for P99)
		 * @return The latency value at the given percentile
		 */
		[[nodiscard]] uint64_t get_percentile(double percentile) const noexcept
		{
			size_t count = reservoir_count.load(std::memory_order_relaxed);
			if (count == 0)
			{
				return 0;
			}

			// Copy reservoir values to local array for sorting
			std::array<uint64_t, kReservoirSize> values{};
			for (size_t i = 0; i < count && i < kReservoirSize; ++i)
			{
				values[i] = reservoir[i].load(std::memory_order_relaxed);
			}

			// Sort the values
			std::sort(values.begin(), values.begin() + count);

			// Calculate percentile index
			size_t idx = static_cast<size_t>(percentile * static_cast<double>(count - 1));
			if (idx >= count)
			{
				idx = count - 1;
			}

			return values[idx];
		}

		/**
		 * @brief Get P50 latency
		 */
		[[nodiscard]] uint64_t p50() const noexcept { return get_percentile(0.50); }

		/**
		 * @brief Get P95 latency
		 */
		[[nodiscard]] uint64_t p95() const noexcept { return get_percentile(0.95); }

		/**
		 * @brief Get P99 latency
		 */
		[[nodiscard]] uint64_t p99() const noexcept { return get_percentile(0.99); }

		/**
		 * @brief Get P99.9 latency
		 */
		[[nodiscard]] uint64_t p999() const noexcept { return get_percentile(0.999); }

		/**
		 * @brief Get average latency
		 */
		[[nodiscard]] double avg() const noexcept
		{
			uint64_t count = sample_count.load(std::memory_order_relaxed);
			if (count == 0)
			{
				return 0.0;
			}
			return static_cast<double>(sum_ns.load(std::memory_order_relaxed)) /
				   static_cast<double>(count);
		}

		/**
		 * @brief Reset the histogram
		 */
		void reset() noexcept
		{
			sample_count.store(0, std::memory_order_relaxed);
			max_ns.store(0, std::memory_order_relaxed);
			min_ns.store(UINT64_MAX, std::memory_order_relaxed);
			sum_ns.store(0, std::memory_order_relaxed);
			reservoir_count.store(0, std::memory_order_relaxed);
			for (auto& val : reservoir)
			{
				val.store(0, std::memory_order_relaxed);
			}
		}
	};

	/**
	 * @brief SIMD utilization metrics
	 *
	 * Tracks SIMD operations vs scalar fallbacks for performance analysis.
	 */
	struct simd_metrics
	{
		std::atomic<uint64_t> simd_operations{0};      ///< SIMD operations performed
		std::atomic<uint64_t> scalar_fallbacks{0};     ///< Scalar fallback operations
		std::atomic<uint64_t> bytes_processed_simd{0}; ///< Bytes processed via SIMD

		simd_metrics() = default;
		~simd_metrics() = default;

		simd_metrics(const simd_metrics& other) noexcept
			: simd_operations(other.simd_operations.load(std::memory_order_relaxed))
			, scalar_fallbacks(other.scalar_fallbacks.load(std::memory_order_relaxed))
			, bytes_processed_simd(other.bytes_processed_simd.load(std::memory_order_relaxed))
		{
		}

		simd_metrics& operator=(const simd_metrics& other) noexcept
		{
			if (this != &other)
			{
				simd_operations.store(other.simd_operations.load(std::memory_order_relaxed), std::memory_order_relaxed);
				scalar_fallbacks.store(other.scalar_fallbacks.load(std::memory_order_relaxed), std::memory_order_relaxed);
				bytes_processed_simd.store(other.bytes_processed_simd.load(std::memory_order_relaxed), std::memory_order_relaxed);
			}
			return *this;
		}

		/**
		 * @brief Calculate SIMD utilization percentage
		 * @return Percentage of operations using SIMD (0.0 to 100.0)
		 */
		[[nodiscard]] double utilization() const noexcept
		{
			uint64_t simd = simd_operations.load(std::memory_order_relaxed);
			uint64_t scalar = scalar_fallbacks.load(std::memory_order_relaxed);
			uint64_t total = simd + scalar;
			if (total == 0)
			{
				return 0.0;
			}
			return static_cast<double>(simd) / static_cast<double>(total) * 100.0;
		}

		/**
		 * @brief Reset all SIMD metrics
		 */
		void reset() noexcept
		{
			simd_operations.store(0, std::memory_order_relaxed);
			scalar_fallbacks.store(0, std::memory_order_relaxed);
			bytes_processed_simd.store(0, std::memory_order_relaxed);
		}
	};

	/**
	 * @brief Cache efficiency metrics
	 *
	 * Tracks cache hit/miss rates for key index and value caches.
	 */
	struct cache_metrics
	{
		std::atomic<uint64_t> key_index_hits{0};     ///< Key index cache hits
		std::atomic<uint64_t> key_index_misses{0};   ///< Key index cache misses
		std::atomic<uint64_t> value_cache_hits{0};   ///< Value cache hits
		std::atomic<uint64_t> value_cache_misses{0}; ///< Value cache misses

		cache_metrics() = default;
		~cache_metrics() = default;

		cache_metrics(const cache_metrics& other) noexcept
			: key_index_hits(other.key_index_hits.load(std::memory_order_relaxed))
			, key_index_misses(other.key_index_misses.load(std::memory_order_relaxed))
			, value_cache_hits(other.value_cache_hits.load(std::memory_order_relaxed))
			, value_cache_misses(other.value_cache_misses.load(std::memory_order_relaxed))
		{
		}

		cache_metrics& operator=(const cache_metrics& other) noexcept
		{
			if (this != &other)
			{
				key_index_hits.store(other.key_index_hits.load(std::memory_order_relaxed), std::memory_order_relaxed);
				key_index_misses.store(other.key_index_misses.load(std::memory_order_relaxed), std::memory_order_relaxed);
				value_cache_hits.store(other.value_cache_hits.load(std::memory_order_relaxed), std::memory_order_relaxed);
				value_cache_misses.store(other.value_cache_misses.load(std::memory_order_relaxed), std::memory_order_relaxed);
			}
			return *this;
		}

		/**
		 * @brief Calculate key index cache hit rate
		 * @return Hit rate percentage (0.0 to 100.0)
		 */
		[[nodiscard]] double key_index_hit_rate() const noexcept
		{
			uint64_t hits = key_index_hits.load(std::memory_order_relaxed);
			uint64_t misses = key_index_misses.load(std::memory_order_relaxed);
			uint64_t total = hits + misses;
			if (total == 0)
			{
				return 0.0;
			}
			return static_cast<double>(hits) / static_cast<double>(total) * 100.0;
		}

		/**
		 * @brief Calculate value cache hit rate
		 * @return Hit rate percentage (0.0 to 100.0)
		 */
		[[nodiscard]] double value_cache_hit_rate() const noexcept
		{
			uint64_t hits = value_cache_hits.load(std::memory_order_relaxed);
			uint64_t misses = value_cache_misses.load(std::memory_order_relaxed);
			uint64_t total = hits + misses;
			if (total == 0)
			{
				return 0.0;
			}
			return static_cast<double>(hits) / static_cast<double>(total) * 100.0;
		}

		/**
		 * @brief Reset all cache metrics
		 */
		void reset() noexcept
		{
			key_index_hits.store(0, std::memory_order_relaxed);
			key_index_misses.store(0, std::memory_order_relaxed);
			value_cache_hits.store(0, std::memory_order_relaxed);
			value_cache_misses.store(0, std::memory_order_relaxed);
		}
	};

	/**
	 * @brief Complete detailed metrics structure
	 *
	 * Aggregates all metric types for comprehensive observability.
	 */
	struct detailed_metrics
	{
		// Current metrics (preserved from existing memory_stats())
		size_t heap_allocations{0};
		size_t stack_allocations{0};

		// Operation counts
		operation_counts operations;

		// Timing metrics
		timing_metrics timing;

		// Latency histograms for each operation type
		latency_histogram serialize_latency;
		latency_histogram deserialize_latency;
		latency_histogram read_latency;
		latency_histogram write_latency;

		// SIMD metrics
		simd_metrics simd;

		// Cache metrics
		cache_metrics cache;

		/**
		 * @brief Calculate SIMD utilization percentage
		 */
		[[nodiscard]] double simd_utilization() const noexcept
		{
			return simd.utilization();
		}

		/**
		 * @brief Calculate overall cache hit rate
		 */
		[[nodiscard]] double cache_hit_rate() const noexcept
		{
			uint64_t total_hits = cache.key_index_hits.load(std::memory_order_relaxed) +
								  cache.value_cache_hits.load(std::memory_order_relaxed);
			uint64_t total_misses = cache.key_index_misses.load(std::memory_order_relaxed) +
									cache.value_cache_misses.load(std::memory_order_relaxed);
			uint64_t total = total_hits + total_misses;
			if (total == 0)
			{
				return 0.0;
			}
			return static_cast<double>(total_hits) / static_cast<double>(total) * 100.0;
		}

		/**
		 * @brief Calculate average serialize latency in nanoseconds
		 */
		[[nodiscard]] double avg_serialize_latency_ns() const noexcept
		{
			return serialize_latency.avg();
		}

		/**
		 * @brief Reset all metrics
		 */
		void reset() noexcept
		{
			heap_allocations = 0;
			stack_allocations = 0;
			operations.reset();
			timing.reset();
			serialize_latency.reset();
			deserialize_latency.reset();
			read_latency.reset();
			write_latency.reset();
			simd.reset();
			cache.reset();
		}
	};

	/**
	 * @brief RAII timer for measuring operation latency
	 *
	 * Automatically records latency when destroyed.
	 */
	class scoped_timer
	{
	public:
		/**
		 * @brief Construct a scoped timer
		 * @param histogram The histogram to record latency to
		 * @param timing_total Optional pointer to timing accumulator
		 */
		explicit scoped_timer(latency_histogram& histogram,
							  std::atomic<uint64_t>* timing_total = nullptr) noexcept
			: histogram_(histogram)
			, timing_total_(timing_total)
			, start_(std::chrono::steady_clock::now())
			, enabled_(true)
		{
		}

		/**
		 * @brief Construct a disabled scoped timer
		 */
		scoped_timer() noexcept
			: histogram_(dummy_histogram_)
			, timing_total_(nullptr)
			, enabled_(false)
		{
		}

		~scoped_timer() noexcept
		{
			if (enabled_)
			{
				auto end = std::chrono::steady_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
					end - start_).count();
				auto latency = static_cast<uint64_t>(duration);

				histogram_.record(latency);
				if (timing_total_)
				{
					timing_total_->fetch_add(latency, std::memory_order_relaxed);
				}
			}
		}

		// Non-copyable, non-movable
		scoped_timer(const scoped_timer&) = delete;
		scoped_timer& operator=(const scoped_timer&) = delete;
		scoped_timer(scoped_timer&&) = delete;
		scoped_timer& operator=(scoped_timer&&) = delete;

	private:
		latency_histogram& histogram_;
		std::atomic<uint64_t>* timing_total_;
		std::chrono::steady_clock::time_point start_;
		bool enabled_;

		static inline latency_histogram dummy_histogram_{};
	};

	/**
	 * @brief Global metrics manager
	 *
	 * Provides static access to metrics collection with enable/disable support.
	 */
	class metrics_manager
	{
	public:
		/**
		 * @brief Get the singleton metrics instance
		 */
		static detailed_metrics& get() noexcept
		{
			static detailed_metrics instance;
			return instance;
		}

		/**
		 * @brief Check if metrics collection is enabled
		 */
		static bool is_enabled() noexcept
		{
			return enabled_.load(std::memory_order_relaxed);
		}

		/**
		 * @brief Enable or disable metrics collection
		 */
		static void set_enabled(bool enabled) noexcept
		{
			enabled_.store(enabled, std::memory_order_relaxed);
		}

		/**
		 * @brief Reset all metrics
		 */
		static void reset() noexcept
		{
			get().reset();
		}

		/**
		 * @brief Create a scoped timer if metrics are enabled
		 */
		static scoped_timer make_timer(latency_histogram& histogram,
									   std::atomic<uint64_t>* timing_total = nullptr) noexcept
		{
			if (is_enabled())
			{
				return scoped_timer(histogram, timing_total);
			}
			return scoped_timer();
		}

	private:
		static inline std::atomic<bool> enabled_{false};
	};

} // namespace container_module
