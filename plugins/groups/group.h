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

#include <group_dbus_stub.h>

#include "plugins/groups/groups-wrapper.h"

namespace Kiran
{
class Group : public SystemDaemon::Groups::GroupStub
{
public:
    Group() = delete;
    Group(const GroupEntry &group_entry);
    virtual ~Group();

    static std::shared_ptr<Group> create_group(const GroupEntry &group_entry);

    void dbus_register();
    void dbus_unregister();

    Glib::DBusObjectPathString get_object_path() { return this->object_path_; }

    void update_group(const GroupEntry &group_entry);

public:
    virtual guint64 gid_get() { return this->gid_; };
    virtual Glib::ustring name_get() { return this->name_; };
    virtual bool local_group_get() { return this->local_group_; };
    virtual bool primary_group_get() { return this->primary_group_; };
    virtual std::vector<Glib::ustring> users_get();

protected:
    virtual void AddUserToGroup(const Glib::ustring &user, MethodInvocation &invocation);
    virtual void RemoveUserFromGroup(const Glib::ustring &user, MethodInvocation &invocation);
    virtual void ChangeGroupName(const Glib::ustring &name, MethodInvocation &invocation);
    virtual void ChangeGroupID(guint64 gid, MethodInvocation &invocation);

    virtual bool gid_setHandler(guint64 value);
    virtual bool name_setHandler(const Glib::ustring &value);
    virtual bool local_group_setHandler(bool value);
    virtual bool primary_group_setHandler(bool value);
    virtual bool users_setHandler(const std::vector<Glib::ustring> &value);

private:
    void add_user_to_group_authorized_cb(MethodInvocation invocation, const Glib::ustring &name);
    void remove_user_from_group_authorized_cb(MethodInvocation invocation, const Glib::ustring &name);
    void change_group_name_authorized_cb(MethodInvocation invocation, const Glib::ustring &new_group_name);
    void change_group_id_authorized_cb(MethodInvocation invocation, guint64 new_gid);

private:
    Glib::RefPtr<Gio::DBus::Connection> dbus_connect_;
    uint32_t object_register_id_;
    Glib::DBusObjectPathString object_path_;

    uint64_t gid_;
    Glib::ustring name_;
    bool local_group_;
    bool primary_group_;
    std::vector<Glib::ustring> group_users_;
};

using GroupVec = std::vector<std::shared_ptr<Group>>;
}  // namespace Kiran
