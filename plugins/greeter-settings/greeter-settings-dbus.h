/**
 * @file greeter-settings-dbus-manager.h
 * @brief description
 * @author yangxiaoqing <yangxiaoqing@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
*/
#ifndef GREETERSETTINGSDBUS_H
#define GREETERSETTINGSDBUS_H

#include <greeter-settings_dbus_stub.h>

#include "plugins/greeter-settings/greeter-settings-wrapper.h"
#include "greeter-settings-manager.h"
namespace Kiran
{

enum class FileChangedType
{
    PASSWD_CHANGED,
    SHADOW_CHANGED,
    GROUP_CHANGED,
    GDM_CHANGED,
};

class GreeterSettingsDbus : public SystemDaemon::GreeterSettingsStub, public std::enable_shared_from_this<GreeterSettingsDbus>
{
public:
    GreeterSettingsDbus();
    virtual ~GreeterSettingsDbus();

    static GreeterSettingsDbus *get_instance() { return m_instance; }

    static void global_init();
    static void global_deinit() { delete m_instance; }

    /**
     * @brief backgroundFile_get返回背景图片.
     * @return
     */
    virtual Glib::ustring backgroundFile_get() { return m_backgroundFile; }
    /**
     * @brief autologinUser_get自动登录用户名，自动登录未开启时为空字符串
     * @return
     */
    virtual Glib::ustring autologinUser_get() { return m_autologinUser; }
    /**
     * @brief autologinTimeout_get自动登录延时，以秒为单位
     * @return
     */
    virtual guint64 autologinTimeout_get() { return m_autologinTimeout; }
    /**
     * @brief allowManualLogin_get是否允许手动输入用户名登录
     * @return
     */
    virtual bool allowManualLogin_get() { return m_allowManualLogin; }
    /**
     * @brief hideUserList_get是否隐藏用户列表
     * @return
     */
    virtual bool hideUserList_get() { return m_hideUserList; }
    /**
     * @brief scaleMode_get界面缩放模式，其中0表示自动，1表示手动，2表示禁用。
     * @return
     */
    virtual guint16 scaleMode_get() { return m_scaleMode; }
    /**
     * @brief scaleFactor_get界面缩放比例，该值只有在scaleMode为1时有效
     * @return
     */
    virtual guint16 scaleFactor_get() { return m_scaleFactor; }

protected:
    /**
     * @brief SetBackgroundFile设置登录界面背景图片路径
     * @param file_path
     * @param invocation
     */
    virtual void SetBackgroundFile(const Glib::ustring & file_path, MethodInvocation &invocation);
    /**
     * @brief SetAutologinUser设置自动登录用户的uid
     * @param user_id
     * @param invocation
     */
    virtual void SetAutologinUser( const Glib::ustring & autologin_user, MethodInvocation &invocation);
    /**
     * @brief SetAutologinTimeout置自动登录延时，单位为秒
     * @param seconds
     * @param invocation
     */
    virtual void SetAutologinTimeout(guint64 seconds, MethodInvocation &invocation);
    /**
     * @brief SetHideUserList设置是否隐藏用户列表
     * @param hide
     * @param invocation
     */
    virtual void SetHideUserList(bool hide, MethodInvocation &invocation);
    /**
     * @brief SetAllowManualLogin设置是否允许手动登录
     * @param allow
     * @param invocation
     */
    virtual void SetAllowManualLogin(bool allow, MethodInvocation &invocation);
    /**
     * @brief SetScaleMode设置界面缩放模式
     * @param mode可以为0、1或2，其中0表示自动，1表示手动，2表示禁用。
     * @param factor为缩放比例，1表示100%，2表示200%。Factor参数只有在mode为1时有效。
     * @param invocation
     */
    virtual void SetScaleMode(guint16 mode, guint16 factor, MethodInvocation &invocation);

    /*下面的这些函数只是为获取属性值服务,不修改配置文件*/
    virtual bool backgroundFile_setHandler(const Glib::ustring & value);
    virtual bool autologinUser_setHandler(const Glib::ustring & value);
    virtual bool autologinTimeout_setHandler(guint64 value);
    virtual bool allowManualLogin_setHandler(bool value);
    virtual bool hideUserList_setHandler(bool value);
    virtual bool scaleMode_setHandler(guint16 value);
    virtual bool scaleFactor_setHandler(guint16 value);
    virtual guint64 uid_get() { return -1/*this->uid_*/; }

private:
    void init();

    void change_background_file_authorized_cb(MethodInvocation invocation, const Glib::ustring &file_path);
    void change_auto_login_user_authorized_cb(MethodInvocation invocation, const Glib::ustring &autologin_user);
    void change_auto_login_timeout_authorized_cb(MethodInvocation invocation, const guint64 &seconds);
    void change_hide_user_list_authorized_cb(MethodInvocation invocation, const bool &hide);
    void change_allow_manual_login_authorized_cb(MethodInvocation invocation, const bool &allow);
    void change_scale_mode_authorized_cb(MethodInvocation invocation, const guint16 &mode, const guint16 &factor);

    bool reload_greeter_settings();

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    bool get_caller_uid(Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, int32_t &uid);

    void on_autologin_delay_changed();
    void on_autologin_user_changed();
    void on_background_file_changed();
    void on_enable_manual_login_changed();
    void on_hide_user_list_changed();
    void on_scale_factor_changed();
    void on_scale_mode_changed();

private:
    static GreeterSettingsDbus *m_instance;
    GreeterSettingsManager *m_prefs;

    Glib::ustring m_backgroundFile;
    Glib::ustring m_autologinUser;
    uint64_t m_autologinTimeout;
    bool m_allowManualLogin;
    bool m_hideUserList;
    uint16_t m_scaleMode;
    uint16_t m_scaleFactor;
    uint32_t m_dbusConnectId;
    uint32_t m_objectRegisterId;
    /*定时器*/
    sigc::connection m_reloadConn;

    Glib::RefPtr<Gio::FileMonitor> m_gdmCustomMonitor;
};

}

#endif // GREETERSETTINGSDBUS_H
