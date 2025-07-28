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

#include "group.h"
#include <kiran-log/qt5-log-i.h>
#include "config-groups.h"
#include "groupadaptor.h"
#include "groups-i.h"
#include "groups-util.h"
#include "groups-wrapper.h"
#include "lib/base/base.h"
#include "lib/base/polkit-proxy.h"

namespace Kiran
{

#define KIRAN_GROUPS_GROUP_OBJECT_PATH "/com/kylinsec/Kiran/SystemDaemon/Groups/Group"
#define KIRAN_GROUPS_GROUP_INTERFACE "com.kylinsec.Kiran.SystemDaemon.Groups.Group"

Group::Group(GroupEntry groupEntry)
{
    m_name = groupEntry.name;
    m_gid = groupEntry.gid;
    m_localGroup = groupEntry.localGroup;
    m_primaryGroup = groupEntry.primaryGroup;
    m_groupUsers = groupEntry.mem;

    m_objectPath = QString("%1/%2").arg(GROUPS_OBJECT_PATH).arg(m_gid);
    m_groupAdaptor = new GroupAdaptor(this);
}

Group::~Group()
{
    dbusUnregister();
}

QSharedPointer<Group> Group::createGroup(GroupEntry groupEntry)
{
    auto group = QSharedPointer<Group>::create(groupEntry);
    return group;
}

void Group::dbusRegister()
{
    auto systemConnection = QDBusConnection::systemBus();
    if (!systemConnection.registerObject(m_objectPath, KIRAN_GROUPS_GROUP_INTERFACE, this))
    {
        KLOG_ERROR() << "Can't register object:" << systemConnection.lastError();
        return;
    }
}

void Group::dbusUnregister()
{
    auto systemConnection = QDBusConnection::systemBus();
    systemConnection.unregisterObject(m_objectPath);
}

QDBusObjectPath Group::getObjectPath()
{
    return QDBusObjectPath(m_objectPath);
}

void Group::updateGroup(GroupEntry groupEntry)
{
    if (m_name != groupEntry.name ||
        m_gid != groupEntry.gid ||
        m_groupUsers != groupEntry.mem ||
        m_localGroup != groupEntry.localGroup ||
        m_primaryGroup != groupEntry.primaryGroup)
    {
        setGID(groupEntry.gid);
        setName(groupEntry.name);
        setUsers(groupEntry.mem);
        setLocalGroup(groupEntry.localGroup);
        setPrimaryGroup(groupEntry.primaryGroup);
        Q_EMIT Changed();
    }
    m_objectPath = QString("%1/%2").arg(KIRAN_GROUPS_GROUP_OBJECT_PATH).arg(m_gid);
}

CHECK_AUTH_WITH_1ARGS(Group,
                      AddUserToGroup,
                      addUserToGroupAuthenticated,
                      AUTH_CHANGE_OWN_USER_DATA,
                      const QString &);
void Group::addUserToGroupAuthenticated(const QDBusMessage &message, const QString &name)
{
    // 判断用户是否存在
    if (!GroupsWrapper::getInstance()->isUserExist(name))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GROUPS_GROUP_USER_EXIST);
    }
    // 判断用户是否已经在组中
    if (m_groupUsers.contains(name))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GROUPS_GROUP_USER_ALREADY_IN_GROUP);
    }

    KLOG_INFO() << "Add user" << name << "to group" << m_name;

    auto program = QString("/usr/sbin/groupmems");
    QStringList arguments = {"-g", m_name, "-a", name};

    SPAWN_WITH_DBUS_MESSAGE(message, program, arguments);

    // 将用户添加到组中
    m_groupUsers.append(name);
    QDBusConnection::systemBus().send(message.createReply());
}

CHECK_AUTH_WITH_1ARGS(Group,
                      RemoveUserFromGroup,
                      removeUserFromGroupAuthenticated,
                      AUTH_CHANGE_OWN_USER_DATA,
                      const QString &);

void Group::removeUserFromGroupAuthenticated(const QDBusMessage &message, const QString &name)
{
    // 判断用户是否存在
    if (!GroupsWrapper::getInstance()->isUserExist(name))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GROUPS_GROUP_USER_EXIST);
    }

    // 判断用户是否在组中
    if (!m_groupUsers.contains(name))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GROUPS_GROUP_USER_NOT_IN_GROUP);
    }

    KLOG_INFO() << "Delete user" << name << "from group" << m_name;

    auto program = QString("/usr/sbin/groupmems");
    QStringList arguments = {"-g", m_name, "-d", name};

    SPAWN_WITH_DBUS_MESSAGE(message, program, arguments);

    // 将用户从组成员中删除
    m_groupUsers.removeAll(name);
    QDBusConnection::systemBus().send(message.createReply());
}

CHECK_AUTH_WITH_1ARGS(Group,
                      ChangeGroupName,
                      changeGroupNameAuthenticated,
                      AUTH_CHANGE_OWN_USER_DATA,
                      const QString &);
void Group::changeGroupNameAuthenticated(const QDBusMessage &message, const QString &newGroupName)
{
    if (QString::compare(newGroupName, m_name, Qt::CaseInsensitive) == 0)
    {
        KLOG_INFO() << "Change group name of" << m_name << "to" << newGroupName;

        auto program = QString("/usr/sbin/groupmod");
        QStringList arguments = {"-n", newGroupName, "--", m_name};

        SPAWN_WITH_DBUS_MESSAGE(message, program, arguments);
    }
    QDBusConnection::systemBus().send(message.createReply());
}

CHECK_AUTH_WITH_1ARGS(Group,
                      ChangeGroupID,
                      changeGroupIDAuthenticated,
                      AUTH_CHANGE_OWN_USER_DATA,
                      uint32_t);
void Group::changeGroupIDAuthenticated(const QDBusMessage &message, uint32_t newGid)
{
    if (newGid != m_gid)
    {
        KLOG_INFO() << "Change group id of" << m_name << "to" << newGid;

        auto strGID = QString::number(newGid);
        auto program = QString("/usr/sbin/groupmod");
        QStringList arguments = {"-g", strGID, "--", m_name};

        SPAWN_WITH_DBUS_MESSAGE(message, program, arguments);
    }
    QDBusConnection::systemBus().send(message.createReply());
}

}  // namespace Kiran
