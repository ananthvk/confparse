#ifndef A_CONFPARSE_H
#define A_CONFPARSE_H
#include <fstream>
#include <map>
#include <sstream>
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

/*
inline Config from_stream(std::istream &is) {}

inline Config from_str(const std::string &s)
{
    std::istringstream iss(s);
    return from_stream(iss);
}

inline Config from_file(const std::string &filepath)
{
    std::ifstream ifs(filepath);
    return from_stream(ifs);
}
*/

} // namespace confparse
#endif // A_CONFPARSE_H