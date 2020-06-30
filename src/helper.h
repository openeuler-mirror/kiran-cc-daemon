/*
 * @Author       : tangjie02
 * @Date         : 2020-06-05 15:21:58
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-18 19:48:58
 * @Description  : 
 * @FilePath     : /kiran-session-daemon/src/helper.h
 */

#pragma once

namespace Kiran
{
class Defer
{
public:
    Defer(std::function<void()> func) : func_(func) {}
    ~Defer() { func_(); }

private:
    std::function<void()> func_;
};

// helper macro for Defer class
#define SCOPE_EXIT(block)              \
    auto _arg_function = __FUNCTION__; \
    Defer _defer_##__LINE__([&]() block)

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

}  // namespace Kiran