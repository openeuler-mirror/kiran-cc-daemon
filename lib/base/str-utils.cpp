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

#include "lib/base/str-utils.h"

#include <algorithm>

namespace Kiran
{
std::vector<std::string> StrUtils::split_lines(const std::string &s)
{
    std::vector<std::string> ret;
    size_t i = 0;
    size_t line_start = 0;
    while (i < s.length())
    {
        if (s[i] == '\n')
        {
            ret.push_back(s.substr(line_start, i - line_start));
            i++;
            line_start = i;
        }
        else if (s[i] == '\r')
        {
            if ((i + 1 < s.length() && s[i + 1] != '\n') ||
                i + 1 >= s.length())
            {
                ret.push_back(s.substr(line_start, i - line_start));
                i++;
                line_start = i;
            }
            else  // if (i + 1 < s.length() && s[i + 1] == '\n')
            {
                ret.push_back(s.substr(line_start, i - line_start));
                i += 2;
                line_start = i;
            }
        }
        else
        {
            i++;
        }
    }
    ret.push_back(s.substr(line_start, s.length() - line_start));
    return ret;
}

std::string StrUtils::tolower(const std::string &str)
{
    std::string new_str = str;
    std::transform(new_str.begin(), new_str.end(), new_str.begin(), ::tolower);
    return new_str;
}

std::string StrUtils::toupper(const std::string &str)
{
    std::string new_str = str;
    std::transform(new_str.begin(), new_str.end(), new_str.begin(), ::toupper);
    return new_str;
}

std::vector<std::string> StrUtils::split_with_char(const std::string &s, char delimiter, bool is_merge_delimiter)
{
    std::vector<std::string> v;
    size_t start = 0;
    size_t i = 0;
    while (i < s.length())
    {
        if (delimiter == s[i])
        {
            if (i > start || !is_merge_delimiter)
            {
                v.push_back(s.substr(start, i - start));
            }
            i++;
            start = i;
        }
        else
        {
            i++;
        }
    }
    v.push_back(s.substr(start, s.length() - start));
    return v;
}

std::string StrUtils::ltrim(const std::string &s)
{
    auto iter = std::find_if(s.begin(), s.end(), [](char c) -> bool
                             { return (std::isspace(c) == 0); });
    return std::string(iter, s.end());
}

std::string StrUtils::rtrim(const std::string &s)
{
    auto iter = std::find_if(s.rbegin(), s.rend(), [](char c) -> bool
                             { return (std::isspace(c) == 0); });
    return std::string(s.begin(), iter.base());
}

std::string StrUtils::trim(const std::string &s)
{
    return StrUtils::ltrim(StrUtils::rtrim(s));
}

std::string StrUtils::json2str(const Json::Value &json)
{
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "";
    return Json::writeString(wbuilder, json);
}

Json::Value StrUtils::str2json(const std::string &str)
{
    KLOG_DEBUG("json str: %s.", str.c_str());
    Json::Value result;
    Json::CharReaderBuilder rbuilder;
    std::unique_ptr<Json::CharReader> reader(rbuilder.newCharReader());
    std::string error;

    RETURN_VAL_IF_TRUE(str.empty(), Json::Value());

    if (!reader->parse(str.c_str(), str.c_str() + str.length(), &result, &error))
    {
        KLOG_WARNING("%s", error.c_str());
        return Json::Value();
    }
    return result;
}

bool StrUtils::startswith(const std::string &str, const std::string &prefix)
{
    RETURN_VAL_IF_TRUE(str.size() < prefix.size(), false);
    return (str.substr(0, prefix.size()) == prefix);
}

bool StrUtils::endswith(const std::string &str, const std::string &suffix)
{
    RETURN_VAL_IF_TRUE(str.size() < suffix.size(), false);
    return (str.substr(str.size() - suffix.size(), suffix.size()) == suffix);
}

bool StrUtils::contains_oneof_substrs(const std::string &str, const std::vector<std::string> &substrs)
{
    for (auto &substr : substrs)
    {
        if (str.find(substr) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

bool StrUtils::contains_allof_substrs(const std::string &str, const std::vector<std::string> &substrs)
{
    for (auto &substr : substrs)
    {
        if (str.find(substr) == std::string::npos)
        {
            return false;
        }
    }
    return true;
}

std::string StrUtils::timestamp2str(time_t t)
{
    struct tm *tm_time;

    RETURN_VAL_IF_TRUE(t == 0, std::string());
    tm_time = localtime(&t);
    return StrUtils::tm2str(tm_time);
}

std::string StrUtils::gdate2str(const GDate *date)
{
    g_autofree gchar *str = g_strdup_printf("%04d-%02d-%02d",
                                            g_date_get_year(date),
                                            g_date_get_month(date),
                                            g_date_get_day(date));

    std::string result = std::string(str);
    return result;
}

std::string StrUtils::tm2str(const struct tm *tm_time)
{
    g_autofree gchar *str = g_strdup_printf("%04d-%02d-%02d %02d:%02d:%02d",
                                            tm_time->tm_year + 1900,
                                            tm_time->tm_mon + 1,
                                            tm_time->tm_mday,
                                            tm_time->tm_hour,
                                            tm_time->tm_min,
                                            tm_time->tm_sec);
    std::string result = std::string(str);
    return result;
}

}  // namespace Kiran