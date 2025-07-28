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

#pragma once

#include <QDBusContext>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include "groups-wrapper.h"

class GroupAdaptor;
namespace Kiran
{
class Group : public QObject,
              protected QDBusContext
{
    Q_OBJECT

public:
    Group() = delete;
    Group(GroupEntry groupEntry);
    virtual ~Group();

    static QSharedPointer<Group> createGroup(GroupEntry groupEntry);

    void dbusRegister();
    void dbusUnregister();
    QDBusObjectPath getObjectPath();

    void updateGroup(GroupEntry groupEntry);

public:
    Q_PROPERTY(uint32_t gid READ getGID WRITE setGID)
    Q_PROPERTY(QString name READ getName WRITE setName)
    Q_PROPERTY(bool local_group READ getLocalGroup WRITE setLocalGroup)
    Q_PROPERTY(bool primary_group READ getPrimaryGroup WRITE setPrimaryGroup)
    Q_PROPERTY(QStringList users READ getUsers WRITE setUsers)

public:
    uint32_t getGID() { return this->m_gid; };
    QString getName() { return this->m_name; };
    bool getLocalGroup() { return this->m_localGroup; };
    bool getPrimaryGroup() { return this->m_primaryGroup; };
    QStringList getUsers() { return this->m_groupUsers; };

    void setGID(uint32_t gid) { this->m_gid = gid; };
    void setName(const QString &name) { this->m_name = name; };
    void setLocalGroup(bool localGroup) { this->m_localGroup = localGroup; };
    void setPrimaryGroup(bool primaryGroup) { this->m_primaryGroup = primaryGroup; };
    void setUsers(const QStringList &users) { this->m_groupUsers = users; };

    /**
     * @brief 添加用户至组
     * @param 用户名
     */
    void AddUserToGroup(const QString &userName);

    /**
     * @brief 从组中移除用户
     * @param 用户名
     */
    void RemoveUserFromGroup(const QString &userName);

    /**
     * @brief 修改组名
     * @param 新组名
     */
    void ChangeGroupName(const QString &newGroupName);

    /**
     * @brief 修改组id
     * @param 新组id
     */
    void ChangeGroupID(uint32_t newGid);

signals:
    void Changed();

private:
    void addUserToGroupAuthenticated(const QDBusMessage &message, const QString &name);
    void removeUserFromGroupAuthenticated(const QDBusMessage &message, const QString &name);
    void changeGroupNameAuthenticated(const QDBusMessage &message, const QString &newGroupName);
    void changeGroupIDAuthenticated(const QDBusMessage &message, uint32_t newGid);

private:
    void init();

private:
    GroupAdaptor *m_groupAdaptor;
    QString m_objectPath;

    uint32_t m_gid;
    QString m_name;
    bool m_localGroup;
    bool m_primaryGroup;
    QStringList m_groupUsers;
};
}  // namespace Kiran
