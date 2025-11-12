// BSD 3-Clause License
//
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * @file iterator_example.cpp
 * @brief Example demonstrating STL iterator support in value_container
 *
 * This example shows how to use the iterator interface added in Sprint 4:
 * - Range-based for loops
 * - STL algorithms (find_if, count_if, for_each)
 * - Iterator properties (begin/end, size, empty)
 */

#include "container/core/container.h"
#include <algorithm>
#include <iostream>
#include <numeric>

using namespace container_module;

void demonstrate_range_based_for()
{
	std::cout << "\n=== Range-Based For Loop ===" << std::endl;

	value_container container;

	// Note: For demonstration, we're showing the concept
	// In actual use, you would populate the container with data
	std::cout << "Container size: " << container.size() << std::endl;
	std::cout << "Container empty: " << (container.empty() ? "yes" : "no") << std::endl;

	// Example of range-based for loop (when container has data)
	std::cout << "Range-based for loop syntax:" << std::endl;
	std::cout << "  for (const auto& value : container) {" << std::endl;
	std::cout << "      // Process each optimized_value" << std::endl;
	std::cout << "  }" << std::endl;
}

void demonstrate_stl_algorithms()
{
	std::cout << "\n=== STL Algorithms ===" << std::endl;

	value_container container;

	// Example 1: find_if
	std::cout << "find_if example:" << std::endl;
	std::cout << "  auto it = std::find_if(container.begin(), container.end()," << std::endl;
	std::cout << "      [](const optimized_value& val) {" << std::endl;
	std::cout << "          return val.type == value_types::int_value;" << std::endl;
	std::cout << "      });" << std::endl;

	// Example 2: count_if
	std::cout << "\ncount_if example:" << std::endl;
	std::cout << "  size_t count = std::count_if(container.begin(), container.end()," << std::endl;
	std::cout << "      [](const optimized_value& val) {" << std::endl;
	std::cout << "          return val.type == value_types::string_value;" << std::endl;
	std::cout << "      });" << std::endl;

	// Example 3: for_each
	std::cout << "\nfor_each example:" << std::endl;
	std::cout << "  std::for_each(container.begin(), container.end()," << std::endl;
	std::cout << "      [](const optimized_value& val) {" << std::endl;
	std::cout << "          std::cout << val.name << std::endl;" << std::endl;
	std::cout << "      });" << std::endl;
}

void demonstrate_iterator_properties()
{
	std::cout << "\n=== Iterator Properties ===" << std::endl;

	value_container container;

	std::cout << "Iterator methods available:" << std::endl;
	std::cout << "  container.begin()   - returns iterator to first element" << std::endl;
	std::cout << "  container.end()     - returns iterator to past-the-end" << std::endl;
	std::cout << "  container.cbegin()  - returns const_iterator to first element" << std::endl;
	std::cout << "  container.cend()    - returns const_iterator to past-the-end" << std::endl;
	std::cout << "  container.size()    - returns number of elements" << std::endl;
	std::cout << "  container.empty()   - returns true if empty" << std::endl;

	std::cout << "\nIterator types:" << std::endl;
	std::cout << "  using iterator = std::vector<optimized_value>::iterator;" << std::endl;
	std::cout << "  using const_iterator = std::vector<optimized_value>::const_iterator;" << std::endl;
}

int main()
{
	std::cout << "========================================" << std::endl;
	std::cout << "Container Iterator Support Examples" << std::endl;
	std::cout << "========================================" << std::endl;

	demonstrate_range_based_for();
	demonstrate_stl_algorithms();
	demonstrate_iterator_properties();

	std::cout << "\n========================================" << std::endl;
	std::cout << "Examples completed successfully!" << std::endl;
	std::cout << "========================================" << std::endl;

	return 0;
}
