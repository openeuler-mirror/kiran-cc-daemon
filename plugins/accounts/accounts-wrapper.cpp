/*
 * @Author       : tangjie02
 * @Date         : 2020-07-23 09:50:11
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-28 20:25:34
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-wrapper.cpp
 */
#include "plugins/accounts/accounts-wrapper.h"

#include <utility>

#include "lib/log.h"

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
        if (spwd == this->spwds_.end())
        {
            passwds_shadows.push_back(std::make_pair(iter->second, nullptr));
        }
        else
        {
            passwds_shadows.push_back(std::make_pair(iter->second, spwd->second));
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
    auto res = getgrouplist(user.c_str(), group, NULL, &ngroups);

    auto groups = g_new(gid_t, ngroups);
    res = getgrouplist(user.c_str(), group, groups, &ngroups);

    return std::vector<uint32_t>(groups, groups + res);
}

void AccountsWrapper::init()
{
    this->passwd_monitor = setup_monitor(PATH_PASSWD, sigc::mem_fun(this, &AccountsWrapper::passwd_changed));
    this->shadow_monitor = setup_monitor(PATH_SHADOW, sigc::mem_fun(this, &AccountsWrapper::shadow_changed));
    this->group_monitor = setup_monitor(PATH_GROUP, sigc::mem_fun(this, &AccountsWrapper::group_changed));
}

Glib::RefPtr<Gio::FileMonitor> AccountsWrapper::setup_monitor(const std::string &path,
                                                              const sigc::slot<void, const Glib::RefPtr<Gio::File> &, const Glib::RefPtr<Gio::File> &, Gio::FileMonitorEvent> &callback)
{
    auto file = Gio::File::create_for_path(path);
    try
    {
        auto monitor = file->monitor_file();
        monitor->signal_changed().connect(callback);
        return monitor;
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("Unable to monitor %s: %s", path, e.what().c_str());
    }

    return Glib::RefPtr<Gio::FileMonitor>();
}

void AccountsWrapper::passwd_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type)
{
    RETURN_IF_TRUE(event_type != Gio::FILE_MONITOR_EVENT_CHANGED && event_type != Gio::FILE_MONITOR_EVENT_CREATED);

    auto fp = fopen(PATH_PASSWD, "r");
    if (fp == NULL)
    {
        LOG_WARNING("Unable to open %s: %s", PATH_PASSWD, g_strerror(errno));
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

    this->file_changed_.emit(FileChangedType::PASSWD_CHANGED);
}

void AccountsWrapper::shadow_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type)
{
    RETURN_IF_TRUE(event_type != Gio::FILE_MONITOR_EVENT_CHANGED && event_type != Gio::FILE_MONITOR_EVENT_CREATED);

    auto fp = fopen(PATH_SHADOW, "r");
    if (fp == NULL)
    {
        LOG_WARNING("Unable to open %s: %s", PATH_SHADOW, g_strerror(errno));
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

    this->file_changed_.emit(FileChangedType::SHADOW_CHANGED);
}

void AccountsWrapper::group_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type)
{
    RETURN_IF_TRUE(event_type != Gio::FILE_MONITOR_EVENT_CHANGED && event_type != Gio::FILE_MONITOR_EVENT_CREATED);

    this->file_changed_.emit(FileChangedType::GROUP_CHANGED);
}

}  // namespace Kiran