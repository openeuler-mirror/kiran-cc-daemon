/**
 * @file          /kiran-cc-daemon/plugins/greeter/greeter-plugin.h
 * @brief description
 * @author yangxiaoqing <yangxiaoqing@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
*/

#pragma once

#include "plugin_i.h"
namespace Kiran
{
class GreeterPlugin : public Plugin
{
public:
    GreeterPlugin();
    virtual ~GreeterPlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
