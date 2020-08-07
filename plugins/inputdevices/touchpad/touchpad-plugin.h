/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:33:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-06 15:38:00
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/touchpad/touchpad-plugin.h
 */
#pragma once

#include "plugin_i.h"
namespace Kiran
{
class TouchPadPlugin : public Plugin
{
public:
    TouchPadPlugin();
    virtual ~TouchPadPlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
