/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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
