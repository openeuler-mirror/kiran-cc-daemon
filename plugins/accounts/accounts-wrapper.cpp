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

#include "accounts-wrapper.h"
#include <grp.h>
#include <pwd.h>
#include <shadow.h>
#include <string.h>
#include <QFileSystemWatcher>
#include <QTimer>
#include <utility>
#include "accounts-util.h"
#include "lib/base/base.h"

namespace Kiran
{
#define PATH_PASSWD "/etc/passwd"
#define PATH_SHADOW "/etc/shadow"
#define PATH_GROUP "/etc/group"

Passwd::Passwd(struct passwd *passwd) : uid(0),
                                        gid(0)
{
    RETURN_IF_FALSE(passwd != NULL);

    name = passwd->pw_name;
    this->passwd = passwd->pw_passwd;
    uid = passwd->pw_uid;
    gid = passwd->pw_gid;
    gecos = passwd->pw_gecos;
    dir = passwd->pw_dir;
    shell = passwd->pw_shell;
}

SPwd::SPwd(struct spwd *sp)
{
    RETURN_IF_FALSE(sp != NULL);

    namp = sp->sp_namp;
    pwdp = sp->sp_pwdp;
    lstchg = sp->sp_lstchg;
    min = sp->sp_min;
    max = sp->sp_max;
    warn = sp->sp_warn;
    inact = sp->sp_inact;
    expire = sp->sp_expire;
    flag = sp->sp_flag;
}

Group::Group(struct group *grp)
{
    RETURN_IF_FALSE(grp != NULL);

    name = grp->gr_name;
    passwd = grp->gr_passwd;
    gid = grp->gr_gid;
    for (auto pos = grp->gr_mem; pos != NULL && *pos != NULL; ++pos)
    {
        mem.push_back(*pos);
    }
}

AccountsWrapper::AccountsWrapper()
{
    m_fsWatcher = new QFileSystemWatcher(QStringList{PATH_PASSWD, PATH_SHADOW, PATH_GROUP}, this);
    m_reloadTimer = new QTimer(this);
}

AccountsWrapper *AccountsWrapper::m_instance = nullptr;
void AccountsWrapper::globalInit()
{
    m_instance = new AccountsWrapper();
    m_instance->init();
}

QVector<PasswdShadow> AccountsWrapper::getPasswdsShadows()
{
    QVector<PasswdShadow> passwdsShadows;

    for (auto iter = m_passwds.begin(); iter != m_passwds.end(); ++iter)
    {
        auto spwd = m_spwds.find(iter.key());
        // 如果spwd不存在，可能是因为账户正在创建中，这个时候不作为返回值，避免出现处理空指针引起的崩溃问题
        if (spwd != m_spwds.end())
        {
            passwdsShadows.push_back(QPair<QSharedPointer<Passwd>, QSharedPointer<SPwd>>(iter.value(), spwd.value()));
        }
        else
        {
            KLOG_WARNING(accounts) << "The shadow info of user" << iter.key() << "isn't found.";
        }
    }

    return passwdsShadows;
}
QSharedPointer<Passwd> AccountsWrapper::getPasswdByName(const QString &userName)
{
    auto iter = m_passwds.find(userName);
    if (iter != m_passwds.end())
    {
        return iter.value();
    }

    auto pwent = getpwnam(userName.toUtf8().data());
    if (pwent != NULL)
    {
        return QSharedPointer<Passwd>::create(pwent);
    }

    return nullptr;
}

QSharedPointer<Passwd> AccountsWrapper::getPasswdByUID(uint64_t uid)
{
    auto iter = m_passwdsByUID.find(uid);
    if (iter != m_passwdsByUID.end())
    {
        return iter.value();
    }

    auto pwent = getpwuid(uid);
    if (pwent != NULL)
    {
        return QSharedPointer<Passwd>::create(pwent);
    }
    return nullptr;
}

QSharedPointer<SPwd> AccountsWrapper::getSpwdByName(const QString &userName)
{
    auto iter = m_spwds.find(userName);
    if (iter != m_spwds.end())
    {
        return iter.value();
    }

    auto spent = getspnam(userName.toUtf8().data());
    if (spent != NULL)
    {
        return QSharedPointer<SPwd>::create(spent);
    }
    return nullptr;
}

QSharedPointer<Group> AccountsWrapper::getGroupByName(const QString &groupName)
{
    auto grp = getgrnam(groupName.toUtf8().data());
    if (grp == NULL)
    {
        return nullptr;
    }
    else
    {
        return QSharedPointer<Group>::create(grp);
    }
}

QVector<uint32_t> AccountsWrapper::getUserGroups(const QString &user, uint32_t group)
{
    int32_t ngroups = 0;
    getgrouplist(user.toLatin1().data(), group, NULL, &ngroups);
    auto groups = new gid_t[ngroups];
    auto res = getgrouplist(user.toLatin1().data(), group, groups, &ngroups);
    return QVector<uint32_t>(groups, groups + res);
}

void AccountsWrapper::init()
{
    reloadPasswd();
    reloadShadow();

    connect(m_fsWatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(idleReload(const QString &)));
    connect(m_reloadTimer, SIGNAL(timeout()), this, SLOT(reload()));
}

void AccountsWrapper::reloadPasswd()
{
    auto fp = fopen(PATH_PASSWD, "r");
    if (fp == NULL)
    {
        KLOG_WARNING(accounts) << "Unable to open" << PATH_PASSWD << ":" << strerror(errno);
        return;
    }

    m_passwds.clear();
    m_passwdsByUID.clear();
    struct passwd *pwent;

    do
    {
        pwent = fgetpwent(fp);
        if (pwent != NULL)
        {
            auto passwd = QSharedPointer<Passwd>::create(pwent);
            m_passwds.insert(passwd->name, passwd);
            m_passwdsByUID.insert(passwd->uid, passwd);
        }

    } while (pwent != NULL);

    KLOG_INFO(accounts) << "Load passwd information from" << PATH_PASSWD << "which contains users" << m_passwds.keys();

    fclose(fp);
}
void AccountsWrapper::reloadShadow()
{
    auto fp = fopen(PATH_SHADOW, "r");
    if (fp == NULL)
    {
        KLOG_WARNING(accounts) << "Unable to open" << PATH_SHADOW << ":" << strerror(errno);
        return;
    }

    m_spwds.clear();

    struct
    {
        struct spwd spbuf;
        char buf[1024];
    } shadow_buffer;

    struct spwd *shadowEntry;

    do
    {
        auto ret = fgetspent_r(fp, &shadow_buffer.spbuf, shadow_buffer.buf, sizeof(shadow_buffer.buf), &shadowEntry);
        if (ret == 0 && shadowEntry != NULL)
        {
            auto spwd = QSharedPointer<SPwd>::create(shadowEntry);
            m_spwds.insert(spwd->namp, spwd);
        }
        else if (errno != EINTR)
        {
            break;
        }
    } while (shadowEntry != NULL);

    fclose(fp);

    KLOG_INFO(accounts) << "Load shadow information from" << PATH_SHADOW << "which contains users" << m_spwds.keys();
}

void AccountsWrapper::idleReload(const QString &filePath)
{
    KLOG_INFO(accounts) << "File" << filePath << "is changed";

    if (!m_reloadTimer->isActive())
    {
        m_reloadTimer->start(100);
    }
}

void AccountsWrapper::reload()
{
    m_reloadTimer->stop();

    reloadPasswd();
    reloadShadow();
    Q_EMIT userChanged();

    // 需要重新添加监听，因为这个文件不是直接被修改，而是修改的是备份文件然后重新替换的，导致监听被移除
    m_fsWatcher->addPaths(QStringList{PATH_PASSWD, PATH_SHADOW, PATH_GROUP});
}

}  // namespace Kiran