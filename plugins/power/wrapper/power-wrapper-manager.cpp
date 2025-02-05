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

#include "power-wrapper-manager.h"
#include "power-login1.h"
#include "power-profiles.h"
#include "power-screensaver.h"
#include "power-session.h"
#include "power-upower.h"

namespace Kiran
{
PowerWrapperManager::PowerWrapperManager()
{
    m_login1 = QSharedPointer<PowerLogin1>::create();
    m_screensaver = QSharedPointer<PowerScreenSaver>::create();
    m_session = QSharedPointer<PowerSession>::create();
    m_upower = QSharedPointer<PowerUPower>::create();
    m_profiles = PowerProfiles::create();
}

PowerWrapperManager* PowerWrapperManager::m_instance = nullptr;
void PowerWrapperManager::globalInit()
{
    m_instance = new PowerWrapperManager();
    m_instance->init();
}

void PowerWrapperManager::init()
{
    m_login1->init();
    m_screensaver->init();
    m_session->init();
    m_upower->init();
    m_profiles->init();
}

}  // namespace Kiran