/**
 * @file          /kiran-cc-daemon/plugins/greeter/greeter-dbus.h
 * @brief description
 * @author yangxiaoqing <yangxiaoqing@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
*/
#pragma once

#include <greeter_dbus_stub.h>
#include "plugins/greeter/greeter-manager.h"

namespace Kiran
{
class GreeterDBus : public SystemDaemon::GreeterStub
{
public:
    GreeterDBus();
    virtual ~GreeterDBus();

    static GreeterDBus *get_instance() { return m_instance; }

    static void global_init();
    static void global_deinit() { delete m_instance; }

    /**
     * @brief b返回背景图片.
     * @return
     */
    virtual Glib::ustring background_get() { return this->background_; }
    /**
     * @brief 自动登录用户名，自动登录未开启时为空字符串
     * @return
     */
    virtual Glib::ustring autologin_user_get() { return this->autologin_user_; }
    /**
     * @brief 自动登录延时，以秒为单位
     * @return
     */
    virtual guint64 autologin_timeout_get() { return this->autologin_timeout_; }
    /**
     * @brief 是否允许手动输入用户名登录
     * @return
     */
    virtual bool allow_manual_login_get() { return this->allow_manual_login_; }
    /**
     * @brief 是否隐藏用户列表
     * @return
     */
    virtual bool hide_user_list_get() { return this->hide_user_list_; }
    /**
     * @brief 界面缩放模式，其中0表示自动，1表示手动，2表示禁用。
     * @return
     */
    virtual guint16 scale_mode_get() { return this->scale_mode_; }
    /**
     * @brief 界面缩放比例，该值只有在scaleMode为1时有效
     * @return
     */
    virtual guint16 scale_factor_get() { return this->scale_factor_; }

protected:
    /**
     * @brief SetBackground设置登录界面背景图片路径
     * @param file_path
     * @param invocation
     */
    virtual void SetBackground(const Glib::ustring &file_path, MethodInvocation &invocation);
    /**
     * @brief SetAutologinUser设置自动登录用户的uid
     * @param user_id
     * @param invocation
     */
    virtual void SetAutologinUser(guint64 autologin_user, MethodInvocation &invocation);
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
    virtual bool background_setHandler(const Glib::ustring &value);
    virtual bool autologin_user_setHandler(const Glib::ustring &value);
    virtual bool autologin_timeout_setHandler(guint64 value);
    virtual bool allow_manual_login_setHandler(bool value);
    virtual bool hide_user_list_setHandler(bool value);
    virtual bool scale_mode_setHandler(guint16 value);
    virtual bool scale_factor_setHandler(guint16 value);

private:
    void init();

    /*下面这些函数修改配置文件，并在修改完成后，调用修改属性变量函数*/
    void change_background_file_authorized_cb(MethodInvocation invocation, Glib::ustring file_path);
    void change_auto_login_user_authorized_cb(MethodInvocation invocation, guint64 autologin_user);
    void change_auto_login_timeout_authorized_cb(MethodInvocation invocation, guint64 seconds);
    void change_hide_user_list_authorized_cb(MethodInvocation invocation, bool hide);
    void change_allow_manual_login_authorized_cb(MethodInvocation invocation, bool allow);
    void change_scale_mode_authorized_cb(MethodInvocation invocation, guint16 mode, guint16 factor);

    bool reload_greeter_settings();

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

    /*监听配置文件变化的槽函数*/
    void on_autologin_delay_changed();
    void on_autologin_user_changed();
    void on_background_file_changed();
    void on_enable_manual_login_changed();
    void on_hide_user_list_changed();
    void on_scale_factor_changed();
    void on_scale_mode_changed();
    Glib::ustring uid_to_name(uid_t uid);

private:
    static GreeterDBus *m_instance;
    GreeterManager *m_prefs;

    Glib::ustring background_;
    Glib::ustring autologin_user_;
    uint64_t autologin_timeout_;
    bool allow_manual_login_;
    bool hide_user_list_;
    uint16_t scale_mode_;
    uint16_t scale_factor_;
    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;
    /*定时器*/
    sigc::connection m_reloadConn;

    Glib::RefPtr<Gio::FileMonitor> m_gdmCustomMonitor;
};

}  // namespace Kiran
