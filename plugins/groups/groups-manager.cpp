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
#include "group.h"
#include "groups-i.h"
#include "groups-wrapper.h"
#include "groupsadaptor.h"
#include "lib/base/base.h"

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
    QStringList addedGroupsName;
    QStringList deletedGroupsName;

    // 移除被删除的用户组
    for (auto iter = m_groups.begin(); iter != m_groups.end(); ++iter)
    {
        auto iter2 = newGroups.find(iter.key());
        if (iter2 == newGroups.end())
        {
            Q_EMIT GroupDeleted(iter.value()->getObjectPath());
            deletedGroupsName << iter.key();
            iter.value()->dbusUnregister();
        }
    }

    // 添加新增的用户组
    for (auto iter = newGroups.begin(); iter != newGroups.end(); ++iter)
    {
        auto iter2 = m_groups.find(iter.key());
        if (iter2 == m_groups.end())
        {
            Q_EMIT GroupAdded(iter.value()->getObjectPath());
            iter.value()->dbusRegister();
            addedGroupsName << iter.key();
        }
    }

    KLOG_INFO() << "Update group cache, add groups" << addedGroupsName << ", deleted groups:" << deletedGroupsName;

    m_groups = newGroups;
}

void GroupsManager::init()
{
    reload();

    connect(m_groupsWrapper, &GroupsWrapper::groupsChanged, this, &GroupsManager::reload);
}

QMap<QString, QSharedPointer<Group>> GroupsManager::loadGroups()
{
    auto groupEntries = GroupsWrapper::getInstance()->getGroups();
    QMap<QString, QSharedPointer<Group>> groups;
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

QSharedPointer<Group> GroupsManager::findAndCreateGroupByGID(uint32_t gid)
{
    auto groupEntry = m_groupsWrapper->getGroupEntryByID(gid);
    if (!groupEntry)
    {
        KLOG_WARNING() << "Unable to lookup group gid" << gid;
        return nullptr;
    }

    auto group = m_groups[groupEntry->name];
    if (!group)
    {
        group = Group::createGroup(*groupEntry);
        group->dbusRegister();

        m_groups.insert(groupEntry->name, group);
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

    auto group = m_groups[groupEntry->name];
    if (!group)
    {
        group = Group::createGroup(*groupEntry);
        group->dbusRegister();

        m_groups.insert(groupEntry->name, group);
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

QDBusObjectPath GroupsManager::FindGroupByID(uint32_t gid)
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

QDBusObjectPath GroupsManager::CreateGroup(const QString &name)
{
}

void GroupsManager::DeleteGroup(uint32_t gid)
{
}

}  // namespace Kiran
