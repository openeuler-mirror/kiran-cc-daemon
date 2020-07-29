/*
 * @Author       : tangjie02
 * @Date         : 2020-07-23 09:50:01
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-29 09:28:06
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-wrapper.h
 */
#pragma once

#include <giomm.h>
#include <grp.h>
#include <pwd.h>
#include <shadow.h>

#include <memory>
#include <string>
#include <vector>

#include "lib/helper.h"

namespace Kiran
{
struct Passwd
{
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

struct SPwd
{
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

struct Group
{
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

    sigc::signal<void, FileChangedType> &signal_file_changed()
    {
        return this->file_changed_;
    };

private:
    void init();

    Glib::RefPtr<Gio::FileMonitor> setup_monitor(const std::string &path,
                                                 const sigc::slot<void, const Glib::RefPtr<Gio::File> &, const Glib::RefPtr<Gio::File> &, Gio::FileMonitorEvent> &callback);

    void passwd_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);
    void shadow_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);
    void group_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);

protected:
    sigc::signal<void, FileChangedType> file_changed_;

private:
    static AccountsWrapper *instance_;

    Glib::RefPtr<Gio::FileMonitor> passwd_monitor;
    Glib::RefPtr<Gio::FileMonitor> shadow_monitor;
    Glib::RefPtr<Gio::FileMonitor> group_monitor;

    std::map<std::string, std::shared_ptr<Passwd>> passwds_;
    std::map<uint64_t, std::weak_ptr<Passwd>> passwds_by_uid_;
    std::map<std::string, std::shared_ptr<SPwd>> spwds_;
};

}  // namespace Kiran