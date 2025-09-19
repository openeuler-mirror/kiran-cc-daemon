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

#include "power-event-button.h"
#include <unistd.h>
#include <KActionCollection>
#include <KGlobalAccel>
#include <QElapsedTimer>
#include <QTimer>
#include "../power-utils.h"
#include "../wrapper/power-login1.h"
#include "../wrapper/power-upower.h"
#include "../wrapper/power-wrapper-manager.h"
#include "lib/base/base.h"

namespace Kiran
{
#define POWER_BUTTON_DUPLICATE_TIMEOUT 125
#define POWER_BUTTON_POWEROFF_TIMEOUT_MILLISECONDS 125

PowerEventButton::PowerEventButton(QObject *parent) : QObject(parent),
                                                      m_login1InhibitFD(-1)
{
    m_upowerClient = PowerWrapperManager::getInstance()->getDefaultUpower();
    m_actionCollection = new KActionCollection(this);
    m_actionCollection->setComponentName("PowerManagement");
    m_actionCollection->setComponentDisplayName(tr("Power Management"));
    m_buttonTimer = new QElapsedTimer();
    m_powerOffTimer = new QTimer(this);
}

PowerEventButton::~PowerEventButton()
{
    if (m_login1InhibitFD > 0)
    {
        close(m_login1InhibitFD);
    }

    if (m_buttonTimer)
    {
        delete m_buttonTimer;
    }
}

void PowerEventButton::init()
{
    // 这里需要对systemd-login1添加抑制器，避免systemd-login1对电源、休眠、挂起按键和合上盖子进行操作。
    auto login1 = PowerWrapperManager::getInstance()->getDefaultLogin1();
    m_login1InhibitFD = login1->inhibit("handle-power-key:handle-suspend-key:handle-lid-switch");

    registerButton(Qt::Key_PowerOff,
                   QLatin1String("PowerOff"),
                   tr("Power Off"),
                   PowerEvent::POWER_EVENT_PRESSED_POWEROFF);

    registerButton(Qt::Key_Suspend,
                   QLatin1String("Suspend"),
                   tr("Suspend"),
                   PowerEvent::POWER_EVENT_PRESSED_SUSPEND);

    registerButton(Qt::Key_Sleep,
                   QLatin1String("Sleep"),
                   tr("Sleep"),
                   PowerEvent::POWER_EVENT_PRESSED_SLEEP);

    registerButton(Qt::Key_Hibernate,
                   QLatin1String("Hibernate"),
                   tr("Hibernate"),
                   PowerEvent::POWER_EVENT_PRESSED_HIBERNATE);

    registerButton(Qt::Key_MonBrightnessUp,
                   QLatin1String("Increase Screen Brightness"),
                   tr("Increase Screen Brightness"),
                   PowerEvent::POWER_EVENT_PRESSED_BRIGHT_UP);

    registerButton(Qt::Key_MonBrightnessDown,
                   QLatin1String("Decrease Screen Brightness"),
                   tr("Decrease Screen Brightness"),
                   PowerEvent::POWER_EVENT_PRESSED_BRIGHT_DOWN);

    registerButton(Qt::Key_KeyboardBrightnessUp,
                   QLatin1String("Increase Keyboard Brightness"),
                   tr("Increase Keyboard Brightness"),
                   PowerEvent::POWER_EVENT_PRESSED_KBD_BRIGHT_UP);

    registerButton(Qt::Key_KeyboardBrightnessDown,
                   QLatin1String("Decrease Keyboard Brightness"),
                   tr("Decrease Keyboard Brightness"),
                   PowerEvent::POWER_EVENT_PRESSED_KBD_BRIGHT_DOWN);

    registerButton(Qt::Key_KeyboardLightOnOff,
                   QLatin1String("Toggle Keyboard Backlight"),
                   tr("Toggle Keyboard Backlight"),
                   PowerEvent::POWER_EVENT_PRESSED_KBD_BRIGHT_TOGGLE);

    // 因为keybinding中已经将super+L作为锁屏的快捷键，且Key_ScreenSaver按键不常用，所以这里先不注册锁屏的快捷键
    // registerButton(Qt::Key_ScreenSaver,
    //                QLatin1String("Lock Screen"),
    //                tr("Lock Screen"),
    //                PowerEvent::POWER_EVENT_PRESSED_LOCK);

    m_buttonTimer->start();

    connect(m_upowerClient.get(), &PowerUPower::lidIsClosedChanged, this, &PowerEventButton::processLidChanged);
    connect(m_powerOffTimer, &QTimer::timeout, this, &PowerEventButton::emitPoweroffSignal);
}

bool PowerEventButton::registerButton(const QKeySequence &key,
                                      const QString &name,
                                      const QString &displayName,
                                      PowerEvent type)
{
    QAction *globalAction = m_actionCollection->addAction(name);
    globalAction->setText(displayName);
    connect(globalAction, &QAction::triggered, this, [this, type, name]
            { this->emitButtonSignal(type, name); });

    return KGlobalAccel::self()->setGlobalShortcut(globalAction, QList<QKeySequence>() << key);
}

void PowerEventButton::emitPoweroffSignal()
{
    KLOG_INFO(power) << "PowerOff button is triggered.";
    Q_EMIT buttonChanged(POWER_EVENT_PRESSED_POWEROFF);
    m_powerOffTimer->stop();
}

void PowerEventButton::emitButtonSignal(PowerEvent type, const QString &name)
{
    // 仅电源按键事件延迟处理，避免单次点击电源按钮短时间触发多次按键事件，导致息屏又立即唤醒
    if (type == POWER_EVENT_PRESSED_POWEROFF)
    {
        KLOG_INFO(power) << "PowerOff button is delay triggered.";
        m_powerOffTimer->start(POWER_BUTTON_POWEROFF_TIMEOUT_MILLISECONDS);
        return;
    }

    if (m_buttonTimer->elapsed() < POWER_BUTTON_DUPLICATE_TIMEOUT)
    {
        KLOG_INFO(power) << "Ignoring duplicate button" << type;
        return;
    }

    KLOG_INFO(power) << name << "button is triggered.";
    Q_EMIT buttonChanged(type);
    m_buttonTimer->restart();
}

void PowerEventButton::processLidChanged(bool lidIsClosed)
{
    KLOG_INFO(power) << "Receive lid" << (lidIsClosed ? "closed" : "opened") << "event.";

    if (lidIsClosed)
    {
        Q_EMIT buttonChanged(PowerEvent::POWER_EVENT_LID_CLOSED);
    }
    else
    {
        Q_EMIT buttonChanged(PowerEvent::POWER_EVENT_LID_OPEN);
    }
}

}  // namespace Kiran