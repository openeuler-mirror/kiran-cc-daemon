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

#pragma once

#include <QDBusContext>
#include <QDBusObjectPath>
#include <QEnableSharedFromThis>
#include <QSharedPointer>
#include "accounts-i.h"
#include "accounts-wrapper.h"
#include "passwd-process.h"

class UserAdaptor;
class QSettings;

namespace Kiran
{
class User : public QObject,
             public QEnableSharedFromThis<User>,
             protected QDBusContext
{
    Q_OBJECT

public:
    User() = delete;
    User(PasswdShadow passwdShadow);
    virtual ~User();

    static QSharedPointer<User> createUser(PasswdShadow passwdShadow);

    void dbusRegister();
    void dbusUnregister();
    QDBusObjectPath getObjectPath();

    void updateFromPasswdShadow(PasswdShadow passwd_shadow);
    // 移除缓存文件
    void removeCacheFile();
    // 检查path是否具备家目录权限
    bool checkDirPermissionAsHome(const QString &path);

public:  // PROPERTIES
    Q_PROPERTY(int account_type READ getAccountType WRITE setAccountType)
    Q_PROPERTY(bool automatic_login READ getAutomaticLogin WRITE setAutomaticLogin)
    Q_PROPERTY(QString email READ getEmail WRITE setEmail)
    Q_PROPERTY(qulonglong gid READ getGID WRITE setGID)
    Q_PROPERTY(QString home_directory READ getHomeDirectory WRITE setHomeDirectory)
    Q_PROPERTY(QString icon_file READ getIconFile WRITE setIconFile)
    Q_PROPERTY(QString language READ getLanguage WRITE setLanguage)
    Q_PROPERTY(bool locked READ getLocked WRITE setLocked)
    Q_PROPERTY(QString password_expiration_policy READ getPasswordExpirationPolicy WRITE setPasswordExpirationPolicy)
    Q_PROPERTY(QString password_hint READ getPasswordHint WRITE setPasswordHint)
    Q_PROPERTY(int password_mode READ getPasswordMode WRITE setPasswordMode)
    Q_PROPERTY(QString real_name READ getRealName WRITE setRealName)
    Q_PROPERTY(QString session READ getSession WRITE setSession)
    Q_PROPERTY(QString session_type READ getSessionType WRITE setSessionType)
    Q_PROPERTY(QString shell READ getShell WRITE setShell)
    Q_PROPERTY(bool system_account READ getSystemAccount WRITE setSystemAccount)
    Q_PROPERTY(qulonglong uid READ getUID WRITE setUID)
    Q_PROPERTY(QString user_name READ getUserName WRITE setUserName)
    Q_PROPERTY(QString x_session READ getXSession WRITE setXSession)

public:
    int getAccountType() { return this->m_accountType; };
    bool getAutomaticLogin() { return this->m_automaticLogin; };
    QString getEmail();
    qulonglong getGID() { return this->m_gid; };
    QString getHomeDirectory() { return this->m_homeDirectory; };
    QString getIconFile();
    QString getLanguage();
    bool getLocked() { return this->m_locked; };
    QString getPasswordExpirationPolicy() { return this->m_passwordExpirationPolicy; }
    QString getPasswordHint();
    int getPasswordMode() { return this->m_passwordMode; };
    QString getRealName() { return this->m_realName; };
    QString getSession();
    QString getSessionType();
    QString getShell() { return this->m_shell; };
    bool getSystemAccount() { return this->m_systemAccount; };
    qulonglong getUID() { return this->m_uid; };
    QString getUserName() { return this->m_userName; };
    QString getXSession();

    void setAccountType(int accountType);
    void setAutomaticLogin(bool automaticLogin);
    void setEmail(const QString &email);
    void setGID(qulonglong gid);
    void setHomeDirectory(const QString &homeDirectory);
    void setIconFile(const QString &icon);
    void setLanguage(const QString &language);
    void setLocked(bool locked);
    void setPasswordExpirationPolicy(const QString &passwordExpirationPolicy);
    void setPasswordHint(const QString &passwordHint);
    void setPasswordMode(int passwordMode);
    void setRealName(const QString &realName);
    void setSession(const QString &session);
    void setSessionType(const QString &sessionType);
    void setShell(const QString &shell);
    void setSystemAccount(bool systemAccount);
    void setUID(qulonglong uid);
    void setUserName(const QString &userName);
    void setXSession(const QString &xsession);

public Q_SLOTS:  // METHODS：大写字母开头的函数均为DBus接口
    // 设置用户类型，分为普通用户和管理员用户，管理员用户的定义可以参考policykit的addAdminRule规则。
    void SetAccountType(int account_type);
    // 设置用户是否自动登录
    void SetAutomaticLogin(bool enabled);
    // 设置用户邮箱
    void SetEmail(const QString &email);
    // 设置用户主目录
    void SetHomeDirectory(const QString &homedir);
    // 设置用户头像文件路径
    void SetIconFile(const QString &filepath);
    // 设置用户使用语言
    void SetLanguage(const QString &language);
    // 是否锁定用户
    void SetLocked(bool locked);
    // 设置用户密码，同时会对用户解除锁定，密码为加密密码
    void SetPassword(const QString &password, const QString &hint);
    // 通过passwd命令设置用户密码
    void SetPasswordByPasswd(const QString &encryptedCurrentPassword, const QString &encryptedNewPassword);
    // 设置用户密码过期信息
    void SetPasswordExpirationPolicy(const QString &options);
    // 设置用户密码提示
    void SetPasswordHint(const QString &hint);
    // 设置密码模式，同时会对用户解除锁定
    void SetPasswordMode(int mode);
    // 设置用户真实姓名
    void SetRealName(const QString &name);
    void SetSession(const QString &session);
    void SetSessionType(const QString &session_type);
    // 设置用户登录shell
    void SetShell(const QString &shell);
    // 设置用户名
    void SetUserName(const QString &name);
    // 设置桌面会话(mate, gnome, etc..)
    void SetXSession(const QString &x_session);

private:
    void setAccountTypeAuthenticated(const QDBusMessage &message, int accountType);
    void setAutomaticLoginAuthenticated(const QDBusMessage &message, bool enabled);
    void setEmailAuthenticated(const QDBusMessage &message, const QString &email);
    void setHomeDirectoryAuthenticated(const QDBusMessage &message, const QString &homedir);
    void setIconFileAuthenticated(const QDBusMessage &message, const QString &filepath);
    void setLanguageAuthenticated(const QDBusMessage &message, const QString &language);
    void setLockedAuthenticated(const QDBusMessage &message, bool locked);
    void setPasswordAuthenticated(const QDBusMessage &message, const QString &password, const QString &hint);
    void setPasswordByPasswdAuthenticated(const QDBusMessage &message,
                                          const QString &encryptedCurrentPassword, const QString &encryptedNewPassword);
    void setPasswordExpirationPolicyAuthenticated(const QDBusMessage &message, const QString &options);
    void setPasswordHintAuthenticated(const QDBusMessage &message, const QString &hint);
    void setPasswordModeAuthenticated(const QDBusMessage &message, int mode);
    void setRealNameAuthenticated(const QDBusMessage &message, const QString &name);
    void setSessionAuthenticated(const QDBusMessage &message, const QString &session);
    void setSessionTypeAuthenticated(const QDBusMessage &message, const QString &sessionType);
    void setShellAuthenticated(const QDBusMessage &message, const QString &shell);
    void setUserNameAuthenticated(const QDBusMessage &message, const QString &name);
    void setXSessionAuthenticated(const QDBusMessage &message, const QString &xsession);

private:
    QString getAuthAction(const QDBusMessage &message, const QString &ownAction);
    // 密码变更后需要更新相关属性
    void processPasswdChanged(const QString &errorDesc, const QDBusMessage &message);

private:
    void init();
    void buildFreedesktopUserObjectPath();
    // 这里只更新与缓存数据无关的变量
    void udpateNocacheVar(PasswdShadow passwd_shadow);
    void initSettings();
    // 根据shadow信息更新密码过期策略
    void updatePasswordExpirationPolicy(QSharedPointer<SPwd> spwd);
    AccountsAccountType accountTtypeFromPwent(QSharedPointer<Passwd> passwd);
    void resetIconFile();
    // 由于切换用户时，登录器通过org.freedesktop.Accounts接口获取图标，Kiran设置/更新用户图标后需要同步到freedesktop
    void syncIconFileToFreedesktop(const QString &iconFile);
    // 获取当前用户的加密密码
    QString getHashedPassphrase();
    // 判断用户密码是否合法
    bool checkPassword(const QString &password);

private:
    QString m_objectPath;
    UserAdaptor *m_userAdaptor;
    // 绑定的passwd和shadow文件
    PasswdShadow m_passwdShadow;
    // 用户在accountsservice服务中的ObjectPath地址
    QString m_freedesktopObjectPath;

    QString defaultIconFile;
    QSharedPointer<Passwd> m_passwd;
    QSharedPointer<SPwd> m_spwd;

    int m_accountType;
    bool m_automaticLogin;
    QString m_homeDirectory;
    qulonglong m_gid;
    bool m_locked;
    QString m_passwordExpirationPolicy;
    int m_passwordMode;
    QString m_realName;
    QString m_shell;
    bool m_systemAccount;
    qulonglong m_uid;
    QString m_userName;
    // 用户缓存数据管理
    QSettings *m_settings;
    QSharedPointer<PasswdProcess> m_passwdProcess;
};

using UserVec = QVector<QSharedPointer<User>>;
}  // namespace Kiran