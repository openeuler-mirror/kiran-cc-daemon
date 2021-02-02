/**
 * @file          /kiran-cc-daemon/plugins/power/power-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "plugin_i.h"
namespace Kiran
{
class PowerPlugin : public Plugin
{
public:
    PowerPlugin();
    virtual ~PowerPlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
