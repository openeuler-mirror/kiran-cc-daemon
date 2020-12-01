/*
 * @Author       : tangjie02
 * @Date         : 2020-12-01 10:13:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-12-01 10:14:38
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/appearance/appearance-plugin.h
 */
#pragma once

#include "plugin_i.h"

namespace Kiran
{
class AppearancePlugin : public Plugin
{
public:
    AppearancePlugin();
    virtual ~AppearancePlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
