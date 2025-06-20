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

#include "user.h"
#include <crypt.h>
#include <grp.h>
#include <sys/types.h>
#include <QDBusConnection>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include "accounts-manager.h"
#include "accounts-util.h"
#include "config-accounts.h"
#include "lib/base/base.h"
#include "lib/base/crypto-helper.h"
#include "lib/base/polkit-proxy.h"
#include "useradaptor.h"

namespace Kiran
{
#define FREEDESKTOP_ACCOUNTS_DBUS_NAME "org.freedesktop.Accounts"
#define FREEDESKTOP_ACCOUNTS_OBJECT_PATH "/org/freedesktop/Accounts"
#define FREEDESKTOP_ACCOUNTS_DBUS_INTERFACE "org.freedesktop.Accounts"
#define FREEDESKTOP_ACCOUNTS_USER_DBUS_INTERFACE "org.freedesktop.Accounts.User"

#define KEYFILE_USER_GROUP_NAME "User"
#define KEYFILE_USER_GROUP_KEY_LANGUAGE "Language"
#define KEYFILE_USER_GROUP_KEY_XSESSION "XSession"
#define KEYFILE_USER_GROUP_KEY_SESSION "Session"
#define KEYFILE_USER_GROUP_KEY_SESSION_TYPE "SessionType"
#define KEYFILE_USER_GROUP_KEY_EMAIL "Email"
#define KEYFILE_USER_GROUP_KEY_PASSWORD_HINT "PasswordHint"
#define KEYFILE_USER_GROUP_KEY_ICON "Icon"

#define KIRAN_ACCOUNTS_USER_OBJECT_PATH "/com/kylinsec/Kiran/SystemDaemon/Accounts/User"
#define KIRAN_ACCOUNTS_USER_INTERFACE "com.kylinsec.Kiran.SystemDaemon.Accounts.User"

User::User(PasswdShadow passwdShadow) : m_passwdShadow(passwdShadow),
                                        m_automaticLogin(0),
                                        m_locked(false),
                                        m_passwordMode(0),
                                        m_systemAccount(false),
                                        m_settings(nullptr)

{
    m_uid = m_passwdShadow.first->uid;
    m_gid = m_passwdShadow.first->gid;
    m_objectPath = QString("%1/%2").arg(KIRAN_ACCOUNTS_USER_OBJECT_PATH).arg(m_uid);

    m_userAdaptor = new UserAdaptor(this);
}

User::~User()
{
    dbusUnregister();
}

CHECK_AUTH_WITH_1ARGS(User,
                      SetAccountType,
                      setAccountTypeAuthenticated,
                      getAuthAction(message(), AUTH_USER_ADMIN),
                      int)

CHECK_AUTH_WITH_1ARGS(User,
                      SetAutomaticLogin,
                      setAutomaticLoginAuthenticated,
                      getAuthAction(message(), AUTH_USER_ADMIN),
                      bool)

CHECK_AUTH_WITH_1ARGS(User,
                      SetEmail,
                      setEmailAuthenticated,
                      getAuthAction(message(), AUTH_CHANGE_OWN_USER_DATA),
                      const QString &)

CHECK_AUTH_WITH_1ARGS(User,
                      SetHomeDirectory,
                      setHomeDirectoryAuthenticated,
                      getAuthAction(message(), AUTH_USER_ADMIN),
                      const QString &)

CHECK_AUTH_WITH_1ARGS(User,
                      SetIconFile,
                      setIconFileAuthenticated,
                      getAuthAction(message(), AUTH_CHANGE_OWN_USER_DATA),
                      const QString &)

CHECK_AUTH_WITH_1ARGS(User,
                      SetLanguage,
                      setLanguageAuthenticated,
                      getAuthAction(message(), AUTH_CHANGE_OWN_USER_DATA),
                      const QString &)

CHECK_AUTH_WITH_1ARGS(User,
                      SetLocked,
                      setLockedAuthenticated,
                      getAuthAction(message(), AUTH_USER_ADMIN),
                      bool)

CHECK_AUTH_WITH_2ARGS(User,
                      SetPassword,
                      setPasswordAuthenticated,
                      getAuthAction(message(), AUTH_CHANGE_OWN_PASSWORD),
                      const QString &,
                      const QString &)

CHECK_AUTH_WITH_2ARGS(User,
                      SetPasswordByPasswd,
                      setPasswordByPasswdAuthenticated,
                      getAuthAction(message(), AUTH_CHANGE_OWN_PASSWORD),
                      const QString &,
                      const QString &)

CHECK_AUTH_WITH_1ARGS(User,
                      SetPasswordExpirationPolicy,
                      setPasswordExpirationPolicyAuthenticated,
                      getAuthAction(message(), AUTH_CHANGE_OWN_USER_DATA),
                      const QString &)

CHECK_AUTH_WITH_1ARGS(User,
                      SetPasswordHint,
                      setPasswordHintAuthenticated,
                      getAuthAction(message(), AUTH_CHANGE_OWN_USER_DATA),
                      const QString &)

CHECK_AUTH_WITH_1ARGS(User,
                      SetPasswordMode,
                      setPasswordModeAuthenticated,
                      getAuthAction(message(), AUTH_CHANGE_OWN_USER_DATA),
                      int32_t)

CHECK_AUTH_WITH_1ARGS(User,
                      SetRealName,
                      setRealNameAuthenticated,
                      getAuthAction(message(), AUTH_CHANGE_OWN_USER_DATA),
                      const QString &)

CHECK_AUTH_WITH_1ARGS(User,
                      SetSession,
                      setSessionAuthenticated,
                      getAuthAction(message(), AUTH_CHANGE_OWN_USER_DATA),
                      const QString &)

CHECK_AUTH_WITH_1ARGS(User,
                      SetSessionType,
                      setSessionTypeAuthenticated,
                      getAuthAction(message(), AUTH_CHANGE_OWN_USER_DATA),
                      const QString &)

CHECK_AUTH_WITH_1ARGS(User,
                      SetShell,
                      setShellAuthenticated,
                      getAuthAction(message(), AUTH_USER_ADMIN),
                      const QString &)

CHECK_AUTH_WITH_1ARGS(User,
                      SetUserName,
                      setUserNameAuthenticated,
                      getAuthAction(message(), AUTH_USER_ADMIN),
                      const QString &)

CHECK_AUTH_WITH_1ARGS(User,
                      SetXSession,
                      setXSessionAuthenticated,
                      getAuthAction(message(), AUTH_CHANGE_OWN_USER_DATA),
                      const QString &)

void User::setAccountTypeAuthenticated(const QDBusMessage &message, int accountType)
{
    if (getAccountType() == accountType)
    {
        QDBusConnection::systemBus().send(message.createReply());
        return;
    }

    auto group = AccountsWrapper::getInstance()->getGroupByName(ADMIN_GROUP);
    if (!group)
    {
        DBUS_ERROR_DELAY_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_GROUP_NOT_FOUND);
    }
    auto adminGID = group->gid;

    auto groupsID = AccountsWrapper::getInstance()->getUserGroups(getUserName(), getGID());

    // 如果是管理员用户，把wheel组添加到最后，否则删除掉wheel group组
    groupsID.removeOne(adminGID);
    if (accountType == int32_t(AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_ADMINISTRATOR))
    {
        groupsID.push_back(adminGID);
    }

    // 拼接数字，用逗号分割
    QString groupsIDArg;
    for (int32_t i = 0; i < groupsID.size(); ++i)
    {
        groupsIDArg.append(QString("%1").arg(groupsID[i]));
        if (i + 1 < groupsID.size())
        {
            groupsIDArg.append(",");
        }
    }

    auto arguments = QStringList{"-G", groupsIDArg, "--", getUserName()};
    SPAWN_WITH_DBUS_MESSAGE(message, QString("/usr/sbin/usermod"), arguments);
    setAccountType(accountType);
    QDBusConnection::systemBus().send(message.createReply());
}

void User::setAutomaticLoginAuthenticated(const QDBusMessage &message, bool enabled)
{
    if (getLocked())
    {
        DBUS_ERROR_DELAY_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_IS_LOCKED);
    }
    if (enabled != getAutomaticLogin())
    {
        auto userName = enabled ? getUserName() : QString();
        AccountsManager::getInstance()->setAutomaticLoginUser(userName);
    }
    QDBusConnection::systemBus().send(message.createReply());
}

void User::setEmailAuthenticated(const QDBusMessage &message, const QString &email)
{
    setEmail(email);
    QDBusConnection::systemBus().send(message.createReply());
}

void User::setHomeDirectoryAuthenticated(const QDBusMessage &message, const QString &homeDirectory)
{
    // getHomeDirectory()或者homeDirectory可能以/结尾，也可能不以/结尾，因此这里统一去掉以/结尾后再进行比较
    if (QDir(getHomeDirectory()).absolutePath() != QDir(homeDirectory).absolutePath())
    {
        SPAWN_WITH_DBUS_MESSAGE(message,
                                QString("/usr/sbin/usermod"),
                                QStringList({"-m", "-d", homeDirectory, "--", getUserName()}));
        setHomeDirectory(homeDirectory);
        resetIconFile();
    }
    QDBusConnection::systemBus().send(message.createReply());
}

void User::setIconFileAuthenticated(const QDBusMessage &message, const QString &filepath)
{
    auto destFilePath = QString("%1/%2").arg(ICONDIR).arg(getUserName());
    // 实际访问的图标位置
    auto iconAccessFilePath = filepath;

    if (filepath.isEmpty() || QFile::exists(filepath))
    {
        QFile::remove(destFilePath);
    }

    /* 将图标文件拷贝一份到系统路径中(destFilePath)，如果原始路径(filepath)的文件对所有用户都有可读权限，
       则图标位置还是从原始路径获取，否则将图标位置改为系统路径。*/
    if (QFile::exists(filepath))
    {
        QFile::copy(filepath, destFilePath);

        QFileInfo fileInfo(filepath);
        if (!fileInfo.permission(QFile::ReadOther))
        {
            iconAccessFilePath = destFilePath;
        }
    }

    setIconFile(iconAccessFilePath);
    syncIconFileToFreedesktop(iconAccessFilePath);
    QDBusConnection::systemBus().send(message.createReply());
}

void User::setLanguageAuthenticated(const QDBusMessage &message, const QString &language)
{
    setLanguage(language);
    QDBusConnection::systemBus().send(message.createReply());
}

void User::setLockedAuthenticated(const QDBusMessage &message, bool locked)
{
    if (getLocked() != locked)
    {
        SPAWN_WITH_DBUS_MESSAGE(message,
                                QString("/usr/sbin/usermod"),
                                QStringList({locked ? "-L" : "-U", "--", getUserName()}));
        setLocked(locked);
        if (getAutomaticLogin() && locked)
        {
            AccountsManager::getInstance()->setAutomaticLoginUser(QString());
            setAutomaticLogin(false);
        }
    }
}
void User::setPasswordAuthenticated(const QDBusMessage &message, const QString &password, const QString &hint)
{
    SPAWN_WITH_DBUS_MESSAGE(message,
                            QString("/usr/sbin/usermod"),
                            QStringList({"-p", password, "--", getUserName()}));

    setPasswordMode(int32_t(AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_REGULAR));
    setLocked(false);
    setPasswordHint(hint);
    QDBusConnection::systemBus().send(message.createReply());
}

void User::setPasswordByPasswdAuthenticated(const QDBusMessage &message,
                                            const QString &encryptedCurrentPassword,
                                            const QString &encryptedNewPassword)
{
    auto rsaPrivateKey = AccountsManager::getInstance()->getRsaPrivateKey();
    auto currentPassword = CryptoHelper::rsaDecrypt(rsaPrivateKey, encryptedCurrentPassword);
    auto newPassword = CryptoHelper::rsaDecrypt(rsaPrivateKey, encryptedNewPassword);

    if (!encryptedCurrentPassword.isEmpty() && currentPassword.isEmpty())
    {
        KLOG_WARNING(accounts) << "Current password decrypt failed.";
        DBUS_ERROR_DELAY_REPLY_AND_RET(CCErrorCode::ERROR_ARGUMENT_INVALID);
    }

    if (!encryptedNewPassword.isEmpty() && newPassword.isEmpty())
    {
        KLOG_WARNING(accounts) << "new password decrypt failed.";
        DBUS_ERROR_DELAY_REPLY_AND_RET(CCErrorCode::ERROR_ARGUMENT_INVALID);
    }

    /* 如果修改的当前用户密码，SetPasswordByPasswd函数不会要求鉴权，所以需要判断当前密码是否合法
       为了安全性考虑，如果getCallerUID函数调用失败，则也需要判断当前密码是否合法。*/
    uint32_t callerUID = 0;
    if ((!AccountsUtil::getCallerUID(message, callerUID) || callerUID == getUID()) &&
        !checkPassword(currentPassword))
    {
        DBUS_ERROR_DELAY_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USRE_CURRENT_PASSWORD_DISMATCH);
    }

    if (m_passwdProcess &&
        m_passwdProcess->getState() != PasswdState::PASSWD_STATE_NONE)
    {
        DBUS_ERROR_DELAY_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_MODIFYING_PASSWORD);
    }
    else
    {
        m_passwdProcess = QSharedPointer<PasswdProcess>::create(sharedFromThis());
    }

    connect(m_passwdProcess.data(), &PasswdProcess::finished,
            this, std::bind(&User::processPasswdChanged, this, std::placeholders::_1, message));
    m_passwdProcess->setDBusCaller(callerUID);
    m_passwdProcess->changePassword(getUserName(), currentPassword, newPassword);
}

void User::setPasswordExpirationPolicyAuthenticated(const QDBusMessage &message, const QString &options)
{
    QString program("/usr/bin/chage");
    QStringList arguments;

    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(options.toUtf8(), &jsonError);

    if (jsonDoc.isNull())
    {
        KLOG_WARNING(accounts) << "Parser standard output failed: " << jsonError.errorString();
        DBUS_ERROR_DELAY_REPLY_AND_RET(CCErrorCode::ERROR_ARGUMENT_INVALID);
    }

    auto jsonRoot = jsonDoc.object();

    for (auto key : jsonRoot.keys())
    {
        auto value = jsonRoot[key].toInt();
        auto strValue = QString("%1").arg(value);

        switch (shash(key.toUtf8().data()))
        {
        case CONNECT(ACCOUNTS_PEP_EXPIRATION_TIME, _hash):
            arguments.push_back("-E");
            arguments.push_back(strValue);
            break;
        case CONNECT(ACCOUNTS_PEP_LAST_CHANGED_TIME, _hash):
            arguments.push_back("-d");
            arguments.push_back(strValue);
            break;
        case CONNECT(ACCOUNTS_PEP_MIN_DAYS, _hash):
            arguments.push_back("-m");
            arguments.push_back(strValue);
            break;
        case CONNECT(ACCOUNTS_PEP_MAX_DAYS, _hash):
            arguments.push_back("-M");
            arguments.push_back(strValue);
            break;
        case CONNECT(ACCOUNTS_PEP_DAYS_TO_WARN, _hash):
            arguments.push_back("-W");
            arguments.push_back(strValue);
            break;
        case CONNECT(ACCOUNTS_PEP_INACTIVE_DAYS, _hash):
            arguments.push_back("-I");
            arguments.push_back(strValue);
            break;
        default:
            KLOG_INFO(accounts) << "The option " << key << "is ignored.";
            break;
        }
    }

    // 没有设置任何参数，返回错误
    if (arguments.size() <= 1)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_PEP_EMPTY);
    }
    arguments.push_back(getUserName());
    SPAWN_WITH_DBUS_MESSAGE(message, program, arguments);
    QDBusConnection::systemBus().send(message.createReply());
}

void User::setPasswordHintAuthenticated(const QDBusMessage &message, const QString &hint)
{
    setPasswordHint(hint);
    QDBusConnection::systemBus().send(message.createReply());
}

void User::setPasswordModeAuthenticated(const QDBusMessage &message, int mode)
{
    if (getPasswordMode() == mode)
    {
        QDBusConnection::systemBus().send(message.createReply());
        return;
    }

    if (mode == int32_t(AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_SET_AT_LOGIN) ||
        mode == int32_t(AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_NONE))
    {
        SPAWN_WITH_DBUS_MESSAGE(message,
                                QString("/usr/bin/passwd"),
                                QStringList({"-d", "--", getUserName()}));

        if (mode == int32_t(AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_SET_AT_LOGIN))
        {
            SPAWN_WITH_DBUS_MESSAGE(message,
                                    QString("/usr/bin/chage"),
                                    QStringList({"-d", "0", "--", getUserName()}));
        }

        setPasswordHint(QString());
    }
    else if (getLocked())
    {
        SPAWN_WITH_DBUS_MESSAGE(message,
                                QString("/usr/sbin/usermod"),
                                QStringList({"-U", "--", getUserName()}));
    }
    setLocked(false);
    setPasswordMode(mode);
    QDBusConnection::systemBus().send(message.createReply());
}

void User::setRealNameAuthenticated(const QDBusMessage &message, const QString &name)
{
    if (getRealName() == name)
    {
        QDBusConnection::systemBus().send(message.createReply());
        return;
    }

    SPAWN_WITH_DBUS_MESSAGE(message,
                            QString("/usr/sbin/usermod"),
                            QStringList({"-c", name, "--", getUserName()}));

    setRealName(name);
    QDBusConnection::systemBus().send(message.createReply());
}

void User::setSessionAuthenticated(const QDBusMessage &message, const QString &session)
{
    setSession(session);
    QDBusConnection::systemBus().send(message.createReply());
}

void User::setSessionTypeAuthenticated(const QDBusMessage &message, const QString &sessionType)
{
    setSessionType(sessionType);
    QDBusConnection::systemBus().send(message.createReply());
}

void User::setShellAuthenticated(const QDBusMessage &message, const QString &shell)
{
    if (getShell() != shell)
    {
        SPAWN_WITH_DBUS_MESSAGE(message,
                                QString("/usr/sbin/usermod"),
                                QStringList({"-s", shell, "--", getUserName()}));
        setShell(shell);
    }

    QDBusConnection::systemBus().send(message.createReply());
}

void User::setUserNameAuthenticated(const QDBusMessage &message, const QString &name)
{
    if (getUserName() == name)
    {
        QDBusConnection::systemBus().send(message.createReply());
        return;
    }

    auto oldName = getUserName();
    SPAWN_WITH_DBUS_MESSAGE(message,
                            QString("/usr/sbin/usermod"),
                            QStringList({"-l", name, "--", getUserName()}));

    SetUserName(name);

    auto oldFilePath = QString("%1/%2").arg(USERDIR).arg(oldName);
    auto newFilePath = QString("%1/%2").arg(USERDIR).arg(name);
    QDBusConnection::systemBus().send(message.createReply());
}

void User::setXSessionAuthenticated(const QDBusMessage &message, const QString &xsession)
{
    setXSession(xsession);
    QDBusConnection::systemBus().send(message.createReply());
}

QString User::getAuthAction(const QDBusMessage &message, const QString &ownAction)
{
    RETURN_VAL_IF_TRUE(ownAction == AUTH_USER_ADMIN, AUTH_USER_ADMIN);

    uint32_t uid = 0;
    if (AccountsUtil::getCallerUID(message, uid) && getUID() == (uid_t)uid)
    {
        return ownAction;
    }
    else
    {
        return AUTH_USER_ADMIN;
    }
}

void User::processPasswdChanged(const QString &errorDesc, const QDBusMessage &message)
{
    QDBusMessage reply;
    if (!errorDesc.isEmpty())
    {
        reply = message.createErrorReply(QDBusError::Failed, errorDesc);
    }
    else
    {
        setPasswordMode(int32_t(AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_REGULAR));
        setLocked(false);
        reply = message.createReply();
    }
    QDBusConnection::systemBus().send(reply);
}

QSharedPointer<User> User::createUser(PasswdShadow passwdShadow)
{
    auto user = QSharedPointer<User>::create(passwdShadow);
    user->init();
    return user;
}

void User::dbusRegister()
{
    auto systemConnection = QDBusConnection::systemBus();
    if (!systemConnection.registerObject(m_objectPath, KIRAN_ACCOUNTS_USER_INTERFACE, this))
    {
        KLOG_ERROR(accounts) << "Can't register object:" << systemConnection.lastError();
        return;
    }
}

void User::dbusUnregister()
{
    auto systemConnection = QDBusConnection::systemBus();
    systemConnection.unregisterObject(m_objectPath);
}

QDBusObjectPath User::getObjectPath()
{
    return QDBusObjectPath(m_objectPath);
}

void User::updateFromPasswdShadow(PasswdShadow passwd_shadow)
{
    udpateNocacheVar(passwd_shadow);
    resetIconFile();
}

void User::removeCacheFile()
{
    auto userFilePath = QString("%1/%2").arg(USERDIR).arg(getUserName());
    QFile::remove(userFilePath);

    auto iconFilePath = QString("%1/%2").arg(ICONDIR).arg(getUserName());
    QFile::remove(iconFilePath);
}

#define GET_SETTINGS_PROPERTY(property)                                          \
    QString User::get##property()                                                \
    {                                                                            \
        auto key = QString("%1/%2").arg(KEYFILE_USER_GROUP_NAME).arg(#property); \
        return m_settings->value(key).toString();                                \
    }

GET_SETTINGS_PROPERTY(Email)
GET_SETTINGS_PROPERTY(Language)
GET_SETTINGS_PROPERTY(PasswordHint)
GET_SETTINGS_PROPERTY(Session)
GET_SETTINGS_PROPERTY(SessionType)
GET_SETTINGS_PROPERTY(XSession)

QString User::getIconFile()
{
    auto key = QString("%1/%2").arg(KEYFILE_USER_GROUP_NAME).arg(KEYFILE_USER_GROUP_KEY_ICON);
    auto iconFilePath = m_settings->value(key).toString();

    if (!QFileInfo(iconFilePath).exists())
    {
        // 使用默认目录的图标
        iconFilePath = QString("%1/%2").arg(ICONDIR).arg(getUserName());
        if (!QFileInfo(iconFilePath).exists())
        {
            // 使用用户主目录的图标
            iconFilePath = QString("%1/.face").arg(getHomeDirectory());
        }
    }
    return iconFilePath;
}

#define SEND_PROPERTY_NOTIFY(property, propertyHump)                                \
    QVariantMap changedProperties;                                                  \
    changedProperties.insert(QStringLiteral(#property), this->get##propertyHump()); \
                                                                                    \
    QDBusMessage message = QDBusMessage::createSignal(                              \
        this->m_objectPath,                                                         \
        QStringLiteral("org.freedesktop.DBus.Properties"),                          \
        QStringLiteral("PropertiesChanged"));                                       \
                                                                                    \
    message.setArguments({                                                          \
        QStringLiteral(KIRAN_ACCOUNTS_USER_INTERFACE),                              \
        changedProperties,                                                          \
        QStringList(),                                                              \
    });                                                                             \
                                                                                    \
    QDBusConnection::systemBus().send(message);

#define SET_MEMBER_PROPERTY(propertyType, property, propertyHump, member) \
    void User::set##propertyHump(propertyType propertyValue)              \
    {                                                                     \
        RETURN_IF_TRUE(propertyValue == this->get##propertyHump());       \
        this->member = propertyValue;                                     \
        SEND_PROPERTY_NOTIFY(property, propertyHump)                      \
    }

#define SET_SETTINGS_PROPERTY(propertyType, property, propertyHump)                  \
    void User::set##propertyHump(propertyType propertyValue)                         \
    {                                                                                \
        RETURN_IF_TRUE(propertyValue == this->get##propertyHump());                  \
        auto key = QString("%1/%2").arg(KEYFILE_USER_GROUP_NAME).arg(#propertyHump); \
        this->m_settings->setValue(key, propertyValue);                              \
        SEND_PROPERTY_NOTIFY(property, propertyHump)                                 \
    }

SET_MEMBER_PROPERTY(int, account_type, AccountType, m_accountType)
SET_MEMBER_PROPERTY(bool, automatic_login, AutomaticLogin, m_automaticLogin)
SET_SETTINGS_PROPERTY(const QString &, email, Email)
SET_MEMBER_PROPERTY(qulonglong, gid, GID, m_gid)
SET_MEMBER_PROPERTY(const QString &, home_directory, HomeDirectory, m_homeDirectory)
SET_SETTINGS_PROPERTY(const QString &, language, Language)
SET_MEMBER_PROPERTY(bool, locked, Locked, m_locked)
SET_MEMBER_PROPERTY(const QString &, password_expiration_policy, PasswordExpirationPolicy, m_passwordExpirationPolicy)
SET_SETTINGS_PROPERTY(const QString &, password_hint, PasswordHint)
SET_MEMBER_PROPERTY(int, password_mode, PasswordMode, m_passwordMode)
SET_MEMBER_PROPERTY(const QString &, real_name, RealName, m_realName)
SET_SETTINGS_PROPERTY(const QString &, session, Session)
SET_SETTINGS_PROPERTY(const QString &, session_type, SessionType)
SET_MEMBER_PROPERTY(const QString &, shell, Shell, m_shell)
SET_MEMBER_PROPERTY(bool, system_account, SystemAccount, m_systemAccount)
SET_MEMBER_PROPERTY(qulonglong, uid, UID, m_uid)
SET_MEMBER_PROPERTY(const QString &, user_name, UserName, m_userName)
SET_SETTINGS_PROPERTY(const QString &, x_session, XSession)

void User::setIconFile(const QString &iconFile)
{
    RETURN_IF_TRUE(iconFile == getIconFile());
    auto key = QString("%1/%2").arg(KEYFILE_USER_GROUP_NAME).arg(KEYFILE_USER_GROUP_KEY_ICON);
    m_settings->setValue(key, iconFile);
    SEND_PROPERTY_NOTIFY(icon_file, IconFile)
}

void User::init()
{
    buildFreedesktopUserObjectPath();
    udpateNocacheVar(m_passwdShadow);
    initSettings();
    // 由于图标路径是维护在缓存中，所以必须等UserCache对象创建后才能操作
    resetIconFile();
}

void User::buildFreedesktopUserObjectPath()
{
    auto sendMessage = QDBusMessage::createMethodCall(FREEDESKTOP_ACCOUNTS_DBUS_NAME,
                                                      FREEDESKTOP_ACCOUNTS_OBJECT_PATH,
                                                      FREEDESKTOP_ACCOUNTS_DBUS_INTERFACE,
                                                      "FindUserById");

    sendMessage << qlonglong(getUID());

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(accounts) << "Call FindUserById failed:" << replyMessage.errorMessage();
        return;
    }

    m_freedesktopObjectPath = replyMessage.arguments().takeFirst().value<QDBusObjectPath>().path();
}

void User::udpateNocacheVar(PasswdShadow passwd_shadow)
{
    QString realName;

    m_passwd = passwd_shadow.first;
    m_spwd = passwd_shadow.second;

    setRealName(m_passwd->gecos);
    setUID(m_passwd->uid);
    setGID(m_passwd->gid);

    auto accountType = accountTtypeFromPwent(m_passwd);
    setAccountType(int(accountType));

    setUserName(m_passwd->name);
    setHomeDirectory(m_passwd->dir);
    setShell(m_passwd->shell);

    QString hashedPassphrase = getHashedPassphrase();
    bool locked = (hashedPassphrase.length() > 0 && hashedPassphrase.at(0) == '!');
    setLocked(locked);

    AccountsPasswordMode mode;

    if (!hashedPassphrase.isEmpty())
    {
        mode = AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_REGULAR;
    }
    else
    {
        mode = AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_NONE;
    }

    if (m_spwd && m_spwd->lstchg == 0)
    {
        mode = AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_SET_AT_LOGIN;
    }

    setPasswordMode(int(mode));

    auto isSystemAccount = !UserClassify::isHuman(m_passwd->uid,
                                                  m_passwd->name,
                                                  m_passwd->shell);
    setSystemAccount(isSystemAccount);
    updatePasswordExpirationPolicy(m_spwd);
}

void User::initSettings()
{
    // 非root的系统用户不缓存数据
    if (getSystemAccount() && getUID() != 0)
    {
        m_settings = new QSettings(this);
    }
    else
    {
        auto fileName = QString("%1/%2").arg(USERDIR, getUserName());
        m_settings = new QSettings(fileName, QSettings::IniFormat, this);
    }
}

void User::updatePasswordExpirationPolicy(QSharedPointer<SPwd> spwd)
{
    auto jsonObj = QJsonObject{
        {ACCOUNTS_PEP_EXPIRATION_TIME, int(spwd->expire)},
        {ACCOUNTS_PEP_LAST_CHANGED_TIME, int(spwd->lstchg)},
        {ACCOUNTS_PEP_MIN_DAYS, int(spwd->min)},
        {ACCOUNTS_PEP_MAX_DAYS, int(spwd->max)},
        {ACCOUNTS_PEP_DAYS_TO_WARN, int(spwd->warn)},
        {ACCOUNTS_PEP_INACTIVE_DAYS, int(spwd->inact)},
    };

    setPasswordExpirationPolicy(QJsonDocument(jsonObj).toJson(QJsonDocument::Compact));
}

AccountsAccountType User::accountTtypeFromPwent(QSharedPointer<Passwd> passwd)
{
    RETURN_VAL_IF_FALSE(passwd, AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD);

    if (passwd->uid == 0)
    {
        KLOG_DEBUG(accounts) << "User is root so account type is administrator";
        return AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_ADMINISTRATOR;
    }

    auto grp = getgrnam(ADMIN_GROUP);
    if (grp == NULL)
    {
        KLOG_INFO(accounts) << ADMIN_GROUP << "group not found";
        return AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD;
    }

    for (int i = 0; grp->gr_mem[i] != NULL; i++)
    {
        if (QString(grp->gr_mem[i]) == passwd->name)
        {
            return AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_ADMINISTRATOR;
        }
    }

    return AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD;
}

void User::resetIconFile()
{
    auto iconFile = getIconFile();
    auto homeDir = getHomeDirectory();

    // 如果用户主目录发生变化，且用户使用的是之前的用户主目录的图标，则重新更新路径。
    if (!iconFile.isEmpty() && iconFile == defaultIconFile)
    {
        defaultIconFile = QString("%1/%2").arg(homeDir).arg(".face");
        if (iconFile != defaultIconFile)
        {
            setIconFile(defaultIconFile);
            syncIconFileToFreedesktop(defaultIconFile);
        }
    }
}

void User::syncIconFileToFreedesktop(const QString &iconFile)
{
    RETURN_IF_TRUE(m_freedesktopObjectPath.isEmpty());

    auto sendMessage = QDBusMessage::createMethodCall(FREEDESKTOP_ACCOUNTS_DBUS_NAME,
                                                      m_freedesktopObjectPath,
                                                      FREEDESKTOP_ACCOUNTS_USER_DBUS_INTERFACE,
                                                      "SetIconFile");

    sendMessage << iconFile;

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(accounts) << "Call SetIconFile failed:" << replyMessage.errorMessage();
        return;
    }
}

QString User::getHashedPassphrase()
{
    if (m_spwd)
    {
        return m_spwd->pwdp;
    }
    else
    {
        // 兼容旧的方式，以前的版本密码是放在/etc/passwd中
        return m_passwd->passwd;
    }
}

bool User::checkPassword(const QString &password)
{
    auto hashedPassphrase = getHashedPassphrase();
    auto encryptedHashedPassphrase = crypt(password.toUtf8().data(),
                                           hashedPassphrase.toUtf8().data());
    return hashedPassphrase == encryptedHashedPassphrase;
}

}  // namespace Kiran