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
 * Author:     meizhigang <meizhigang@kylinos.com.cn>
 */

#include "plugins/clipboard/clipboard-plugin.h"
#include <gtk3-log-i.h>
#include "plugins/clipboard/clipboard-manager.h"

PLUGIN_EXPORT_FUNC_DEF(ClipboardPlugin);

namespace Kiran
{
ClipboardPlugin::ClipboardPlugin()
{
}

ClipboardPlugin::~ClipboardPlugin()
{
}

void ClipboardPlugin::activate()
{
    KLOG_PROFILE("active clipboard plugin.");

    ClipboardManager::global_init();
}

void ClipboardPlugin::deactivate()
{
    KLOG_PROFILE("deactive clipboard plugin.");

    ClipboardManager::global_deinit();
}
}  // namespace Kiran
