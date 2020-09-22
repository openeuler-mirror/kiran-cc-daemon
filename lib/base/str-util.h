/*
 * @Author       : tangjie02
 * @Date         : 2020-08-17 15:13:15
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 14:32:04
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/lib/base/str-util.h
 */
#pragma once

#include "lib/base/base.h"

namespace fmt
{
template <>
struct formatter<Glib::ustring>
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext &ctx) const
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    format_context::iterator format(const Glib::ustring &str, FormatContext &ctx)
    {
        return format_to(ctx.begin(), "{0}", str.raw());
    }
};
}  // namespace fmt

namespace Kiran
{
class StrUtil
{
public:
    StrUtil(){};
    virtual ~StrUtil(){};

    static std::vector<std::string> split_lines(const std::string &s);
    static std::string tolower(const std::string &str);
    static std::string toupper(const std::string &str);
    static std::vector<std::string> split_with_char(const std::string &s, char delimiter, bool is_merge_delimiter = false);

    template <class T>
    static std::string join(const std::vector<T> &vec, const std::string &join_chars);
};

template <class T>
std::string StrUtil::join(const std::vector<T> &vec, const std::string &join_chars)
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