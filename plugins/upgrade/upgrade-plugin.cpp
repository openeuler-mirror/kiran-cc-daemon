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

#include "upgrade-plugin.h"
#include <QCoreApplication>
#include <QTranslator>
#include "config.h"
#include "lib/base/base.h"
#include "lib/base/misc-utils.h"
#include "manager.h"

namespace Kiran
{
void UpgradePlugin::activate()
{
    m_translator = MiscUtils::installTranslator(QString("%1-%2").arg(PROJECT_NAME).arg("upgrade"));

    Manager::globalInit();
}

void UpgradePlugin::deactivate()
{
    Manager::globalDeinit();

    MiscUtils::removeTranslator(m_translator);
}

}  // namespace Kiran