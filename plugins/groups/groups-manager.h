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
#include <QDBusObjectPath>
#include <QObject>
#include <QSharedPointer>

class GroupsAdaptor;

namespace Kiran
{
class Group;
class GroupsWrapper;
class GroupsManager : public QObject,
                      protected QDBusContext
{
    Q_OBJECT

public:
    GroupsManager() = delete;
    GroupsManager(GroupsWrapper *groupsWrapper);
    virtual ~GroupsManager();

    static GroupsManager *getInstance() { return m_instance; };
    static void globalInit(GroupsWrapper *groupsWrapper);
    static void globalDeinit() { delete m_instance; };

    /**
     * @brief 获取系统中所有的用户组
     * @param
     * @return 用户组dbus 路径列表
     */
    QList<QDBusObjectPath> ListCachedGroups();

    /**
     * @brief 通过组名查找用户组
     * @param 组名
     * @return 组对应的QDBusObjectPath对象
     */
    QDBusObjectPath FindGroupByName(const QString &name);

    /**
     * @brief 通过组id查找用户组
     * @param 组id
     * @return 组对应的QDBusObjectPath对象
     */
    QDBusObjectPath FindGroupByID(uint32_t gid);

    /**
     * @brief 创建用户组
     * @param 组名
     * @return 组对应的QDBusObjectPath对象
     */
    QDBusObjectPath CreateGroup(const QString &name);

    /**
     * @brief 删除用户组
     * @param 组id
     */
    void DeleteGroup(uint32_t gid);

protected:
    void reload();

Q_SIGNALS:
    void GroupAdded(const QDBusObjectPath &group);
    void GroupDeleted(const QDBusObjectPath &group);

private:
    void createGroupAuthenticated(const QDBusMessage &message,
                                  const QString &name);

    void deleteGroupAuthenticated(const QDBusMessage &message,
                                  uint32_t gid);

private:
    void init();
    QMap<QString, QSharedPointer<Group>> loadGroups();
    QSharedPointer<Group> findAndCreateGroupByGID(uint32_t gid);
    QSharedPointer<Group> findAndCreateGroupByName(const QString &name);

private:
    static GroupsManager *m_instance;
    GroupsWrapper *m_groupsWrapper;
    GroupsAdaptor *m_adaptor;

    QMap<QString, QSharedPointer<Group>> m_groups;
};
}  // namespace Kiran
