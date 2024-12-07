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
 * Note: True, true, 1 evaluate to true, while double or any other value evaluates to false
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
        if constexpr (std::is_same<std::decay_t<T>, std::string>::value ||  // Handle strings
                      std::is_same<std::decay_t<T>, const char *>::value || // Handle const char*
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
            static_assert(False<T>{}, "Unsupported type for ValueType");
        }
    }

    bool is_empty() const { return is_empty_; }

    std::string as_string() const { return val; }

    template <typename T> T parse() const
    {
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
    ConfigParserOptions options;

    // Line number of current line being processed, useful in error messages
    size_t line_no;

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
        oss << "Syntax error at line " << line_no << ": " << message;
        throw parse_error(oss.str());
    }

    std::string preprocess_line(std::string line)
    {
        if (options.should_lines_be_left_trimmed)
            ltrim(line);
        if (options.should_lines_be_right_trimmed)
            rtrim(line);

        if (!options.should_allow_empty_lines && line.empty())
            throw_error("Empty line found");

        if (options.should_allow_comments)
        {
            auto pos = line.find_first_of(options.single_line_comments);
            if (pos != std::string::npos)
            {
                // A comment character was found
                // TODO: Support escaping of comment char in string
                // TODO: Also allow flag for inline comments
                line.erase(pos);
            }
        }

        return line;
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
    ConfigParser() {}

    Config from_stream(std::istream &is)
    {
        Config cfg;
        std::string line;
        line_no = 0;
        while (std::getline(is, line))
        {
            ++line_no;
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
        return from_stream(ifs);
    }
};

} // namespace confparse
#endif // A_CONFPARSE_H