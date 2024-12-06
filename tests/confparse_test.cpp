#include "confparse.hpp"
#include <gtest/gtest.h>

TEST(TestConparse, Addition)
{
    auto result = confparse::add(3, 5);
    ASSERT_EQ(result, 8);
    result = confparse::add(4, 5);
    ASSERT_EQ(result, 9);
    result = confparse::add(5, 5);
    ASSERT_EQ(result, 10);
}

TEST(TestConfparse, Addition2)
{
    auto result = confparse::add(3, 5);
    ASSERT_EQ(result, 8);
    result = confparse::add(4, 5);
    ASSERT_EQ(result, 9);
    result = confparse::add(5, 5);
    ASSERT_EQ(result, 10);
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}