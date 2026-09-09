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

#include "plugins/groups/group.h"

#include <fmt/format.h>

#include <algorithm>
#include <cinttypes>

#include "lib/base/base.h"
#include "lib/dbus/dbus.h"
#include "plugins/groups/groups-util.h"
#include "plugins/groups/groups-wrapper.h"

namespace Kiran
{
#define KIRAN_GROUPS_GROUP_OBJECT_PATH "/com/kylinsec/Kiran/SystemDaemon/Groups/Group"

Group::Group(const GroupEntry &group_entry) : object_register_id_(0),
                                              gid_(group_entry.gid),
                                              name_(group_entry.name),
                                              local_group_(group_entry.local_group),
                                              primary_group_(group_entry.primary_group)
{
    for (const auto &user : group_entry.mem)
    {
        this->group_users_.push_back(user);
    }
}

Group::~Group()
{
    this->dbus_unregister();
}

std::shared_ptr<Group> Group::create_group(const GroupEntry &group_entry)
{
    return std::make_shared<Group>(group_entry);
}

void Group::dbus_register()
{
    this->object_path_ = fmt::format(KIRAN_GROUPS_GROUP_OBJECT_PATH "/{0}", this->gid_);
    try
    {
        this->dbus_connect_ = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SYSTEM);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_GROUPS("Failed to get system bus: %s.", e.what().c_str());
        return;
    }

    this->object_register_id_ = this->register_object(this->dbus_connect_, this->object_path_.c_str());
}

void Group::dbus_unregister()
{
    if (this->object_register_id_)
    {
        this->unregister_object();
        this->object_register_id_ = 0;
    }
}

void Group::update_group(const GroupEntry &group_entry)
{
    if (this->gid_ != group_entry.gid)
    {
        KLOG_WARNING_GROUPS("Can't update group %s, the gid is changed from %" PRIu64 " to %" PRIu64 "!",
                            group_entry.name.c_str(), this->gid_, group_entry.gid);
        return;
    }

    std::vector<Glib::ustring> users;
    for (const auto &user : group_entry.mem)
    {
        users.push_back(user);
    }

    if (this->name_ != group_entry.name ||
        this->group_users_ != users ||
        this->local_group_ != group_entry.local_group ||
        this->primary_group_ != group_entry.primary_group)
    {
        this->name_set(group_entry.name);
        this->users_set(users);
        this->local_group_set(group_entry.local_group);
        this->primary_group_set(group_entry.primary_group);

        this->GroupChanged_signal.emit(this->get_object_path());
    }
}

std::vector<Glib::ustring> Group::users_get()
{
    return this->group_users_;
}

void Group::AddUserToGroup(const Glib::ustring &user, MethodInvocation &invocation)
{
    AuthManager::get_instance()->start_auth_check(AUTH_GROUP_ADMIN,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&Group::add_user_to_group_authorized_cb,
                                                            this,
                                                            std::placeholders::_1,
                                                            user));
}

void Group::RemoveUserFromGroup(const Glib::ustring &user, MethodInvocation &invocation)
{
    AuthManager::get_instance()->start_auth_check(AUTH_GROUP_ADMIN,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&Group::remove_user_from_group_authorized_cb,
                                                            this,
                                                            std::placeholders::_1,
                                                            user));
}

void Group::ChangeGroupName(const Glib::ustring &name, MethodInvocation &invocation)
{
    AuthManager::get_instance()->start_auth_check(AUTH_GROUP_ADMIN,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&Group::change_group_name_authorized_cb,
                                                            this,
                                                            std::placeholders::_1,
                                                            name));
}

void Group::ChangeGroupID(guint64 gid, MethodInvocation &invocation)
{
    AuthManager::get_instance()->start_auth_check(AUTH_GROUP_ADMIN,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&Group::change_group_id_authorized_cb,
                                                            this,
                                                            std::placeholders::_1,
                                                            gid));
}

bool Group::gid_setHandler(guint64 value)
{
    this->gid_ = value;
    return true;
}

bool Group::name_setHandler(const Glib::ustring &value)
{
    this->name_ = value;
    return true;
}

bool Group::local_group_setHandler(bool value)
{
    this->local_group_ = value;
    return true;
}

bool Group::primary_group_setHandler(bool value)
{
    this->primary_group_ = value;
    return true;
}

bool Group::users_setHandler(const std::vector<Glib::ustring> &value)
{
    this->group_users_ = value;
    return true;
}

void Group::add_user_to_group_authorized_cb(MethodInvocation invocation, const Glib::ustring &name)
{
    // 判断用户是否存在
    if (!GroupsWrapper::get_instance()->is_user_exist(name.raw()))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GROUPS_GROUP_USER_NOT_EXIST);
    }
    // 判断用户是否已经在组中
    if (std::find(this->group_users_.begin(), this->group_users_.end(), name) != this->group_users_.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GROUPS_GROUP_USER_ALREADY_IN_GROUP);
    }

    KLOG_INFO_GROUPS("Add user %s to group %s", name.c_str(), this->name_.c_str());

    std::vector<std::string> argv = {"/usr/sbin/groupmems", "-g", this->name_.raw(), "-a", name.raw()};
    SPAWN_DBUS_WITH_ARGS(invocation, argv);
    invocation.ret();
}

void Group::remove_user_from_group_authorized_cb(MethodInvocation invocation, const Glib::ustring &name)
{
    // 判断用户是否存在
    if (!GroupsWrapper::get_instance()->is_user_exist(name.raw()))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GROUPS_GROUP_USER_NOT_EXIST);
    }

    // 判断用户是否在组中
    if (std::find(this->group_users_.begin(), this->group_users_.end(), name) == this->group_users_.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GROUPS_GROUP_USER_NOT_IN_GROUP);
    }

    KLOG_INFO_GROUPS("Remove user %s from group %s", name.c_str(), this->name_.c_str());

    std::vector<std::string> argv = {"/usr/sbin/groupmems", "-g", this->name_.raw(), "-d", name.raw()};
    SPAWN_DBUS_WITH_ARGS(invocation, argv);
    invocation.ret();
}

void Group::change_group_name_authorized_cb(MethodInvocation invocation, const Glib::ustring &new_group_name)
{
    if (new_group_name != this->name_)
    {
        KLOG_INFO_GROUPS("Change group name of %s to %s", this->name_.c_str(), new_group_name.c_str());

        std::vector<std::string> argv = {"/usr/sbin/groupmod", "-n", new_group_name.raw(), "--", this->name_.raw()};
        SPAWN_DBUS_WITH_ARGS(invocation, argv);
    }
    invocation.ret();
}

void Group::change_group_id_authorized_cb(MethodInvocation invocation, guint64 new_gid)
{
    if (new_gid != this->gid_)
    {
        KLOG_INFO_GROUPS("Change group id of %s to %" PRIu64, this->name_.c_str(), new_gid);

        std::vector<std::string> argv = {"/usr/sbin/groupmod", "-g", fmt::format("{0}", new_gid), "--", this->name_.raw()};
        SPAWN_DBUS_WITH_ARGS(invocation, argv);
    }
    invocation.ret();
}

}  // namespace Kiran
