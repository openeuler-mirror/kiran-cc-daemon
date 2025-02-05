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

#pragma once

#include <QObject>
#include <QSharedPointer>

namespace Kiran
{

class PowerLogin1;
class PowerScreenSaver;
class PowerSession;
class PowerUPower;
class PowerProfiles;

class PowerWrapperManager : public QObject
{
    Q_OBJECT

public:
    PowerWrapperManager();
    virtual ~PowerWrapperManager(){};

    static PowerWrapperManager* getInstance() { return m_instance; };

    static void globalInit();

    static void globalDeinit() { delete m_instance; };

    QSharedPointer<PowerLogin1> getDefaultLogin1() { return m_login1; };
    QSharedPointer<PowerScreenSaver> getDefaultScreensaver() { return m_screensaver; };
    QSharedPointer<PowerSession> getDefaultSession() { return m_session; };
    QSharedPointer<PowerUPower> getDefaultUpower() { return m_upower; };
    QSharedPointer<PowerProfiles> getDefaultProfiles() { return m_profiles; };

private:
    void init();

private:
    static PowerWrapperManager* m_instance;

    QSharedPointer<PowerLogin1> m_login1;
    QSharedPointer<PowerScreenSaver> m_screensaver;
    QSharedPointer<PowerSession> m_session;
    QSharedPointer<PowerUPower> m_upower;
    QSharedPointer<PowerProfiles> m_profiles;
};
}  // namespace Kiran