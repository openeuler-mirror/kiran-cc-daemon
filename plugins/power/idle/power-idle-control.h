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
#include "power-i.h"

class QGSettings;

namespace Kiran
{
class PowerWrapperManager;
class PowerBacklight;
class PowerIdleTimer;
class PowerUPower;

class PowerIdleControl : public QObject
{
    Q_OBJECT

public:
    PowerIdleControl() = delete;
    PowerIdleControl(PowerWrapperManager* wrapperManager, PowerBacklight* backlight);
    virtual ~PowerIdleControl();

    static PowerIdleControl* getInstance() { return m_instance; };

    static void globalInit(PowerWrapperManager* wrapperManager, PowerBacklight* backlight);

    static void globalDeinit() { delete m_instance; };

private:
    void init();

    void updateIdleTimer();

    void switchToNormal();
    void switchToDim();
    void switchToBlank();
    void switchToSleep();

    void processBatteryChanged(bool);
    void processSettingsChanged(const QString& key);
    void processIdleModeChanged(int mode);
    void processKbdBrightnessChanged(int32_t brightness_percentage);
    void processMonitorBrightnessChanged(int32_t brightness_percentage);

private:
    static PowerIdleControl* m_instance;

    PowerWrapperManager* m_wrapperManager;
    PowerBacklight* m_backlight;

    PowerIdleTimer* m_idleTimer;
    QGSettings* m_powerSettings;

    QSharedPointer<PowerUPower> m_upowerClient;

    int32_t m_computerIdleTime;
    PowerAction m_computerIdleAction;

    int32_t m_displayIdleTime;
    PowerAction m_displayIdleAction;

    // 设置显示器变暗成功
    bool m_displayDimmedSet;
};
}  // namespace Kiran