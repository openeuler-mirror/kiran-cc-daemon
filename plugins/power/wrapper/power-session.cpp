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

#include "power-session.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include "lib/base/base.h"

namespace Kiran
{
#define MATE_SESSION_DBUS_NAME "org.gnome.SessionManager"
#define MATE_SESSION_DBUS_OBJECT "/org/gnome/SessionManager"
#define MATE_SESSION_DBUS_INTERFACE "org.gnome.SessionManager"

#define MATE_SESSION_PRECENSE_DBUS_OBJECT "/org/gnome/SessionManager/Presence"
#define MATE_SESSION_PRECENSE_DBUS_INTERFACE "org.gnome.SessionManager.Presence"

enum GsmPresenceStatus
{
    GSM_PRESENCE_STATUS_AVAILABLE = 0,
    GSM_PRESENCE_STATUS_INVISIBLE,
    GSM_PRESENCE_STATUS_BUSY,
    GSM_PRESENCE_STATUS_IDLE,
};

enum GsmInhibitorFlag
{
    GSM_INHIBITOR_FLAG_LOGOUT = 1 << 0,
    GSM_INHIBITOR_FLAG_SWITCH_USER = 1 << 1,
    GSM_INHIBITOR_FLAG_SUSPEND = 1 << 2,
    GSM_INHIBITOR_FLAG_IDLE = 1 << 3
};

PowerSession::PowerSession() : m_isIdle(false),
                               isIdleInhibited(false),
                               isSuspendInhibited(false)
{
}

void PowerSession::init()
{
    m_isIdle = getIdle();
    isIdleInhibited = getInhibited(GSM_INHIBITOR_FLAG_IDLE);
    isSuspendInhibited = getInhibited(GSM_INHIBITOR_FLAG_SUSPEND);

    QDBusConnection::sessionBus().connect(MATE_SESSION_DBUS_NAME,
                                          MATE_SESSION_DBUS_OBJECT,
                                          MATE_SESSION_DBUS_INTERFACE,
                                          "InhibitorAdded",
                                          this,
                                          SLOT(processInhibitorChanged(const QDBusMessage&)));

    QDBusConnection::sessionBus().connect(MATE_SESSION_DBUS_NAME,
                                          MATE_SESSION_DBUS_OBJECT,
                                          MATE_SESSION_DBUS_INTERFACE,
                                          "InhibitorRemoved",
                                          this,
                                          SLOT(processInhibitorChanged(const QDBusMessage&)));

    QDBusConnection::sessionBus().connect(MATE_SESSION_DBUS_NAME,
                                          MATE_SESSION_PRECENSE_DBUS_OBJECT,
                                          MATE_SESSION_PRECENSE_DBUS_INTERFACE,
                                          "StatusChanged",
                                          this,
                                          SLOT(processPresenceStatusChanged(const QDBusMessage&)));
}

bool PowerSession::canSuspend()
{
    auto sendMessage = QDBusMessage::createMethodCall(MATE_SESSION_DBUS_NAME,
                                                      MATE_SESSION_DBUS_OBJECT,
                                                      MATE_SESSION_DBUS_INTERFACE,
                                                      "CanSuspend");

    auto replyMessage = QDBusConnection::sessionBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(power) << "Call CanSuspend return error:" << replyMessage.errorMessage();
        return false;
    }
    else
    {
        return replyMessage.arguments().takeFirst().value<bool>();
    }
}

void PowerSession::suspend()
{
    if (!canSuspend())
    {
        KLOG_WARNING(power) << "The session manager doesn't allow suspend.";
        return;
    }

    auto sendMessage = QDBusMessage::createMethodCall(MATE_SESSION_DBUS_NAME,
                                                      MATE_SESSION_DBUS_OBJECT,
                                                      MATE_SESSION_DBUS_INTERFACE,
                                                      "Suspend");

    auto replyMessage = QDBusConnection::sessionBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(power) << "Call Suspend return error:" << replyMessage.errorMessage();
    }
}

bool PowerSession::canHibernate()
{
    auto sendMessage = QDBusMessage::createMethodCall(MATE_SESSION_DBUS_NAME,
                                                      MATE_SESSION_DBUS_OBJECT,
                                                      MATE_SESSION_DBUS_INTERFACE,
                                                      "CanHibernate");

    auto replyMessage = QDBusConnection::sessionBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(power) << "Call CanHibernate return error:" << replyMessage.errorMessage();
        return false;
    }
    else
    {
        return replyMessage.arguments().takeFirst().value<bool>();
    }
}

void PowerSession::hibernate()
{
    if (!canHibernate())
    {
        KLOG_WARNING("The session manager doesn't allow hibernate.");
        return;
    }

    auto sendMessage = QDBusMessage::createMethodCall(MATE_SESSION_DBUS_NAME,
                                                      MATE_SESSION_DBUS_OBJECT,
                                                      MATE_SESSION_DBUS_INTERFACE,
                                                      "Hibernate");

    auto replyMessage = QDBusConnection::sessionBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(power) << "Call Hibernate return error:" << replyMessage.errorMessage();
    }
}

bool PowerSession::canShutdown()
{
    auto sendMessage = QDBusMessage::createMethodCall(MATE_SESSION_DBUS_NAME,
                                                      MATE_SESSION_DBUS_OBJECT,
                                                      MATE_SESSION_DBUS_INTERFACE,
                                                      "CanShutdown");

    auto replyMessage = QDBusConnection::sessionBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(power) << "Call CanShutdown return error:" << replyMessage.errorMessage();
        return false;
    }
    else
    {
        return replyMessage.arguments().takeFirst().value<bool>();
    }
}

void PowerSession::shutdown()
{
    if (!canShutdown())
    {
        KLOG_WARNING("The session manager doesn't allow shutdown.");
        return;
    }

    auto sendMessage = QDBusMessage::createMethodCall(MATE_SESSION_DBUS_NAME,
                                                      MATE_SESSION_DBUS_OBJECT,
                                                      MATE_SESSION_DBUS_INTERFACE,
                                                      "Shutdown");

    auto replyMessage = QDBusConnection::sessionBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(power) << "Call Shutdown return error:" << replyMessage.errorMessage();
    }
}

uint32_t PowerSession::getStatus()
{
    QDBusInterface interface(MATE_SESSION_DBUS_NAME,
                             MATE_SESSION_PRECENSE_DBUS_OBJECT,
                             MATE_SESSION_PRECENSE_DBUS_INTERFACE,
                             QDBusConnection::sessionBus());
    auto status = interface.property("status").toUInt();
    return status;
}

bool PowerSession::getInhibited(uint32_t flag)
{
    auto sendMessage = QDBusMessage::createMethodCall(MATE_SESSION_DBUS_NAME,
                                                      MATE_SESSION_DBUS_OBJECT,
                                                      MATE_SESSION_DBUS_INTERFACE,
                                                      "IsInhibited");

    sendMessage << flag;

    auto replyMessage = QDBusConnection::sessionBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(power) << "Call IsInhibited return error:" << replyMessage.errorMessage();
        return false;
    }
    else
    {
        return replyMessage.arguments().takeFirst().value<bool>();
    }
}

void PowerSession::processInhibitorChanged(const QDBusMessage& message)
{
    auto isIdleInhibited = getInhibited(GSM_INHIBITOR_FLAG_IDLE);
    auto isSuspendInhibited = getInhibited(GSM_INHIBITOR_FLAG_SUSPEND);

    if (isIdleInhibited != isIdleInhibited || isSuspendInhibited != isSuspendInhibited)
    {
        isIdleInhibited = isIdleInhibited;
        isSuspendInhibited = isSuspendInhibited;
        Q_EMIT inhibitorChanged();
    }
}

void PowerSession::processPresenceStatusChanged(const QDBusMessage& message)
{
    auto status = message.arguments().takeFirst().value<uint>();
    KLOG_DEBUG(power) << "Sm presence status is changed to" << status;

    bool isIdle = (status == GSM_PRESENCE_STATUS_IDLE);
    if (isIdle != m_isIdle)
    {
        m_isIdle = isIdle;
        Q_EMIT idleStatusChanged(isIdle);
    }
}

}  // namespace Kiran