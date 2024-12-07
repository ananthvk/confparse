#ifndef A_CONFPARSE_H
#define A_CONFPARSE_H
#include <fstream>
#include <map>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

namespace confparse
{

/**
 * @brief This class represents a value in the configuration, it can either be an integer, double,
 * a string, or a boolean
 * Note: True, true, 1 evaluate to true. False, false, 0 evaluate to false, All other values result
 * in an error
 * @note The value is always stored as a string, and converted when required, so if you are
 * repeatedly using a value, store it in a variable before using it to improve performance.
 * Also an empty value is not the same as an empty string
 */


class parse_error : public std::runtime_error
{
  public:
    parse_error(const std::string &message) : std::runtime_error(message) {}
};

template <class...> struct False : std::bool_constant<false>
{
};

class ValueType
{
    std::string val;
    bool is_empty_;

  public:
    ValueType() : is_empty_(true) {}

    template <typename T> ValueType(const T &value) : is_empty_{false}
    {
        if constexpr (std::is_same<T, bool>::value)
        {
            // Special handling for bool, set the value as True or False
            val = (value) ? "True" : "False";
        }
        else if constexpr (std::is_same<std::decay_t<T>, std::string>::value || // Handle strings
                           std::is_same<std::decay_t<T>,
                                        const char *>::value || // Handle const char*
                           std::is_same<std::decay_t<std::remove_extent_t<T>>,
                                        char>::value) // Handle const char[N] arrays
        {
            val = value;
        }
        else if constexpr (std::is_arithmetic<T>::value)
        {
            // Convert numeric types to string
            val = std::to_string(value);
        }
        else
        {
            // Try using stringstream to attempt an conversion
            std::ostringstream oss;
            oss << value;
            val = oss.str();
        }
    }

    bool is_empty() const { return is_empty_; }

    std::string as_string() const { return val; }

    template <typename T> T parse() const
    {
        if constexpr (std::is_same<T, bool>::value)
        {
            if (val == "True" || val == "true" || val == "1")
            {
                return true;
            }
            if (val == "False" || val == "false" || val == "0")
            {
                return false;
            }
            else
            {
                throw parse_error("Parse error: Invalid literal for bool\n    | " + val);
            }
        }
        std::istringstream iss(val);
        T parsed_val;
        if ((!(iss >> parsed_val)) || !(iss.eof()))
        {
            // If the value could not be parsed, or if there are some more characters
            throw parse_error("Parse error: Invalid literal during parse()\n    | " + val);
        }
        return parsed_val;
    }

    // Return default_value if the value cannot be parsed
    template <typename T> T try_parse(T default_value) const
    {
        if constexpr (std::is_same<T, bool>::value)
        {
            if (val == "True" || val == "true" || val == "1")
            {
                return true;
            }
            if (val == "False" || val == "false" || val == "0")
            {
                return false;
            }
            else
            {
                return default_value;
            }
        }
        std::istringstream iss(val);
        T parsed_val;
        if ((!(iss >> parsed_val)) || !(iss.eof()))
        {
            return default_value;
        }
        return parsed_val;
    }

    // Checks if the value can be parsed into the required type
    template <typename T> bool is() const
    {
        if constexpr (std::is_same<T, bool>::value)
        {
            if (val == "True" || val == "true" || val == "1" || val == "False" || val == "false" ||
                val == "0")
                return true;
            else
                return false;
        }
        std::istringstream iss(val);
        T parsed_val;
        if ((!(iss >> parsed_val)) || !(iss.eof()))
        {
            return false;
        }
        return true;
    }
};

/**
 * @brief This class represents a configuration object which is a collection of key-value pairs,
 * where key is a string and value is an integer, string, or a real number
 */
class Config
{
  public:
    const ValueType &get(const std::string &key) const
    {
        auto iter = cfg_map.find(key);
        if (iter == cfg_map.end())
        {
            return empty_value;
        }
        return iter->second;
    }

    template <typename T> void set(const std::string &key, const T &value)
    {
        cfg_map[key] = ValueType{value};
    }

    size_t size() const { return cfg_map.size(); }

    ValueType &operator[](const std::string &key) { return cfg_map[key]; }

    void erase(const std::string &key)
    {
        auto iter = cfg_map.find(key);
        if (iter != cfg_map.end())
        {
            cfg_map.erase(iter);
        }
    }

    auto begin() { return cfg_map.begin(); }

    auto end() { return cfg_map.end(); }

    auto cbegin() { return cfg_map.cbegin(); }

    auto cend() { return cfg_map.cend(); }

  private:
    ValueType empty_value;
    std::map<std::string, ValueType> cfg_map;
};

struct ConfigParserOptions
{
    std::string whitespace_characters = " \t\r";
    std::string delimiters = "=";
    // Only single character single line comments supported
    std::string single_line_comments = "#;";
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

class ConfigParser
{
  private:

    // Line number of current line being processed, useful in error messages
    size_t line_no;
    // Also store current line, useful for error reporting
    std::string current_line;

    // Trim the string by removing spaces at both ends, inplace
    std::string &ltrim(std::string &s)
    {
        s.erase(0, s.find_first_not_of(options.whitespace_characters));
        return s;
    }

    std::string &rtrim(std::string &s)
    {
        s.erase(s.find_last_not_of(options.whitespace_characters) + 1);
        return s;
    }

    void throw_error(const std::string &message)
    {
        std::ostringstream oss;
        oss << "Syntax error: " << message << "\n";
        oss << "Line " << line_no << " | " << current_line << "\n";
        throw parse_error(oss.str());
    }

    std::string preprocess_line(std::string line)
    {
        if (options.should_lines_be_left_trimmed)
            ltrim(line);
        if (options.should_lines_be_right_trimmed)
            rtrim(line);

        if (line.empty())
        {
            if (!options.should_allow_empty_lines)
                throw_error("Empty line found");
            return line;
        }

        if (!options.should_allow_comments)
        {
            return line;
        }

        // If comments are allowed, this code should remove comment escape sequence
        // i.e. if the comment characters are ; and #
        // \# and \; should become just # and ;
        // It ignores all other escape sequences

        char previous_character = line[0];
        auto is_first_char_comment = options.single_line_comments.find(line[0]) != std::string::npos;
        if (is_first_char_comment)
        {
            // If the first character is itself a comment character, skip the entire process
            return "";
        }

        std::string line_cpy;
        line_cpy.reserve(line.size());

        line_cpy.push_back(previous_character);

        for (size_t i = 1; i < line.size(); ++i)
        {
            auto is_comment_char = options.single_line_comments.find(line[i]) != std::string::npos;
            if (is_comment_char)
            {
                if (previous_character == options.escape_character)
                    line_cpy.pop_back();
                else
                    break;
            }

            line_cpy.push_back(line[i]);
            previous_character = line[i];
        }
        return line_cpy;
    }

    // Returns the index of the delimiter in the line, if the delimiter is not found, or is at an
    // invalid position, raises an error
    size_t find_delimiter(const std::string &line)
    {
        size_t index = line.find_first_of(options.delimiters);
        if (index == std::string::npos)
            throw_error("No delimiter found");

        // Check if the delimiter is at the beginning of the line or at the end
        if (index == 0)
            throw_error("Empty key, delimiter at beginning of line");
        if ((index + 1) == line.size() && !options.should_allow_empty_values)
            throw_error("Empty value, delimiter at end of line");
        return index;
    }

    std::pair<std::string, ValueType> parse_line(const std::string &line, size_t delimiter_index)
    {
        // Split the line into key and value part
        auto key = line.substr(0, delimiter_index);
        auto value = line.substr(delimiter_index + 1);

        if (options.should_keys_be_trimmed)
        {
            rtrim(key);
            ltrim(key);
        }

        if (options.should_values_be_trimmed)
        {
            ltrim(value);
            rtrim(value);
        }

        return std::make_pair(key, value);
    }


  public:
    ConfigParserOptions options;

    ConfigParser() {}

    Config from_stream(std::istream &is)
    {
        Config cfg;
        std::string line;
        line_no = 0;
        while (std::getline(is, line))
        {
            ++line_no;
            current_line = line;
            line = preprocess_line(line);
            if (line.empty())
                continue;
            auto delimiter_index = find_delimiter(line);
            auto kv = parse_line(line, delimiter_index);
            cfg[kv.first] = kv.second;
        }
        if (is.bad())
        {
            std::ostringstream oss;
            oss << "Error while reading config file: " << strerror(errno);
            throw parse_error(oss.str());
        }
        return cfg;
    }

    Config from_str(const std::string &s)
    {
        std::istringstream iss(s);
        return from_stream(iss);
    }

    Config from_file(const std::string &filepath)
    {
        std::ifstream ifs(filepath);
        if (!ifs)
        {
            std::ostringstream oss;
            oss << "File error: Error while reading config file: " << strerror(errno);
            throw parse_error(oss.str());
        }
        return from_stream(ifs);
    }
};

} // namespace confparse
#endif // A_CONFPARSE_H