#include "utils.h"

#include <gtest/gtest.h>

TEST(Utils,TestStringSplitting){
    std::vector<std::string>chunks = split_string("this hit that ice cold");
    ASSERT_EQ(chunks.size(),5);
}