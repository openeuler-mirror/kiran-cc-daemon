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

#include "plugins/groups/groups-manager.h"

#include <cinttypes>

#include "groups-i.h"
#include "lib/base/base.h"
#include "lib/dbus/dbus.h"
#include "plugins/groups/groups-util.h"

namespace Kiran
{
GroupsManager::GroupsManager(GroupsWrapper *groups_wrapper) : groups_wrapper_(groups_wrapper),
                                                              dbus_connect_id_(0),
                                                              object_register_id_(0)
{
}

GroupsManager::~GroupsManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

GroupsManager *GroupsManager::instance_ = nullptr;
void GroupsManager::global_init(GroupsWrapper *groups_wrapper)
{
    instance_ = new GroupsManager(groups_wrapper);
    instance_->init();
}

void GroupsManager::init()
{
    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                 GROUPS_DBUS_NAME,
                                                 sigc::mem_fun(this, &GroupsManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &GroupsManager::on_name_acquired),
                                                 sigc::mem_fun(this, &GroupsManager::on_name_lost));

    this->groups_wrapper_->signal_groups_changed().connect(sigc::mem_fun(this, &GroupsManager::reload));
    this->reload();
}

void GroupsManager::reload()
{
    auto new_groups = this->load_groups();

    for (auto iter = this->groups_.begin(); iter != this->groups_.end(); ++iter)
    {
        auto iter2 = new_groups.find(iter->first);
        if (iter2 == new_groups.end())
        {
            this->GroupDeleted_signal.emit(iter->second->get_object_path());
            iter->second->dbus_unregister();
        }
    }

    for (auto iter = new_groups.begin(); iter != new_groups.end(); ++iter)
    {
        auto iter2 = this->groups_.find(iter->first);
        if (iter2 == this->groups_.end())
        {
            iter->second->dbus_register();
            this->GroupAdded_signal.emit(iter->second->get_object_path());
        }
    }

    KLOG_INFO_GROUPS("Update group cache, old size %zu, new size %zu.",
                     this->groups_.size(), new_groups.size());

    this->groups_ = new_groups;
}

std::map<uint64_t, std::shared_ptr<Group>> GroupsManager::load_groups()
{
    auto group_entries = this->groups_wrapper_->get_groups();
    std::map<uint64_t, std::shared_ptr<Group>> groups;

    for (auto iter = group_entries.begin(); iter != group_entries.end(); ++iter)
    {
        std::shared_ptr<Group> group;
        auto old_iter = this->groups_.find(iter->first);
        if (old_iter != this->groups_.end())
        {
            group = old_iter->second;
            group->update_group(*iter->second);
        }
        else
        {
            group = Group::create_group(*iter->second);
        }
        groups.emplace(iter->first, group);
    }
    return groups;
}

std::shared_ptr<Group> GroupsManager::find_and_create_group_by_gid(uint64_t gid)
{
    auto group_entry = this->groups_wrapper_->get_group_entry_by_id(gid);
    if (!group_entry)
    {
        KLOG_WARNING_GROUPS("Unable to lookup group gid %" PRIu64, gid);
        return nullptr;
    }

    auto iter = this->groups_.find(group_entry->gid);
    if (iter != this->groups_.end())
    {
        return iter->second;
    }

    auto group = Group::create_group(*group_entry);
    group->dbus_register();
    this->groups_.emplace(group_entry->gid, group);
    KLOG_INFO_GROUPS("Add new group %s to groups cache.", group->name_get().c_str());
    this->GroupAdded_signal.emit(group->get_object_path());
    return group;
}

std::shared_ptr<Group> GroupsManager::find_and_create_group_by_name(const std::string &name)
{
    auto group_entry = this->groups_wrapper_->get_group_entry_by_name(name);
    if (!group_entry)
    {
        KLOG_WARNING_GROUPS("Unable to lookup group name %s", name.c_str());
        return nullptr;
    }

    auto iter = this->groups_.find(group_entry->gid);
    if (iter != this->groups_.end())
    {
        return iter->second;
    }

    auto group = Group::create_group(*group_entry);
    group->dbus_register();
    this->groups_.emplace(group_entry->gid, group);
    KLOG_INFO_GROUPS("Add new group %s to groups cache.", group->name_get().c_str());
    this->GroupAdded_signal.emit(group->get_object_path());
    return group;
}

void GroupsManager::ListCachedGroups(MethodInvocation &invocation)
{
    std::vector<Glib::DBusObjectPathString> cached_groups;
    for (auto iter = this->groups_.begin(); iter != this->groups_.end(); ++iter)
    {
        if (iter->second)
        {
            cached_groups.push_back(iter->second->get_object_path());
        }
    }
    invocation.ret(cached_groups);
}

void GroupsManager::FindGroupByName(const Glib::ustring &name, MethodInvocation &invocation)
{
    auto group = this->find_and_create_group_by_name(name.raw());
    if (group)
    {
        invocation.ret(group->get_object_path());
    }
    else
    {
        DBUS_ERROR_REPLY(CCErrorCode::ERROR_GROUPS_GROUP_NOT_FOUND_2);
    }
}

void GroupsManager::FindGroupByID(guint64 gid, MethodInvocation &invocation)
{
    auto group = this->find_and_create_group_by_gid(gid);
    if (group)
    {
        invocation.ret(group->get_object_path());
    }
    else
    {
        DBUS_ERROR_REPLY(CCErrorCode::ERROR_GROUPS_GROUP_NOT_FOUND_1);
    }
}

void GroupsManager::CreateGroup(const Glib::ustring &name,
                                const std::vector<Glib::ustring> &users,
                                MethodInvocation &invocation)
{
    std::vector<std::string> user_names;
    for (const auto &user : users)
    {
        user_names.push_back(user.raw());
    }
    KLOG_DEBUG_GROUPS("Create group %s, users: [%s].", name.c_str(), StrUtils::join(user_names, ", ").c_str());

    AuthManager::get_instance()->start_auth_check(AUTH_GROUP_ADMIN,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&GroupsManager::create_group_authorized_cb,
                                                            this,
                                                            std::placeholders::_1,
                                                            name,
                                                            users));
}

void GroupsManager::DeleteGroup(guint64 gid, MethodInvocation &invocation)
{
    KLOG_DEBUG_GROUPS("Delete group with gid %" PRIu64, gid);

    AuthManager::get_instance()->start_auth_check(AUTH_GROUP_ADMIN,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&GroupsManager::delete_group_authorized_cb,
                                                            this,
                                                            std::placeholders::_1,
                                                            gid));
}

void GroupsManager::create_group_authorized_cb(MethodInvocation invocation,
                                               const Glib::ustring &name,
                                               const std::vector<Glib::ustring> &users)
{
    auto group_entry = this->groups_wrapper_->get_group_entry_by_name(name.raw());
    if (group_entry)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GROUPS_GROUP_ALREADY_EXIST);
    }

    std::vector<std::string> user_names;
    for (const auto &user : users)
    {
        user_names.push_back(user.raw());
    }
    KLOG_INFO_GROUPS("Create group %s, users: [%s]", name.c_str(), StrUtils::join(user_names, ", ").c_str());

    std::vector<std::string> argv = {"/usr/sbin/groupadd"};
    if (!user_names.empty())
    {
        argv.push_back("-U");
        argv.push_back(StrUtils::join(user_names, ","));
    }
    argv.push_back("--");
    argv.push_back(name.raw());

    SPAWN_DBUS_WITH_ARGS(invocation, argv);

    auto group = this->find_and_create_group_by_name(name.raw());
    if (group)
    {
        // 创建组时默认为非主组；只有创建用户或 usermod -g 才会改主组并触发 /etc/passwd 变化
        group->local_group_set(true);
        invocation.ret(group->get_object_path());
    }
    else
    {
        DBUS_ERROR_REPLY(CCErrorCode::ERROR_GROUPS_GROUP_NOT_FOUND_3);
    }
}

void GroupsManager::delete_group_authorized_cb(MethodInvocation invocation, uint64_t gid)
{
    auto group_entry = this->groups_wrapper_->get_group_entry_by_id(gid);
    if (!group_entry)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GROUPS_GROUP_NOT_FOUND_4);
    }

    KLOG_INFO_GROUPS("Delete group %s with gid %" PRIu64, group_entry->name.c_str(), gid);

    std::vector<std::string> argv = {"/usr/sbin/groupdel", "--", group_entry->name};
    SPAWN_DBUS_WITH_ARGS(invocation, argv);
    invocation.ret();
}

void GroupsManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    if (!connect)
    {
        KLOG_WARNING_GROUPS("Failed to connect dbus with %s.", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, GROUPS_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_GROUPS("Register object_path %s fail: %s.", GROUPS_OBJECT_PATH, e.what().c_str());
    }
}

void GroupsManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_DEBUG_GROUPS("Success to register dbus name: %s.", name.c_str());
}

void GroupsManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_WARNING_GROUPS("Failed to register dbus name: %s.", name.c_str());
}

}  // namespace Kiran
