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
#include <QStringList>

namespace Kiran
{
class PowerUPower;

class PowerBacklightHelper : public QObject
{
    Q_OBJECT

public:
    PowerBacklightHelper();
    virtual ~PowerBacklightHelper();

    void init();

    // 是否支持亮度设置
    bool supportBacklight();
    QString getBacklightDir() { return m_backlightDir; };

    // 获取亮度值
    int32_t getBrightnessValue();
    // 获取亮度最大值
    int32_t getBrightnessMaxValue();
    // 设置亮度值
    bool setBrightnessValue(int32_t brightness_value, QString &error);

Q_SIGNALS:
    // 亮度变化的信号
    void brightnessChanged(int brightnessValue);

private:
    QString getBacklightFilepath();

private:
    // 优先搜索的背光配置目录
    const static QStringList m_backlightSearchSubdirs;
    // 背光配置目录
    QString m_backlightDir;
    // 当前亮度值
    int32_t m_brightnessValue;
    QSharedPointer<PowerUPower> m_upowerClient;
};
}  // namespace Kiran
