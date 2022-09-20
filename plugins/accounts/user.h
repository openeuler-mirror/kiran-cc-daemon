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

#pragma once

#include <user_dbus_stub.h>

#include "accounts-i.h"
#include "plugins/accounts/accounts-wrapper.h"
#include "plugins/accounts/passwd-wrapper.h"
#include "plugins/accounts/user-cache.h"

namespace Kiran
{
class User : public SystemDaemon::Accounts::UserStub, public std::enable_shared_from_this<User>
{
public:
    User() = delete;
    User(PasswdShadow passwd_shadow);
    virtual ~User();

    static std::shared_ptr<User> create_user(PasswdShadow passwd_shadow);

    void dbus_register();
    void dbus_unregister();

    Glib::DBusObjectPathString get_object_path() { return this->object_path_; }

    void freeze_notify();
    void thaw_notify();

    void update_from_passwd_shadow(PasswdShadow passwd_shadow);
    // 移除缓存文件
    void remove_cache_file();
    // 判断认证数据是否存在
    bool match_auth_data(int32_t mode, const std::string &data_id);

public:
    virtual guint64 uid_get() { return this->uid_; };
    virtual guint64 gid_get() { return this->gid_; };
    virtual Glib::ustring user_name_get() { return this->user_name_; };
    virtual Glib::ustring real_name_get() { return this->real_name_; };
    // 参考AccountsAccountType
    virtual gint32 account_type_get() { return this->account_type_; };
    virtual Glib::ustring home_directory_get() { return this->home_directory_; };
    virtual Glib::ustring shell_get() { return this->shell_; };
    virtual Glib::ustring email_get() { return this->user_cache_->get_string(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_EMAIL); };
    virtual Glib::ustring language_get() { return this->user_cache_->get_string(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_LANGUAGE); };
    virtual Glib::ustring session_get() { return this->user_cache_->get_string(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_SESSION); };
    virtual Glib::ustring session_type_get() { return this->user_cache_->get_string(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_SESSION_TYPE); };
    virtual Glib::ustring x_session_get() { return this->user_cache_->get_string(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_XSESSION); };
    virtual Glib::ustring icon_file_get();
    virtual bool locked_get() { return this->locked_; };
    virtual gint32 password_mode_get() { return this->password_mode_; };
    virtual Glib::ustring password_hint_get() { return this->user_cache_->get_string(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_PASSWORD_HINT); };
    virtual bool automatic_login_get() { return this->automatic_login_; };
    virtual bool system_account_get() { return this->system_account_; };
    virtual gint32 auth_modes_get();
    virtual Glib::ustring password_expiration_policy_get() { return this->password_expiration_policy_; }

protected:
    // 设置用户名
    virtual void SetUserName(const Glib::ustring &name, MethodInvocation &invocation);
    //设置用户真实姓名
    virtual void SetRealName(const Glib::ustring &name, MethodInvocation &invocation);
    // 设置用户邮箱
    virtual void SetEmail(const Glib::ustring &email, MethodInvocation &invocation);
    // 设置用户使用语言
    virtual void SetLanguage(const Glib::ustring &language, MethodInvocation &invocation);
    // 设置桌面会话(mate, gnome, etc..)
    virtual void SetXSession(const Glib::ustring &x_session, MethodInvocation &invocation);
    virtual void SetSession(const Glib::ustring &session, MethodInvocation &invocation);
    virtual void SetSessionType(const Glib::ustring &session_type, MethodInvocation &invocation);
    // 设置用户主目录
    virtual void SetHomeDirectory(const Glib::ustring &homedir, MethodInvocation &invocation);
    // 设置用户登陆shell
    virtual void SetShell(const Glib::ustring &shell, MethodInvocation &invocation);
    // 设置用户头像文件路径
    virtual void SetIconFile(const Glib::ustring &filename, MethodInvocation &invocation);
    // 是否锁定用户
    virtual void SetLocked(bool locked, MethodInvocation &invocation);
    // 设置用户类型，分为普通用户和管理员用户，管理员用户的定义可以参考policykit的addAdminRule规则。
    virtual void SetAccountType(gint32 accountType, MethodInvocation &invocation);
    // 设置密码模式，同时会对用户解除锁定
    virtual void SetPasswordMode(gint32 mode, MethodInvocation &invocation);
    // 设置用户密码，同时会对用户解除锁定，密码为加密密码
    virtual void SetPassword(const Glib::ustring &password, const Glib::ustring &hint, MethodInvocation &invocation);
    // 通过passwd命令设置用户密码
    virtual void SetPasswordByPasswd(const Glib::ustring &current_password,
                                     const Glib::ustring &new_password,
                                     MethodInvocation &invocation);
    // 设置用户密码提示
    virtual void SetPasswordHint(const Glib::ustring &hint, MethodInvocation &invocation);
    // 设置用户是否自动登陆
    virtual void SetAutomaticLogin(bool enabled, MethodInvocation &invocation);
    // 设置用户密码过期信息
    virtual void SetPasswordExpirationPolicy(const Glib::ustring &options, MethodInvocation &invocation);
    /**
     * @brief 添加/录入认证数据
     * @param {mode} 参考AccountsPasswordMode定义
     * @param {name} 认证数据的名字 ，例如指纹1、指纹2
     * @param {data_id} 认证数据的唯一标识
     * @return 如果名字已经存在或者mode不支持，则返回错误
     */
    virtual void AddAuthItem(gint32 mode, const Glib::ustring &name, const Glib::ustring &data_id, MethodInvocation &invocation);
    // 删除认证项
    virtual void DelAuthItem(gint32 mode, const Glib::ustring &name, MethodInvocation &invocation);
    // 获取认证项，获取操作不需要加权限控制，否则登陆锁屏界面无法取到指纹数据做验证
    virtual void GetAuthItems(gint32 mode, MethodInvocation &invocation);
    // 开启或关闭认证，如果未开启认证，就算录入了数据也不会使用
    virtual void EnableAuthMode(gint32 mode, bool enabled, MethodInvocation &invocation);

    virtual bool uid_setHandler(guint64 value);
    virtual bool gid_setHandler(guint64 value);
    virtual bool user_name_setHandler(const Glib::ustring &value);
    virtual bool real_name_setHandler(const Glib::ustring &value);
    virtual bool account_type_setHandler(gint32 value);
    virtual bool home_directory_setHandler(const Glib::ustring &value);
    virtual bool shell_setHandler(const Glib::ustring &value);
    virtual bool email_setHandler(const Glib::ustring &value)
    {
        return this->user_cache_->set_value(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_EMAIL, value);
    }
    virtual bool language_setHandler(const Glib::ustring &value)
    {
        return this->user_cache_->set_value(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_LANGUAGE, value);
    };
    virtual bool session_setHandler(const Glib::ustring &value)
    {
        return this->user_cache_->set_value(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_SESSION, value);
    };
    virtual bool session_type_setHandler(const Glib::ustring &value)
    {
        return this->user_cache_->set_value(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_SESSION_TYPE, value);
    };
    virtual bool x_session_setHandler(const Glib::ustring &value)
    {
        return this->user_cache_->set_value(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_XSESSION, value);
    };
    virtual bool icon_file_setHandler(const Glib::ustring &value)
    {
        return this->user_cache_->set_value(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_ICON, value);
    };
    virtual bool password_hint_setHandler(const Glib::ustring &value)
    {
        return this->user_cache_->set_value(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_PASSWORD_HINT, value);
    }

    virtual bool locked_setHandler(bool value);
    virtual bool password_mode_setHandler(gint32 value);
    virtual bool automatic_login_setHandler(bool value);
    virtual bool system_account_setHandler(bool value);
    virtual bool auth_modes_setHandler(gint32 value)
    {
        return this->user_cache_->set_value(KEYFILE_USER_GROUP_NAME, KEYFILE_USER_GROUP_KEY_AUTH_MODES, value);
    }
    virtual bool password_expiration_policy_setHandler(const Glib::ustring &value);

private:
    void init();

    // 这里只更新与缓存数据无关的变量
    void udpate_nocache_var(PasswdShadow passwd_shadow);
    // 获取指定认证模式的认证项
    VPSS get_auth_items(int32_t mode);

    // 根据shadow信息更新密码过期策略
    void update_password_expiration_policy(std::shared_ptr<SPwd> spwd);

    std::string get_auth_action(MethodInvocation &invocation, const std::string &own_action);
    void change_user_name_authorized_cb(MethodInvocation invocation, const Glib::ustring &name);
    void change_real_name_authorized_cb(MethodInvocation invocation, const Glib::ustring &name);
    void change_email_authorized_cb(MethodInvocation invocation, const Glib::ustring &email);
    void change_language_authorized_cb(MethodInvocation invocation, const Glib::ustring &language);
    void change_x_session_authorized_cb(MethodInvocation invocation, const Glib::ustring &x_session);
    void change_session_authorized_cb(MethodInvocation invocation, const Glib::ustring &session);
    void change_session_type_authorized_cb(MethodInvocation invocation, const Glib::ustring &session_type);
    void change_home_dir_authorized_cb(MethodInvocation invocation, const Glib::ustring &home_dir);
    void change_shell_authorized_cb(MethodInvocation invocation, const Glib::ustring &shell);
    void change_icon_file_authorized_cb(MethodInvocation invocation, const Glib::ustring &icon_file);
    void change_locked_authorized_cb(MethodInvocation invocation, bool locked);
    void change_account_type_authorized_cb(MethodInvocation invocation, int32_t account_type);
    void change_password_mode_authorized_cb(MethodInvocation invocation, int32_t password_mode);
    void change_password_authorized_cb(MethodInvocation invocation, const Glib::ustring &password, const Glib::ustring &password_hint);
    void change_password_by_passwd_authorized_cb(MethodInvocation invocation,
                                                 const Glib::ustring &current_password,
                                                 const Glib::ustring &new_password);
    void on_exec_passwd_finished(const std::string &error_desc, MethodInvocation invocation);
    void change_password_hint_authorized_cb(MethodInvocation invocation, const Glib::ustring &password_hint);
    void change_auto_login_authorized_cb(MethodInvocation invocation, bool auto_login);
    void change_password_expiration_policy_cb(MethodInvocation invocation, const Glib::ustring &options);
    void become_user(std::shared_ptr<Passwd> passwd);
    void add_auth_item_authorized_cb(MethodInvocation invocation, int32_t mode, const Glib::ustring &name, const Glib::ustring &data_id);
    void del_auth_item_authorized_cb(MethodInvocation invocation, int32_t mode, const Glib::ustring &name);
    // void get_auth_items_authorized_cb(MethodInvocation invocation, int32_t mode);
    void enable_auth_mode_authorized_cb(MethodInvocation invocation, int32_t mode, bool enabled);
    // 模式转为对应的keyfile的group_name
    std::string mode_to_groupname(int32_t mode);

    bool icon_file_changed(const Glib::ustring &value);
    AccountsAccountType account_type_from_pwent(std::shared_ptr<Passwd> passwd);
    void reset_icon_file();

    void move_extra_data(const std::string &old_name, const std::string &new_name);

private:
private:
    Glib::RefPtr<Gio::DBus::Connection> dbus_connect_;

    // 绑定的passwd和shadow文件
    PasswdShadow passwd_shadow_;

    uint32_t object_register_id_;

    Glib::DBusObjectPathString object_path_;

    std::string default_icon_file_;
    std::shared_ptr<Passwd> passwd_;
    std::shared_ptr<SPwd> spwd_;

    uint64_t uid_;
    uint64_t gid_;
    Glib::ustring user_name_;
    Glib::ustring real_name_;
    int32_t account_type_;
    Glib::ustring home_directory_;
    Glib::ustring shell_;
    bool locked_;
    int32_t password_mode_;
    bool automatic_login_;
    bool system_account_;
    Glib::ustring password_expiration_policy_;

    std::shared_ptr<PasswdWrapper> passwd_wrapper_;

    // 用户缓存数据管理
    std::shared_ptr<UserCache> user_cache_;
};

using UserVec = std::vector<std::shared_ptr<User>>;
}  // namespace Kiran