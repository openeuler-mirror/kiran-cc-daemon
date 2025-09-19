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

class KActionCollection;
class QKeySequence;
class QElapsedTimer;
class QTimer;

namespace Kiran
{
class PowerUPower;

class PowerEventButton : public QObject
{
    Q_OBJECT

public:
    PowerEventButton(QObject *parent = nullptr);
    virtual ~PowerEventButton();

    void init();

Q_SIGNALS:
    // 按键被按下的信号
    void buttonChanged(PowerEvent type);

private:
    bool registerButton(const QKeySequence &key,
                        const QString &name,
                        const QString &displayName,
                        PowerEvent type);

    // 发送按键信号，如果跟上一次发送的按键信号相同且时间间隔较短，则忽略该次按键信号的发送
    void emitButtonSignal(PowerEvent type, const QString &name);
    // 电源按钮单独处理
    void emitPoweroffSignal();
    void processLidChanged(bool lidIsClosed);

private:
    // 抑制systemd-login1对电源、休眠、挂起按键和合上盖子进行操作。
    int32_t m_login1InhibitFD;
    KActionCollection *m_actionCollection;
    QSharedPointer<PowerUPower> m_upowerClient;
    QElapsedTimer *m_buttonTimer;
    QTimer *m_powerOffTimer;
};
}  // namespace Kiran