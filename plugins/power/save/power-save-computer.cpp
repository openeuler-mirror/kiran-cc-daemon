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

#include "power-save-computer.h"
#include "../wrapper/power-session.h"
#include "../wrapper/power-wrapper-manager.h"

namespace Kiran
{
PowerSaveComputer::PowerSaveComputer(QObject *parent) : QObject(parent)
{
    m_session = PowerWrapperManager::getInstance()->getDefaultSession();
}

void PowerSaveComputer::init()
{
}

void PowerSaveComputer::suspend()
{
    m_session->suspend();
}

void PowerSaveComputer::hibernate()
{
    m_session->hibernate();
}

void PowerSaveComputer::shutdown()
{
    m_session->shutdown();
}

}  // namespace Kiran
