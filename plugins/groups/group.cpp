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
#include "groupadaptor.h"
#include "groups-i.h"

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

}  // namespace Kiran
