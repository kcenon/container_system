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

#include <fstream>
#include <iostream>
#include <memory>
#include "container/container.h"

using namespace container_module;

int main() {
    // Create a container with all basic types
    auto cont = std::make_shared<value_container>();
    cont->set_message_type("cross_lang_test");

    // Null value (type 0) - not implemented in C++, skip

    // Bool value (type 1)
    cont->set("bool_true", true);
    cont->set("bool_false", false);

    // Short value (type 2)
    cont->set("short_neg", static_cast<short>(-1000));
    cont->set("short_pos", static_cast<short>(1000));

    // UShort value (type 3)
    cont->set("ushort", static_cast<unsigned short>(50000));

    // Int value (type 4)
    cont->set("int_neg", -1000000);
    cont->set("int_pos", 1000000);

    // UInt value (type 5)
    cont->set("uint", 3000000000U);

    // Long value (type 6) - 32-bit enforced
    cont->set("long_32bit", 2000000000L);

    // ULong value (type 7) - 32-bit enforced
    cont->set("ulong_32bit", 3500000000UL);

    // LLong value (type 8) - 64-bit
    cont->set("llong_64bit", 5000000000LL);

    // ULLong value (type 9) - 64-bit
    cont->set("ullong_64bit", 10000000000ULL);

    // Float value (type 10)
    cont->set("float_pi", 3.14159f);

    // Double value (type 11)
    cont->set("double_pi", 3.141592653589793);

    // Bytes value (type 12)
    std::vector<uint8_t> bytes_data = {0x01, 0x02, 0x03, 0xFF, 0xFE};
    cont->set("bytes_test", bytes_data);

    // String value (type 13)
    cont->set("string_hello", "Hello from C++!");
    cont->set("string_utf8", "UTF-8: 한글 테스트");

    // Container value (type 14) - nested
    auto nested = std::make_shared<value_container>();
    nested->set_message_type("nested_container");
    nested->set("nested_int", 42);
    nested->set("nested_str", "nested");
    cont->add(nested);

    // Serialize to file
    auto serialized = cont->serialize_array();

    std::ofstream outfile("test_data_cpp.bin", std::ios::binary);
    if (!outfile) {
        std::cerr << "Failed to create output file" << std::endl;
        return 1;
    }

    outfile.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
    outfile.close();

    std::cout << "Generated test_data_cpp.bin (" << serialized.size() << " bytes)" << std::endl;
    std::cout << "Container has " << cont->size() << " values" << std::endl;

    // Also generate a simple test case with just one long value
    auto simple = std::make_shared<value_container>();
    simple->set_message_type("simple_test");
    simple->set("timestamp", 1234567890L);
    auto simple_data = simple->serialize_array();

    std::ofstream simple_file("test_data_cpp_simple.bin", std::ios::binary);
    simple_file.write(reinterpret_cast<const char*>(simple_data.data()), simple_data.size());
    simple_file.close();

    std::cout << "Generated test_data_cpp_simple.bin (" << simple_data.size() << " bytes)" << std::endl;

    return 0;
}
