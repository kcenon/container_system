// BSD 3-Clause License
// Copyright (c) 2026, kcenon
// See the LICENSE file in the project root for full license information.

#include "utilities/core/formatter.h"
#include <gtest/gtest.h>
#include <iterator>
#include <string>
#include <vector>

class FormatterBufferTest : public testing::TestWithParam<size_t>
{
};

TEST_P(FormatterBufferTest, AppendsCompletePayloadAndSuffix)
{
    const std::string payload(GetParam(), 'X');
    std::string output = "prefix:";
    utility_module::formatter::format_to(std::back_inserter(output), "[large,12,{}];", payload);
    EXPECT_EQ(output, "prefix:[large,12," + payload + "];");

    // The wrapper accepts output iterators for containers other than string.
    std::vector<char> bytes;
    utility_module::formatter::format_to(std::back_inserter(bytes), "{};", payload);
    EXPECT_EQ(std::string(bytes.begin(), bytes.end()), payload + ";");
}

INSTANTIATE_TEST_SUITE_P(BufferBoundaries, FormatterBufferTest,
                         testing::Values(0u, 255u, 256u, 257u, 512u, 1024u, 10u * 1024u,
                                         1024u * 1024u));

#if UTILITY_MODULE_HAS_STD_FORMAT
TEST(FormatterTest, InvalidFormatDoesNotAppendPartialOutput)
{
    std::string output = "prefix:";
    utility_module::formatter::format_to(std::back_inserter(output), "{} {", 42);
    EXPECT_EQ(output, "prefix:{} {");
}
#endif
