/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "plugins/accounts/accounts-wrapper.h"

#include <utility>

#include "plugins/accounts/accounts-util.h"

namespace Kiran
{
#define PATH_PASSWD "/etc/passwd"
#define PATH_SHADOW "/etc/shadow"
#define PATH_GROUP "/etc/group"

AccountsWrapper::AccountsWrapper()
{
}

AccountsWrapper *AccountsWrapper::instance_ = nullptr;
void AccountsWrapper::global_init()
{
    instance_ = new AccountsWrapper();
    instance_->init();
}

std::vector<PasswdShadow> AccountsWrapper::get_passwds_shadows()
{
    std::vector<PasswdShadow> passwds_shadows;

    for (auto iter = this->passwds_.begin(); iter != this->passwds_.end(); ++iter)
    {
        auto spwd = this->spwds_.find(iter->first);
        // 如果spwd不存在，可能是因为账户正在创建中，这个时候不作为返回值，避免出现处理空指针引起的崩溃问题
        if (spwd != this->spwds_.end())
        {
            passwds_shadows.push_back(std::make_pair(iter->second, spwd->second));
        }
        else
        {
            KLOG_DEBUG_ACCOUNTS("The shadow info isn't found.");
        }
    }

    return passwds_shadows;
}
std::shared_ptr<Passwd> AccountsWrapper::get_passwd_by_name(const std::string &user_name)
{
    auto iter = this->passwds_.find(user_name);
    if (iter != this->passwds_.end())
    {
        return iter->second;
    }

    auto pwent = getpwnam(user_name.c_str());
    if (pwent != NULL)
    {
        return std::make_shared<Passwd>(pwent);
    }

    return nullptr;
}

std::shared_ptr<Passwd> AccountsWrapper::get_passwd_by_uid(uint64_t uid)
{
    auto iter = this->passwds_by_uid_.find(uid);
    if (iter != this->passwds_by_uid_.end() && !iter->second.expired())
    {
        return iter->second.lock();
    }

    auto pwent = getpwuid(uid);
    if (pwent != NULL)
    {
        return std::make_shared<Passwd>(pwent);
    }
    return nullptr;
}

std::shared_ptr<SPwd> AccountsWrapper::get_spwd_by_name(const std::string &user_name)
{
    auto iter = this->spwds_.find(user_name);
    if (iter != this->spwds_.end())
    {
        return iter->second;
    }

    auto spent = getspnam(user_name.c_str());
    if (spent != NULL)
    {
        return std::make_shared<SPwd>(spent);
    }
    return nullptr;
}

std::shared_ptr<Group> AccountsWrapper::get_group_by_name(const std::string &group_name)
{
    auto grp = getgrnam(group_name.c_str());
    if (grp == NULL)
    {
        return nullptr;
    }
    else
    {
        return std::make_shared<Group>(grp);
    }
}

std::vector<uint32_t> AccountsWrapper::get_user_groups(const std::string &user,
                                                       uint32_t group)
{
    int32_t ngroups = 0;
    getgrouplist(user.c_str(), group, NULL, &ngroups);

    auto groups = g_new(gid_t, ngroups);
    auto res = getgrouplist(user.c_str(), group, groups, &ngroups);

    return std::vector<uint32_t>(groups, groups + res);
}

void AccountsWrapper::init()
{
    this->passwd_monitor_ = FileUtils::make_monitor_file(PATH_PASSWD, sigc::mem_fun(this, &AccountsWrapper::passwd_changed));
    this->shadow_monitor_ = FileUtils::make_monitor_file(PATH_SHADOW, sigc::mem_fun(this, &AccountsWrapper::shadow_changed));
    this->group_monitor_ = FileUtils::make_monitor_file(PATH_GROUP, sigc::mem_fun(this, &AccountsWrapper::group_changed));

    this->reload_passwd();
    this->reload_shadow();
}

void AccountsWrapper::passwd_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type)
{
    RETURN_IF_TRUE(event_type != Gio::FILE_MONITOR_EVENT_CHANGED && event_type != Gio::FILE_MONITOR_EVENT_CREATED);

    this->reload_passwd();

    this->file_changed_.emit(FileChangedType::PASSWD_CHANGED);
}

void AccountsWrapper::shadow_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type)
{
    RETURN_IF_TRUE(event_type != Gio::FILE_MONITOR_EVENT_CHANGED && event_type != Gio::FILE_MONITOR_EVENT_CREATED);

    this->reload_shadow();

    this->file_changed_.emit(FileChangedType::SHADOW_CHANGED);
}

void AccountsWrapper::group_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type)
{
    RETURN_IF_TRUE(event_type != Gio::FILE_MONITOR_EVENT_CHANGED && event_type != Gio::FILE_MONITOR_EVENT_CREATED);

    this->file_changed_.emit(FileChangedType::GROUP_CHANGED);
}

void AccountsWrapper::reload_passwd()
{
    auto fp = fopen(PATH_PASSWD, "r");
    if (fp == NULL)
    {
        KLOG_WARNING_ACCOUNTS("Unable to open %s: %s", PATH_PASSWD, g_strerror(errno));
        return;
    }

    this->passwds_.clear();
    this->passwds_by_uid_.clear();
    struct passwd *pwent;

    do
    {
        pwent = fgetpwent(fp);
        if (pwent != NULL)
        {
            auto passwd = std::make_shared<Passwd>(pwent);
            this->passwds_.emplace(passwd->pw_name, passwd);
            this->passwds_by_uid_.emplace(passwd->pw_uid, passwd);
        }

    } while (pwent != NULL);
}
void AccountsWrapper::reload_shadow()
{
    auto fp = fopen(PATH_SHADOW, "r");
    if (fp == NULL)
    {
        KLOG_WARNING_ACCOUNTS("Unable to open %s: %s", PATH_SHADOW, g_strerror(errno));
        return;
    }

    this->spwds_.clear();

    struct
    {
        struct spwd spbuf;
        char buf[1024];
    } shadow_buffer;

    struct spwd *shadow_entry;

    do
    {
        auto ret = fgetspent_r(fp, &shadow_buffer.spbuf, shadow_buffer.buf, sizeof(shadow_buffer.buf), &shadow_entry);
        if (ret == 0 && shadow_entry != NULL)
        {
            auto spwd = std::make_shared<SPwd>(shadow_entry);
            this->spwds_.emplace(spwd->sp_namp, spwd);
        }
        else if (errno != EINTR)
        {
            break;
        }
    } while (shadow_entry != NULL);

    fclose(fp);
}

}  // namespace Kiran