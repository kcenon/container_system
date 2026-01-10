/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
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
 * @file internal/async/generator.h
 * @brief C++20 coroutine generator for lazy sequence generation
 *
 * Provides a generator type for producing sequences of values lazily.
 * Useful for streaming large data in chunks.
 *
 * @code
 * // Example usage:
 * generator<int> range(int start, int end) {
 *     for (int i = start; i < end; ++i) {
 *         co_yield i;
 *     }
 * }
 *
 * for (int value : range(0, 10)) {
 *     // Process value
 * }
 * @endcode
 *
 * @see container_module::async::generator
 */

#pragma once

#include <coroutine>
#include <exception>
#include <iterator>
#include <utility>
#include <type_traits>
#include <memory>

namespace container_module::async
{
    /**
     * @brief Forward declaration of generator
     */
    template<typename T>
    class generator;

    namespace detail
    {
        /**
         * @brief Promise type for generator
         */
        template<typename T>
        struct generator_promise
        {
            using value_type = std::remove_reference_t<T>;
            using reference_type = std::conditional_t<std::is_reference_v<T>, T, T&>;
            using pointer_type = value_type*;

            pointer_type current_value_{nullptr};
            std::exception_ptr exception_;

            /**
             * @brief Get return object (the generator)
             */
            [[nodiscard]] generator<T> get_return_object() noexcept;

            /**
             * @brief Suspend at start - generators are lazy
             */
            [[nodiscard]] std::suspend_always initial_suspend() noexcept
            {
                return {};
            }

            /**
             * @brief Suspend at final point
             */
            [[nodiscard]] std::suspend_always final_suspend() noexcept
            {
                return {};
            }

            /**
             * @brief Yield a value
             */
            std::suspend_always yield_value(value_type& value) noexcept
            {
                current_value_ = std::addressof(value);
                return {};
            }

            /**
             * @brief Yield an rvalue
             */
            std::suspend_always yield_value(value_type&& value) noexcept
            {
                current_value_ = std::addressof(value);
                return {};
            }

            /**
             * @brief Generators don't return values directly
             */
            void return_void() noexcept {}

            /**
             * @brief Handle unhandled exceptions
             */
            void unhandled_exception() noexcept
            {
                exception_ = std::current_exception();
            }

            /**
             * @brief Get the current value
             */
            [[nodiscard]] reference_type value() const noexcept
            {
                return *current_value_;
            }

            /**
             * @brief Rethrow stored exception if any
             */
            void rethrow_if_exception()
            {
                if (exception_)
                {
                    std::rethrow_exception(exception_);
                }
            }
        };

    } // namespace detail

    /**
     * @brief Coroutine generator for lazy sequence generation
     *
     * A generator is a coroutine that can produce a sequence of values
     * lazily using co_yield. Each value is produced on-demand when
     * the iterator is advanced.
     *
     * Properties:
     * - Lazy evaluation (values computed on-demand)
     * - Move-only (non-copyable)
     * - STL-compatible iteration
     * - Exception propagation
     *
     * @tparam T The type of values produced by the generator
     */
    template<typename T>
    class generator
    {
    public:
        using promise_type = detail::generator_promise<T>;
        using handle_type = std::coroutine_handle<promise_type>;
        using value_type = std::remove_reference_t<T>;
        using reference = std::conditional_t<std::is_reference_v<T>, T, T&>;
        using pointer = value_type*;

        /**
         * @brief Iterator for generator
         */
        class iterator
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = generator::value_type;
            using reference = generator::reference;
            using pointer = generator::pointer;

            /**
             * @brief Default constructor - creates end iterator
             */
            iterator() noexcept : handle_(nullptr) {}

            /**
             * @brief Construct from coroutine handle
             */
            explicit iterator(handle_type handle) noexcept : handle_(handle) {}

            /**
             * @brief Dereference operator
             */
            [[nodiscard]] reference operator*() const noexcept
            {
                return handle_.promise().value();
            }

            /**
             * @brief Arrow operator
             */
            [[nodiscard]] pointer operator->() const noexcept
            {
                return std::addressof(handle_.promise().value());
            }

            /**
             * @brief Pre-increment operator
             */
            iterator& operator++()
            {
                handle_.resume();
                if (handle_.done())
                {
                    handle_.promise().rethrow_if_exception();
                }
                return *this;
            }

            /**
             * @brief Post-increment operator
             */
            void operator++(int)
            {
                ++(*this);
            }

            /**
             * @brief Equality comparison
             */
            [[nodiscard]] bool operator==(std::default_sentinel_t) const noexcept
            {
                return handle_.done();
            }

            /**
             * @brief Inequality comparison
             */
            [[nodiscard]] bool operator!=(std::default_sentinel_t) const noexcept
            {
                return !handle_.done();
            }

        private:
            handle_type handle_;
        };

        /**
         * @brief Default constructor - creates empty generator
         */
        generator() noexcept : handle_(nullptr) {}

        /**
         * @brief Construct from coroutine handle
         */
        explicit generator(handle_type handle) noexcept : handle_(handle) {}

        /**
         * @brief Move constructor
         */
        generator(generator&& other) noexcept
            : handle_(std::exchange(other.handle_, nullptr)) {}

        /**
         * @brief Move assignment operator
         */
        generator& operator=(generator&& other) noexcept
        {
            if (this != &other)
            {
                if (handle_)
                {
                    handle_.destroy();
                }
                handle_ = std::exchange(other.handle_, nullptr);
            }
            return *this;
        }

        /**
         * @brief Destructor - destroys the coroutine if owned
         */
        ~generator()
        {
            if (handle_)
            {
                handle_.destroy();
            }
        }

        // Non-copyable
        generator(const generator&) = delete;
        generator& operator=(const generator&) = delete;

        /**
         * @brief Check if generator is valid
         */
        [[nodiscard]] bool valid() const noexcept
        {
            return handle_ != nullptr;
        }

        /**
         * @brief Check if generator is valid
         */
        [[nodiscard]] explicit operator bool() const noexcept
        {
            return valid();
        }

        /**
         * @brief Get iterator to beginning
         *
         * Resumes the coroutine to get the first value
         */
        [[nodiscard]] iterator begin()
        {
            if (handle_)
            {
                handle_.resume();
                if (handle_.done())
                {
                    handle_.promise().rethrow_if_exception();
                }
            }
            return iterator{handle_};
        }

        /**
         * @brief Get sentinel for end
         */
        [[nodiscard]] std::default_sentinel_t end() noexcept
        {
            return {};
        }

    private:
        handle_type handle_;
    };

    // Implementation of get_return_object
    template<typename T>
    generator<T> detail::generator_promise<T>::get_return_object() noexcept
    {
        return generator<T>{
            std::coroutine_handle<detail::generator_promise<T>>::from_promise(*this)};
    }

    /**
     * @brief Create a generator from a range
     *
     * @tparam Range The range type
     * @param range The range to iterate
     * @return A generator that yields each element of the range
     */
    template<typename Range>
    generator<typename std::ranges::range_value_t<Range>> from_range(Range&& range)
    {
        for (auto&& element : range)
        {
            co_yield std::forward<decltype(element)>(element);
        }
    }

    /**
     * @brief Create a generator that yields a fixed number of elements
     *
     * @tparam T The element type
     * @param gen The source generator
     * @param count Maximum number of elements to yield
     * @return A generator limited to count elements
     */
    template<typename T>
    generator<T> take(generator<T> gen, size_t count)
    {
        size_t yielded = 0;
        for (auto&& element : gen)
        {
            if (yielded++ >= count)
            {
                break;
            }
            co_yield std::forward<decltype(element)>(element);
        }
    }

} // namespace container_module::async
