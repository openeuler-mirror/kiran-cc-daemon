/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:34:09
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 15:29:39
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/xsettings/xsettings-plugin.cpp
 */

#include "plugins/xsettings/xsettings-plugin.h"

#include <cstdio>

#include "lib/base/log.h"
#include "plugins/xsettings/xsettings-manager.h"

PLUGIN_EXPORT_FUNC_DEF(XSettingsPlugin);

namespace Kiran
{
XSettingsPlugin::XSettingsPlugin()
{
}

XSettingsPlugin::~XSettingsPlugin()
{
}

void XSettingsPlugin::activate()
{
    SETTINGS_PROFILE("active xsettings plugin.");
    XSettingsManager::global_init();
}

void XSettingsPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive xsettings plugin.");
    XSettingsManager::global_deinit();
}
}  // namespace Kiran