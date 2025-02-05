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

#include "lib/base/base.h"
#include "power-i.h"

namespace Kiran
{
// 背光设备亮度控制基类
class PowerBacklightPercentage : public QObject
{
    Q_OBJECT
public:
    PowerBacklightPercentage(QObject *parent = nullptr) : QObject(parent){};
    virtual ~PowerBacklightPercentage(){};

    virtual void init() = 0;

    virtual PowerDeviceType getType() = 0;

    // 设置亮度百分比
    virtual bool setBrightness(int32_t percentage) = 0;
    // 获取亮度百分比，如果小于0，则说明不支持调节亮度
    virtual int32_t getBrightness() = 0;

    // 增加亮度百分比
    virtual bool brightnessUp() = 0;
    // 降低亮度百分比
    virtual bool brightnessDown() = 0;

Q_SIGNALS:
    // 亮度发生变化
    void brightnessChanged(int32_t percentage);
};

class PowerBacklightAbsolute : public QObject
{
    Q_OBJECT

public:
    PowerBacklightAbsolute(){};
    virtual ~PowerBacklightAbsolute(){};

    // 设置亮度值
    virtual bool setBrightnessValue(int32_t brightnessValue) = 0;
    // 获取亮度值
    virtual int32_t getBrightnessValue() = 0;
    // 获取亮度最大最小值
    virtual bool getBrightnessRange(int32_t &min, int32_t &max) = 0;
};

using PowerBacklightAbsoluteList = QVector<QSharedPointer<PowerBacklightAbsolute>>;

class PowerBacklightMonitors : public QObject
{
    Q_OBJECT

public:
    PowerBacklightMonitors(){};
    virtual ~PowerBacklightMonitors(){};

    virtual void init() = 0;

    // 获取所有显示器亮度设置对象
    virtual PowerBacklightAbsoluteList getMonitors() = 0;

Q_SIGNALS:
    void monitorChanged();
    void brightnessChanged();
};

}  // namespace Kiran