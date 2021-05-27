/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:33:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-20 15:39:08
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/xsettings/xsettings-plugin.h
 */
#pragma once

#include "plugin-i.h"
namespace Kiran
{
class XSettingsPlugin : public Plugin
{
public:
    XSettingsPlugin();
    virtual ~XSettingsPlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
