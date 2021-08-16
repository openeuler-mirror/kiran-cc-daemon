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
