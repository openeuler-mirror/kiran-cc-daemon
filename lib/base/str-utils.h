/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <json/json.h>
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
        return format_to(ctx.out(), "{0}", str.raw());
    }
};
}  // namespace fmt

namespace Kiran
{
class StrUtils
{
public:
    StrUtils(){};
    virtual ~StrUtils(){};

    static std::vector<std::string> split_lines(const std::string &s);
    static std::string tolower(const std::string &str);
    static std::string toupper(const std::string &str);
    static std::vector<std::string> split_with_char(const std::string &s, char delimiter, bool is_merge_delimiter = false);
    static std::vector<std::string> split_once_with_char(const std::string &s, char delimiter);

    // 去掉字符串前后的空白字符
    static std::string ltrim(const std::string &s);
    static std::string rtrim(const std::string &s);
    static std::string trim(const std::string &s);

    // json字符串和Json::Value相互转化
    static std::string json2str(const Json::Value &json);
    static Json::Value str2json(const std::string &str);

    // 判断str是否以prefix字符串开头
    static bool startswith(const std::string &str, const std::string &prefix);
    static bool endswith(const std::string &str, const std::string &suffix);

    // 字符串包含任何一个子串则返回true，否则返回false
    static bool contains_oneof_substrs(const std::string &str, const std::vector<std::string> &substrs);
    // 字符串包含所有子串则返回true，否则返回false
    static bool contains_allof_substrs(const std::string &str, const std::vector<std::string> &substrs);

    // 格式化日期
    static std::string timestamp2str(time_t t);
    static std::string gdate2str(const GDate *date);
    static std::string tm2str(const struct tm *tm_time);

    template <class T>
    static std::string join(const std::vector<T> &vec, const std::string &join_chars);
};

template <class T>
std::string StrUtils::join(const std::vector<T> &vec, const std::string &join_chars)
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