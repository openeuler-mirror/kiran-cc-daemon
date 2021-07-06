/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
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


#include "plugins/appearance/appearance-plugin.h"

#include <gtk3-log-i.h>
#include "plugins/appearance/appearance-manager.h"

PLUGIN_EXPORT_FUNC_DEF(AppearancePlugin);

namespace Kiran
{
AppearancePlugin::AppearancePlugin()
{
}

AppearancePlugin::~AppearancePlugin()
{
}

void AppearancePlugin::activate()
{
    KLOG_PROFILE("active appearance plugin.");
    AppearanceManager::global_init();
}

void AppearancePlugin::deactivate()
{
    KLOG_PROFILE("deactive appearance plugin.");
    AppearanceManager::global_deinit();
}
}  // namespace Kiran