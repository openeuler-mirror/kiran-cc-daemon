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

#include "power-plugin.h"
#include <QTranslator>
#include "backlight/power-backlight.h"
#include "config.h"
#include "event/power-event-control.h"
#include "idle/power-idle-control.h"
#include "lib/base/misc-utils.h"
#include "notification/power-notification-manager.h"
#include "power-manager.h"
#include "save/power-save.h"
#include "wrapper/power-wrapper-manager.h"

namespace Kiran
{

void PowerPlugin::activate()
{
    m_translator = MiscUtils::installTranslator(QString("%1-%2").arg(PROJECT_NAME).arg("power"));

    PowerWrapperManager::globalInit();
    PowerBacklight::globalInit();
    PowerManager::globalInit(PowerWrapperManager::getInstance(), PowerBacklight::getInstance());
    PowerSave::globalInit(PowerWrapperManager::getInstance(), PowerBacklight::getInstance());
    PowerIdleControl::globalInit(PowerWrapperManager::getInstance(), PowerBacklight::getInstance());
    PowerEventControl::globalInit(PowerWrapperManager::getInstance(), PowerBacklight::getInstance());
    PowerNotificationManager::globalInit(PowerWrapperManager::getInstance());
}

void PowerPlugin::deactivate()
{
    PowerNotificationManager::globalDeinit();
    PowerEventControl::globalDeinit();
    PowerIdleControl::globalDeinit();
    PowerSave::globalDeinit();
    PowerManager::globalDeinit();
    PowerBacklight::globalDeinit();
    PowerWrapperManager::globalDeinit();

    MiscUtils::removeTranslator(m_translator);
}
}  // namespace Kiran