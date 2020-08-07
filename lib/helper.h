/*
 * @Author       : tangjie02
 * @Date         : 2020-06-05 15:21:58
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-07 11:31:38
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/lib/helper.h
 */

#pragma once

#include <fmt/format.h>
#include <stdio.h>

#include <functional>
#include <vector>
namespace Kiran
{
#define CONNECTION(text1, text2) text1##text2
#define CONNECT(text1, text2) CONNECTION(text1, text2)

class Defer
{
public:
    Defer(std::function<void(std::string)> func, std::string fun_name) : func_(func),
                                                                         fun_name_(fun_name) {}
    ~Defer() { func_(fun_name_); }

private:
    std::function<void(std::string)> func_;
    std::string fun_name_;
};

// helper macro for Defer class
#define SCOPE_EXIT(block) Defer CONNECT(_defer_, __LINE__)([&](std::string _arg_function) block, __FUNCTION__)

#define RETURN_VAL_IF_FALSE(cond, val) \
    {                                  \
        if (!(cond)) return val;       \
    }

#define RETURN_VAL_IF_TRUE(cond, val) \
    {                                 \
        if (cond) return val;         \
    }

#define RETURN_IF_FALSE(cond) \
    {                         \
        if (!(cond)) return;  \
    }

#define RETURN_IF_TRUE(cond) \
    {                        \
        if (cond) return;    \
    }

#define CONTINUE_IF_FALSE(cond) \
    {                           \
        if (!(cond)) continue;  \
    }

#define CONTINUE_IF_TRUE(cond) \
    {                          \
        if (cond) continue;    \
    }

#define IGNORE_EXCEPTION(expr)       \
    {                                \
        try                          \
        {                            \
            expr;                    \
        }                            \
        catch (const Glib::Error &e) \
        {                            \
        }                            \
    }

#define POINTER_TO_STRING(p) ((p) ? p : std::string())

using StringHash = uint32_t;

constexpr StringHash prime = 9973;
constexpr StringHash basis = 0xCBF29CE4ul;
constexpr StringHash hash_compile_time(char const *str, StringHash last_value = basis)
{
    return *str ? hash_compile_time(str + 1, (StringHash)((*str ^ last_value) * (uint64_t)prime)) : last_value;
}

inline StringHash shash(char const *str)
{
    StringHash ret{basis};

    while (*str)
    {
        ret ^= *str;
        ret *= prime;
        str++;
    }

    return ret;
}

/// compile-time hash of string.
/// usage: "XXX"_hash
constexpr StringHash operator"" _hash(char const *p, size_t)
{
    return hash_compile_time(p);
}

std::vector<std::string> split_lines(const std::string &s);
std::string str_tolower(const std::string &str);

template <class T>
std::string join_vector(const std::vector<T> &vec, const std::string &join_chars)
{
    std::string str;
    for (size_t i = 0; i < vec.size(); ++i)
    {
        str += fmt::format("{0}", vec[i]);
        if (i + 1 < vec.size())
        {
            str += fmt::format("{0}", join_chars);
        }
    }
    return str;
}

}  // namespace Kiran