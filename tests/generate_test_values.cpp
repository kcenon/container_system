#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

using namespace container_module;

int main() {
    std::cout << "Generating individual value test files..." << std::endl;

    // Bool value (type 1)
    {
        bool_value val("bool_true", true);
        auto serialized = val.serialize();
        std::ofstream f("test_bool.bin", std::ios::binary);
        f.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        f.close();
        std::cout << "Generated test_bool.bin (" << serialized.size() << " bytes)" << std::endl;
    }

    // Short value (type 2)
    {
        short_value val("short_test", static_cast<short>(-1000));
        auto serialized = val.serialize();
        std::ofstream f("test_short.bin", std::ios::binary);
        f.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        f.close();
        std::cout << "Generated test_short.bin (" << serialized.size() << " bytes)" << std::endl;
    }

    // UShort value (type 3)
    {
        ushort_value val("ushort_test", static_cast<unsigned short>(50000));
        auto serialized = val.serialize();
        std::ofstream f("test_ushort.bin", std::ios::binary);
        f.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        f.close();
        std::cout << "Generated test_ushort.bin (" << serialized.size() << " bytes)" << std::endl;
    }

    // Int value (type 4)
    {
        int_value val("int_test", -1000000);
        auto serialized = val.serialize();
        std::ofstream f("test_int.bin", std::ios::binary);
        f.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        f.close();
        std::cout << "Generated test_int.bin (" << serialized.size() << " bytes)" << std::endl;
    }

    // UInt value (type 5)
    {
        uint_value val("uint_test", 3000000000U);
        auto serialized = val.serialize();
        std::ofstream f("test_uint.bin", std::ios::binary);
        f.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        f.close();
        std::cout << "Generated test_uint.bin (" << serialized.size() << " bytes)" << std::endl;
    }

    // Long value (type 6) - 32-bit enforced - THIS IS THE CRITICAL TEST
    {
        long_value val("long_32bit", 2000000000L);
        auto serialized = val.serialize();
        std::ofstream f("test_long.bin", std::ios::binary);
        f.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        f.close();
        std::cout << "Generated test_long.bin (" << serialized.size() << " bytes, type="
                  << static_cast<int>(val.type()) << ")" << std::endl;
    }

    // ULong value (type 7) - 32-bit enforced
    {
        ulong_value val("ulong_32bit", 3500000000UL);
        auto serialized = val.serialize();
        std::ofstream f("test_ulong.bin", std::ios::binary);
        f.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        f.close();
        std::cout << "Generated test_ulong.bin (" << serialized.size() << " bytes)" << std::endl;
    }

    // LLong value (type 8) - 64-bit
    {
        llong_value val("llong_64bit", 5000000000LL);
        auto serialized = val.serialize();
        std::ofstream f("test_llong.bin", std::ios::binary);
        f.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        f.close();
        std::cout << "Generated test_llong.bin (" << serialized.size() << " bytes)" << std::endl;
    }

    // ULLong value (type 9) - 64-bit
    {
        ullong_value val("ullong_64bit", 10000000000ULL);
        auto serialized = val.serialize();
        std::ofstream f("test_ullong.bin", std::ios::binary);
        f.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        f.close();
        std::cout << "Generated test_ullong.bin (" << serialized.size() << " bytes)" << std::endl;
    }

    // Float value (type 10)
    {
        float_value val("float_pi", 3.14159f);
        auto serialized = val.serialize();
        std::ofstream f("test_float.bin", std::ios::binary);
        f.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        f.close();
        std::cout << "Generated test_float.bin (" << serialized.size() << " bytes)" << std::endl;
    }

    // Double value (type 11)
    {
        double_value val("double_pi", 3.141592653589793);
        auto serialized = val.serialize();
        std::ofstream f("test_double.bin", std::ios::binary);
        f.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        f.close();
        std::cout << "Generated test_double.bin (" << serialized.size() << " bytes)" << std::endl;
    }

    // Bytes value (type 12)
    {
        std::vector<uint8_t> bytes_data = {0x01, 0x02, 0x03, 0xFF, 0xFE};
        bytes_value val("bytes_test", bytes_data);
        auto serialized = val.serialize();
        std::ofstream f("test_bytes.bin", std::ios::binary);
        f.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        f.close();
        std::cout << "Generated test_bytes.bin (" << serialized.size() << " bytes)" << std::endl;
    }

    // String value (type 13)
    {
        string_value val("string_hello", "Hello from C++!");
        auto serialized = val.serialize();
        std::ofstream f("test_string.bin", std::ios::binary);
        f.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        f.close();
        std::cout << "Generated test_string.bin (" << serialized.size() << " bytes)" << std::endl;
    }

    // UTF-8 String
    {
        string_value val("string_utf8", "UTF-8: 한글 테스트");
        auto serialized = val.serialize();
        std::ofstream f("test_string_utf8.bin", std::ios::binary);
        f.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        f.close();
        std::cout << "Generated test_string_utf8.bin (" << serialized.size() << " bytes)" << std::endl;
    }

    std::cout << "\nAll test files generated successfully!" << std::endl;
    std::cout << "These files can be used to test cross-language deserialization." << std::endl;

    return 0;
}
