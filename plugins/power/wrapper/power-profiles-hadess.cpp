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

#include "power-profiles-hadess.h"
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QVariantMap>
#include "lib/base/base.h"
#include "power-i.h"

namespace Kiran
{
#define PROFILES_HADESS_MODE_SAVER "power-saver"
#define PROFILES_HADESS_MODE_BALANCED "balanced"
#define PROFILES_HADESS_MODE_PERFORMANCE "performance"

#define PROFILES_HADESS_DBUS_NAME "net.hadess.PowerProfiles"
#define PROFILES_HADESS_DBUS_OBJECT_PATH "/net/hadess/PowerProfiles"
#define PROFILES_HADESS_DBUS_INTERFACE "net.hadess.PowerProfiles"
#define PROFILES_HADESS_DBUS_PROP_ACTIVE_PROFILE "ActiveProfile"

PowerProfilesHadess::PowerProfilesHadess()
{
}

void PowerProfilesHadess::init()
{
    QDBusConnection::systemBus().connect(PROFILES_HADESS_DBUS_NAME,
                                         PROFILES_HADESS_DBUS_OBJECT_PATH,
                                         QStringLiteral("org.freedesktop.DBus.Properties"),
                                         "PropertiesChanged",
                                         this,
                                         SLOT(processPropertiesChanged(const QDBusMessage &)));
}

bool PowerProfilesHadess::switchProfile(int32_t profileMode)
{
    auto profileModeStr = porfileModeEnum2Str(profileMode);
    auto sendMessage = QDBusMessage::createMethodCall(PROFILES_HADESS_DBUS_NAME,
                                                      PROFILES_HADESS_DBUS_OBJECT_PATH,
                                                      "org.freedesktop.DBus.Properties",
                                                      "Set");

    sendMessage << QString(PROFILES_HADESS_DBUS_INTERFACE)
                << QString(PROFILES_HADESS_DBUS_PROP_ACTIVE_PROFILE)
                << QVariant::fromValue(QDBusVariant(profileModeStr));

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_INFO(power) << "Call set ActiveProfile property return error:" << replyMessage.errorMessage();
        return false;
    }

    KLOG_INFO(power) << "Switch power active profile to" << profileModeStr;
    return true;
}

uint32_t PowerProfilesHadess::holdProfile(int32_t profileMode, const QString &reason)
{
    auto profileModeStr = porfileModeEnum2Str(profileMode);
    auto sendMessage = QDBusMessage::createMethodCall(PROFILES_HADESS_DBUS_NAME,
                                                      PROFILES_HADESS_DBUS_OBJECT_PATH,
                                                      PROFILES_HADESS_DBUS_INTERFACE,
                                                      "HoldProfile");

    sendMessage << profileModeStr << reason << "kiran-session-daemon";

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_INFO(power) << "Call HoldProfile return error:" << replyMessage.errorMessage();
        return false;
    }
    else
    {
        KLOG_INFO(power) << "Hold power active profile to" << profileModeStr;
        return replyMessage.arguments().takeFirst().value<uint32_t>();
    }
}

void PowerProfilesHadess::releaseProfile(uint32_t cookie)
{
    auto sendMessage = QDBusMessage::createMethodCall(PROFILES_HADESS_DBUS_NAME,
                                                      PROFILES_HADESS_DBUS_OBJECT_PATH,
                                                      PROFILES_HADESS_DBUS_INTERFACE,
                                                      "ReleaseProfile");

    sendMessage << cookie;

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_INFO(power) << "Call ReleaseProfile return error:" << replyMessage.errorMessage();
    }
}

int32_t PowerProfilesHadess::getActiveProfile()
{
    QDBusInterface interface(PROFILES_HADESS_DBUS_NAME,
                             PROFILES_HADESS_DBUS_OBJECT_PATH,
                             PROFILES_HADESS_DBUS_INTERFACE,
                             QDBusConnection::systemBus());

    auto profileModeStr = interface.property(PROFILES_HADESS_DBUS_PROP_ACTIVE_PROFILE).toString();
    return porfileModeStr2Enum(profileModeStr);
}

QString PowerProfilesHadess::porfileModeEnum2Str(int32_t profileMode)
{
    switch (profileMode)
    {
    case PowerProfileMode::POWER_PROFILE_MODE_SAVER:
        return PROFILES_HADESS_MODE_SAVER;
    case PowerProfileMode::POWER_PROFILE_MODE_BALANCED:
        return PROFILES_HADESS_MODE_BALANCED;
    case PowerProfileMode::POWER_PROFILE_MODE_PERFORMANCE:
        return PROFILES_HADESS_MODE_PERFORMANCE;
    default:
    {
        KLOG_WARNING(power) << "Unknown profile mode" << profileMode << ", so return performance as current profile mode.";
        return PROFILES_HADESS_MODE_PERFORMANCE;
    }
    }
}

int32_t PowerProfilesHadess::porfileModeStr2Enum(const QString &profileModeStr)
{
    switch (shash(profileModeStr.toLatin1().data()))
    {
    case CONNECT(PROFILES_HADESS_MODE_SAVER, _hash):
        return PowerProfileMode::POWER_PROFILE_MODE_SAVER;
    case CONNECT(PROFILES_HADESS_MODE_BALANCED, _hash):
        return PowerProfileMode::POWER_PROFILE_MODE_BALANCED;
    case CONNECT(PROFILES_HADESS_MODE_PERFORMANCE, _hash):
        return PowerProfileMode::POWER_PROFILE_MODE_PERFORMANCE;

    default:
    {
        KLOG_WARNING(power) << "Unknown profile mode" << profileModeStr << ", so return performance as current profile mode.";
        return PowerProfileMode::POWER_PROFILE_MODE_PERFORMANCE;
    }
    }
}

void PowerProfilesHadess::processPropertiesChanged(const QDBusMessage &message)
{
    QList<QVariant> args = message.arguments();
    RETURN_IF_TRUE(args.count() != 3);

    QVariantMap changedProperties = qdbus_cast<QVariantMap>(args.at(1).value<QDBusArgument>());
    auto iter = changedProperties.find(PROFILES_HADESS_DBUS_PROP_ACTIVE_PROFILE);
    RETURN_IF_TRUE(iter == changedProperties.end());
    auto profileModeStr = iter.value().toString();
    auto profileMode = porfileModeStr2Enum(profileModeStr);
    Q_EMIT activeProfileChanged(profileMode);
}

}  // namespace Kiran
