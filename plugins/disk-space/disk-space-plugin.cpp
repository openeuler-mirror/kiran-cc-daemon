/**
 * Copyright (c) 2020 ~ 2026 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:    gaobo <gaobo@kylinsec.com.cn>
 */

#include "disk-space-plugin.h"
#include <QTranslator>
#include "config.h"
#include "disk-space-manager.h"
#include "lib/base/misc-utils.h"

namespace Kiran
{
void DiskSpaceMonitorPlugin::activate()
{
    m_translator = MiscUtils::installTranslator(QString("%1-%2").arg(PROJECT_NAME).arg("disk-space"));
    DiskSpaceManager::globalInit();
}

void DiskSpaceMonitorPlugin::deactivate()
{
    DiskSpaceManager::globalDeinit();
    MiscUtils::removeTranslator(m_translator);
    m_translator = nullptr;
}
}  // namespace Kiran
