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

#include "lib/base/base.h"

namespace Kiran
{
class PowerUtils
{
public:
    PowerUtils(){};
    virtual ~PowerUtils(){};

    static std::string get_time_translation(uint32_t seconds);
    static std::string action_enum2str(uint32_t action);
    static std::string event_enum2str(uint32_t event);
    static std::string device_enum2str(uint32_t device);
    static std::string supply_enum2str(uint32_t supply);
};
}  // namespace Kiran