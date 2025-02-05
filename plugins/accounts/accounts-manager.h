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

#pragma once

#include <QDBusContext>
#include <QDBusObjectPath>
#include <QSharedPointer>
#include "user-classify.h"
#include "user.h"

class AccountsAdaptor;
class KConfig;

namespace Kiran
{
class AccountsWrapper;

class AccountsManager : public QObject,
                        protected QDBusContext
{
    Q_OBJECT

public:
    AccountsManager() = delete;
    AccountsManager(AccountsWrapper *accountsWrapper);
    virtual ~AccountsManager();

    static AccountsManager *getInstance() { return m_instance; };
    static void globalInit(AccountsWrapper *passwdWrapper);
    static void globalDeinit() { delete m_instance; };

    void setAutomaticLoginUser(const QString &userName);
    QString getRsaPrivateKey() { return m_rsaPrivateKey; };

public:
    // PROPERTIES
    Q_PROPERTY(QString rsa_public_key READ getRsaPublicKey)
    QString getRsaPublicKey() const { return m_rsaPublicKey; };

public Q_SLOTS:
    // METHODS
    // 创建一个用户，用户可以为普通用户和管理员用户，管理员用户的定义可以参考policykit的addAdminRule规则。
    QDBusObjectPath CreateUser(const QString &name, const QString &realname, int accountType, qlonglong uid);
    // 删除一个用户
    void DeleteUser(qulonglong uid, bool remove_files);
    // 通过uid获取用户的DBusObjectPath，如果DBusObjectPath不存在则会创建一个新的。
    QDBusObjectPath FindUserById(qulonglong uid);
    // 通过name获取用户的DBusObjectPath
    QDBusObjectPath FindUserByName(const QString &name);
    // 获取非系统用户的DBusObjectPath，非系统用户一般为用户自己创建的账号。例如root为系统账户
    QList<QDBusObjectPath> GetNonSystemUsers();
Q_SIGNALS:  // SIGNALS
    void UserAdded(const QDBusObjectPath &user);
    void UserDeleted(const QDBusObjectPath &user);

private:
    void createUserAuthenticated(const QDBusMessage &message,
                                 const QString &name,
                                 const QString &realname,
                                 int account_type,
                                 qlonglong uid);
    void deleteUserAuthenticated(const QDBusMessage &message,
                                 qulonglong uid,
                                 bool removeFiles);

private:
    void init();
    void updateAutomaticLogin();
    void setAutomaticLoginUserToSettings(const QString &userName);
    QString getAutomaticLoginUserFromSettings();
    QMap<QString, QSharedPointer<User>> loadUsers();
    QSharedPointer<User> addNewUserForPwent(QSharedPointer<Passwd> pwent, QSharedPointer<SPwd> spent);
    QSharedPointer<User> findAndCreateUserByID(uint64_t uid);
    QSharedPointer<User> findAndCreateUserByName(const QString &userName);
    bool isExplicitlyRequestedUser(const QString &userName);
    // 是否为三权用户
    bool isSecurityPolicyUser(uint64_t uid);

private Q_SLOTS:
    // 根据系统文件重新读取用户列表，并更新缓存变量m_users
    void reloadUsers();

private:
    static AccountsManager *m_instance;
    AccountsAdaptor *m_adaptor;
    AccountsWrapper *m_accountsWrapper;
    QString m_rsaPrivateKey;
    QString m_rsaPublicKey;
    QFileSystemWatcher *m_lightdmFileWatcher;
    KConfig *m_lightdmSettings;
    QMap<QString, QSharedPointer<User>> m_users;
    QSet<QString> m_explicitlyRequestedUsers;
    QString m_automaticLoginUser;
};
}  // namespace Kiran