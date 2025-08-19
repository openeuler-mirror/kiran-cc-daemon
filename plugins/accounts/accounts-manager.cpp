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

#include "accounts-manager.h"
#include <KConfig>
#include <KConfigGroup>
#include <QDBusConnection>
#include <QFileSystemWatcher>
#include <QSettings>
#include <cinttypes>
#include <cstdint>
#include "accounts-i.h"
#include "accounts-util.h"
#include "accounts-wrapper.h"
#include "accountsadaptor.h"
#include "config-accounts.h"
#include "lib/base/base.h"
#include "lib/base/crypto-helper.h"
#include "lib/base/polkit-proxy.h"

namespace Kiran
{
// 最少需要512长度
#define RSA_KEY_LENGTH 512

#define LIGHTDM_PROFILE_PATH "/usr/share/lightdm/lightdm.conf.d/99-kiran-greeter-login.conf"
#define LIGHTDM_GROUP_NAME "Seat:seat0"
#define LIGHTDM_KEY_AUTOLOGIN_USER "autologin-user"

#define LOGIN1_DBUS_NAME "org.freedesktop.login1"
#define LOGIN1_DBUS_OBJECT_PATH "/org/freedesktop/login1"
#define LOGIN1_MANAGER_DBUS_INTERFACE "org.freedesktop.login1.Manager"

AccountsManager::AccountsManager(AccountsWrapper *accountsWrapper) : m_adaptor(nullptr),
                                                                     m_accountsWrapper(accountsWrapper)
{
    m_lightdmFileWatcher = new QFileSystemWatcher(QStringList{LIGHTDM_PROFILE_PATH}, this);
    auto systemConnection = QDBusConnection::systemBus();
    if (!systemConnection.registerService(ACCOUNTS_DBUS_NAME))
    {
        KLOG_WARNING(accounts) << "Failed to register dbus name: " << ACCOUNTS_DBUS_NAME;
        return;
    }

    if (!systemConnection.registerObject(ACCOUNTS_OBJECT_PATH, ACCOUNTS_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR(accounts) << "Can't register object:" << systemConnection.lastError();
        return;
    }

    m_adaptor = new AccountsAdaptor(this);
}

AccountsManager::~AccountsManager()
{
}

AccountsManager *AccountsManager::m_instance = nullptr;
void AccountsManager::globalInit(AccountsWrapper *passwdWrapper)
{
    m_instance = new AccountsManager(passwdWrapper);
    m_instance->init();
}

void AccountsManager::setAutomaticLoginUser(const QString &userName)
{
    RETURN_IF_TRUE(m_automaticLoginUser == userName);

    if (!m_automaticLoginUser.isEmpty())
    {
        auto user = findAndCreateUserByName(m_automaticLoginUser);
        if (user)
        {
            user->setAutomaticLogin(false);
        }
    }

    if (!userName.isEmpty())
    {
        auto user = findAndCreateUserByName(userName);
        if (user)
        {
            user->setAutomaticLogin(true);
        }
    }

    m_automaticLoginUser = userName;
    if (getAutomaticLoginUserFromSettings() != userName)
    {
        setAutomaticLoginUserToSettings(userName);
    }
}

CHECK_AUTH_WITH_4ARGS_AND_RETVAL(AccountsManager,
                                 QDBusObjectPath,
                                 CreateUser,
                                 createUserAuthenticated,
                                 AUTH_USER_ADMIN,
                                 const QString &,
                                 const QString &,
                                 int,
                                 qlonglong);

void AccountsManager::DeleteUser(qulonglong uid, bool removeFiles)
{
    // 如果是三权用户，则禁止删除
    if (isSecurityPolicyUser(uid))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_DELETE_THREE_AUTH_USER);
    }

    // 如果用户已经登录, 则禁止删除
    auto sendMessage = QDBusMessage::createMethodCall(LOGIN1_DBUS_NAME,
                                                      LOGIN1_DBUS_OBJECT_PATH,
                                                      LOGIN1_MANAGER_DBUS_INTERFACE,
                                                      "GetUser");

    sendMessage << uint(uid);

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_INFO(accounts) << "Call GetUser return error:" << replyMessage.errorMessage();
    }
    else
    {
        auto userObjectPath = replyMessage.arguments().takeFirst().value<QDBusObjectPath>();
        if (!userObjectPath.path().isEmpty())
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_ALREADY_LOGIN);
        }
    }

    setDelayedReply(true);
    PolkitProxy::getDefault()->checkAuthorization(AUTH_USER_ADMIN,
                                                  true,
                                                  this->message(),
                                                  QStringLiteral("AccountsManager::DeleteUser"),
                                                  std::bind(&AccountsManager::deleteUserAuthenticated, this, std::placeholders::_1, uid, removeFiles));

    return;
}

QDBusObjectPath AccountsManager::FindUserById(qulonglong uid)
{
    auto user = findAndCreateUserByID(uid);

    if (user)
    {
        return user->getObjectPath();
    }
    else
    {
        DBUS_ERROR_REPLY(CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_1);
    }
    return QDBusObjectPath();
}

QDBusObjectPath AccountsManager::FindUserByName(const QString &name)
{
    auto user = findAndCreateUserByName(name);

    if (user)
    {
        return user->getObjectPath();
    }
    else
    {
        DBUS_ERROR_REPLY(CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_2);
    }
    return QDBusObjectPath();
}

QList<QDBusObjectPath> AccountsManager::GetNonSystemUsers()
{
    QList<QDBusObjectPath> nonSystemUsers;
    for (auto iter = m_users.begin(); iter != m_users.end(); ++iter)
    {
        if (!iter.value()->getSystemAccount())
        {
            nonSystemUsers.push_back(iter.value()->getObjectPath());
        }
    }
    return nonSystemUsers;
}

void AccountsManager::createUserAuthenticated(const QDBusMessage &message,
                                              const QString &name,
                                              const QString &realname,
                                              int accountType,
                                              qlonglong uid)
{
    auto pwent = m_accountsWrapper->getPasswdByName(name);

    if (pwent)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_ALREADY_EXIST);
    }

    KLOG_INFO(accounts) << "Create user" << name;

    QString program = QString("/usr/sbin/useradd");
    QStringList arguments = {"-m", "-c", realname};
    switch (accountType)
    {
    case int32_t(AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_ADMINISTRATOR):
        arguments.append(QStringList{"-G", ADMIN_GROUP});
        break;
    case int32_t(AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD):
        break;
    default:
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_UNKNOWN_ACCOUNT_TYPE);
    }
    break;
    }

    if (uid > 0)
    {
        arguments.append(QStringList{"-u", QString("%1").arg(uid)});
    }

    arguments.append(QStringList{"--", name});
    SPAWN_WITH_DBUS_MESSAGE(message, program, arguments);

    auto user = findAndCreateUserByName(name);
    if (user)
    {
        user->setSystemAccount(false);
        auto replyMessage = message.createReply(user->getObjectPath());
        QDBusConnection::systemBus().send(replyMessage);
    }
    else
    {
        DBUS_ERROR_REPLY(CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_3);
    }

    return;
}

void AccountsManager::deleteUserAuthenticated(const QDBusMessage &message,
                                              qulonglong uid,
                                              bool removeFiles)
{
    if (uid == 0)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_DELETE_ROOT_USER);
    }

    auto user = findAndCreateUserByID(uid);

    if (!user)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_4);
    }

    KLOG_INFO(accounts) << "Delete user" << user->getUserName();

    // 忽略取消自动登录出错情况
    if (m_automaticLoginUser == user->getUserName())
    {
        setAutomaticLoginUser(QString());
    }
    user->removeCacheFile();

    QString program("/usr/sbin/userdel");
    QStringList arguments;

    if (removeFiles)
    {
        arguments = QStringList({"-f", "-r", "--", user->getUserName()});
    }
    else
    {
        arguments = QStringList({"-f", "--", user->getUserName()});
    }

    SPAWN_WITH_DBUS_MESSAGE(message, program, arguments);
    QDBusConnection::systemBus().send(message.createReply());
}

void AccountsManager::init()
{
    CryptoHelper::generateRSAKey(RSA_KEY_LENGTH, m_rsaPrivateKey, m_rsaPublicKey);
    reloadUsers();
    updateAutomaticLogin();

    connect(m_accountsWrapper, SIGNAL(userChanged()), this, SLOT(reloadUsers()));
    connect(m_lightdmFileWatcher, &QFileSystemWatcher::fileChanged, this, [this](const QString &path)
            { this->updateAutomaticLogin(); });
}

void AccountsManager::updateAutomaticLogin()
{
    auto userName = getAutomaticLoginUserFromSettings();
    setAutomaticLoginUser(userName);

    KLOG_INFO(accounts) << "The automatic login user is" << userName;
}

void AccountsManager::setAutomaticLoginUserToSettings(const QString &userName)
{
    KConfig lightdmSettings(LIGHTDM_PROFILE_PATH, KConfig::SimpleConfig);
    auto seatGroup = lightdmSettings.group(LIGHTDM_GROUP_NAME);
    seatGroup.writeEntry(LIGHTDM_KEY_AUTOLOGIN_USER, userName);
}

QString AccountsManager::getAutomaticLoginUserFromSettings()
{
    KConfig lightdmSettings(LIGHTDM_PROFILE_PATH, KConfig::SimpleConfig);
    auto seatGroup = lightdmSettings.group(LIGHTDM_GROUP_NAME);
    return seatGroup.readEntry(LIGHTDM_KEY_AUTOLOGIN_USER, QString());
}

QMap<QString, QSharedPointer<User>> AccountsManager::loadUsers()
{
    auto passwdsShadows = m_accountsWrapper->getPasswdsShadows();
    QMap<QString, QSharedPointer<User>> users;
    QStringList skipUsersName;

    for (auto iter = passwdsShadows.begin(); iter != passwdsShadows.end(); ++iter)
    {
        QSharedPointer<User> user;
        auto pwent = iter->first;

        // 除了root用户和通过DBUS显示请求的系统用户以外，其他系统用户默认不加载。
        if (!isExplicitlyRequestedUser(pwent->name) &&
            !UserClassify::isHuman(pwent->uid, pwent->name, pwent->shell) &&
            pwent->uid != 0)
        {
            skipUsersName.push_back(pwent->name);
            continue;
        }

        auto oldIter = m_users.find(pwent->name);

        if (oldIter == m_users.end())
        {
            user = User::createUser(*iter);
        }
        else
        {
            user = oldIter.value();
            user->updateFromPasswdShadow(*iter);
        }

        users.insert(user->getUserName(), user);
    }

    KLOG_INFO(accounts) << "Load user list which ignored users contain" << skipUsersName;
    return users;
}

QSharedPointer<User> AccountsManager::addNewUserForPwent(QSharedPointer<Passwd> pwent, QSharedPointer<SPwd> spent)
{
    auto user = User::createUser(QPair<QSharedPointer<Passwd>, QSharedPointer<SPwd>>(pwent, spent));
    user->dbusRegister();

    if (m_users.contains(user->getUserName()))
    {
        KLOG_WARNING(accounts) << "User" << user->getUserName() << "is already exist.";
        return m_users.value(user->getUserName());
    }
    else
    {
        m_users.insert(user->getUserName(), user);
        KLOG_INFO(accounts) << "Add new user" << user->getUserName() << "to users cache.";
        Q_EMIT UserAdded(user->getObjectPath());
        return user;
    }
}

QSharedPointer<User> AccountsManager::findAndCreateUserByID(uint64_t uid)
{
    auto pwent = m_accountsWrapper->getPasswdByUID(uid);
    if (!pwent)
    {
        KLOG_WARNING(accounts) << "Unable to lookup user name" << uid;
        return nullptr;
    }

    auto user = m_users.value(QString(pwent->name));
    if (!user)
    {
        KLOG_INFO(accounts) << "Unable to lookup object for user" << uid << ", prepare to add it.";
        auto spent = m_accountsWrapper->getSpwdByName(pwent->name);
        user = addNewUserForPwent(pwent, spent);
        m_explicitlyRequestedUsers.insert(pwent->name);
    }

    return user;
}

QSharedPointer<User> AccountsManager::findAndCreateUserByName(const QString &userName)
{
    auto pwent = m_accountsWrapper->getPasswdByName(userName);
    if (!pwent)
    {
        KLOG_WARNING(accounts) << "Unable to lookup user name" << userName;
        return nullptr;
    }

    auto user = m_users.value(userName);
    if (!user)
    {
        KLOG_INFO(accounts) << "Unable to lookup object for user" << userName << ", prepare to add it.";
        auto spent = m_accountsWrapper->getSpwdByName(pwent->name);
        user = addNewUserForPwent(pwent, spent);
        m_explicitlyRequestedUsers.insert(pwent->name);
    }
    return user;
}

bool AccountsManager::isExplicitlyRequestedUser(const QString &userName)
{
    return (m_explicitlyRequestedUsers.find(userName) != m_explicitlyRequestedUsers.end());
}

bool AccountsManager::isSecurityPolicyUser(uint64_t uid)
{
    auto user = findAndCreateUserByID(uid);
    if (user)
    {
        if (user->getUserName() == "audadm" ||
            user->getUserName() == "sysadm" ||
            user->getUserName() == "secadm")
        {
            return true;
        }
    }

    return false;
}

void AccountsManager::reloadUsers()
{
    auto newUsers = loadUsers();
    QStringList addedUsersName;
    QStringList deletedUsersName;

    // 移除被删除的用户
    for (auto iter = m_users.begin(); iter != m_users.end(); ++iter)
    {
        auto iter2 = newUsers.find(iter.key());
        if (iter2 == newUsers.end())
        {
            Q_EMIT UserDeleted(iter.value()->getObjectPath());
            deletedUsersName.push_back(iter.value()->getUserName());
            iter.value()->dbusUnregister();
            iter.value()->removeCacheFile();
        }
    }

    // 添加新增的用户
    for (auto iter = newUsers.begin(); iter != newUsers.end(); ++iter)
    {
        auto iter2 = m_users.find(iter.key());
        if (iter2 == m_users.end())
        {
            iter.value()->dbusRegister();
            addedUsersName.push_back(iter.value()->getUserName());
            Q_EMIT UserAdded(iter.value()->getObjectPath());
        }
    }

    KLOG_INFO(accounts) << "Update users cache, add users" << addedUsersName << ", deleted users:" << deletedUsersName;

    m_users = newUsers;
}

}  // namespace Kiran
