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

    ASSERT_EQ(cfg.get("KEY3").parse<int>(), 123456789);
    ASSERT_EQ(cfg.get("KEY1").as_string(), "This is the value for this key");
    ASSERT_EQ(cfg.get("KEY2").as_string(), "");
    ASSERT_NEAR(cfg.get("KEY4").parse<double>(), 3.1415, 0.001);

    ASSERT_EQ(cfg.size(), 4);

    cfg.set("KEY4", "this is a string");
    ASSERT_EQ(cfg.get("KEY4").as_string(), "this is a string");
    ASSERT_EQ(cfg.get("KEY4").is_empty(), false);

    cfg.erase("KEY4");
    ASSERT_EQ(cfg.get("KEY4").is_empty(), true);

    cfg["floatkey"] = 3.89;
    cfg["numkey"] = 3;
    cfg["strkey"] = "This is string!!";

    ASSERT_NEAR(cfg.get("floatkey").parse<float>(), 3.89, 0.01);
    ASSERT_EQ(cfg.get("numkey").parse<int>(), 3);
    ASSERT_EQ(cfg.get("strkey").as_string(), "This is string!!");
}

TEST(TestConfig, Bools)
{
    confparse::Config cfg;
    cfg.set("Key1", true);
    cfg.set("Key2", false);
    ASSERT_EQ(cfg.get("Key1").as_string(), "True");
    ASSERT_EQ(cfg.get("Key2").as_string(), "False");
    ASSERT_EQ(cfg.get("Key1").parse<bool>(), true);
    ASSERT_EQ(cfg.get("Key2").parse<bool>(), false);
}

TEST(TestConfigParser, SimpleParsingStrings)
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

    cfg = parser.from_str(R"(
        AppName=ExampleApp
        Version=1.0.0
        Language=en
        Value=
        Empty=
    )");

    ASSERT_EQ(cfg.get("AppName").as_string(), "ExampleApp");
    ASSERT_EQ(cfg.get("Version").as_string(), "1.0.0");
    ASSERT_EQ(cfg.get("Language").as_string(), "en");
    ASSERT_EQ(cfg.size(), 5);
}

TEST(TestConfigParser, SimpleParsingStringsErrorConditions)
{
    confparse::ConfigParser parser;
    auto cfg = parser.from_str("");
    ASSERT_EQ(cfg.size(), 0);
    
    EXPECT_THROW(parser.from_str("=Value"), confparse::parse_error);
    EXPECT_THROW(parser.from_str("ValueKey"), confparse::parse_error);
}

TEST(TestConfigParser, SimpleParsingStringsWithComments)
{
    confparse::ConfigParser parser;
    auto cfg = parser.from_str("");
    ASSERT_EQ(cfg.size(), 0);

    cfg = parser.from_str("Key=Value");
    ASSERT_EQ(cfg.get("Key").as_string(), "Value");

    cfg = parser.from_str("Key=Value    # This is the first key\nSecond=Third    # This is the second key   \nFourth=Fifth # This is another key\n# Comment line");
    ASSERT_EQ(cfg.get("Key").as_string(), "Value");
    ASSERT_EQ(cfg.get("Second").as_string(), "Third");
    ASSERT_EQ(cfg.get("Fourth").as_string(), "Fifth");

    cfg = parser.from_str(R"(
        # Name of the app, note it should be capitalized
        AppName=ExampleApp
        # App version
        Version=1.0.0
        # Do not use
        Value=
        ; Another type of comment
        Empty=
        ; Comments here
        ; Another line comment
        # More comments
        # Language, can be two digit language code
        Language=en
    )");

    ASSERT_EQ(cfg.get("AppName").as_string(), "ExampleApp");
    ASSERT_EQ(cfg.get("Version").as_string(), "1.0.0");
    ASSERT_EQ(cfg.get("Language").as_string(), "en");
    ASSERT_EQ(cfg.size(), 5);
}


TEST(TestConfigParser, SimpleParsingNumbers)
{
    confparse::ConfigParser parser;
    auto cfg = parser.from_str("");
    ASSERT_EQ(cfg.size(), 0);

    cfg = parser.from_str("Key=3");
    ASSERT_EQ(cfg.get("Key").parse<int>(), 3);

    cfg = parser.from_str("Key=1888\nSecond=-512\nFourth=-3.1415\n");
    ASSERT_EQ(cfg.get("Key").parse<int>(), 1888);
    ASSERT_EQ(cfg.get("Second").parse<int>(), -512);
    ASSERT_NEAR(cfg.get("Fourth").parse<double>(), -3.1415, 0.01);

    cfg = parser.from_str(R"(
        AppId=1552
        AppScore=3172.3421
        AppInstalled1=1
        AppInstalled2=True
        AppInstalled3=true
        AppInstalled4=False
        AppInstalled5=False
        AppInstalled6=0
        AppInstalled7=SomethingElse
    )");

    ASSERT_EQ(cfg.get("AppId").parse<int>(), 1552);
    ASSERT_NEAR(cfg.get("AppScore").parse<double>(), 3172.3421, 0.01);
    ASSERT_EQ(cfg.get("AppInstalled1").parse<int>(), 1);
    ASSERT_EQ(cfg.get("AppInstalled1").parse<bool>(), true);
    ASSERT_EQ(cfg.get("AppInstalled2").parse<bool>(), true);
    ASSERT_EQ(cfg.get("AppInstalled3").parse<bool>(), true);
    ASSERT_EQ(cfg.get("AppInstalled4").parse<bool>(), false);
    ASSERT_EQ(cfg.get("AppInstalled5").parse<bool>(), false);
    ASSERT_EQ(cfg.get("AppInstalled6").parse<bool>(), false);
    ASSERT_EQ(cfg.get("AppInstalled6").parse<int>(), 0);

    EXPECT_THROW(cfg.get("AppInstalled7").parse<bool>(), confparse::parse_error);
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}