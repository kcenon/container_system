#include <fstream>
#include <iostream>
#include <memory>
#include "container/container.h"
#include "container/values/bool_value.h"
#include "container/values/numeric_value.h"
#include "container/values/string_value.h"
#include "container/values/bytes_value.h"

using namespace container_module;

int main() {
    // Create a container with all basic types
    auto cont = std::make_shared<value_container>();
    cont->set_message_type("cross_lang_test");

    // Null value (type 0) - not implemented in C++, skip

    // Bool value (type 1)
    cont->add(std::make_shared<bool_value>("bool_true", true));
    cont->add(std::make_shared<bool_value>("bool_false", false));

    // Short value (type 2)
    cont->add(std::make_shared<short_value>("short_neg", static_cast<short>(-1000)));
    cont->add(std::make_shared<short_value>("short_pos", static_cast<short>(1000)));

    // UShort value (type 3)
    cont->add(std::make_shared<ushort_value>("ushort", static_cast<unsigned short>(50000)));

    // Int value (type 4)
    cont->add(std::make_shared<int_value>("int_neg", -1000000));
    cont->add(std::make_shared<int_value>("int_pos", 1000000));

    // UInt value (type 5)
    cont->add(std::make_shared<uint_value>("uint", 3000000000U));

    // Long value (type 6) - 32-bit enforced
    cont->add(std::make_shared<long_value>("long_32bit", 2000000000L));

    // ULong value (type 7) - 32-bit enforced
    cont->add(std::make_shared<ulong_value>("ulong_32bit", 3500000000UL));

    // LLong value (type 8) - 64-bit
    cont->add(std::make_shared<llong_value>("llong_64bit", 5000000000LL));

    // ULLong value (type 9) - 64-bit
    cont->add(std::make_shared<ullong_value>("ullong_64bit", 10000000000ULL));

    // Float value (type 10)
    cont->add(std::make_shared<float_value>("float_pi", 3.14159f));

    // Double value (type 11)
    cont->add(std::make_shared<double_value>("double_pi", 3.141592653589793));

    // Bytes value (type 12)
    std::vector<uint8_t> bytes_data = {0x01, 0x02, 0x03, 0xFF, 0xFE};
    cont->add(std::make_shared<bytes_value>("bytes_test", bytes_data));

    // String value (type 13)
    cont->add(std::make_shared<string_value>("string_hello", "Hello from C++!"));
    cont->add(std::make_shared<string_value>("string_utf8", "UTF-8: 한글 테스트"));

    // Container value (type 14) - nested
    auto nested = std::make_shared<value_container>();
    nested->set_message_type("nested_container");
    nested->add(std::make_shared<int_value>("nested_int", 42));
    nested->add(std::make_shared<string_value>("nested_str", "nested"));
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
    simple->add(std::make_shared<long_value>("timestamp", 1234567890L));
    auto simple_data = simple->serialize_array();

    std::ofstream simple_file("test_data_cpp_simple.bin", std::ios::binary);
    simple_file.write(reinterpret_cast<const char*>(simple_data.data()), simple_data.size());
    simple_file.close();

    std::cout << "Generated test_data_cpp_simple.bin (" << simple_data.size() << " bytes)" << std::endl;

    return 0;
}
