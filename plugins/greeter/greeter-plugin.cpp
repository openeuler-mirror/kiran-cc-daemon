/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     yangxiaoqing <yangxiaoqing@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#include "plugins/greeter/greeter-plugin.h"
#include <cstdio>

#include <gtk3-log-i.h>
#include "plugins/greeter/greeter-dbus.h"

PLUGIN_EXPORT_FUNC_DEF(GreeterPlugin);

namespace Kiran
{
GreeterPlugin::GreeterPlugin()
{
}

GreeterPlugin::~GreeterPlugin()
{
}

void GreeterPlugin::activate()
{
    KLOG_PROFILE("active greeter settings plugin.");

    // GreeterSettingsWrapper::global_init();
    GreeterDBus::global_init();
}

void GreeterPlugin::deactivate()
{
    KLOG_PROFILE("deactive greeter settings plugin.");

    GreeterDBus::global_deinit();
    //GreeterSettingsWrapper::global_deinit();
}

}  // namespace Kiran
