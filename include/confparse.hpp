#ifndef A_CONFPARSE_H
#define A_CONFPARSE_H
#include <fstream>
#include <map>
#include <sstream>
#include <string.h>
#include <variant>

namespace confparse
{

/**
 * @brief This class represents a value in the configuration, it can either be an integer, double or
 * a string
 */
class ValueType
{
    std::variant<std::monostate, long long int, long double, std::string> val;
    bool is_empty_;

  public:
    ValueType() : is_empty_(true) {}

    template <typename T> ValueType(const T &value) : val{value}, is_empty_{false} {}

    bool is_empty() const { return is_empty_; }

    std::string as_string() const { return std::get<std::string>(val); }

    long long int as_int() const { return std::get<long long int>(val); }

    long double as_real() const { return std::get<long double>(val); }
};

class parse_error : public std::runtime_error
{
  public:
    parse_error(const std::string &message) : std::runtime_error(message) {}
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
    std::string whitespace_characters;
    std::string delimiters;
    bool should_empty_lines_be_skipped;
    bool should_parse_all_values_as_string;
    bool should_allow_empty_values, should_allow_empty_lines;
    bool should_lines_be_left_trimmed;
    bool should_lines_be_right_trimmed;
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

        if (options.should_parse_all_values_as_string)
        {
            return std::make_pair(key, value);
        }
        // TODO: Other formats
    }


  public:
    ConfigParser()
    {
        options.whitespace_characters = " \t\r";
        options.delimiters = "=";
        options.should_allow_empty_lines = true;
        options.should_allow_empty_values = true;
        options.should_empty_lines_be_skipped = true;
        options.should_parse_all_values_as_string = true;
        options.should_lines_be_left_trimmed = true;
        options.should_lines_be_right_trimmed = true;
    }

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