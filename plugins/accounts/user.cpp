/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 13:58:22
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-14 14:04:25
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/user.cpp
 */

#include "plugins/accounts/user.h"

#include <fmt/format.h>

#include "lib/log.h"
#include "plugins/accounts/accounts-common.h"

namespace Kiran
{
#define ACCOUNTS_USER_OBJECT_PATH "/com/unikylin/Kiran/SystemDaemon/Accounts/User"

User::User(ActUser *act_user) : dbus_connect_id_(0),
                                object_register_id_(0),
                                act_user_(act_user)
{
    g_object_ref(this->act_user_);

    auto uid = act_user_get_uid(this->act_user_);
    this->object_path_ = fmt::format(ACCOUNTS_USER_OBJECT_PATH "/{0}", uid);
}

User::~User()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
    if (this->object_register_id_)
    {
        this->unregister_object();
    }

    if (this->act_user_)
    {
        g_object_unref(this->act_user_);
    }
}

void User::init()
{
    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                 ACCOUNTS_DBUS_NAME,
                                                 sigc::mem_fun(this, &User::on_bus_acquired),
                                                 sigc::mem_fun(this, &User::on_name_acquired),
                                                 sigc::mem_fun(this, &User::on_name_lost));
}

void User::SetUserName(const Glib::ustring &name,
                       MethodInvocation &invocation)
{
    act_user_set_user_name(this->act_user_, name.c_str());
    invocation.ret();
}

void User::SetRealName(const Glib::ustring &name,
                       MethodInvocation &invocation)
{
    act_user_set_real_name(this->act_user_, name.c_str());
    invocation.ret();
}

void User::SetEmail(const Glib::ustring &email,
                    MethodInvocation &invocation)
{
    act_user_set_email(this->act_user_, email.c_str());
    invocation.ret();
}

void User::SetLanguage(const Glib::ustring &language,
                       MethodInvocation &invocation)
{
    act_user_set_language(this->act_user_, language.c_str());
    invocation.ret();
}

void User::SetXSession(const Glib::ustring &x_session,
                       MethodInvocation &invocation)
{
    act_user_set_x_session(this->act_user_, x_session.c_str());
    invocation.ret();
}

void User::SetSession(const Glib::ustring &session,
                      MethodInvocation &invocation)
{
    act_user_set_session(this->act_user_, session.c_str());
    invocation.ret();
}

void User::SetSessionType(const Glib::ustring &session_type,
                          MethodInvocation &invocation)
{
    act_user_set_session_type(this->act_user_, session_type.c_str());
    invocation.ret();
}

void User::SetLocation(const Glib::ustring &location,
                       MethodInvocation &invocation)
{
    act_user_set_location(this->act_user_, location.c_str());
    invocation.ret();
}

void User::SetHomeDirectory(const Glib::ustring &homedir,
                            MethodInvocation &invocation)
{
    invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_NOT_SUPPORTED), "the function is not supported."));
}

void User::SetShell(const Glib::ustring &shell,
                    MethodInvocation &invocation)
{
    invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_NOT_SUPPORTED), "the function is not supported."));
}

void User::SetIconFile(const Glib::ustring &filename,
                       MethodInvocation &invocation)
{
    act_user_set_icon_file(this->act_user_, filename.c_str());
    invocation.ret();
}

void User::SetLocked(bool locked,
                     MethodInvocation &invocation)
{
    act_user_set_locked(this->act_user_, locked);
    invocation.ret();
}

void User::SetAccountType(gint32 accountType,
                          MethodInvocation &invocation)
{
    act_user_set_account_type(this->act_user_, ActUserAccountType(accountType));
    invocation.ret();
}

void User::SetPasswordMode(gint32 mode,
                           MethodInvocation &invocation)
{
    act_user_set_password_mode(this->act_user_, ActUserPasswordMode(mode));
    invocation.ret();
}

void User::SetPassword(const Glib::ustring &password,
                       const Glib::ustring &hint,
                       MethodInvocation &invocation)
{
    act_user_set_password(this->act_user_, password.c_str(), hint.c_str());
    invocation.ret();
}

void User::SetPasswordHint(const Glib::ustring &hint,
                           MethodInvocation &invocation)
{
    act_user_set_password_hint(this->act_user_, hint.c_str());
    invocation.ret();
}

void User::SetAutomaticLogin(bool enabled,
                             MethodInvocation &invocation)
{
    act_user_set_automatic_login(this->act_user_, enabled);
    invocation.ret();
}

void User::GetPasswordExpirationPolicy(MethodInvocation &invocation)
{
    gint64 expiration_time;
    gint64 last_change_time;
    gint64 min_days_between_changes;
    gint64 max_days_between_changes;
    gint64 days_to_warn;
    gint64 days_after_expiration_until_lock;
    act_user_get_password_expiration_policy(this->act_user_,
                                            &expiration_time,
                                            &last_change_time,
                                            &min_days_between_changes,
                                            &max_days_between_changes,
                                            &days_to_warn,
                                            &days_after_expiration_until_lock);

    invocation.ret(expiration_time,
                   last_change_time,
                   min_days_between_changes,
                   max_days_between_changes,
                   days_to_warn,
                   days_after_expiration_until_lock);
}

bool User::Uid_setHandler(guint64 value)
{
    LOG_WARNING("function is not surpported .");
    return false;
}

guint64 User::Uid_get()
{
    return act_user_get_uid(this->act_user_);
}

bool User::UserName_setHandler(const Glib::ustring &value)
{
    act_user_set_user_name(this->act_user_, value.c_str());
    return true;
}

Glib::ustring User::UserName_get()
{
    return act_user_get_user_name(this->act_user_);
}

bool User::RealName_setHandler(const Glib::ustring &value)
{
    act_user_set_real_name(this->act_user_, value.c_str());
    return true;
}

Glib::ustring User::RealName_get()
{
    return act_user_get_real_name(this->act_user_);
}

bool User::AccountType_setHandler(gint32 value)
{
    act_user_set_account_type(this->act_user_, ActUserAccountType(value));
    return true;
}

gint32 User::AccountType_get()
{
    return act_user_get_account_type(this->act_user_);
}

bool User::HomeDirectory_setHandler(const Glib::ustring &value)
{
    LOG_WARNING("function is not surpported .");
    return false;
}

Glib::ustring User::HomeDirectory_get()
{
    return act_user_get_home_dir(this->act_user_);
}

bool User::Shell_setHandler(const Glib::ustring &value)
{
    LOG_WARNING("function is not surpported .");
    return false;
}

Glib::ustring User::Shell_get()
{
    return act_user_get_shell(this->act_user_);
}

bool User::Email_setHandler(const Glib::ustring &value)
{
    act_user_set_email(this->act_user_, value.c_str());
    return true;
}

Glib::ustring User::Email_get()
{
    return act_user_get_email(this->act_user_);
}

bool User::Language_setHandler(const Glib::ustring &value)
{
    act_user_set_language(this->act_user_, value.c_str());
    return true;
}

Glib::ustring User::Language_get()
{
    return act_user_get_language(this->act_user_);
}

bool User::Session_setHandler(const Glib::ustring &value)
{
    act_user_set_session(this->act_user_, value.c_str());
    return true;
}

Glib::ustring User::Session_get()
{
    return act_user_get_session(this->act_user_);
}

bool User::SessionType_setHandler(const Glib::ustring &value)
{
    act_user_set_session_type(this->act_user_, value.c_str());
    return true;
}

Glib::ustring User::SessionType_get()
{
    return act_user_get_session_type(this->act_user_);
}

bool User::XSession_setHandler(const Glib::ustring &value)
{
    act_user_set_x_session(this->act_user_, value.c_str());
    return true;
}

Glib::ustring User::XSession_get()
{
    return act_user_get_x_session(this->act_user_);
}

bool User::Location_setHandler(const Glib::ustring &value)
{
    act_user_set_location(this->act_user_, value.c_str());
    return true;
}

Glib::ustring User::Location_get()
{
    return act_user_get_location(this->act_user_);
}

bool User::LoginFrequency_setHandler(guint64 value)
{
    LOG_WARNING("function is not surpported .");
    return false;
}

guint64 User::LoginFrequency_get()
{
    return act_user_get_login_frequency(this->act_user_);
}

bool User::LoginTime_setHandler(gint64 value)
{
    LOG_WARNING("function is not surpported .");
    return false;
}

gint64 User::LoginTime_get()
{
    return act_user_get_login_time(this->act_user_);
}

bool User::LoginHistory_setHandler(const std::vector<std::tuple<gint64, gint64, std::map<Glib::ustring, Glib::VariantBase>>> &value)
{
    LOG_WARNING("function is not surpported .");
    return false;
}

std::vector<std::tuple<gint64, gint64, std::map<Glib::ustring, Glib::VariantBase>>> User::LoginHistory_get()
{
    using HistoryElemType = std::tuple<gint64, gint64, std::map<Glib::ustring, Glib::VariantBase>>;
    std::vector<HistoryElemType> login_history;

    Glib::VariantBase base;
    base.init(act_user_get_login_history(this->act_user_), true);

    try
    {
        auto container_base = Glib::VariantBase::cast_dynamic<Glib::VariantContainerBase>(base);

        gsize child_num = container_base.get_n_children();
        for (gsize i = 0; i < child_num; ++i)
        {
            Glib::VariantBase child_base;
            container_base.get_child(child_base, i);

            if (child_base.get_type().get_string() != "(xxa{sv})")
            {
                LOG_WARNING("the element format for login_history must be (xxa{sv}). but now it's %s\n", child_base.get_type().get_string().c_str());
                return std::vector<HistoryElemType>();
            }

            auto entry = Glib::VariantBase::cast_dynamic<Glib::Variant<HistoryElemType>>(child_base);
            auto history = entry.get();

            login_history.push_back(history);
        }
    }
    catch (std::bad_cast &e)
    {
        LOG_WARNING("failed to read login_history: %s\n", e.what());
        return std::vector<HistoryElemType>();
    }

    return login_history;
}

bool User::IconFile_setHandler(const Glib::ustring &value)
{
    act_user_set_icon_file(this->act_user_, value.c_str());
    return true;
}

Glib::ustring User::IconFile_get()
{
    return act_user_get_icon_file(this->act_user_);
}

bool User::Saved_setHandler(bool value)
{
    LOG_WARNING("function is not surpported .");
    return false;
}

bool User::Saved_get()
{
    return act_user_get_saved(this->act_user_);
}

bool User::Locked_setHandler(bool value)
{
    act_user_set_locked(this->act_user_, value);
    return true;
}

bool User::Locked_get()
{
    return act_user_get_locked(this->act_user_);
}

bool User::PasswordMode_setHandler(gint32 value)
{
    act_user_set_password_mode(this->act_user_, ActUserPasswordMode(value));
    return true;
}

gint32 User::PasswordMode_get()
{
    return act_user_get_password_mode(this->act_user_);
}

bool User::PasswordHint_setHandler(const Glib::ustring &value)
{
    act_user_set_password_hint(this->act_user_, value.c_str());
    return true;
}

Glib::ustring User::PasswordHint_get()
{
    return act_user_get_password_hint(this->act_user_);
}

bool User::AutomaticLogin_setHandler(bool value)
{
    act_user_set_automatic_login(this->act_user_, value);
    return true;
}

bool User::AutomaticLogin_get()
{
    return act_user_get_automatic_login(this->act_user_);
}

bool User::SystemAccount_setHandler(bool value)
{
    LOG_WARNING("function is not surpported .");
    return false;
}

bool User::SystemAccount_get()
{
    return act_user_is_system_account(this->act_user_);
}

bool User::LocalAccount_setHandler(bool value)
{
    LOG_WARNING("function is not surpported .");
    return false;
}

bool User::LocalAccount_get()
{
    return act_user_is_local_account(this->act_user_);
}

void User::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    try
    {
        this->object_register_id_ = this->register_object(connect, this->object_path_.c_str());
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("register object_path %s fail: %s.", this->object_path_.c_str(), e.what().c_str());
    }
}

void User::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
}

void User::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
}
}  // namespace Kiran