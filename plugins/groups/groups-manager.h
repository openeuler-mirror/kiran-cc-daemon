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

#include <groups_dbus_stub.h>

#include "plugins/groups/group.h"
#include "plugins/groups/groups-wrapper.h"

namespace Kiran
{
class GroupsManager : public SystemDaemon::GroupsStub
{
public:
    GroupsManager() = delete;
    GroupsManager(GroupsWrapper *groups_wrapper);
    virtual ~GroupsManager();

    static GroupsManager *get_instance() { return instance_; };

    static void global_init(GroupsWrapper *groups_wrapper);

    static void global_deinit() { delete instance_; };

protected:
    virtual void ListCachedGroups(MethodInvocation &invocation);
    virtual void FindGroupByID(guint64 gid, MethodInvocation &invocation);
    virtual void FindGroupByName(const Glib::ustring &name, MethodInvocation &invocation);
    virtual void CreateGroup(const Glib::ustring &name,
                             const std::vector<Glib::ustring> &users,
                             MethodInvocation &invocation);
    virtual void DeleteGroup(guint64 gid, MethodInvocation &invocation);

private:
    void init();
    void reload();
    std::map<uint64_t, std::shared_ptr<Group>> load_groups();
    std::shared_ptr<Group> find_and_create_group_by_gid(uint64_t gid);
    std::shared_ptr<Group> find_and_create_group_by_name(const std::string &name);

    void create_group_authorized_cb(MethodInvocation invocation,
                                    const Glib::ustring &name,
                                    const std::vector<Glib::ustring> &users);
    void delete_group_authorized_cb(MethodInvocation invocation, uint64_t gid);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static GroupsManager *instance_;

    GroupsWrapper *groups_wrapper_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;

    std::map<uint64_t, std::shared_ptr<Group>> groups_;
};
}  // namespace Kiran
