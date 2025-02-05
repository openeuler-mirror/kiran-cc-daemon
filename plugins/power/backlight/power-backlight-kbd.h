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

#include "power-backlight-interface.h"

class QDBusMessage;

namespace Kiran
{
// 键盘亮度控制
class PowerBacklightKbd : public PowerBacklightPercentage
{
    Q_OBJECT

public:
    PowerBacklightKbd(QObject* parent = nullptr);
    virtual ~PowerBacklightKbd();

    virtual void init() override;

    virtual PowerDeviceType getType() override { return PowerDeviceType::POWER_DEVICE_TYPE_KBD; };

    // 设置亮度百分比
    virtual bool setBrightness(int32_t percentage) override;
    // 获取亮度百分比，如果小于0，则说明不支持调节亮度
    virtual int32_t getBrightness() override { return m_brightnessPercentage; };

    // 增加亮度
    virtual bool brightnessUp() override;
    // 降低亮度
    virtual bool brightnessDown() override;

private:
    // 获取亮度值
    int32_t getBrightnessValue();
    // 设置亮度值
    bool setBrightnessValue(int32_t value);
    // 获取最大亮度值
    int32_t getMaxBrightnessValue();

    int32_t brightnessPercent2Discrete(int32_t percentage, int32_t levels);
    int32_t brightnessDiscrete2Percent(int32_t discrete, int32_t levels);

private Q_SLOTS:
    void processBrightnessChanged(const QDBusMessage& message);

private:
    int32_t m_brightnessValue;
    int32_t m_brightnessPercentage;
    int32_t m_maxBrightnessValue;
};
}  // namespace Kiran