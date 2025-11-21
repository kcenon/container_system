#include <cstdint>
#include <vector>
#include <string>
#include "container/core/container.h"

// Fuzzer entry point
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // Don't test empty input as it's trivial
    if (Size == 0) {
        return 0;
    }

    // Construct a vector from the raw data
    std::vector<uint8_t> input_data(Data, Data + Size);

    try {
        // Fuzz the constructor that parses the header and values
        // We alternate based on the first byte's parity to cover both paths
        // bit 0: parse_header_only
        bool parse_header_only = (Data[0] & 1) == 0;
        
        container_module::value_container container(input_data, parse_header_only);

        // Exercise read methods to ensure internal state is consistent
        [[maybe_unused]] auto src = container.source_id();
        [[maybe_unused]] auto type = container.message_type();

        // Exercise serialization paths
        // These should be safe even if the container contains garbage (as long as it was constructed "successfully")
        // Note: The constructor might throw if the data is completely invalid, which is fine.
        if (!parse_header_only) {
            container.to_json();
            container.serialize_array();
        }
        
    } catch (...) {
        // Catch all exceptions. The fuzzer should only fail on crashes (segfaults, ASan violations),
        // not on handled C++ exceptions (e.g., parse errors).
    }

    return 0;
}
