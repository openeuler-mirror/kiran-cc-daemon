/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:33:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-06 14:35:47
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/mouse/mouse-plugin.h
 */
#pragma once

#include "plugin_i.h"
namespace Kiran
{
class MousePlugin : public Plugin
{
public:
    MousePlugin();
    virtual ~MousePlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
