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

#include "power-login1.h"
#include <fcntl.h>
#include <glib.h>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusUnixFileDescriptor>
#include "lib/base/base.h"

namespace Kiran
{
#define LOGIN1_DBUS_NAME "org.freedesktop.login1"
#define LOGIN1_DBUS_OBJECT_PATH "/org/freedesktop/login1"
#define LOGIN1_MANAGER_DBUS_INTERFACE "org.freedesktop.login1.Manager"

PowerLogin1::PowerLogin1()
{
}

void PowerLogin1::init()
{
}

int32_t PowerLogin1::inhibit(const QString& what)
{
    auto sendMessage = QDBusMessage::createMethodCall(LOGIN1_DBUS_NAME,
                                                      LOGIN1_DBUS_OBJECT_PATH,
                                                      LOGIN1_MANAGER_DBUS_INTERFACE,
                                                      "Inhibit");

    auto currentUserName = g_get_user_name();

    sendMessage << what
                << POINTER_TO_STRING(currentUserName)
                << QString("The power plugin of kiran-session-daemon handles these events")
                << QString("block");

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_INFO(power) << "Call Inhibit return error:" << replyMessage.errorMessage();
    }
    else
    {
        auto systemdInhibitFd = replyMessage.arguments().takeFirst().value<QDBusUnixFileDescriptor>();
        return fcntl(systemdInhibitFd.fileDescriptor(), F_DUPFD_CLOEXEC, 0);
    }
    return -1;
}

bool PowerLogin1::suspend()
{
    auto sendMessage = QDBusMessage::createMethodCall(LOGIN1_DBUS_NAME,
                                                      LOGIN1_DBUS_OBJECT_PATH,
                                                      LOGIN1_MANAGER_DBUS_INTERFACE,
                                                      "Suspend");

    sendMessage << false;

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_INFO(power) << "Call Suspend return error:" << replyMessage.errorMessage();
        return false;
    }

    return true;
}

bool PowerLogin1::hibernate()
{
    auto sendMessage = QDBusMessage::createMethodCall(LOGIN1_DBUS_NAME,
                                                      LOGIN1_DBUS_OBJECT_PATH,
                                                      LOGIN1_MANAGER_DBUS_INTERFACE,
                                                      "Hibernate");

    sendMessage << false;

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_INFO(power) << "Call Hibernate return error:" << replyMessage.errorMessage();
        return false;
    }

    return true;
}

bool PowerLogin1::shutdown()
{
    auto sendMessage = QDBusMessage::createMethodCall(LOGIN1_DBUS_NAME,
                                                      LOGIN1_DBUS_OBJECT_PATH,
                                                      LOGIN1_MANAGER_DBUS_INTERFACE,
                                                      "PowerOff");

    sendMessage << false;

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_INFO(power) << "Call PowerOff return error:" << replyMessage.errorMessage();
        return false;
    }

    return true;
}

}  // namespace Kiran