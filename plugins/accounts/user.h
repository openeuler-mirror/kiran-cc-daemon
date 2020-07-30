/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 13:58:17
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-30 15:43:49
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/user.h
 */
#pragma once

#include <plugins/accounts/accounts-wrapper.h>
#include <user_dbus_stub.h>

namespace Kiran
{
enum class AccountType
{
    ACCOUNT_TYPE_STANDARD,
    ACCOUNT_TYPE_ADMINISTRATOR,
    ACCOUNT_TYPE_LAST
};

enum class PasswordMode
{
    // 正常情况
    PASSWORD_MODE_REGULAR,
    // 登陆时需要设置密码
    PASSWORD_MODE_SET_AT_LOGIN,
    // 设置为无密码登陆
    PASSWORD_MODE_NONE,
    PASSWORD_MODE_LAST
};

class User : public SystemDaemon::Accounts::UserStub, public std::enable_shared_from_this<User>
{
public:
    User() = delete;
    User(uint64_t uid);
    virtual ~User();

    void dbus_register();
    void dbus_unregister();

    Glib::DBusObjectPathString get_object_path() { return this->object_path_; }

    void update_from_passwd_shadow(PasswdShadow passwd_shadow);

    void freeze_notify();
    void thaw_notify();

    void save_cache_file();
    void load_cache_file();
    void remove_cache_file();

public:
    virtual guint64 Uid_get() { return this->uid_; };
    virtual Glib::ustring UserName_get() { return this->user_name_; };
    virtual Glib::ustring RealName_get() { return this->real_name_; };
    virtual gint32 AccountType_get() { return this->account_type_; };
    virtual Glib::ustring HomeDirectory_get() { return this->home_directory_; };
    virtual Glib::ustring Shell_get() { return this->shell_; };
    virtual Glib::ustring Email_get() { return this->email_; };
    virtual Glib::ustring Language_get() { return this->language_; };
    virtual Glib::ustring Session_get() { return this->session_; };
    virtual Glib::ustring SessionType_get() { return this->session_type_; };
    virtual Glib::ustring XSession_get() { return this->xsession_; };
    virtual Glib::ustring IconFile_get() { return this->icon_file_; };
    virtual bool Locked_get() { return this->locked_; };
    virtual gint32 PasswordMode_get() { return this->password_mode_; };
    virtual Glib::ustring PasswordHint_get() { return this->password_hint_; };
    virtual bool AutomaticLogin_get() { return this->automatic_login_; };
    virtual bool SystemAccount_get() { return this->system_account_; };

public:
    bool get_gid() { return this->passwd_->pw_gid; };

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
    // 设置用户密码
    virtual void SetPassword(const Glib::ustring &password, const Glib::ustring &hint, MethodInvocation &invocation);
    // 设置用户密码提示
    virtual void SetPasswordHint(const Glib::ustring &hint, MethodInvocation &invocation);
    // 设置用户是否自动登陆
    virtual void SetAutomaticLogin(bool enabled, MethodInvocation &invocation);
    // 获取用户密码过期信息
    virtual void GetPasswordExpirationPolicy(MethodInvocation &invocation);

    virtual bool Uid_setHandler(guint64 value)
    {
        this->uid_ = value;
        return true;
    };
    virtual bool UserName_setHandler(const Glib::ustring &value)
    {
        this->user_name_ = value;
        return true;
    };
    virtual bool RealName_setHandler(const Glib::ustring &value)
    {
        this->real_name_ = value;
        return true;
    };
    virtual bool AccountType_setHandler(gint32 value)
    {
        this->account_type_ = value;
        return true;
    };
    virtual bool HomeDirectory_setHandler(const Glib::ustring &value)
    {
        this->home_directory_ = value;
        return true;
    };
    virtual bool Shell_setHandler(const Glib::ustring &value)
    {
        this->shell_ = value;
        return true;
    };
    virtual bool Email_setHandler(const Glib::ustring &value)
    {
        this->email_ = value;
        return true;
    };
    virtual bool Language_setHandler(const Glib::ustring &value)
    {
        this->language_ = value;
        return true;
    };
    virtual bool Session_setHandler(const Glib::ustring &value)
    {
        this->session_ = value;
        return true;
    };
    virtual bool SessionType_setHandler(const Glib::ustring &value)
    {
        this->session_type_ = value;
        return true;
    };
    virtual bool XSession_setHandler(const Glib::ustring &value)
    {
        this->xsession_ = value;
        return true;
    };

    virtual bool IconFile_setHandler(const Glib::ustring &value)
    {
        this->icon_file_ = value;
        return true;
    };
    virtual bool Locked_setHandler(bool value)
    {
        this->locked_ = value;
        return true;
    };
    virtual bool PasswordMode_setHandler(gint32 value)
    {
        this->password_mode_ = value;
        return true;
    };
    virtual bool PasswordHint_setHandler(const Glib::ustring &value)
    {
        this->password_hint_ = value;
        return true;
    };
    virtual bool AutomaticLogin_setHandler(bool value)
    {
        this->automatic_login_ = value;
        return true;
    };
    virtual bool SystemAccount_setHandler(bool value)
    {
        this->system_account_ = value;
        return true;
    };

private:
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
    void change_password_hint_authorized_cb(MethodInvocation invocation, const Glib::ustring &password_hint);
    void change_auto_login_authorized_cb(MethodInvocation invocation, bool auto_login);
    void become_user(std::shared_ptr<Passwd> passwd);
    void get_password_expiration_policy_authorized_cb(MethodInvocation invocation);

    AccountType account_type_from_pwent(std::shared_ptr<Passwd> passwd);
    void reset_icon_file();

    void save_to_keyfile(std::shared_ptr<Glib::KeyFile> keyfile);
    void move_extra_data(const std::string &old_name, const std::string &new_name);

private:
    Glib::RefPtr<Gio::DBus::Connection> dbus_connect_;

    uint32_t object_register_id_;

    Glib::DBusObjectPathString object_path_;

    std::string default_icon_file_;
    std::shared_ptr<Passwd> passwd_;
    std::shared_ptr<SPwd> spwd_;

    uint64_t uid_;
    Glib::ustring user_name_;
    Glib::ustring real_name_;
    int32_t account_type_;
    Glib::ustring home_directory_;
    Glib::ustring shell_;
    Glib::ustring email_;
    Glib::ustring language_;
    Glib::ustring session_;
    Glib::ustring session_type_;
    Glib::ustring xsession_;
    Glib::ustring icon_file_;
    bool locked_;
    int32_t password_mode_;
    Glib::ustring password_hint_;
    bool automatic_login_;
    bool system_account_;

    std::shared_ptr<Glib::KeyFile> keyfile_;
};
}  // namespace Kiran