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

#include "src/session-guarder.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include "config.h"

namespace Kiran
{
#define MATE_SESSION_DBUS_NAME "org.gnome.SessionManager"
#define MATE_SESSION_DBUS_OBJECT "/org/gnome/SessionManager"
#define MATE_SESSION_DBUS_INTERFACE "org.gnome.SessionManager"
#define MATE_SESSION_PRIVATE_DBUS_INTERFACE "org.gnome.SessionManager.ClientPrivate"

SessionGuarder::SessionGuarder()
{
}

SessionGuarder* SessionGuarder::m_instance = nullptr;

void SessionGuarder::globalInit()
{
    m_instance = new SessionGuarder();
    m_instance->init();
}

void SessionGuarder::init()
{
    auto sendMessage = QDBusMessage::createMethodCall(MATE_SESSION_DBUS_NAME,
                                                      MATE_SESSION_DBUS_OBJECT,
                                                      MATE_SESSION_DBUS_INTERFACE,
                                                      "RegisterClient");
    // 因为设置了开机启动，所以需要向SessionManager发起注册请求，否则SessionManager会长时间等待导致进入桌面延时
    auto startupID = qgetenv("DESKTOP_AUTOSTART_ID");

    if (startupID.isEmpty())
    {
        KLOG_WARNING() << "Not found startup id from environment variable DESKTOP_AUTOSTART_ID";
        return;
    }

    sendMessage << QString("kiran-session-daemon") << QString(startupID);

    auto replyMessage = QDBusConnection::sessionBus().call(sendMessage, QDBus::Block, DBUS_TIMEOUT_MS);

    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING() << "Call RegisterClient failed: " << replyMessage.errorMessage();
    }
    else
    {
        auto variant1 = replyMessage.arguments().takeFirst();
        m_clientObjectPath = variant1.value<QDBusObjectPath>();

        QDBusConnection::sessionBus().connect(MATE_SESSION_DBUS_NAME,
                                              m_clientObjectPath.path(),
                                              MATE_SESSION_PRIVATE_DBUS_INTERFACE,
                                              QLatin1String("QueryEndSession"),
                                              this,
                                              SLOT(responseSessionQueryEnd()));

        QDBusConnection::sessionBus().connect(MATE_SESSION_DBUS_NAME,
                                              m_clientObjectPath.path(),
                                              MATE_SESSION_PRIVATE_DBUS_INTERFACE,
                                              QLatin1String("EndSession"),
                                              this,
                                              SLOT(responseSessionEnd()));
    }
}

void SessionGuarder::responseSessionQueryEnd()
{
    auto sendMessage = QDBusMessage::createMethodCall(MATE_SESSION_DBUS_NAME,
                                                      m_clientObjectPath.path(),
                                                      MATE_SESSION_PRIVATE_DBUS_INTERFACE,
                                                      "EndSessionResponse");
    sendMessage << true << QString();
    auto replyMessage = QDBusConnection::sessionBus().call(sendMessage, QDBus::Block, DBUS_TIMEOUT_MS);

    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING() << "Call EndSessionResponse failed: " << replyMessage.errorMessage();
    }
}

void SessionGuarder::responseSessionEnd()
{
    // 发送会话退出的信号，让资源释放等逻辑先处理完，然后再回复dbus消息，否则可能会导致资源释放到一半进程直接被干掉了
    Q_EMIT sessionEnd();

    auto sendMessage = QDBusMessage::createMethodCall(MATE_SESSION_DBUS_NAME,
                                                      m_clientObjectPath.path(),
                                                      MATE_SESSION_PRIVATE_DBUS_INTERFACE,
                                                      "EndSessionResponse");
    sendMessage << true << QString();
    auto replyMessage = QDBusConnection::sessionBus().call(sendMessage, QDBus::Block, DBUS_TIMEOUT_MS);

    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING() << "Call EndSessionResponse failed: " << replyMessage.errorMessage();
    }

    QCoreApplication::quit();
}
}  // namespace Kiran