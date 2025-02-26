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

#include "display-plugin.h"
#include <QTranslator>
#include "config.h"
#include "display-manager.h"
#include "lib/base/misc-utils.h"

namespace Kiran
{
DisplayPlugin::DisplayPlugin()
{
    m_translator = nullptr;
}

void DisplayPlugin::activate()
{
    m_translator = MiscUtils::installTranslator(QString("%1-%2").arg(PROJECT_NAME).arg("display"));

    DisplayManager::globalInit();
}

void DisplayPlugin::deactivate()
{
    DisplayManager::globalDeinit();

    MiscUtils::removeTranslator(m_translator);
}
}  // namespace Kiran
