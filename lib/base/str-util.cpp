/*
 * @Author       : tangjie02
 * @Date         : 2020-08-17 15:13:22
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 14:30:41
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/lib/base/str-util.cpp
 */

#include "lib/base/str-util.h"

#include <algorithm>

namespace Kiran
{
std::vector<std::string> StrUtil::split_lines(const std::string& s)
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

std::string StrUtil::tolower(const std::string& str)
{
    std::string new_str = str;
    std::transform(new_str.begin(), new_str.end(), new_str.begin(), ::tolower);
    return new_str;
}

std::string StrUtil::toupper(const std::string& str)
{
    std::string new_str = str;
    std::transform(new_str.begin(), new_str.end(), new_str.begin(), ::toupper);
    return new_str;
}

std::vector<std::string> StrUtil::split_with_char(const std::string& s, char delimiter, bool is_merge_delimiter)
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
}  // namespace Kiran