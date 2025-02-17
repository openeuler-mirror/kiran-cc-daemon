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
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 */

#include "greeter-manager.h"
#include <KConfig>
#include <KConfigGroup>
#include <QFileSystemWatcher>
#include <QSettings>
#include <QSignalBlocker>
#include "greeteradaptor.h"
#include "lib/base/base.h"
#include "lib/base/polkit-proxy.h"

#define LIGHTDM_PROFILE_PATH "/usr/share/lightdm/lightdm.conf.d/99-kiran-greeter-login.conf"
#define LIGHTDM_GROUP_NAME "Seat:seat0"

#define GREETER_PROFILE_PATH "/etc/lightdm/kiran-greeter.conf"
#define GREETER_GROUP_NAME "Greeter"

#define KEY_AUTOLOGIN_USER "autologin-user"
#define KEY_AUTOLOGIN_DELAY "autologin-user-timeout"
#define KEY_AUTOLOGIN_SESSION "autologin-session"
#define DEFAULT_AUTOLOGIN_SESSION "kiran"

#define KEY_LIGHTDM_HIDE_USER_LIST "greeter-hide-users"
#define KEY_LIGHTDM_ENABLE_MANUAL_LOGIN "greeter-show-manual-login"

#define KEY_GREETER_ENABLE_MANUAL_LOGIN "enable-manual-login"
#define KEY_GREETER_HIDE_USER_LIST "user-list-hiding"

#define KEY_BACKGROUND_FILE "background-picture-uri"
#define KEY_SCALE_FACTOR "scale-factor"
#define KEY_ENABLE_SCALING "enable-scaling"

namespace Kiran
{
#define AUTH_SET_LOGIN_OPTION "com.kylinsec.kiran.system-daemon.greeter.set-login-option"
GreeterManager::GreeterManager() : m_scaleMode(GREETER_SCALING_MODE_AUTO),
                                   m_autologinDelay(0),
                                   m_scaleFactor(1),
                                   m_enableManualLogin(true),
                                   m_hideUserList(false)
{
    m_adaptor = new GreeterAdaptor(this);
    m_settingsWatcher = new QFileSystemWatcher(this);
}

GreeterManager::~GreeterManager()
{
}

GreeterManager *GreeterManager::m_instance = nullptr;
void GreeterManager::globalInit()
{
    m_instance = new GreeterManager();
    m_instance->init();
}

bool GreeterManager::getAllowManualLogin() const
{
    return m_enableManualLogin;
}

qulonglong GreeterManager::getAutologinTimeout() const
{
    return m_autologinDelay;
}

QString GreeterManager::getAutologinUser() const
{
    return m_autologinUser;
}

QString GreeterManager::getBackground() const
{
    return m_backgroundFile;
}

bool GreeterManager::getHideUserList() const
{
    return m_hideUserList;
}

ushort GreeterManager::getScaleFactor() const
{
    return m_scaleFactor;
}

ushort GreeterManager::getScaleMode() const
{
    return m_scaleMode;
}

#define SEND_PROPERTY_NOTIFY(property, propertyHump)                          \
    QVariantMap changedProperties;                                            \
    changedProperties.insert(QStringLiteral(#property), get##propertyHump()); \
                                                                              \
    QDBusMessage signalMessage = QDBusMessage::createSignal(                  \
        GREETER_OBJECT_PATH,                                                  \
        QStringLiteral("org.freedesktop.DBus.Properties"),                    \
        QStringLiteral("PropertiesChanged"));                                 \
                                                                              \
    signalMessage.setArguments({                                              \
        GREETER_DBUS_INTERFACE_NAME,                                          \
        changedProperties,                                                    \
        QStringList(),                                                        \
    });                                                                       \
    QDBusConnection::systemBus().send(signalMessage);

#define WRITE_LIGHTDM_SETTINGS(key, value)                                \
    KConfig lightdmSettings(LIGHTDM_PROFILE_PATH, KConfig::SimpleConfig); \
    auto group = lightdmSettings.group(LIGHTDM_GROUP_NAME);               \
    KLOG_INFO(greeter) << "Set" << key << "to" << value;                  \
    group.writeEntry(key, value);                                         \
    group.sync();

#define WRITE_GREETER_SETTINGS(key, value)                                 \
    QSettings greeterSettings(GREETER_PROFILE_PATH, QSettings::IniFormat); \
    auto concatKey = QString("%1/%2").arg(GREETER_GROUP_NAME).arg(key);    \
    KLOG_INFO(greeter) << "Set" << key << "to" << value;                   \
    greeterSettings.setValue(concatKey, value);

void GreeterManager::setAllowManualLogin(bool allow)
{
    RETURN_IF_TRUE(allow == m_enableManualLogin);
    m_enableManualLogin = allow;
    WRITE_LIGHTDM_SETTINGS(KEY_LIGHTDM_ENABLE_MANUAL_LOGIN, allow);
    SEND_PROPERTY_NOTIFY(allow_manual_login, AllowManualLogin);
}

void GreeterManager::setAutologinTimeout(qulonglong seconds)
{
    RETURN_IF_TRUE(seconds == m_autologinDelay);
    m_autologinDelay = seconds;
    WRITE_LIGHTDM_SETTINGS(KEY_AUTOLOGIN_DELAY, seconds)
    SEND_PROPERTY_NOTIFY(autologin_timeout, AutologinTimeout);
}

void GreeterManager::setAutologinUser(const QString &userName)
{
    RETURN_IF_TRUE(userName == m_autologinUser);
    m_autologinUser = userName;
    WRITE_LIGHTDM_SETTINGS(KEY_AUTOLOGIN_USER, userName);
    SEND_PROPERTY_NOTIFY(autologin_user, AutologinUser);
}

void GreeterManager::setBackground(const QString &filePath)
{
    RETURN_IF_TRUE(filePath == m_backgroundFile);
    m_backgroundFile = filePath;
    WRITE_GREETER_SETTINGS(KEY_BACKGROUND_FILE, filePath);
    SEND_PROPERTY_NOTIFY(background, Background);
}

void GreeterManager::setHideUserList(bool hide)
{
    RETURN_IF_TRUE(hide == m_hideUserList);
    m_hideUserList = hide;
    WRITE_LIGHTDM_SETTINGS(KEY_LIGHTDM_HIDE_USER_LIST, hide);
    SEND_PROPERTY_NOTIFY(hide_user_list, HideUserList);
}

void GreeterManager::setScaleFactor(ushort scaleFactor)
{
    RETURN_IF_TRUE(scaleFactor == m_scaleFactor);
    m_scaleFactor = scaleFactor;
    WRITE_GREETER_SETTINGS(KEY_SCALE_FACTOR, scaleFactor);
    SEND_PROPERTY_NOTIFY(scale_factor, ScaleFactor);
}

void GreeterManager::setScaleMode(ushort scaleMode)
{
    RETURN_IF_TRUE(scaleMode == m_scaleMode);
    m_scaleMode = scaleMode;
    auto scaleModeStr = scaleModeEnum2Str(scaleMode);
    WRITE_GREETER_SETTINGS(KEY_ENABLE_SCALING, scaleModeStr);
    SEND_PROPERTY_NOTIFY(scale_mode, ScaleMode);
}

CHECK_AUTH_WITH_1ARGS(GreeterManager,
                      SetAllowManualLogin,
                      setAllowManualLoginAuthenticated,
                      AUTH_SET_LOGIN_OPTION,
                      bool)

CHECK_AUTH_WITH_1ARGS(GreeterManager,
                      SetAutologinTimeout,
                      setAutologinTimeoutAuthenticated,
                      AUTH_SET_LOGIN_OPTION,
                      qulonglong)

CHECK_AUTH_WITH_1ARGS(GreeterManager,
                      SetAutologinUser,
                      setAutologinUserAuthenticated,
                      AUTH_SET_LOGIN_OPTION,
                      const QString &)

CHECK_AUTH_WITH_1ARGS(GreeterManager,
                      SetBackground,
                      setBackgroundAuthenticated,
                      AUTH_SET_LOGIN_OPTION,
                      const QString &)

CHECK_AUTH_WITH_1ARGS(GreeterManager,
                      SetHideUserList,
                      setHideUserListAuthenticated,
                      AUTH_SET_LOGIN_OPTION,
                      bool)

CHECK_AUTH_WITH_2ARGS(GreeterManager,
                      SetScaleMode,
                      setScaleModeAuthenticated,
                      AUTH_SET_LOGIN_OPTION,
                      ushort,
                      ushort)

void GreeterManager::setAllowManualLoginAuthenticated(const QDBusMessage &message, bool allow)
{
    setAllowManualLogin(allow);
    QDBusConnection::systemBus().send(message.createReply());
}

void GreeterManager::setAutologinTimeoutAuthenticated(const QDBusMessage &message, qulonglong seconds)
{
    setAutologinTimeout(seconds);
    QDBusConnection::systemBus().send(message.createReply());
}

void GreeterManager::setAutologinUserAuthenticated(const QDBusMessage &message, const QString &userName)
{
    setAutologinUser(userName);
    QDBusConnection::systemBus().send(message.createReply());
}

void GreeterManager::setBackgroundAuthenticated(const QDBusMessage &message, const QString &filePath)
{
    setBackground(filePath);
    QDBusConnection::systemBus().send(message.createReply());
}

void GreeterManager::setHideUserListAuthenticated(const QDBusMessage &message, bool hide)
{
    setHideUserList(hide);
    QDBusConnection::systemBus().send(message.createReply());
}

void GreeterManager::setScaleModeAuthenticated(const QDBusMessage &message, ushort mode, ushort factor)
{
    setScaleMode(mode);
    setScaleFactor(factor);
    QDBusConnection::systemBus().send(message.createReply());
}

void GreeterManager::init()
{
    QSignalBlocker blocker(this);

    loadPropsFromSettings();

    m_settingsWatcher->addPath(GREETER_PROFILE_PATH);
    m_settingsWatcher->addPath(LIGHTDM_PROFILE_PATH);

    auto systemConnection = QDBusConnection::systemBus();
    if (!systemConnection.registerService(GREETER_DBUS_NAME))
    {
        KLOG_WARNING() << "Failed to register dbus name: " << GREETER_DBUS_NAME;
        return;
    }

    if (!systemConnection.registerObject(GREETER_OBJECT_PATH, GREETER_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR() << "Can't register object:" << systemConnection.lastError();
        return;
    }

    connect(m_settingsWatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(processSettingsChanged(const QString &)));
}

void GreeterManager::loadPropsFromSettings()
{
    KConfig lightdmSettings(LIGHTDM_PROFILE_PATH, KConfig::SimpleConfig);
    QSettings greeterSettings(GREETER_PROFILE_PATH, QSettings::IniFormat);
    int scaleFactor = 1;

    greeterSettings.beginGroup(GREETER_GROUP_NAME);
    auto seatGroup = lightdmSettings.group(LIGHTDM_GROUP_NAME);

    m_autologinUser = seatGroup.readEntry(KEY_AUTOLOGIN_USER);
    m_autologinDelay = seatGroup.readEntry(KEY_AUTOLOGIN_DELAY, 0);
    m_enableManualLogin = seatGroup.readEntry(KEY_LIGHTDM_ENABLE_MANUAL_LOGIN, true);
    m_hideUserList = seatGroup.readEntry(KEY_LIGHTDM_HIDE_USER_LIST, false);
    m_backgroundFile = greeterSettings.value(KEY_BACKGROUND_FILE).toString();
    m_scaleMode = scaleModeStr2Enum(greeterSettings.value(KEY_ENABLE_SCALING).toString());
    scaleFactor = greeterSettings.value(KEY_SCALE_FACTOR).toInt();
    m_scaleFactor = (scaleFactor <= 1) ? 1 : 2;

    greeterSettings.endGroup();
}

int GreeterManager::scaleModeStr2Enum(const QString &str)
{
    switch (shash(str.toLatin1().data()))
    {
    case "manual"_hash:
        return GREETER_SCALING_MODE_MANUAL;
    case "disable"_hash:
        return GREETER_SCALING_MODE_DISABLE;
    default:
        break;
    }
    return GREETER_SCALING_MODE_AUTO;
}

QString GreeterManager::scaleModeEnum2Str(int scaleMode)
{
    switch (scaleMode)
    {
    case GREETER_SCALING_MODE_AUTO:
        return "auto";
    case GREETER_SCALING_MODE_MANUAL:
        return "manual";
    case GREETER_SCALING_MODE_DISABLE:
        return "disable";
    default:
        break;
    }
    return "auto";
}

void GreeterManager::processSettingsChanged(const QString &path)
{
    KLOG_INFO(greeter) << "File" << path << "is changed";

    switch (shash(path.toLatin1().data()))
    {
    case CONNECT(LIGHTDM_PROFILE_PATH, _hash):
    {
        KConfig lightdmSettings(LIGHTDM_PROFILE_PATH, KConfig::SimpleConfig);
        auto seatGroup = lightdmSettings.group(LIGHTDM_GROUP_NAME);
        auto autologinUser = seatGroup.readEntry(KEY_AUTOLOGIN_USER, QString());
        auto autologinDelay = seatGroup.readEntry(KEY_AUTOLOGIN_DELAY, 0);
        auto enableManualLogin = seatGroup.readEntry(KEY_LIGHTDM_ENABLE_MANUAL_LOGIN, true);
        auto hideUserList = seatGroup.readEntry(KEY_LIGHTDM_HIDE_USER_LIST, false);

        setAutologinUser(autologinUser);
        setAutologinTimeout(autologinDelay);
        setAllowManualLogin(enableManualLogin);
        setHideUserList(hideUserList);
        m_settingsWatcher->addPath(LIGHTDM_PROFILE_PATH);
        break;
    }
    case CONNECT(GREETER_PROFILE_PATH, _hash):
    {
        QSettings greeterSettings(GREETER_PROFILE_PATH, QSettings::IniFormat);

        greeterSettings.beginGroup(GREETER_GROUP_NAME);

        auto backgroundFile = greeterSettings.value(KEY_BACKGROUND_FILE).toString();
        auto scaleMode = scaleModeStr2Enum(greeterSettings.value(KEY_ENABLE_SCALING).toString());
        auto scaleFactor = greeterSettings.value(KEY_SCALE_FACTOR).toInt();
        scaleFactor = (scaleFactor <= 1) ? 1 : 2;

        setBackground(backgroundFile);
        setScaleMode(scaleMode);
        setScaleFactor(scaleFactor);
        greeterSettings.endGroup();

        m_settingsWatcher->addPath(GREETER_PROFILE_PATH);
        break;
    }
    default:
        break;
    }
}

}  // namespace Kiran