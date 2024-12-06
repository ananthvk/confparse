#include "cppstarter.hpp"
#include <gtest/gtest.h>

TEST(TestCppstarter, Addition)
{
    auto result = cppstarter::add(3, 5);
    ASSERT_EQ(result, 8);
    result = cppstarter::add(4, 5);
    ASSERT_EQ(result, 9);
    result = cppstarter::add(5, 5);
    ASSERT_EQ(result, 10);
}

TEST(TestCppstarter, Addition2)
{
    auto result = cppstarter::add(3, 5);
    ASSERT_EQ(result, 8);
    result = cppstarter::add(4, 5);
    ASSERT_EQ(result, 9);
    result = cppstarter::add(5, 5);
    ASSERT_EQ(result, 10);
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}