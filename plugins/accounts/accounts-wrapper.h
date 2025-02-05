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

#include <QLoggingCategory>
#include <QMap>
#include <QObject>
#include <QVector>

struct passwd;
struct spwd;
struct group;
class QFileSystemWatcher;
class QTimer;

namespace Kiran
{
class Passwd
{
public:
    Passwd() = delete;
    Passwd(struct passwd *passwd);

    // Username.
    QString name;
    // Hashed passphrase, if shadow database not in use (see shadow.h).
    QString passwd;
    // User ID.
    uint32_t uid;
    // Group ID.
    uint32_t gid;
    // Real name.
    QString gecos;
    // Home directory.
    QString dir;
    // Shell program.
    QString shell;
};

class SPwd
{
public:
    SPwd() = delete;
    SPwd(struct spwd *sp);
    QString namp;           /* Login name.  */
    QString pwdp;           /* Hashed passphrase.  */
    long int lstchg;        /* Date of last change.  */
    long int min;           /* Minimum number of days between changes.  */
    long int max;           /* Maximum number of days between changes.  */
    long int warn;          /* Number of days to warn user to change the password.  */
    long int inact;         /* Number of days the account may be inactive.  */
    long int expire;        /* Number of days since 1970-01-01 until  account expires.  */
    unsigned long int flag; /* Reserved.  */
};

class Group
{
public:
    Group() = delete;
    Group(struct group *grp);

    QString name;             /* Group name.	*/
    QString passwd;           /* Password.	*/
    uint32_t gid;             /* Group ID.	*/
    std::vector<QString> mem; /* Member list.	*/
};

using PasswdShadow = QPair<QSharedPointer<Passwd>, QSharedPointer<SPwd>>;

class AccountsWrapper : public QObject
{
    Q_OBJECT

public:
    AccountsWrapper();

    static AccountsWrapper *getInstance() { return m_instance; };
    static void globalInit();
    static void globalDeinit() { delete m_instance; };

    QVector<PasswdShadow> getPasswdsShadows();
    QSharedPointer<Passwd> getPasswdByName(const QString &userName);
    QSharedPointer<Passwd> getPasswdByUID(uint64_t uid);
    QSharedPointer<SPwd> getSpwdByName(const QString &userName);
    QSharedPointer<Group> getGroupByName(const QString &groupName);
    QVector<uint32_t> getUserGroups(const QString &user, uint32_t group);

Q_SIGNALS:
    void userChanged();

private:
    void init();

    void reloadPasswd();
    void reloadShadow();

private Q_SLOTS:
    void idleReload(const QString &filePath);
    // 配置文件更新，重新加载
    void reload();

private:
    static AccountsWrapper *m_instance;
    // 监控passwd/shadow/group文件变化
    QFileSystemWatcher *m_fsWatcher;
    QTimer *m_reloadTimer;

    // <userName, Passwd>
    QMap<QString, QSharedPointer<Passwd>> m_passwds;
    // <userUID, Passwd>
    QMap<uint64_t, QSharedPointer<Passwd>> m_passwdsByUID;
    // <userName, SPwd>
    QMap<QString, QSharedPointer<SPwd>> m_spwds;
};

}  // namespace Kiran