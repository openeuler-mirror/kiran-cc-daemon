/**
 * @file greeter-settings-manager.h
 *
 * @brief Kiran登录界面配置管理器
 * @author songchuanfei <songchuanfei@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
 */

#ifndef GREETER_SETTINGS_MANAGER_H
#define GREETER_SETTINGS_MANAGER_H

#include <glibmm.h>
#include <giomm/filemonitor.h>
#include <giomm/file.h>
#include "greeter-settings-data.h"

class GreeterSettingsManager : public sigc::trackable
{
public:

    /**
     * @brief 获取当前登录配置管理器实例，该实例将一直存在，无需调用delete进行释放
     * @return 当前登录配置管理器实例对象
     */
    static GreeterSettingsManager *get_instance();
    ~GreeterSettingsManager();

    /**
     * @brief 加载登录配置，原来的配置信息将被清空。该接口将同时加载lightdm配置文件和greeter配置文件
     * @return 加载成功返回true，失败返回false
     */
    bool load();

    /**
     * @brief 保存登录配置
     * @return 保存成功返回true，失败返回false
     */
    bool save();

    /**
     * @brief   获取当前配置的自动登录的用户名
     * @return  返回获取到的用户名，如果自动登录没有配置，返回空字符串
     */
    std::string get_autologin_user() const;

    /**
     * @brief  获取当前配置的自动登录延时时间，以秒为单位
     * @return 返回自动登录延时时间，如果该时间未配置，则返回0
     */
    uint32_t get_autologin_delay() const;

    /**
     * @brief 获取当前配置的屏幕缩放率，除非当前配置的屏幕缩放模式为SCALING_MANUAL，否则总是返回1
     * @return 当前配置的屏幕缩放率
     */
    uint32_t get_scale_factor() const;

    /**
     * @brief 获取当前配置的屏幕缩放模式
     * @return 当前配置的屏幕缩放模式 @see GreeterScalingMode 
     */
    GreeterScalingMode get_scale_mode() const;

    /**
     * @brief 获取当前配置的登录界面背景图片路径
     * @return 当前配置的背景图片路径，未配置时返回""
     */
    std::string get_background_file() const;

    /**
     * @brief 获取当前配置中是否允许手动输入用户名登录
     * @return 是否允许手动输入用户名登录, true表示允许，false表示不允许
     */
    bool get_enable_manual_login() const;

    /**
     * @brief 获取当前配置是否要隐藏用户列表
     * @return 是否要隐藏用户列表，true表示隐藏，false表示显示
     */
    bool get_hide_user_list() const;

    /**
     * @brief 将autologin_user设置为自动登录用户
     *        该接口不会检查autologin_user对应的用户是否存在。
     * @note  该接口只是将配置写入缓存，要同步到配置文件，需调用 save() 接口
     * 
     * @param[in] autologin_user 自动登录的用户名，不能为空
     * @return (void)
     */
    void set_autologin_user(const std::string &autologin_user);

    /**
     * @brief 设置自动登录延时
     * @note  该接口只是将配置写入缓存，要同步到配置文件，需调用 save() 接口
     * 
     * @param[in] autologin_delay 自动登录延时，单位为秒
     */
    void set_autologin_delay(uint32_t autologin_delay);

    /**
     * @brief 设置是否允许手动输入用户名登录
     * @note  该接口只是将配置写入缓存，要同步到配置文件，需调用 save() 接口
     * 
     * @param[in] enable_manual_login 是否允许手动输入用户名登录，true表示允许，false表示不允许
     */
    void set_enable_manual_login(bool enable_manual_login);

    /**
     * @brief 设置是否隐藏用户列表
     * @note  该接口只是将配置写入缓存，要同步到配置文件，需调用 save() 接口
     * 
     * @param[in] hide_user_list 是否隐藏用户列表，true表示隐藏，false表示不隐藏
     */
    void set_hide_user_list(bool hide_user_list);

    /**
     * @brief 设置界面缩放模式为mode
     * @note  该接口只是将配置写入缓存，要同步到配置文件，需调用 save() 接口
     * 
     * @param[in] mode 界面缩放模式
     *                 @see GreeterScalingMode
     */
    void set_scale_mode(GreeterScalingMode mode);

    /**
     * @brief 设置界面缩放率
     * @note  缩放率设置只有在缩放模式为 SCALING_MANUAL 时才有效，否则将被忽略.
     * @note  该接口只是将配置写入缓存，要同步到配置文件，需调用 save() 接口
     * 
     * @param[in] scale_factor 界面缩放率
     */
    void set_scale_factor(uint32_t scale_factor);

    /**
     * @brief 设置界面背景图片路径.
     * @note  该接口只是将配置写入缓存，要同步到配置文件，需调用 save() 接口
     * 
     * @param[in] greeterBackground 背景图片路径
     */
    void set_background_file(const std::string &background_file);


    /**
     * @brief 信号: 自动登录延时设置发生变化时触发
     */
    sigc::signal<void> signal_autologin_delay_changed();

    /**
     * @brief 信号: 自动登录用户设置发生变化时触发
     */
    sigc::signal<void> signal_autologin_user_changed();

    /**
     * @brief 信号: 屏幕缩放模式设置发生变化时触发
     */
    sigc::signal<void> signal_scale_mode_changed();

    /**
     * @brief 信号: 屏幕缩放比例设置发生变化时触发
     */
    sigc::signal<void> signal_scale_factor_changed();

    /**
     * @brief 信号: 登录界面背景图片发生变化时触发
     */
    sigc::signal<void> signal_background_file_changed();

    /**
     * @brief 信号: 允许手动登录设置发生变化时触发
     */
    sigc::signal<void> signal_enable_manual_login_changed();

    /**
     * @brief 信号：隐藏用户列表设置发生变化时触发
     */
    sigc::signal<void> signal_hide_user_list_changed();

protected:
    /**
     * @brief 初始化登录配置文件监控
     */
    void init_settings_monitor();

    /**
     * @brief 回调函数: 登录配置文件发生变化时调用
     *        @see Gio::FileMonitor::signal_changed()
     * @param file
     * @param other_file
     * @param event_type
     */
    void on_profile_changed(const Glib::RefPtr<Gio::File> &file,
                            const Glib::RefPtr<Gio::File> &other_file,
                            Gio::FileMonitorEvent event_type);

private:
    /* 禁止调用构造函数初始化对象 */
    explicit GreeterSettingsManager();

    /**
     * @brief 从greeter配置文件中读取配置信息
     * @param[out]      data        用于存储配置信息的对象
     * @param[in,out]   settings    用于存储配置文件内容的KeyFile对象，可以为空
     * @return 加载成功返回true，失败返回false
     */
    bool load_greeter_settings(GreeterSettingsData *data,
                               Glib::KeyFile *settings = nullptr);


    /**
     * @brief 从lightdm配置文件中读取配置信息
     * @param[out]      data        用于存储配置信息的对象
     * @param[in,out]   settings    用于存储配置文件内容的KeyFile对象，可以为空
     * @return 加载成功返回true，失败返回false
     */
    bool load_lightdm_settings(GreeterSettingsData *data,
                               Glib::KeyFile *settings = nullptr);

    /**
     * @brief 检查给定的keyfile里组group下是否有名称为key的配置项存在
     * @param settings 待检查的KeyFile
     * @param group    key所在的组名称
     * @param key      配置项名称
     * @return 是否存在，存在返回true,不存在返回false
     */
    bool settings_has_key(Glib::KeyFile *settings, const Glib::ustring &group, const Glib::ustring &key);

private:
    Glib::KeyFile *lightdm_settings, *greeter_settings;                 /* 配置文件 */
    Glib::RefPtr<Gio::FileMonitor> lightdm_monitor, greeter_monitor;    /* 配置文件监视器 */
    Glib::RefPtr<Gio::File> lightdm_conf, greeter_conf;                 /* 配置文件 */
    GreeterSettingsData *priv;                                          /* 配置数据 */


    sigc::signal<void> m_signal_autologin_delay_changed;
    sigc::signal<void> m_signal_autologin_user_changed;
    sigc::signal<void> m_signal_scale_factor_changed;
    sigc::signal<void> m_signal_scale_mode_changed;
    sigc::signal<void> m_signal_background_file_changed;
    sigc::signal<void> m_signal_enable_manual_login_changed;
    sigc::signal<void> m_signal_hide_user_list_changed;

};

#endif // GREETER_SETTINGS_MANAGER_H
