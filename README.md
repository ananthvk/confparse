# confparse
A C++ header only library to parse simple INI like configuration files.

## How to use?
This is a header only library, so it does not require additional steps, just copy `confparse.h` from `include/` directory and use it in your project.

## Running tests
Install Meson, and a backend (such as Ninja) and run the following commands:

```
$ meson wrap install gtest
```

If you have clang, along with sanitizers installed
```
$ CXX=clang++ meson setup -Db_sanitize=address -Ddevelopment=true -Denable-tests=true -Db_lundef=false --reconfigure builddir
```

Otherwise
```
$ meson setup -Denable-tests=true --reconfigure builddir
```

```
$ cd builddir
$ meson test -v
```

## Example usage

```cpp
#include "confparse.hpp"
#include <iostream>

int main()
{
    try
    {
        confparse::ConfigParser parser;
        auto cfg = parser.from_str(R"(
            # General settings
            Version=0.0.1                             # App version
            AppName=confparse                         # Name of the app
            URL=https://github.com/ananthvk/confparse # Repo URL
            
            # Additional details
            AppId = 137
            Score = 8.15
            ; Another style of comments
        )");
        // Print all the keys
        for (const auto &kv : cfg)
        {
            std::cout << kv.first << ": " << kv.second.as_string() << std::endl;
        }
        // Get specific details
        std::cout << "App score: " << cfg.get("Score").parse<float>() << std::endl;
        std::cout << "App Id: " << cfg.get("AppId").parse<float>() << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}
```

## Customizing usage
This is the default values for various options used by the parser.
```cpp
struct ConfigParserOptions
{
    std::string whitespace_characters = " \t\r";
    std::string delimiters = "=";
    // Only single character single line comments supported
    std::string single_line_comments = "#;";
    std::string string_delimiter_characters = "\"'";
    char escape_character = '\\';
    bool should_allow_empty_lines = true;
    bool should_allow_empty_values = true;
    bool should_empty_lines_be_skipped = true;
    bool should_lines_be_left_trimmed = true;
    bool should_lines_be_right_trimmed = true;
    bool should_keys_be_trimmed = true;
    bool should_values_be_trimmed = true;
    bool should_allow_comments = true;
};
```

You can modify these options in the parser object, for example to change the delimiter to `:`
```
confparse::ConfigParser parser;
parser.options.delimiters = ":";
// Rest of the code
```
## TODO
- Implement flag to allow/disallow inline comments
- Implement sections