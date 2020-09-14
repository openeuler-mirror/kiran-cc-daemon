/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:33:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-04 16:37:35
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/display/display-plugin.h
 */
#pragma once

#include "plugin_i.h"
namespace Kiran
{
class DisplayPlugin : public Plugin
{
public:
    DisplayPlugin();
    virtual ~DisplayPlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
