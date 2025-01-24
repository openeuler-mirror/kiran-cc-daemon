/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
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

#include "power-profiles-tuned.h"
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusVariant>
#include <QVariantMap>
#include "lib/base/base.h"
#include "power-i.h"

namespace Kiran
{
#define PROFILES_TUNED_MODE_SAVER "powersave"
#define PROFILES_TUNED_MODE_BALANCED "balanced"
#define PROFILES_TUNED_MODE_PERFORMANCE "throughput-performance"

#define PROFILES_TUNED_DBUS_NAME "com.redhat.tuned"
#define PROFILES_TUNED_DBUS_OBJECT_PATH "/Tuned"
#define PROFILES_TUNED_DBUS_INTERFACE "com.redhat.tuned.control"

QDBusArgument &operator<<(QDBusArgument &arg, const SwitchProfileResult &result)
{
    arg.beginStructure();
    arg << result.successed << result.reason;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, SwitchProfileResult &result)
{
    arg.beginStructure();
    arg >> result.successed >> result.reason;
    arg.endStructure();
    return arg;
}

PowerProfilesTuned::PowerProfilesTuned()
{
}

void PowerProfilesTuned::init()
{
    qDBusRegisterMetaType<Kiran::SwitchProfileResult>();

    QDBusConnection::systemBus().connect(PROFILES_TUNED_DBUS_NAME,
                                         PROFILES_TUNED_DBUS_OBJECT_PATH,
                                         PROFILES_TUNED_DBUS_INTERFACE,
                                         "profile_changed",
                                         this,
                                         SLOT(processProfileChanged(const QDBusMessage &)));
}

bool PowerProfilesTuned::switchProfile(int32_t profileMode)
{
    auto retval = holdProfile(profileMode, QString());
    return (retval >= 0);
}

uint32_t PowerProfilesTuned::holdProfile(int32_t profileMode, const QString &)
{
    auto profileModeStr = porfileModeEnum2Str(profileMode);
    auto sendMessage = QDBusMessage::createMethodCall(PROFILES_TUNED_DBUS_NAME,
                                                      PROFILES_TUNED_DBUS_OBJECT_PATH,
                                                      PROFILES_TUNED_DBUS_INTERFACE,
                                                      "switch_profile");

    sendMessage << profileModeStr;

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_INFO(power) << "Call switch_profile return error:" << replyMessage.errorMessage();
        return -1;
    }
    else
    {
        auto result = replyMessage.arguments().takeFirst().value<SwitchProfileResult>();
        if (!result.successed)
        {
            KLOG_WARNING(power) << "Failed to call switch_profile:" << result.reason;
            return -1;
        }
    }

    KLOG_INFO(power) << "Hold power active profile to" << profileModeStr;
    return true;
}

int32_t PowerProfilesTuned::getActiveProfile()
{
    QString profileModeStr;
    auto sendMessage = QDBusMessage::createMethodCall(PROFILES_TUNED_DBUS_NAME,
                                                      PROFILES_TUNED_DBUS_OBJECT_PATH,
                                                      PROFILES_TUNED_DBUS_INTERFACE,
                                                      "active_profile");

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_INFO(power) << "Call active_profile return error:" << replyMessage.errorMessage();
        return POWER_PROFILE_MODE_PERFORMANCE;
    }
    else
    {
        profileModeStr = replyMessage.arguments().takeFirst().value<QString>();
        return porfileModeStr2Enum(profileModeStr);
    }
}

QString PowerProfilesTuned::porfileModeEnum2Str(int32_t profileMode)
{
    switch (profileMode)
    {
    case PowerProfileMode::POWER_PROFILE_MODE_SAVER:
        return PROFILES_TUNED_MODE_SAVER;
    case PowerProfileMode::POWER_PROFILE_MODE_BALANCED:
        return PROFILES_TUNED_MODE_BALANCED;
    case PowerProfileMode::POWER_PROFILE_MODE_PERFORMANCE:
        return PROFILES_TUNED_MODE_PERFORMANCE;
    default:
    {
        KLOG_WARNING(power) << "Unknown profile mode" << profileMode << ", so return performance as current profile mode.";
        return PROFILES_TUNED_MODE_PERFORMANCE;
    }
    }
}

int32_t PowerProfilesTuned::porfileModeStr2Enum(const QString &profileModeStr)
{
    switch (shash(profileModeStr.toLatin1().data()))
    {
    case CONNECT(PROFILES_TUNED_MODE_SAVER, _hash):
        return PowerProfileMode::POWER_PROFILE_MODE_SAVER;
    case CONNECT(PROFILES_TUNED_MODE_BALANCED, _hash):
        return PowerProfileMode::POWER_PROFILE_MODE_BALANCED;
    case CONNECT(PROFILES_TUNED_MODE_PERFORMANCE, _hash):
        return PowerProfileMode::POWER_PROFILE_MODE_PERFORMANCE;

    default:
    {
        KLOG_WARNING(power) << "Unknown profile mode" << profileModeStr << ", so return performance as current profile mode.";
        return PowerProfileMode::POWER_PROFILE_MODE_PERFORMANCE;
    }
    }
}

void PowerProfilesTuned::processProfileChanged(const QDBusMessage &message)
{
    QList<QVariant> args = message.arguments();
    RETURN_IF_TRUE(args.count() != 3);

    auto profileModeStr = args.at(0).value<QString>();
    auto successed = args.at(1).value<bool>();
    auto reason = args.at(2).value<QString>();

    if (!successed)
    {
        KLOG_WARNING(power) << "profile_change signal return failed, reason:" << reason;
    }
    else
    {
        Q_EMIT activeProfileChanged(porfileModeStr2Enum(profileModeStr));
    }
}

}  // namespace Kiran
