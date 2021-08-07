/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#pragma once

#include <grp.h>
#include <pwd.h>
#include <shadow.h>

#include <memory>
#include <string>
#include <vector>

#include "lib/base/base.h"

namespace Kiran
{
class Passwd
{
public:
    Passwd() = delete;
    Passwd(struct passwd *passwd)
    {
        RETURN_IF_FALSE(passwd != NULL);

        this->pw_name = POINTER_TO_STRING(passwd->pw_name);
        this->pw_passwd = POINTER_TO_STRING(passwd->pw_passwd);
        this->pw_uid = passwd->pw_uid;
        this->pw_gid = passwd->pw_gid;
        this->pw_gecos = POINTER_TO_STRING(passwd->pw_gecos);
        this->pw_dir = POINTER_TO_STRING(passwd->pw_dir);
        this->pw_shell = POINTER_TO_STRING(passwd->pw_shell);
    }

    std::string pw_name;   /* Username.  */
    std::string pw_passwd; /* Hashed passphrase, if shadow database not in use (see shadow.h).  */
    uint32_t pw_uid;       /* User ID.  */
    uint32_t pw_gid;       /* Group ID.  */
    std::string pw_gecos;  /* Real name.  */
    std::string pw_dir;    /* Home directory.  */
    std::string pw_shell;  /* Shell program.  */
};

class SPwd
{
public:
    SPwd() = delete;
    SPwd(struct spwd *sp)
    {
        RETURN_IF_FALSE(sp != NULL);

        this->sp_namp = POINTER_TO_STRING(sp->sp_namp);
        if (sp->sp_pwdp)
        {
            this->sp_pwdp = std::make_shared<std::string>(sp->sp_pwdp);
        }
        this->sp_lstchg = sp->sp_lstchg;
        this->sp_min = sp->sp_min;
        this->sp_max = sp->sp_max;
        this->sp_warn = sp->sp_warn;
        this->sp_inact = sp->sp_inact;
        this->sp_expire = sp->sp_expire;
        this->sp_flag = sp->sp_flag;
    }
    std::string sp_namp;                  /* Login name.  */
    std::shared_ptr<std::string> sp_pwdp; /* Hashed passphrase.  */
    long int sp_lstchg;                   /* Date of last change.  */
    long int sp_min;                      /* Minimum number of days between changes.  */
    long int sp_max;                      /* Maximum number of days between changes.  */
    long int sp_warn;                     /* Number of days to warn user to change the password.  */
    long int sp_inact;                    /* Number of days the account may be inactive.  */
    long int sp_expire;                   /* Number of days since 1970-01-01 until  account expires.  */
    unsigned long int sp_flag;            /* Reserved.  */
};

class Group
{
public:
    Group() = delete;
    Group(struct group *grp)
    {
        RETURN_IF_FALSE(grp != NULL);

        this->gr_name = POINTER_TO_STRING(grp->gr_name);
        this->gr_passwd = POINTER_TO_STRING(grp->gr_passwd);
        this->gr_gid = grp->gr_gid;
        for (auto pos = grp->gr_mem; pos != NULL && *pos != NULL; ++pos)
        {
            this->gr_mem.push_back(*pos);
        }
    }

    std::string gr_name;             /* Group name.	*/
    std::string gr_passwd;           /* Password.	*/
    uint32_t gr_gid;                 /* Group ID.	*/
    std::vector<std::string> gr_mem; /* Member list.	*/
};

enum class FileChangedType
{
    PASSWD_CHANGED,
    SHADOW_CHANGED,
    GROUP_CHANGED,
    GDM_CHANGED,
};

using PasswdShadow = std::pair<std::shared_ptr<Passwd>, std::shared_ptr<SPwd>>;

class AccountsWrapper
{
    // using FileChangedCallBack = void (Kiran::PasswdWrapper::*)(const Glib::RefPtr<Gio::File> &, const Glib::RefPtr<Gio::File> &, Gio::FileMonitorEvent);

public:
    AccountsWrapper();

    static AccountsWrapper *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    std::vector<PasswdShadow> get_passwds_shadows();

    std::shared_ptr<Passwd> get_passwd_by_name(const std::string &user_name);
    std::shared_ptr<Passwd> get_passwd_by_uid(uint64_t uid);
    std::shared_ptr<SPwd> get_spwd_by_name(const std::string &user_name);
    std::shared_ptr<Group> get_group_by_name(const std::string &group_name);
    std::vector<uint32_t> get_user_groups(const std::string &user, uint32_t group);

    sigc::signal<void, FileChangedType> &signal_file_changed() { return this->file_changed_; };

private:
    void init();

    void passwd_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);
    void shadow_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);
    void group_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);

    void reload_passwd();
    void reload_shadow();

protected:
    sigc::signal<void, FileChangedType> file_changed_;

private:
    static AccountsWrapper *instance_;

    Glib::RefPtr<Gio::FileMonitor> passwd_monitor_;
    Glib::RefPtr<Gio::FileMonitor> shadow_monitor_;
    Glib::RefPtr<Gio::FileMonitor> group_monitor_;

    std::map<std::string, std::shared_ptr<Passwd>> passwds_;
    std::map<uint64_t, std::weak_ptr<Passwd>> passwds_by_uid_;
    std::map<std::string, std::shared_ptr<SPwd>> spwds_;
};

}  // namespace Kiran