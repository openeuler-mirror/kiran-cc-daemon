/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
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

#include "keys-touchpad.h"
#include <QDBusMessage>
#include <QDBusServiceWatcher>
#include <QKeySequence>
#include "keybinding-i.h"
#include "lib/base/base.h"
#include "touchpad-i.h"
#include "touchpad_dbus_proxy.h"

namespace Kiran
{

#define ACTION_NAME_TOUCHPAD_TOGGLE "touchpad-toggle"
#define ACTION_NAME_TOUCHPAD_ON "touchpad-on"
#define ACTION_NAME_TOUCHPAD_OFF "touchpad-off"

KeysTouchpad::KeysTouchpad() : KeysComponent("Touchpad", tr("Touchpad")),
                               m_touchpadProxy(nullptr),
                               m_touchpadServiceWatcher(nullptr),
                               m_hasTouchpadDevice(false)
{
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(TOUCHPAD_DBUS_NAME))
    {
        initTouchpadProxy();
    }
    else
    {
        m_touchpadServiceWatcher = new QDBusServiceWatcher(TOUCHPAD_DBUS_NAME,
                                                           QDBusConnection::sessionBus(),
                                                           QDBusServiceWatcher::WatchForRegistration, this);

        connect(m_touchpadServiceWatcher, &QDBusServiceWatcher::serviceRegistered, this, [this]()
                { this->initTouchpadProxy(); });
    }
}

void KeysTouchpad::init()
{
    registerShortCut(Qt::Key_TouchpadToggle, ACTION_NAME_TOUCHPAD_TOGGLE, tr("Toggle touchpad"));
    registerShortCut(Qt::Key_TouchpadOn, ACTION_NAME_TOUCHPAD_ON, tr("Enable touchpad"));
    registerShortCut(Qt::Key_TouchpadOff, ACTION_NAME_TOUCHPAD_OFF, tr("Disable touchpad"));
}

void KeysTouchpad::initTouchpadProxy()
{
    if (m_touchpadProxy)
    {
        delete m_touchpadProxy;
        m_touchpadProxy = nullptr;
    }

    m_touchpadProxy = new TouchpadProxy(TOUCHPAD_DBUS_NAME, TOUCHPAD_OBJECT_PATH, QDBusConnection::sessionBus(), this);
    m_hasTouchpadDevice = m_touchpadProxy->has_touchpad();
}

void KeysTouchpad::enableTouchpad(bool enable)
{
    if (!m_hasTouchpadDevice)
    {
        // TODO：如果没有触摸板，则显示禁用图标
        return;
    }

    auto sendMessage = QDBusMessage::createMethodCall(TOUCHPAD_DBUS_NAME,
                                                      TOUCHPAD_OBJECT_PATH,
                                                      "org.freedesktop.DBus.Properties",
                                                      "Set");

    // TODO:测试第三个参数是否生效
    sendMessage << QString(TOUCHPAD_DBUS_INTERFACE_NAME)
                << QString("touchpad_enabled")
                << QVariant::fromValue(QDBusVariant(enable));
    QDBusConnection::sessionBus().asyncCall(sendMessage);

    // TODO:显示禁用或启用图标
}

void KeysTouchpad::triggerShortCut(const QString &name)
{
    switch (shash(name.toLatin1().data()))
    {
    case CONNECT(ACTION_NAME_TOUCHPAD_TOGGLE, _hash):
    {
        auto currentEnabled = m_touchpadProxy->touchpad_enabled();
        enableTouchpad(!currentEnabled);
        break;
    }
    case CONNECT(ACTION_NAME_TOUCHPAD_ON, _hash):
        enableTouchpad(true);
        break;
    case CONNECT(ACTION_NAME_TOUCHPAD_OFF, _hash):
        enableTouchpad(false);
        break;
    default:
        break;
    }
}
}  // namespace Kiran
