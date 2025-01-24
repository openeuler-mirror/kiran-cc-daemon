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

#include "accounts-plugin.h"
#include <QCoreApplication>
#include <QTranslator>
#include "accounts-manager.h"
#include "accounts-wrapper.h"
#include "config.h"
#include "lib/base/misc-utils.h"

namespace Kiran
{
void AccountsPlugin::activate()
{
    m_translator = MiscUtils::installTranslator(QString("%1-%2").arg(PROJECT_NAME).arg("accounts"));

    AccountsWrapper::globalInit();
    AccountsManager::globalInit(AccountsWrapper::getInstance());
}

void AccountsPlugin::deactivate()
{
    AccountsManager::globalDeinit();
    AccountsWrapper::globalDeinit();

    MiscUtils::removeTranslator(m_translator);
}

}  // namespace Kiran