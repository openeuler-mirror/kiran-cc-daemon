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

#include <power-i.h>
#include <QObject>
#include <QSharedPointer>

class QGSettings;

namespace Kiran
{
class PowerBacklightPercentage;
class PowerWrapperManager;
class PowerBacklight;
class PowerUPower;
class PowerUPowerDevice;
class PowerScreenSaver;
class PowerEventButton;

// 处理按键事件和电源电量变化事件
class PowerEventControl : public QObject
{
    Q_OBJECT

public:
    PowerEventControl() = delete;
    PowerEventControl(PowerWrapperManager* wrapperManager, PowerBacklight* backlight);
    virtual ~PowerEventControl();

    static PowerEventControl* getInstance() { return m_instance; };
    static void globalInit(PowerWrapperManager* wrapperManager, PowerBacklight* backlight);
    static void globalDeinit() { delete m_instance; };

private:
    void init();
    // 执行正在充电事件
    void chargingEvent();
    // 执行放电事件
    void dischargingEvent(QSharedPointer<PowerUPowerDevice> device);
    // 执行电量过低事件
    void chargeLowEvent(QSharedPointer<PowerUPowerDevice> device);
    // 执行电量不足事件
    void chargeActionEvent(QSharedPointer<PowerUPowerDevice> device);
    void doChargeCriticalAction(PowerAction action);

    void processButtonChanged(PowerEvent evnet);
    void processKbdBrightnessChanged(int32_t brightness_value);
    void processDeviceStatusChanged(QSharedPointer<PowerUPowerDevice> device, int event);

private:
    static PowerEventControl* m_instance;

    PowerEventButton* m_eventButton;

    PowerWrapperManager* m_wrapperManager;
    QSharedPointer<PowerUPower> m_upowerClient;
    QSharedPointer<PowerScreenSaver> m_screensaver;

    PowerBacklight* m_backlight;
    PowerBacklightPercentage* m_backlightKbd;
    PowerBacklightPercentage* m_backlightMonitor;
    // 键盘上一次设置的非0值
    int32_t m_kbdLastNozeroBrightness;
    uint32_t m_lidClosedThrottle;
    QGSettings* m_powerSettings;
    // 设置显示器变暗成功
    bool m_displayDimmedSet;
};
}  // namespace Kiran