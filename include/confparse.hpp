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

class ConfigParser
{
  public:
    Config from_stream(std::istream &is)
    {
        Config cfg;
        std::string line;
        size_t line_no = 1;
        while (std::getline(is, line))
        {
            size_t index;
            // No equals sign was found, invalid syntax
            if ((index = line.find('=')) == std::string::npos)
            {
                std::ostringstream oss;
                oss << "Syntax error at line " << line_no << ": No '=' found";
                throw parse_error(oss.str());
            }

            // Split the line into key and value part
            auto key = line.substr(0, index);
            auto value = line.substr(index + 1);

            cfg[key] = value;

            ++line_no;
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