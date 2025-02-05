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

#include "power-screensaver.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QThread>
#include "lib/base/base.h"

namespace Kiran
{
#define SCREENSAVER_DBUS_NAME "com.kylinsec.Kiran.ScreenSaver"
#define SCREENSAVER_DBUS_OBJECT_PATH "/com/kylinsec/Kiran/ScreenSaver"
#define SCREENSAVER_DBUS_INTERFACE "com.kylinsec.Kiran.ScreenSaver"

// 屏保锁屏后，检查锁屏状态的最大次数
#define SCREENSAVER_LOCK_CHECK_MAX_COUNT 50

PowerScreenSaver::PowerScreenSaver()
{
}

void PowerScreenSaver::init()
{
}

bool PowerScreenSaver::lock()
{
    // Lock函数无返回，因此这里使用异步调用后再循环检查屏幕保护程序是否已经启动
    auto sendMessage = QDBusMessage::createMethodCall(SCREENSAVER_DBUS_NAME,
                                                      SCREENSAVER_DBUS_OBJECT_PATH,
                                                      SCREENSAVER_DBUS_INTERFACE,
                                                      "Lock");

    QDBusConnection::sessionBus().asyncCall(sendMessage);

    bool isRunning = false;
    for (int32_t i = 0; i < SCREENSAVER_LOCK_CHECK_MAX_COUNT; ++i)
    {
        // 如果屏幕保护程序已经运行，则停止检查
        if (checkRunning())
        {
            isRunning = true;
            break;
        }
        KLOG_INFO(power) << "Timeout waiting for screensaver";
        QThread::msleep(100);
    }

    if (!isRunning)
    {
        KLOG_WARNING(power) << "Failed to lock screen.";
        return false;
    }

    return true;
}

uint32_t PowerScreenSaver::addThrottle(const QString& reason)
{
    auto sendMessage = QDBusMessage::createMethodCall(SCREENSAVER_DBUS_NAME,
                                                      SCREENSAVER_DBUS_OBJECT_PATH,
                                                      SCREENSAVER_DBUS_INTERFACE,
                                                      "Throttle");

    sendMessage << QString("Power screensaver") << reason;

    auto replyMessage = QDBusConnection::sessionBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(power) << "Call Lock return error:" << replyMessage.errorMessage();
        return 0;
    }
    else
    {
        auto cookie = replyMessage.arguments().takeFirst().value<uint>();
        KLOG_DEBUG(power) << "The cookie is" << cookie;
        return cookie;
    }
}

uint32_t PowerScreenSaver::lockAndThrottle(const QString& reason)
{
    RETURN_VAL_IF_FALSE(lock(), 0);
    return addThrottle(reason);
}

bool PowerScreenSaver::removeThrottle(uint32_t cookie)
{
    auto sendMessage = QDBusMessage::createMethodCall(SCREENSAVER_DBUS_NAME,
                                                      SCREENSAVER_DBUS_OBJECT_PATH,
                                                      SCREENSAVER_DBUS_INTERFACE,
                                                      "UnThrottle");

    sendMessage << cookie;

    auto replyMessage = QDBusConnection::sessionBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(power) << "Call UnThrottle return error:" << replyMessage.errorMessage();
        return false;
    }
    return true;
}

bool PowerScreenSaver::poke()
{
    auto sendMessage = QDBusMessage::createMethodCall(SCREENSAVER_DBUS_NAME,
                                                      SCREENSAVER_DBUS_OBJECT_PATH,
                                                      SCREENSAVER_DBUS_INTERFACE,
                                                      "SimulateUserActivity");
    QDBusConnection::sessionBus().asyncCall(sendMessage);
    return true;
}

bool PowerScreenSaver::checkRunning()
{
    auto sendMessage = QDBusMessage::createMethodCall(SCREENSAVER_DBUS_NAME,
                                                      SCREENSAVER_DBUS_OBJECT_PATH,
                                                      SCREENSAVER_DBUS_INTERFACE,
                                                      "GetActive");

    auto replyMessage = QDBusConnection::sessionBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(power) << "Call GetActive return error:" << replyMessage.errorMessage();
        return false;
    }
    else
    {
        auto actived = replyMessage.arguments().takeFirst().value<bool>();
        return actived;
    }
}

}  // namespace Kiran