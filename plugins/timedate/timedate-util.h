/*
 * @Author       : tangjie02
 * @Date         : 2020-08-31 14:42:34
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-29 10:34:31
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

    // 获取RTC硬件时钟的时区是否为LOCAL
    static bool is_local_rtc();
    // 获取系统时区
    static std::string get_timezone();
    // 获取zone时区跟GTM时区之间的时间差
    static int64_t get_gmt_offset(const std::string &zone);
};
}  // namespace Kiran
