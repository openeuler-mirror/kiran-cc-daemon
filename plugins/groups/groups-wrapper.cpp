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

#include "plugins/groups/groups-wrapper.h"

namespace Kiran
{
#define MINIMUM_GID 1000

#define PATH_PASSWD "/etc/passwd"
#define PATH_GSHADOW "/etc/gshadow"
#define PATH_GROUP "/etc/group"

GroupEntry::GroupEntry(struct group *grp)
{
    this->name = POINTER_TO_STRING(grp->gr_name);
    this->gid = grp->gr_gid;
    this->passwd = POINTER_TO_STRING(grp->gr_passwd);
    for (auto pos = grp->gr_mem; pos != NULL && *pos != NULL; ++pos)
    {
        this->mem.push_back(*pos);
    }
    this->local_group = false;
    this->primary_group = false;
}

GroupsWrapper::GroupsWrapper()
{
}

GroupsWrapper::~GroupsWrapper()
{
    if (this->reload_conn_)
    {
        this->reload_conn_.disconnect();
    }
}

GroupsWrapper *GroupsWrapper::instance_ = nullptr;
void GroupsWrapper::global_init()
{
    instance_ = new GroupsWrapper();
    instance_->init();
}

std::map<uint64_t, std::shared_ptr<GroupEntry>> GroupsWrapper::get_groups()
{
    return this->groups_;
}

std::shared_ptr<GroupEntry> GroupsWrapper::get_group_entry_by_id(uint64_t gid)
{
    auto iter = this->groups_.find(gid);
    if (iter != this->groups_.end())
    {
        return iter->second;
    }

    auto grent = getgrgid(gid);
    if (grent != nullptr)
    {
        return std::make_shared<GroupEntry>(grent);
    }
    return nullptr;
}

std::shared_ptr<GroupEntry> GroupsWrapper::get_group_entry_by_name(const std::string &name)
{
    for (auto iter = this->groups_.begin(); iter != this->groups_.end(); ++iter)
    {
        if (iter->second && iter->second->name == name)
        {
            return iter->second;
        }
    }

    auto grent = getgrnam(name.c_str());
    if (grent != nullptr)
    {
        return std::make_shared<GroupEntry>(grent);
    }
    return nullptr;
}

bool GroupsWrapper::is_user_exist(const std::string &user_name)
{
    if (this->pw_users_.find(user_name) != this->pw_users_.end())
    {
        return true;
    }

    if (getpwnam(user_name.c_str()) != nullptr)
    {
        this->pw_users_.insert(user_name);
        return true;
    }
    return false;
}

void GroupsWrapper::init()
{
    this->reload_groups();
    this->reload_primary_group();

    this->passwd_monitor_ = FileUtils::make_monitor_file(PATH_PASSWD, sigc::mem_fun(this, &GroupsWrapper::file_changed));
    this->gshadow_monitor_ = FileUtils::make_monitor_file(PATH_GSHADOW, sigc::mem_fun(this, &GroupsWrapper::file_changed));
    this->group_monitor_ = FileUtils::make_monitor_file(PATH_GROUP, sigc::mem_fun(this, &GroupsWrapper::file_changed));
}

void GroupsWrapper::file_changed(const Glib::RefPtr<Gio::File> &file,
                                 const Glib::RefPtr<Gio::File> &other_file,
                                 Gio::FileMonitorEvent event_type)
{
    RETURN_IF_TRUE(event_type != Gio::FILE_MONITOR_EVENT_CHANGED &&
                   event_type != Gio::FILE_MONITOR_EVENT_CREATED);

    KLOG_INFO_GROUPS("File %s is changed.", file ? file->get_path().c_str() : "");

    if (this->reload_conn_)
    {
        return;
    }

    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    this->reload_conn_ = timeout.connect(sigc::mem_fun(this, &GroupsWrapper::reload_timeout), 100);
}

bool GroupsWrapper::reload_timeout()
{
    this->reload();
    return false;
}

void GroupsWrapper::reload()
{
    this->reload_groups();
    this->reload_primary_group();
    this->groups_changed_.emit();
}

void GroupsWrapper::reload_primary_group()
{
    auto fp = fopen(PATH_PASSWD, "r");
    if (fp == NULL)
    {
        KLOG_WARNING_GROUPS("Unable to open %s: %s", PATH_PASSWD, strerror(errno));
        return;
    }

    this->pw_users_.clear();
    struct passwd *pwent;

    do
    {
        pwent = fgetpwent(fp);
        if (pwent == NULL)
        {
            break;
        }
        this->pw_users_.insert(pwent->pw_name);

        auto grent = getgrgid(pwent->pw_gid);
        if (grent == NULL)
        {
            continue;
        }
        auto iter = this->groups_.find(grent->gr_gid);
        if (iter != this->groups_.end())
        {
            iter->second->primary_group = true;
        }
    } while (pwent != NULL);

    fclose(fp);
}

void GroupsWrapper::reload_groups()
{
    auto fp = fopen(PATH_GROUP, "r");
    if (fp == NULL)
    {
        KLOG_WARNING_GROUPS("Unable to open %s: %s", PATH_GROUP, strerror(errno));
        return;
    }

    this->groups_.clear();
    struct group *grent = nullptr;

    while ((grent = fgetgrent(fp)) != NULL)
    {
        auto group_entry = std::make_shared<GroupEntry>(grent);
        group_entry->local_group = grent->gr_gid >= MINIMUM_GID;
        this->groups_.emplace(group_entry->gid, group_entry);
    }

    KLOG_INFO_GROUPS("Load group information from %s which contains %zu groups.",
                     PATH_GROUP, this->groups_.size());

    fclose(fp);
}

}  // namespace Kiran
