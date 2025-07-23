/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
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

#include "groups-plugin.h"
#include <kiran-log/qt5-log-i.h>
#include <QCoreApplication>
#include <QTranslator>
#include "config.h"
#include "groups-manager.h"
#include "groups-wrapper.h"
#include "lib/base/misc-utils.h"
namespace Kiran
{
void GroupsPlugin::activate()
{
    m_translator = MiscUtils::installTranslator(QString("%1-%2").arg(PROJECT_NAME).arg("groups"));

    GroupsWrapper::globalInit();
    GroupsManager::globalInit(GroupsWrapper::getInstance());
}

void GroupsPlugin::deactivate()
{
    GroupsManager::globalDeinit();
    GroupsWrapper::globalDeinit();

    MiscUtils::removeTranslator(m_translator);
}

}  // namespace Kiran