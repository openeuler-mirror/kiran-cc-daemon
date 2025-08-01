/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#include "groups-wrapper.h"
#include <kiran-log/qt5-log-i.h>

namespace Kiran
{
#define MINIMUM_GID 1000

#define PATH_PASSWD "/etc/passwd"
#define PATH_SHADOW "/etc/gshadow"
#define PATH_GROUP "/etc/group"

GroupEntry::GroupEntry(group *grp)
{
    name = grp->gr_name;
    gid = grp->gr_gid;
    passwd = grp->gr_passwd;
    for (auto pos = grp->gr_mem; pos != NULL && *pos != NULL; ++pos)
    {
        mem << *pos;
    }
    localGroup = false;
    primaryGroup = false;
}

GroupsWrapper::GroupsWrapper()
{
    m_fsWatcher = new QFileSystemWatcher(QStringList{PATH_PASSWD, PATH_SHADOW, PATH_GROUP}, this);
    m_reloadTimer = new QTimer(this);
}

GroupsWrapper *GroupsWrapper::m_instance = nullptr;
void GroupsWrapper::globalInit()
{
    m_instance = new GroupsWrapper();
    m_instance->init();
}

QMap<QString, QSharedPointer<GroupEntry>> GroupsWrapper::getGroups()
{
    return m_groups;
}

QSharedPointer<GroupEntry> GroupsWrapper::getGroupEntryByID(qulonglong gid)
{
    auto iter = m_groups.constBegin();
    while (iter != m_groups.constEnd())
    {
        if (iter.value() && iter.value()->gid == gid)
        {
            return iter.value();
        }
        ++iter;
    }

    auto grent = getgrgid(gid);
    if (grent != nullptr)
    {
        return QSharedPointer<GroupEntry>::create(grent);
    }
    return nullptr;
}

QSharedPointer<GroupEntry> GroupsWrapper::getGroupEntryByName(QString name)
{
    auto iter = m_groups.find(name);
    if (iter != m_groups.end())
    {
        return iter.value();
    }

    auto grent = getgrnam(name.toUtf8().data());
    if (grent != nullptr)
    {
        return QSharedPointer<GroupEntry>::create(grent);
    }
    return nullptr;
}

bool GroupsWrapper::isUserExist(const QString &userName)
{
    if (m_pwUsers.contains(userName))
    {
        return true;
    }

    if (getpwnam(userName.toUtf8().data()) != nullptr)
    {
        m_pwUsers.append(userName);
        return true;
    }
    return false;
}

void GroupsWrapper::idleReload(const QString &filePath)
{
    KLOG_INFO() << "File" << filePath << "is changed";

    if (!m_reloadTimer->isActive())
    {
        m_reloadTimer->start(100);
    }
}

void GroupsWrapper::reload()
{
    m_reloadTimer->stop();

    // 先加载系统中所有用户组
    reloadGroups();
    // 更新组中的主组信息
    reloadPrimaryGroup();

    Q_EMIT groupsChanged();

    // 需要重新添加监听，因为这个文件不是直接被修改，而是修改的是备份文件然后重新替换的，导致监听被移除
    m_fsWatcher->addPaths(QStringList{PATH_PASSWD, PATH_SHADOW, PATH_GROUP});
}

void GroupsWrapper::init()
{
    // 先加载系统中所有用户组
    reloadGroups();
    // 更新组中的主组信息
    reloadPrimaryGroup();

    connect(m_fsWatcher, &QFileSystemWatcher::fileChanged, this, &GroupsWrapper::idleReload);
    connect(m_reloadTimer, &QTimer::timeout, this, &GroupsWrapper::reload);
}

void GroupsWrapper::reloadPrimaryGroup()
{
    auto fp = fopen(PATH_PASSWD, "r");
    if (fp == NULL)
    {
        KLOG_WARNING() << "Unable to open" << PATH_PASSWD << ":" << strerror(errno);
        return;
    }

    m_pwUsers.clear();
    struct passwd *pwent;

    do
    {
        pwent = fgetpwent(fp);
        if (pwent == NULL)
        {
            break;
        }
        // 存储系统中的用户名
        m_pwUsers.append(pwent->pw_name);

        // 查询该用户的组id,并设置该组为主组
        auto grent = getgrgid(pwent->pw_gid);
        if (grent == NULL)
        {
            continue;
        }
        if (m_groups.contains(grent->gr_name))
        {
            m_groups[grent->gr_name]->primaryGroup = true;
        }
    } while (pwent != NULL);

    fclose(fp);
}

void GroupsWrapper::reloadGroups()
{
    auto fp = fopen(PATH_GROUP, "r");
    if (fp == NULL)
    {
        KLOG_WARNING() << "Unable to open" << PATH_GROUP << ":" << strerror(errno);
        return;
    }

    m_groups.clear();
    struct group *grent = nullptr;

    while ((grent = fgetgrent(fp)) != NULL)
    {
        auto groupEntry = QSharedPointer<GroupEntry>::create(grent);
        groupEntry->localGroup = grent->gr_gid >= MINIMUM_GID;

        m_groups.insert(groupEntry->name, groupEntry);
    }

    KLOG_INFO() << "Load group information from " << PATH_GROUP << "which contains groups" << m_groups.keys();

    fclose(fp);
}

}  // namespace Kiran
