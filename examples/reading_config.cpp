#include "confparse.hpp"
#include <iostream>

int main()
{
    try
    {
        confparse::ConfigParser parser;
        auto cfg = parser.from_file("example_config.cfg");
        for (const auto &kv : cfg)
        {
            std::cout << kv.first << ": " << kv.second.as_string() << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}
