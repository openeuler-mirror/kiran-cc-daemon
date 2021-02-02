/**
 * @file          /kiran-cc-daemon/plugins/power/power-utils.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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
};
}  // namespace Kiran