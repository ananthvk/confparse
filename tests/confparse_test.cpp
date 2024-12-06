#include "confparse.hpp"
#include <gtest/gtest.h>

TEST(TestConfig, EmptyConfig)
{
    confparse::Config cfg;
    ASSERT_EQ(cfg.size(), 0);
}

TEST(TestConfig, GetAndSet)
{
    confparse::Config cfg;
    ASSERT_EQ(cfg.get("non existent key").is_empty(), true);
    ASSERT_EQ(cfg.get("").is_empty(), true);
    ASSERT_EQ(cfg.get("Some other key").is_empty(), true);

    cfg.set("KEY1", "This is the value for this key");
    cfg.set("KEY2", "");
    cfg.set("KEY3", 123456789);
    cfg.set("KEY4", 3.1415);

    ASSERT_EQ(cfg.get("KEY3").as_int(), 123456789);
    ASSERT_EQ(cfg.get("KEY1").as_string(), "This is the value for this key");
    ASSERT_EQ(cfg.get("KEY2").as_string(), "");
    ASSERT_NEAR(cfg.get("KEY4").as_real(), 3.1415, 0.001);

    ASSERT_EQ(cfg.size(), 4);

    cfg.set("KEY4", "this is a string");
    ASSERT_EQ(cfg.get("KEY4").as_string(), "this is a string");
    ASSERT_EQ(cfg.get("KEY4").is_empty(), false);

    cfg.erase("KEY4");
    ASSERT_EQ(cfg.get("KEY4").is_empty(), true);

    cfg["floatkey"] = 3.89;
    cfg["numkey"] = 3;
    cfg["strkey"] = "This is string!!";

    ASSERT_NEAR(cfg.get("floatkey").as_real(), 3.89, 0.01);
    ASSERT_EQ(cfg.get("numkey").as_int(), 3);
    ASSERT_EQ(cfg.get("strkey").as_string(), "This is string!!");
}

TEST(TestConfigParser, SimpleParsing)
{
    confparse::ConfigParser parser;
    auto cfg = parser.from_str("");
    ASSERT_EQ(cfg.size(), 0);

    cfg = parser.from_str("Key=Value");
    ASSERT_EQ(cfg.get("Key").as_string(), "Value");

    cfg = parser.from_str("Key=Value\nSecond=Third\nFourth=Fifth\n");
    ASSERT_EQ(cfg.get("Key").as_string(), "Value");
    ASSERT_EQ(cfg.get("Second").as_string(), "Third");
    ASSERT_EQ(cfg.get("Fourth").as_string(), "Fifth");
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}