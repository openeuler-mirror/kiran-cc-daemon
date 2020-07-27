/*
 * @Author       : tangjie02
 * @Date         : 2020-06-05 15:22:03
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-27 11:54:56
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/lib/helper.cpp
 */
#include "lib/helper.h"

#include <algorithm>
#include <string>
#include <vector>

namespace Kiran
{
std::vector<std::string> split_lines(const std::string& s)
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

std::string str_tolower(const std::string& str)
{
    std::string new_str = str;
    std::transform(new_str.begin(), new_str.end(), new_str.begin(), tolower);
    return new_str;
}

}  // namespace Kiran