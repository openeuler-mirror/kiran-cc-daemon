/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#include "plugins/groups/groups-plugin.h"

#include "lib/base/base.h"
#include "plugins/groups/groups-manager.h"
#include "plugins/groups/groups-wrapper.h"

PLUGIN_EXPORT_FUNC_DEF(GroupsPlugin);

namespace Kiran
{
GroupsPlugin::GroupsPlugin()
{
}

GroupsPlugin::~GroupsPlugin()
{
}

void GroupsPlugin::activate()
{
    KLOG_DEBUG_GROUPS("Active groups plugin.");

    GroupsWrapper::global_init();
    GroupsManager::global_init(GroupsWrapper::get_instance());
}

void GroupsPlugin::deactivate()
{
    KLOG_DEBUG_GROUPS("Deactive groups plugin.");

    GroupsManager::global_deinit();
    GroupsWrapper::global_deinit();
}

}  // namespace Kiran
