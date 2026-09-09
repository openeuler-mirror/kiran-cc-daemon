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

#include <grp.h>
#include <pwd.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "lib/base/base.h"

namespace Kiran
{
class GroupEntry
{
public:
    GroupEntry() = delete;
    GroupEntry(struct group *grp);

    std::string name;
    std::string passwd;
    uint64_t gid;
    std::vector<std::string> mem;
    bool local_group;
    bool primary_group;
};

class GroupsWrapper
{
public:
    GroupsWrapper();
    virtual ~GroupsWrapper();

    static GroupsWrapper *get_instance() { return instance_; };
    static void global_init();
    static void global_deinit() { delete instance_; };

    std::map<uint64_t, std::shared_ptr<GroupEntry>> get_groups();
    std::shared_ptr<GroupEntry> get_group_entry_by_id(uint64_t gid);
    std::shared_ptr<GroupEntry> get_group_entry_by_name(const std::string &name);
    bool is_user_exist(const std::string &user_name);

    sigc::signal<void> &signal_groups_changed() { return this->groups_changed_; };

private:
    void init();
    void file_changed(const Glib::RefPtr<Gio::File> &file,
                      const Glib::RefPtr<Gio::File> &other_file,
                      Gio::FileMonitorEvent event_type);
    bool reload_timeout();
    void reload();
    void reload_primary_group();
    void reload_groups();

private:
    static GroupsWrapper *instance_;

    std::map<uint64_t, std::shared_ptr<GroupEntry>> groups_;
    std::set<std::string> pw_users_;

    Glib::RefPtr<Gio::FileMonitor> passwd_monitor_;
    Glib::RefPtr<Gio::FileMonitor> gshadow_monitor_;
    Glib::RefPtr<Gio::FileMonitor> group_monitor_;
    sigc::connection reload_conn_;

    sigc::signal<void> groups_changed_;
};
}  // namespace Kiran
