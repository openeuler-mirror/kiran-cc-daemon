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

#include "groups-manager.h"
#include <kiran-log/qt5-log-i.h>
#include <QDBusConnection>
#include "config-groups.h"
#include "group.h"
#include "groups-i.h"
#include "groups-util.h"
#include "groups-wrapper.h"
#include "groupsadaptor.h"
#include "lib/base/base.h"
#include "lib/base/polkit-proxy.h"

namespace Kiran
{
GroupsManager::GroupsManager(GroupsWrapper *groupsWrapper) : m_groupsWrapper(groupsWrapper)
{
    auto systemConnection = QDBusConnection::systemBus();
    if (!systemConnection.registerService(GROUPS_DBUS_NAME))
    {
        KLOG_WARNING() << "Failed to register dbus name: " << GROUPS_DBUS_NAME;
        return;
    }

    if (!systemConnection.registerObject(GROUPS_OBJECT_PATH, GROUPS_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR() << "Can't register object:" << systemConnection.lastError();
        return;
    }

    m_adaptor = new GroupsAdaptor(this);
}

GroupsManager::~GroupsManager()
{
}

GroupsManager *GroupsManager::m_instance = nullptr;
void GroupsManager::globalInit(GroupsWrapper *groupsWrapper)
{
    m_instance = new GroupsManager(groupsWrapper);
    m_instance->init();
}

void GroupsManager::reload()
{
    auto newGroups = loadGroups();
    QMap<qulonglong, QSharedPointer<Group>> addedGroups;
    QMap<qulonglong, QSharedPointer<Group>> deletedGroups;

    // 移除被删除的用户组
    for (auto iter = m_groups.begin(); iter != m_groups.end(); ++iter)
    {
        auto iter2 = newGroups.find(iter.key());
        if (iter2 == newGroups.end())
        {
            deletedGroups.insert(iter.key(), iter.value());
        }
    }

    // 添加新增的用户组
    for (auto iter = newGroups.begin(); iter != newGroups.end(); ++iter)
    {
        auto iter2 = m_groups.find(iter.key());
        if (iter2 == m_groups.end())
        {
            addedGroups.insert(iter.key(), iter.value());
        }
    }

    KLOG_INFO() << "Update group cache, add groups" << addedGroups.keys() << ", deleted groups:" << deletedGroups.keys();

    m_groups = newGroups;

    // 单独处理被删除和添加的用户组。
    // 取消注册被删除的组
    auto iter = deletedGroups.begin();
    while (iter != deletedGroups.end())
    {
        auto iter2 = m_groups.find(iter.key());
        if (iter2 == m_groups.end())
        {
            Q_EMIT GroupDeleted(iter.value()->getObjectPath());
            // 这里直接从QMap中移除，引用计数为0,调用Group析构函数，自动注销dbus对象
            iter = deletedGroups.erase(iter);  // 安全删除并获取下一个有效迭代器
        }
        else
        {
            ++iter;
        }
    }

    // 注册新增组
    for (auto iter = addedGroups.begin(); iter != addedGroups.end(); ++iter)
    {
        auto iter2 = m_groups.find(iter.key());
        if (iter2 != m_groups.end())
        {
            Q_EMIT GroupAdded(iter.value()->getObjectPath());
            iter.value()->dbusRegister();
        }
    }
}

void GroupsManager::init()
{
    reload();

    connect(m_groupsWrapper, &GroupsWrapper::groupsChanged, this, &GroupsManager::reload);
}

QMap<qulonglong, QSharedPointer<Group>> GroupsManager::loadGroups()
{
    auto groupEntries = GroupsWrapper::getInstance()->getGroups();
    QMap<qulonglong, QSharedPointer<Group>> groups;
    QSharedPointer<Group> group;

    for (auto iter = groupEntries.begin(); iter != groupEntries.end(); ++iter)
    {
        if (m_groups.contains(iter.key()))
        {
            // update Group
            group = m_groups[iter.key()];
            group->updateGroup(*iter.value());
        }
        else
        {
            group = Group::createGroup(*iter.value());
        }
        groups.insert(iter.key(), group);
    }
    return groups;
}

QSharedPointer<Group> GroupsManager::findAndCreateGroupByGID(qulonglong gid)
{
    auto groupEntry = m_groupsWrapper->getGroupEntryByID(gid);
    if (!groupEntry)
    {
        KLOG_WARNING() << "Unable to lookup group gid" << gid;
        return nullptr;
    }

    auto group = m_groups[groupEntry->gid];
    if (!group)
    {
        group = Group::createGroup(*groupEntry);
        group->dbusRegister();

        m_groups.insert(groupEntry->gid, group);
        KLOG_INFO() << "Add new group" << group->getName() << "to groups cache.";
        Q_EMIT GroupAdded(group->getObjectPath());
    }

    return group;
}

QSharedPointer<Group> GroupsManager::findAndCreateGroupByName(const QString &name)
{
    auto groupEntry = m_groupsWrapper->getGroupEntryByName(name);
    if (!groupEntry)
    {
        KLOG_WARNING() << "Unable to lookup group name" << name;
        return nullptr;
    }

    auto group = m_groups[groupEntry->gid];
    if (!group)
    {
        group = Group::createGroup(*groupEntry);
        group->dbusRegister();

        m_groups.insert(groupEntry->gid, group);
        KLOG_INFO() << "Add new group" << group->getName() << "to groups cache.";
        Q_EMIT GroupAdded(group->getObjectPath());
    }

    return group;
}

QList<QDBusObjectPath> GroupsManager::ListCachedGroups()
{
    QList<QDBusObjectPath> cachedGroups;
    for (auto iter = m_groups.begin(); iter != m_groups.end(); ++iter)
    {
        if (iter.value())
        {
            cachedGroups.push_back(iter.value()->getObjectPath());
        }
    }
    return cachedGroups;
}

QDBusObjectPath GroupsManager::FindGroupByName(const QString &name)
{
    auto group = findAndCreateGroupByName(name);

    if (group)
    {
        return group->getObjectPath();
    }
    else
    {
        DBUS_ERROR_REPLY(CCErrorCode::ERROR_GROUPS_GROUP_NOT_FOUND_2);
    }
    return QDBusObjectPath();
}

QDBusObjectPath GroupsManager::FindGroupByID(qulonglong gid)
{
    auto group = findAndCreateGroupByGID(gid);

    if (group)
    {
        return group->getObjectPath();
    }
    else
    {
        DBUS_ERROR_REPLY(CCErrorCode::ERROR_GROUPS_GROUP_NOT_FOUND_1);
    }
    return QDBusObjectPath();
}

CHECK_AUTH_WITH_2ARGS_AND_RETVAL(GroupsManager,
                                 QDBusObjectPath,
                                 CreateGroup,
                                 createGroupAuthenticated,
                                 AUTH_GROUP_ADMIN,
                                 const QString &,
                                 const QStringList &);

void GroupsManager::createGroupAuthenticated(const QDBusMessage &message,
                                             const QString &name,
                                             const QStringList &users)
{
    auto groupEntry = m_groupsWrapper->getGroupEntryByName(name);
    if (groupEntry)
    {
        DBUS_ERROR_DELAY_REPLY_AND_RET(CCErrorCode::ERROR_GROUPS_GROUP_ALREADY_EXIST);
    }

    KLOG_INFO() << "Create group" << name;

    auto program = QString("/usr/sbin/groupadd");
    QStringList arguments = {name};

    if (users.size() > 0)
    {
        arguments.append("-U");
        arguments.append(users.join(","));
    }

    SPAWN_WITH_DBUS_MESSAGE(message, program, arguments);

    auto group = findAndCreateGroupByName(name);
    if (group)
    {
        // 创建组时默认为非主组
        /**
         * 只有在创建用户或者使用sudo usermod -g newgroup username命令才能修改主组，
         * 这些操作会修改/etc/passwd文件变化，重新加载组信息
         */
        group->setLocalGroup(true);

        auto replyMessage = message.createReply(group->getObjectPath());
        QDBusConnection::systemBus().send(replyMessage);
    }
    else
    {
        DBUS_ERROR_REPLY(CCErrorCode::ERROR_GROUPS_GROUP_NOT_FOUND_3);
    }

    return;
}

CHECK_AUTH_WITH_1ARGS(GroupsManager,
                      DeleteGroup,
                      deleteGroupAuthenticated,
                      AUTH_GROUP_ADMIN,
                      qulonglong);

void GroupsManager::deleteGroupAuthenticated(const QDBusMessage &message,
                                             qulonglong gid)
{
    auto groupEntry = m_groupsWrapper->getGroupEntryByID(gid);
    if (!groupEntry)
    {
        DBUS_ERROR_DELAY_REPLY_AND_RET(CCErrorCode::ERROR_GROUPS_GROUP_NOT_FOUND_4);
    }

    KLOG_INFO() << "Delete group" << groupEntry->name << "with gid" << gid;

    auto program = QString("/usr/sbin/groupdel");
    QStringList arguments = {"--", groupEntry->name};

    SPAWN_WITH_DBUS_MESSAGE(message, program, arguments);
    QDBusConnection::systemBus().send(message.createReply());

    return;
}

}  // namespace Kiran
