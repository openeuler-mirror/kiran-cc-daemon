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

#include "plugins/accounts/user.h"

#include <fmt/format.h>
#include <gio/gunixinputstream.h>
#include <glib/gstdio.h>
#include <grp.h>
#include <json/json.h>

#include <cinttypes>

#include "lib/base/base.h"
#include "lib/dbus/dbus.h"
#include "plugins/accounts/accounts-manager.h"
#include "plugins/accounts/accounts-util.h"
#include "plugins/accounts/user-classify.h"

namespace Kiran
{
#define USERDIR "/var/lib/AccountsService/users"
#define ICONDIR "/var/lib/AccountsService/icons"
#define ACCOUNTS_USER_OBJECT_PATH "/com/kylinsec/Kiran/SystemDaemon/Accounts/User"

User::User(PasswdShadow passwd_shadow) : passwd_shadow_(passwd_shadow),
                                         object_register_id_(0),
                                         locked_(false),
                                         password_mode_(0),
                                         automatic_login_(0),
                                         system_account_(false)

{
    this->uid_ = this->passwd_shadow_.first->pw_uid;
}

User::~User()
{
    this->dbus_unregister();
}

std::shared_ptr<User> User::create_user(PasswdShadow passwd_shadow)
{
    auto user = std::make_shared<User>(passwd_shadow);
    user->init();
    return user;
}

void User::dbus_register()
{
    KLOG_PROFILE("Uid: %" PRIu64, this->uid_);
    this->object_path_ = fmt::format(ACCOUNTS_USER_OBJECT_PATH "/{0}", this->uid_get());
    try
    {
        this->dbus_connect_ = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SYSTEM);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("failed to get system bus: %s.", e.what().c_str());
        return;
    }

    this->object_register_id_ = this->register_object(this->dbus_connect_, this->object_path_.c_str());
}

void User::dbus_unregister()
{
    KLOG_PROFILE("Uid: %" PRIu64, this->uid_);

    if (this->object_register_id_)
    {
        this->unregister_object();
        this->object_register_id_ = 0;
    }
}

void User::freeze_notify()
{
    KLOG_PROFILE("Uid: %" PRIu64, this->uid_);
    if (this->dbus_connect_)
    {
        this->dbus_connect_->freeze_notify();
    }
}

void User::thaw_notify()
{
    KLOG_PROFILE("Uid: %" PRIu64, this->uid_);
    if (this->dbus_connect_)
    {
        this->dbus_connect_->thaw_notify();
    }
}

#define SET_STR_VALUE_FROM_KEYFILE(key, fun)           \
    {                                                  \
        try                                            \
        {                                              \
            auto s = keyfile->get_string("User", key); \
            this->fun(s);                              \
        }                                              \
        catch (const Glib::KeyFileError &e)            \
        {                                              \
            KLOG_DEBUG("%s", e.what().c_str());        \
        }                                              \
    }

#define SET_BOOL_VALUE_FROM_KEYFILE(key, fun)           \
    {                                                   \
        try                                             \
        {                                               \
            auto b = keyfile->get_boolean("User", key); \
            this->fun(b);                               \
        }                                               \
        catch (const Glib::KeyFileError &e)             \
        {                                               \
            KLOG_DEBUG("%s", e.what().c_str());         \
        }                                               \
    }

void User::remove_cache_file()
{
    auto user_filename = Glib::build_filename(USERDIR, this->user_name_get());
    g_remove(user_filename.c_str());

    auto icon_filename = Glib::build_filename(ICONDIR, this->user_name_get());
    g_remove(icon_filename.c_str());
}

bool User::match_auth_data(int32_t mode, const std::string &data_id)
{
    auto auth_items = this->get_auth_items(mode);
    for (auto auth_item : auth_items)
    {
        RETURN_VAL_IF_TRUE(auth_item.second == data_id, true);
    }
    return false;
}

void User::update_from_passwd_shadow(PasswdShadow passwd_shadow)
{
    this->udpate_nocache_var(passwd_shadow);
    this->reset_icon_file();
}

Glib::ustring User::icon_file_get()
{
    auto cache_filename = this->user_cache_->get_string(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_ICON);
    std::string filename = cache_filename;
    if (!g_file_test(filename.c_str(), G_FILE_TEST_EXISTS))
    {
        // 使用默认目录的图标
        filename = Glib::build_filename(ICONDIR, this->user_name_get());
        if (!g_file_test(filename.c_str(), G_FILE_TEST_EXISTS))
        {
            // 使用用户主目录的图标
            auto home_dir = this->home_directory_get();
            filename = Glib::build_filename(home_dir, ".face");
            if (!g_file_test(filename.c_str(), G_FILE_TEST_EXISTS))
            {
                filename = Glib::ustring();
            }
        }
    }

    // 获取函数中不能有设置操作，所以这里做延时处理
    if (filename != cache_filename)
    {
        auto idle = Glib::MainContext::get_default()->signal_idle();
        idle.connect(sigc::bind(sigc::mem_fun(this, &User::icon_file_changed), filename));
    }

    return filename;
}

gint32 User::auth_modes_get()
{
    auto auth_modes = this->user_cache_->get_int(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_AUTH_MODES);
    // 如果没有设置验证方式，默认使用密码验证
    RETURN_VAL_IF_TRUE(auth_modes == AccountsAuthMode::ACCOUNTS_AUTH_MODE_NONE, AccountsAuthMode::ACCOUNTS_AUTH_MODE_PASSWORD);
    return auth_modes;
}

#define USER_SET_ZERO_PROP_AUTH(fun, callback, auth)                                                            \
    void User::fun(MethodInvocation &invocation)                                                                \
    {                                                                                                           \
        KLOG_PROFILE("");                                                                                       \
        std::string action_id = this->get_auth_action(invocation, auth);                                        \
        RETURN_IF_TRUE(action_id.empty());                                                                      \
                                                                                                                \
        AuthManager::get_instance()->start_auth_check(action_id,                                                \
                                                      TRUE,                                                     \
                                                      invocation.getMessage(),                                  \
                                                      std::bind(&User::callback, this, std::placeholders::_1)); \
                                                                                                                \
        return;                                                                                                 \
    }

#define USER_SET_ONE_PROP_AUTH(fun, callback, auth, type1)                                                             \
    void User::fun(type1 value,                                                                                        \
                   MethodInvocation &invocation)                                                                       \
    {                                                                                                                  \
        KLOG_PROFILE("");                                                                                              \
        std::string action_id = this->get_auth_action(invocation, auth);                                               \
        RETURN_IF_TRUE(action_id.empty());                                                                             \
                                                                                                                       \
        AuthManager::get_instance()->start_auth_check(action_id,                                                       \
                                                      TRUE,                                                            \
                                                      invocation.getMessage(),                                         \
                                                      std::bind(&User::callback, this, std::placeholders::_1, value)); \
                                                                                                                       \
        return;                                                                                                        \
    }

#define USER_SET_TWO_PROP_AUTH(fun, callback, auth, type1, type2)                                                               \
    void User::fun(type1 value1,                                                                                                \
                   type2 value2,                                                                                                \
                   MethodInvocation &invocation)                                                                                \
    {                                                                                                                           \
        KLOG_PROFILE("");                                                                                                       \
        std::string action_id = this->get_auth_action(invocation, auth);                                                        \
        RETURN_IF_TRUE(action_id.empty());                                                                                      \
                                                                                                                                \
        AuthManager::get_instance()->start_auth_check(action_id,                                                                \
                                                      TRUE,                                                                     \
                                                      invocation.getMessage(),                                                  \
                                                      std::bind(&User::callback, this, std::placeholders::_1, value1, value2)); \
                                                                                                                                \
        return;                                                                                                                 \
    }

#define USER_SET_THREE_PROP_AUTH(fun, callback, auth, type1, type2, type3)                                                              \
    void User::fun(type1 value1,                                                                                                        \
                   type2 value2,                                                                                                        \
                   type3 value3,                                                                                                        \
                   MethodInvocation &invocation)                                                                                        \
    {                                                                                                                                   \
        KLOG_PROFILE("");                                                                                                               \
        std::string action_id = this->get_auth_action(invocation, auth);                                                                \
        RETURN_IF_TRUE(action_id.empty());                                                                                              \
                                                                                                                                        \
        AuthManager::get_instance()->start_auth_check(action_id,                                                                        \
                                                      TRUE,                                                                             \
                                                      invocation.getMessage(),                                                          \
                                                      std::bind(&User::callback, this, std::placeholders::_1, value1, value2, value3)); \
                                                                                                                                        \
        return;                                                                                                                         \
    }

USER_SET_ONE_PROP_AUTH(SetUserName, change_user_name_authorized_cb, AUTH_USER_ADMIN, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetRealName, change_real_name_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetEmail, change_email_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetLanguage, change_language_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetXSession, change_x_session_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetSession, change_session_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetSessionType, change_session_type_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetHomeDirectory, change_home_dir_authorized_cb, AUTH_USER_ADMIN, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetShell, change_shell_authorized_cb, AUTH_USER_ADMIN, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetIconFile, change_icon_file_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetLocked, change_locked_authorized_cb, AUTH_USER_ADMIN, bool);
USER_SET_ONE_PROP_AUTH(SetAccountType, change_account_type_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, int32_t);
USER_SET_ONE_PROP_AUTH(SetPasswordMode, change_password_mode_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, int32_t);
USER_SET_TWO_PROP_AUTH(SetPassword, change_password_authorized_cb, AUTH_CHANGE_OWN_PASSWORD, const Glib::ustring &, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetPasswordHint, change_password_hint_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetAutomaticLogin, change_auto_login_authorized_cb, AUTH_USER_ADMIN, bool);
USER_SET_ONE_PROP_AUTH(SetPasswordExpirationPolicy, change_password_expiration_policy_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_THREE_PROP_AUTH(AddAuthItem, add_auth_item_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, int32_t, const Glib::ustring &, const Glib::ustring &);
USER_SET_TWO_PROP_AUTH(DelAuthItem, del_auth_item_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, int32_t, const Glib::ustring &);
// USER_SET_ONE_PROP_AUTH(GetAuthItems, get_auth_items_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, int32_t);
USER_SET_TWO_PROP_AUTH(EnableAuthMode, enable_auth_mode_authorized_cb, AUTH_USER_ADMIN, int32_t, bool);

void User::GetAuthItems(gint32 mode, MethodInvocation &invocation)
{
    KLOG_PROFILE("mdoe: %d.", mode);

    auto auth_items = this->get_auth_items(mode);
    Json::Value auth_items_value;
    Json::FastWriter writer;
    try
    {
        for (uint32_t i = 0; i < auth_items.size(); ++i)
        {
            auth_items_value[i]["name"] = auth_items[i].first;
            auth_items_value[i]["data_id"] = auth_items[i].second;
        }
        auto result = writer.write(auth_items_value);
        invocation.ret(result);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_GET_AUTHITEM_EXCEPTION);
    }
}

void User::init()
{
    this->udpate_nocache_var(this->passwd_shadow_);
    this->user_cache_ = std::make_shared<UserCache>(this->shared_from_this());
    // 由于图标路径是维护在缓存中，所以必须等UserCache对象创建后才能操作
    this->reset_icon_file();
}

void User::udpate_nocache_var(PasswdShadow passwd_shadow)
{
    Glib::ustring real_name;

    // 暂时冻结通知，等更新完数据后再发送通知，避免dbus属性重复发送变化的信号
    this->freeze_notify();
    SCOPE_EXIT({
        this->thaw_notify();
    });

    this->passwd_ = passwd_shadow.first;
    this->spwd_ = passwd_shadow.second;

    if (!this->passwd_->pw_gecos.empty())
    {
        if (Glib::ustring(this->passwd_->pw_gecos).validate())
        {
            real_name = this->passwd_->pw_gecos;
        }
        else
        {
            KLOG_WARNING("User %s has invalid UTF-8 in GECOS field.  It would be a good thing to check /etc/passwd.",
                         this->passwd_->pw_name.c_str() ? this->passwd_->pw_name.c_str() : "");
        }
    }

    this->real_name_set(real_name);
    this->uid_set(this->passwd_->pw_uid);

    auto account_type = this->account_type_from_pwent(this->passwd_);
    this->account_type_set(int32_t(account_type));

    this->user_name_set(this->passwd_->pw_name);
    this->home_directory_set(this->passwd_->pw_dir);
    this->shell_set(this->passwd_->pw_shell);
    // TODO:
    // this->reset_icon_file();

    std::shared_ptr<std::string> passwd;
    bool locked = false;

    if (this->spwd_)
        passwd = this->spwd_->sp_pwdp;

    if (passwd && passwd->length() > 0 && passwd->at(0) == '!')
    {
        locked = true;
    }
    else
    {
        locked = false;
    }

    this->locked_set(locked);

    AccountsPasswordMode mode;

    if (!passwd || !passwd->empty())
    {
        mode = AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_REGULAR;
    }
    else
    {
        mode = AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_NONE;
    }

    if (this->spwd_ && this->spwd_->sp_lstchg == 0)
    {
        mode = AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_SET_AT_LOGIN;
    }

    this->password_mode_set(int32_t(mode));

    auto is_system_account = !UserClassify::is_human(this->passwd_->pw_uid, this->passwd_->pw_name, this->passwd_->pw_shell);
    this->system_account_set(is_system_account);
    this->update_password_expiration_policy(this->spwd_);
}

VPSS User::get_auth_items(int32_t mode)
{
    auto group_name = this->mode_to_groupname(mode);
    RETURN_VAL_IF_TRUE(group_name.empty(), VPSS());
    return this->user_cache_->get_group_kv(group_name);
}

void User::update_password_expiration_policy(std::shared_ptr<SPwd> spwd)
{
    Json::Value values;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "";

    try
    {
        values[ACCOUNTS_PEP_EXPIRATION_TIME] = int64_t(spwd->sp_expire);
        values[ACCOUNTS_PEP_LAST_CHANGED_TIME] = int64_t(spwd->sp_lstchg);
        values[ACCOUNTS_PEP_MIN_DAYS] = int64_t(spwd->sp_min);
        values[ACCOUNTS_PEP_MAX_DAYS] = int64_t(spwd->sp_max);
        values[ACCOUNTS_PEP_DAYS_TO_WARN] = int64_t(spwd->sp_warn);
        values[ACCOUNTS_PEP_INACTIVE_DAYS] = int64_t(spwd->sp_inact);

        auto password_expiration_policy = Json::writeString(wbuilder, values);
        this->password_expiration_policy_set(password_expiration_policy);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
    }
}

std::string User::get_auth_action(MethodInvocation &invocation, const std::string &own_action)
{
    KLOG_PROFILE("own action: %s.", own_action.c_str());
    RETURN_VAL_IF_TRUE(own_action == AUTH_USER_ADMIN, AUTH_USER_ADMIN);

    std::string action_id;
    int32_t uid;

    if (!AccountsUtil::get_caller_uid(invocation.getMessage(), uid))
    {
        DBUS_ERROR_REPLY(CCErrorCode::ERROR_ACCOUNTS_USER_UNKNOWN_CALLER_UID_1);
        return std::string();
    }

    if (this->uid_get() == (uid_t)uid)
    {
        return own_action;
    }
    else
    {
        return AUTH_USER_ADMIN;
    }
}

void User::change_user_name_authorized_cb(MethodInvocation invocation, const Glib::ustring &name)
{
    KLOG_PROFILE("UserName: %s", name.c_str());

    if (this->user_name_get() != name)
    {
        auto old_name = this->user_name_get();

        SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", "-l", name, "--", this->user_name_get().raw());

        this->user_name_set(name);
        this->move_extra_data(old_name, name);
    }

    invocation.ret();
}

void User::change_real_name_authorized_cb(MethodInvocation invocation, const Glib::ustring &name)
{
    KLOG_PROFILE("RealName: %s", name.c_str());
    if (this->real_name_get() != name)
    {
        SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", "-c", name, "--", this->user_name_get().raw());

        this->real_name_set(name);
    }

    invocation.ret();
}

#define USER_AUTH_CHECK_CB(fun, prop)                                       \
    void User::fun(MethodInvocation invocation, const Glib::ustring &value) \
    {                                                                       \
        KLOG_PROFILE(#prop ": %s", value.c_str());                          \
        if (this->prop##_get() != value)                                    \
        {                                                                   \
            this->prop##_set(value);                                        \
        }                                                                   \
                                                                            \
        invocation.ret();                                                   \
    }

USER_AUTH_CHECK_CB(change_email_authorized_cb, email);
USER_AUTH_CHECK_CB(change_language_authorized_cb, language);
USER_AUTH_CHECK_CB(change_x_session_authorized_cb, x_session);
USER_AUTH_CHECK_CB(change_session_authorized_cb, session);
USER_AUTH_CHECK_CB(change_session_type_authorized_cb, session_type);

void User::change_home_dir_authorized_cb(MethodInvocation invocation, const Glib::ustring &home_dir)
{
    KLOG_PROFILE("HomeDir: %s", home_dir.c_str());

    if (this->home_directory_get() != home_dir)
    {
        SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", "-m", "-d", home_dir, "--", this->user_name_get().raw())

        this->home_directory_set(home_dir);
        this->reset_icon_file();
    }
    invocation.ret();
}

void User::change_shell_authorized_cb(MethodInvocation invocation, const Glib::ustring &shell)
{
    KLOG_PROFILE("Shell: %s", shell.c_str());

    if (this->shell_get() != shell)
    {
        SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", "-s", shell, "--", this->user_name_get().raw());
        this->shell_set(shell);
    }
    invocation.ret();
}

void User::become_user(std::shared_ptr<Passwd> passwd)
{
    if (!passwd ||
        initgroups(passwd->pw_name.c_str(), passwd->pw_gid) ||
        setgid(passwd->pw_gid) != 0 ||
        setuid(passwd->pw_uid) != 0)
    {
        exit(1);
    }
}

void User::change_icon_file_authorized_cb(MethodInvocation invocation, const Glib::ustring &icon_file)
{
    KLOG_PROFILE("IconFile: %s", icon_file.c_str());

    auto filename = icon_file;

    do
    {
        if (icon_file.empty())
        {
            auto path = Glib::build_filename(ICONDIR, this->user_name_get());
            g_remove(path.c_str());
            break;
        }

        auto file = Gio::File::create_for_path(icon_file);
        filename = file->get_path();
        Glib::RefPtr<Gio::FileInfo> file_info;
        try
        {
            file_info = file->query_info(G_FILE_ATTRIBUTE_UNIX_MODE "," G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_STANDARD_SIZE);
        }
        catch (const Glib::Error &e)
        {
            KLOG_WARNING("%s.", e.what().c_str());
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_UNKNOWN_CALLER_UID_1);
        }

        if (file_info->get_file_type() != Gio::FileType::FILE_TYPE_REGULAR)
        {
            KLOG_WARNING("File %s is not a regular file.", filename.c_str());
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_FILE_TYPE_NQ_REGULAR);
        }

        auto size = file_info->get_attribute_uint64(G_FILE_ATTRIBUTE_STANDARD_SIZE);
        if (size > 1048576)
        {
            KLOG_WARNING("File %s is too large to be used as an icon", filename.c_str());
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_FILE_SIZE_TOO_BIG);
        }

        // copy file to directory ICONDIR
        int32_t uid;
        if (!AccountsUtil::get_caller_uid(invocation.getMessage(), uid))
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_UNKNOWN_CALLER_UID_2);
        }

        auto dest_path = Glib::build_filename(ICONDIR, this->user_name_get());
        auto dest_file = Gio::File::create_for_path(dest_path);
        Glib::RefPtr<Gio::FileOutputStream> output;
        try
        {
            output = dest_file->replace();
        }
        catch (const Glib::Error &e)
        {
            KLOG_WARNING("Creating file %s failed: %s", dest_path.c_str(), e.what().c_str());
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_REPLACE_OUTPUT_STREAM);
        }

        std::vector<std::string> argv = {"/bin/cat", filename.raw()};
        auto pwent = AccountsWrapper::get_instance()->get_passwd_by_uid(uid);

        int32_t std_out;
        try
        {
            Glib::spawn_async_with_pipes(std::string(),
                                         argv,
                                         Glib::SPAWN_DEFAULT,
                                         sigc::bind(sigc::mem_fun(this, &User::become_user), pwent),
                                         nullptr,
                                         nullptr,
                                         &std_out,
                                         nullptr);
        }
        catch (const Glib::Error &e)
        {
            KLOG_WARNING("%s", e.what().c_str());
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_SPAWN_READ_FILE_FAILED);
        }

        auto input = Glib::wrap(g_unix_input_stream_new(std_out, false));
        gssize bytes = 0;
        try
        {
            bytes = output->splice(input, Gio::OUTPUT_STREAM_SPLICE_CLOSE_TARGET);
        }
        catch (const Glib::Error &e)
        {
            KLOG_WARNING("%s", e.what().c_str());
        }

        if (bytes < 0 || (uint64_t)bytes != size)
        {
            KLOG_WARNING("Failed to Copye file %s to %s", filename.c_str(), dest_path.c_str());
            DBUS_ERROR_REPLY(CCErrorCode::ERROR_ACCOUNTS_USER_COPY_FILE_FAILED);
            IGNORE_EXCEPTION(dest_file->remove());
            return;
        }

        auto mode = file_info->get_attribute_uint32(G_FILE_ATTRIBUTE_UNIX_MODE);
        if ((mode & S_IROTH) == 0)
        {
            filename = dest_path;
        }

    } while (0);

    this->icon_file_set(filename);
    invocation.ret();
}

void User::change_locked_authorized_cb(MethodInvocation invocation, bool locked)
{
    KLOG_PROFILE("Locked: %d", locked);

    if (this->locked_get() != locked)
    {
        SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", locked ? "-L" : "-U", "--", this->user_name_get().raw());
        this->locked_set(locked);
        if (this->automatic_login_get() && locked)
        {
            CCErrorCode error_code = CCErrorCode::SUCCESS;
            AccountsManager::get_instance()->set_automatic_login(this->shared_from_this(), false, error_code);
            this->automatic_login_set(false);
        }
    }
    invocation.ret();
}

void User::change_account_type_authorized_cb(MethodInvocation invocation, int32_t account_type)
{
    KLOG_PROFILE("AccountType: %d", account_type);

    if (this->account_type_get() != account_type)
    {
        auto grp = AccountsWrapper::get_instance()->get_group_by_name(ADMIN_GROUP);
        if (!grp)
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_GROUP_NOT_FOUND);
        }
        auto admin_gid = grp->gr_gid;

        auto groups = AccountsWrapper::get_instance()->get_user_groups(this->user_name_get(), this->get_gid());
        std::string groups_join;
        for (auto i = 0; i < (int)groups.size(); ++i)
        {
            if (groups[i] != admin_gid)
            {
                groups_join += fmt::format("{0}{1}", groups_join.empty() ? std::string() : std::string(","), groups[i]);
            }
        }

        if (account_type == int32_t(AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_ADMINISTRATOR))
        {
            groups_join += fmt::format("{0}{1}", groups_join.empty() ? std::string() : std::string(","), admin_gid);
        }

        SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", "-G", groups_join, "--", this->user_name_get().raw());
        this->account_type_set(account_type);
    }
    invocation.ret();
}

void User::change_password_mode_authorized_cb(MethodInvocation invocation, int32_t password_mode)
{
    KLOG_PROFILE("PasswordMode: %d", password_mode);

    if (this->password_mode_get() != password_mode)
    {
        this->freeze_notify();
        SCOPE_EXIT({
            this->thaw_notify();
        });

        if (password_mode == int32_t(AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_SET_AT_LOGIN) ||
            password_mode == int32_t(AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_NONE))
        {
            SPAWN_WITH_LOGIN_UID(invocation, "/usr/bin/passwd", "-d", "--", this->user_name_get().raw());

            if (password_mode == int32_t(AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_SET_AT_LOGIN))
            {
                SPAWN_WITH_LOGIN_UID(invocation, "/usr/bin/chage", "-d", "0", "--", this->user_name_get().raw());
            }

            this->password_hint_set(std::string());
        }
        else if (this->locked_get())
        {
            SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", "-U", "--", this->user_name_get().raw());
        }
        this->locked_set(false);
        this->password_mode_set(password_mode);
    }

    invocation.ret();
}

void User::change_password_authorized_cb(MethodInvocation invocation, const Glib::ustring &password, const Glib::ustring &password_hint)
{
    KLOG_PROFILE("Password: %s PasswordHint: %s", password.c_str(), password_hint.c_str());

    this->freeze_notify();
    SCOPE_EXIT({
        this->thaw_notify();
    });

    SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", "-p", password.raw(), "--", this->user_name_get().raw());

    this->password_mode_set(int32_t(AccountsPasswordMode::ACCOUNTS_PASSWORD_MODE_REGULAR));
    this->locked_set(false);
    this->password_hint_set(password_hint);
    invocation.ret();
}

USER_AUTH_CHECK_CB(change_password_hint_authorized_cb, password_hint);

void User::change_auto_login_authorized_cb(MethodInvocation invocation, bool auto_login)
{
    KLOG_PROFILE("AutoLogin: %d", auto_login);

    if (this->locked_get())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_IS_LOCKED);
    }
    CCErrorCode error_code;
    if (!AccountsManager::get_instance()->set_automatic_login(this->shared_from_this(), auto_login, error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }
    invocation.ret();
}

void User::change_password_expiration_policy_cb(MethodInvocation invocation, const Glib::ustring &options)
{
    KLOG_DEBUG("options: %s.", options.c_str());

    Json::Value values;
    std::string error;
    std::vector<std::string> argv{"/usr/bin/chage"};

    this->freeze_notify();
    SCOPE_EXIT({
        this->thaw_notify();
    });

    if (!StrUtils::json_str2value(options, values, error))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_PEP_INVALID);
    }

    for (auto key : values.getMemberNames())
    {
        if (!values[key].isInt64())
        {
            KLOG_WARNING("The type of option %s must be Int64", key.c_str());
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_PEP_INVALID);
        }

        auto value = values[key].asInt64();

        switch (shash(key.c_str()))
        {
        case CONNECT(ACCOUNTS_PEP_EXPIRATION_TIME, _hash):
            argv.push_back("-E");
            argv.push_back(fmt::format("{0}", value));
            break;
        case CONNECT(ACCOUNTS_PEP_LAST_CHANGED_TIME, _hash):
            argv.push_back("-d");
            argv.push_back(fmt::format("{0}", value));
            break;
        case CONNECT(ACCOUNTS_PEP_MIN_DAYS, _hash):
            argv.push_back("-m");
            argv.push_back(fmt::format("{0}", value));
            break;
        case CONNECT(ACCOUNTS_PEP_MAX_DAYS, _hash):
            argv.push_back("-M");
            argv.push_back(fmt::format("{0}", value));
            break;
        case CONNECT(ACCOUNTS_PEP_DAYS_TO_WARN, _hash):
            argv.push_back("-W");
            argv.push_back(fmt::format("{0}", value));
            break;
        case CONNECT(ACCOUNTS_PEP_INACTIVE_DAYS, _hash):
            argv.push_back("-I");
            argv.push_back(fmt::format("{0}", value));
            break;
        default:
            KLOG_DEBUG("The option %s is ignored.", key.c_str());
            break;
        }
    }

    // 没有设置任何参数，返回错误
    if (argv.size() <= 1)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_PEP_EMPTY);
    }
    argv.push_back(this->user_name_get());

    CCErrorCode error_code;
    if (!AccountsUtil::spawn_with_login_uid(invocation.getMessage(), argv, error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }

    invocation.ret();
}

void User::add_auth_item_authorized_cb(MethodInvocation invocation,
                                       int32_t mode,
                                       const Glib::ustring &name,
                                       const Glib::ustring &data_id)
{
    KLOG_PROFILE("mdoe: %d, name: %s, data_id: %s.", mode, name.c_str(), data_id.c_str());
    auto group_name = this->mode_to_groupname(mode);

    if (group_name.length() == 0)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_AUTHENTICATION_UNSUPPORTED_2);
    }

    if (this->user_cache_->get_string(group_name, name).length() != 0)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_AUTHMODE_NAME_ALREADY_EXIST);
    }

    if (!this->user_cache_->set_value(group_name, name, data_id))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_AUTH_SAVE_DATA_FAILED);
    }

    invocation.ret();
    this->AuthItemChanged_signal.emit(mode);
}

void User::del_auth_item_authorized_cb(MethodInvocation invocation,
                                       int32_t mode,
                                       const Glib::ustring &name)
{
    KLOG_PROFILE("mdoe: %d, name: %s.", mode, name.c_str());

    auto group_name = this->mode_to_groupname(mode);

    if (group_name.length() == 0)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_AUTHENTICATION_UNSUPPORTED_3);
    }

    if (!this->user_cache_->remove_key(group_name, name))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_AUTH_DEL_DATA_FAILED);
    }

    invocation.ret();
    this->AuthItemChanged_signal.emit(mode);
}

void User::enable_auth_mode_authorized_cb(MethodInvocation invocation, int32_t mode, bool enabled)
{
    KLOG_PROFILE("mode: %d, enabled: %d.", mode, enabled);

    if (mode >= AccountsAuthMode::ACCOUNTS_AUTH_MODE_LAST || mode < 0)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_AUTHENTICATION_UNSUPPORTED_4);
    }

    auto current_mode = this->auth_modes_get();
    if (enabled)
    {
        current_mode = current_mode | mode;
    }
    else
    {
        current_mode = current_mode & (~mode);
    }

    if (!current_mode)
    {
        KLOG_WARNING("All authorization mode is off, the authorization mode will automatically be set to password authorization mode.");
    }

    this->auth_modes_set(current_mode);
    invocation.ret();
}

std::string User::mode_to_groupname(int32_t mode)
{
    switch (mode)
    {
    case AccountsAuthMode::ACCOUNTS_AUTH_MODE_FINGERPRINT:
        return KEYFILE_FINGERPRINT_GROUP_NAME;
    case AccountsAuthMode::ACCOUNTS_AUTH_MODE_FACE:
        return KEYFILE_FACE_GROUP_NAME;
    default:
        break;
    }
    return std::string();
}

#define USER_PROP_SET_HANDLER(prop, type)                              \
    bool User::prop##_setHandler(type value)                           \
    {                                                                  \
        KLOG_PROFILE("value: %s.", fmt::format("{0}", value).c_str()); \
        this->prop##_ = value;                                         \
        return true;                                                   \
    }

USER_PROP_SET_HANDLER(uid, guint64);
USER_PROP_SET_HANDLER(user_name, const Glib::ustring &);
USER_PROP_SET_HANDLER(real_name, const Glib::ustring &);
USER_PROP_SET_HANDLER(account_type, gint32);
USER_PROP_SET_HANDLER(home_directory, const Glib::ustring &);
USER_PROP_SET_HANDLER(shell, const Glib::ustring &);
USER_PROP_SET_HANDLER(locked, bool);
USER_PROP_SET_HANDLER(password_mode, gint32);
USER_PROP_SET_HANDLER(automatic_login, bool);
USER_PROP_SET_HANDLER(system_account, bool);
USER_PROP_SET_HANDLER(password_expiration_policy, const Glib::ustring &);

bool User::icon_file_changed(const Glib::ustring &value)
{
    this->icon_file_set(value);
    return false;
}

AccountsAccountType User::account_type_from_pwent(std::shared_ptr<Passwd> passwd)
{
    g_return_val_if_fail(passwd, AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD);

    struct group *grp;
    gint i;

    if (passwd->pw_uid == 0)
    {
        KLOG_DEBUG("user is root so account type is administrator");
        return AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_ADMINISTRATOR;
    }

    grp = getgrnam(ADMIN_GROUP);
    if (grp == NULL)
    {
        KLOG_DEBUG(ADMIN_GROUP " group not found");
        return AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD;
    }

    for (i = 0; grp->gr_mem[i] != NULL; i++)
    {
        if (g_strcmp0(grp->gr_mem[i], passwd->pw_name.c_str()) == 0)
        {
            return AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_ADMINISTRATOR;
        }
    }

    return AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD;
}

void User::reset_icon_file()
{
    auto icon_file = this->icon_file_get();
    auto home_dir = this->home_directory_get();
    bool icon_is_default = (icon_file.length() == 0 || icon_file == this->default_icon_file_);
    // 更新默认图标路径，因为用户主目录可能已经变化
    this->default_icon_file_ = Glib::build_filename(home_dir, ".face");
    // 如果用户之前未使用图标或者使用的是默认图标，则重新更新路径（主目录可能发生变化）。
    if (icon_is_default)
    {
        this->icon_file_set(this->default_icon_file_);
    }
}

void User::move_extra_data(const std::string &old_name, const std::string &new_name)
{
    auto old_filename = Glib::build_filename(USERDIR, old_name);
    auto new_filename = Glib::build_filename(USERDIR, new_name);
    g_rename(old_filename.c_str(), new_filename.c_str());
}

}  // namespace Kiran