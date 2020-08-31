/*
 * @Author       : tangjie02
 * @Date         : 2020-08-31 14:42:34
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-31 14:52:51
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/timedate/timedate-util.h
 */

#pragma once

#include <string>

namespace Kiran
{
class TimedateUtil
{
public:
    TimedateUtil(){};
    virtual ~TimedateUtil(){};

    static int64_t get_gmt_offset(const std::string &zone);
};
}  // namespace Kiran
