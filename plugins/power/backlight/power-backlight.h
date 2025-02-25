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
#include "power-i.h"

namespace Kiran
{
class PowerBacklightPercentage;

// 背光设备的亮度控制管理
class PowerBacklight : public QObject
{
    Q_OBJECT

public:
    PowerBacklight();
    virtual ~PowerBacklight();

    static PowerBacklight* getInstance() { return m_instance; };

    static void globalInit();

    static void globalDeinit() { delete m_instance; };

    PowerBacklightPercentage* getBacklightDevice(PowerDeviceType device);

Q_SIGNALS:
    // 背光设备亮度发生变化
    void brightnessChanged(PowerBacklightPercentage* backlight, int32_t brightnessPercentage);

private:
    void init();

    void processBacklightBrightnessChanged(int32_t brightnessPercentage, PowerBacklightPercentage* backlight);

private:
    static PowerBacklight* m_instance;

    PowerBacklightPercentage* m_backlightKbd;
    PowerBacklightPercentage* m_backlightMonitor;
};
}  // namespace Kiran