#include "container/core/container.h"
#include "container/values/string_value.h"
#include "utilities/core/formatter.h"
#include <iostream>

using namespace container_module;
using namespace utility_module;

int main() {
    // Check which formatter is being used
    #if HAS_STD_FORMAT == 1
        std::cout << "Using std::format" << std::endl;
    #elif HAS_STD_FORMAT == 0
        std::cout << "Using fmt library" << std::endl;
    #else
        std::cout << "Using FALLBACK (no formatting!)" << std::endl;
    #endif
    std::cout << std::endl;
    // Create container
    auto container = std::make_unique<value_container>();
    container->set_source("src", "sub");
    container->set_target("tgt", "");
    container->set_message_type("test");

    // Add values
    container->add(std::make_shared<string_value>("str", "hello"));

    // Serialize
    std::string serialized = container->serialize();
    std::cout << "=== Serialized data ===" << std::endl;
    std::cout << serialized << std::endl;
    std::cout << "=== Length: " << serialized.size() << " ===" << std::endl;

    // Deserialize
    auto new_container = std::make_unique<value_container>(serialized, false);

    std::cout << "\n=== Deserialized values ===" << std::endl;
    std::cout << "source_id: '" << new_container->source_id() << "'" << std::endl;
    std::cout << "source_sub_id: '" << new_container->source_sub_id() << "'" << std::endl;
    std::cout << "target_id: '" << new_container->target_id() << "'" << std::endl;
    std::cout << "message_type: '" << new_container->message_type() << "'" << std::endl;

    auto val = new_container->get_value("str");
    std::cout << "get_value('str'): '" << val->to_string() << "'" << std::endl;

    return 0;
}
