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

namespace KScreen
{
class Dpms;
}

namespace Kiran
{
enum PowerDpmsLevel
{
    POWER_DPMS_LEVEL_ON,
    POWER_DPMS_LEVEL_STANDBY,
    POWER_DPMS_LEVEL_SUSPEND,
    POWER_DPMS_LEVEL_OFF,
    POWER_DPMS_LEVEL_UNKNOWN
};

class PowerSaveDpms : public QObject
{
    Q_OBJECT

public:
    PowerSaveDpms(QObject *parent = nullptr) : QObject(parent){};
    virtual void setLevel(PowerDpmsLevel level){};
};

#ifdef WITH_DPMS_KDE

class PowerSaveDpmsKDE : public PowerSaveDpms
{
    Q_OBJECT

public:
    PowerSaveDpmsKDE(QObject *parent = nullptr);
    virtual void setLevel(PowerDpmsLevel level) override;

private:
    KScreen::Dpms *m_dpms;
};

#endif

class PowerSaveDpmsX11 : public PowerSaveDpms
{
    Q_OBJECT

public:
    PowerSaveDpmsX11(QObject *parent = nullptr);
    // 设置节能模式
    virtual void setLevel(PowerDpmsLevel level) override;

private:
    uint16_t levelK2Xcb(PowerDpmsLevel level);
    PowerDpmsLevel levelXcb2K(uint16_t level);

private:
    // dpms对该客户端是否可用
    bool m_capable;
};

class PowerSaveDpmsDummy : public PowerSaveDpms
{
    Q_OBJECT

public:
    PowerSaveDpmsDummy(QObject *parent = nullptr) : PowerSaveDpms(parent){};
    virtual void setLevel(PowerDpmsLevel) override{};
};
}  // namespace Kiran