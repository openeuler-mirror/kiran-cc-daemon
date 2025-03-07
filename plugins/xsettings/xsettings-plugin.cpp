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

#include "xsettings-plugin.h"
#include <QGuiApplication>
#include "lib/base/base.h"
#include "xsettings-manager.h"

namespace Kiran
{
void XSettingsPlugin::activate()
{
    if (qGuiApp->platformName() == "xcb")
    {
        XSettingsManager::globalInit();
    }
    else
    {
        KLOG_INFO(xsettings) << "xsettings cannot init on xcb platform";
    }
}

void XSettingsPlugin::deactivate()
{
    if (qGuiApp->platformName() == "xcb")
    {
        XSettingsManager::globalDeinit();
    }
}
}  // namespace Kiran
