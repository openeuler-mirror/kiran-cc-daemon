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

#include "plugins/display/display-manager.h"

#include <fstream>

#include <glib/gi18n.h>
#include "config.h"
#include "lib/base/base.h"
#include "plugins/display/display-util.h"
#include "xsettings-i.h"

namespace Kiran
{
#define DISPLAY_SCHEMA_ID "com.kylinsec.kiran.display"
#define DISPLAY_SCHEMA_STYLE "display-style"
#define DISPLAY_SCHEMA_DYNAMIC_SCALING_WINDOW "dynamic-scaling-window"
#define DISPLAY_SCHEMA_MAX_SCREEN_RECORD_NUMBER "max-screen-record-number"
#define SCREEN_CHANGED_ADAPT "screen-changed-adaptation"

#define DISPLAY_CONF_DIR "kylinsec/" PROJECT_NAME "/display"
#define DISPLAY_FILE_NAME "display.xml"

#define MONITOR_JOIN_CHAR ","
#define XRANDR_CMD "xrandr"

#define DEFAULT_MAX_SCREEN_RECORD_NUMBER 100

DisplayManager::DisplayManager(XrandrManager *xrandr_manager) : xrandr_manager_(xrandr_manager),
                                                                default_style_(DisplayStyle::DISPLAY_STYLE_EXTEND),
                                                                window_scaling_factor_(0),
                                                                dynamic_scaling_window_(false),
                                                                max_screen_record_number_(DEFAULT_MAX_SCREEN_RECORD_NUMBER),
                                                                dbus_connect_id_(0),
                                                                object_register_id_(0)
{
    this->config_file_path_ = Glib::build_filename(Glib::get_user_config_dir(),
                                                   DISPLAY_CONF_DIR,
                                                   DISPLAY_FILE_NAME);

    this->display_settings_ = Gio::Settings::create(DISPLAY_SCHEMA_ID);
    this->xsettings_settings_ = Gio::Settings::create(XSETTINGS_SCHEMA_ID);
}

DisplayManager::~DisplayManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

DisplayManager *DisplayManager::instance_ = nullptr;
void DisplayManager::global_init(XrandrManager *xrandr_manager)
{
    instance_ = new DisplayManager(xrandr_manager);
    instance_->init();
}

DisplayMonitorVec DisplayManager::get_connected_monitors()
{
    DisplayMonitorVec monitors;
    for (const auto &iter : this->monitors_)
    {
        if (iter.second->connected_get())
        {
            monitors.push_back(iter.second);
        }
    }
    return monitors;
}

DisplayMonitorVec DisplayManager::get_enabled_monitors()
{
    DisplayMonitorVec monitors;
    for (const auto &iter : this->monitors_)
    {
        if (iter.second->enabled_get())
        {
            monitors.push_back(iter.second);
        }
    }
    return monitors;
}

void DisplayManager::ListMonitors(MethodInvocation &invocation)
{
    std::vector<Glib::ustring> object_paths;
    for (const auto &iter : this->monitors_)
    {
        object_paths.push_back(iter.second->get_object_path());
    }
    invocation.ret(object_paths);
}

void DisplayManager::SwitchStyle(guint32 style, MethodInvocation &invocation)
{
    CCErrorCode error_code = CCErrorCode::SUCCESS;
    if (!this->switch_style(DisplayStyle(style), error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }
    invocation.ret();
}

void DisplayManager::SetDefaultStyle(guint32 style, MethodInvocation &invocation)
{
    if (style >= DisplayStyle::DISPLAY_STYLE_LAST)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_UNKNOWN_DISPLAY_STYLE_2);
    }
    this->default_style_set(style);
    invocation.ret();
}

void DisplayManager::ApplyChanges(MethodInvocation &invocation)
{
    CCErrorCode error_code = CCErrorCode::SUCCESS;
    if (!this->apply(error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }
    invocation.ret();
}

void DisplayManager::RestoreChanges(MethodInvocation &invocation)
{
    CCErrorCode error_code = CCErrorCode::SUCCESS;
    if (!this->switch_style(this->default_style_, error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }
    invocation.ret();
}

void DisplayManager::SetPrimary(const Glib::ustring &name, MethodInvocation &invocation)
{
    KLOG_DEBUG_DISPLAY("Set primary name to: %s.", name.c_str());

    if (name.length() == 0)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_PRIMARY_MONITOR_IS_EMPTY);
    }

    if (!this->get_monitor_by_name(name))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_NOTFOUND_PRIMARY_MONITOR_BY_NAME);
    }

    this->primary_set(name);
    invocation.ret();
}

void DisplayManager::Save(MethodInvocation &invocation)
{
    CCErrorCode error_code = CCErrorCode::SUCCESS;

    if (!this->save_config(error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }

    invocation.ret();
}

void DisplayManager::SetWindowScalingFactor(gint32 window_scaling_factor, MethodInvocation &invocation)
{
    if (this->window_scaling_factor_get() == window_scaling_factor)
    {
        invocation.ret();
        return;
    }

    if (!this->dynamic_scaling_window_)
    {
        std::string standard_error;
        auto command_line = fmt::format("/usr/bin/notify-send \"{0}\"", _("The scaling rate can only take effect after logging out and logging in again"));
        Glib::spawn_command_line_sync(command_line, nullptr, &standard_error);
        if (!standard_error.empty())
        {
            KLOG_WARNING_DISPLAY("Failed to run notify-send: %s", standard_error.c_str());
        }
    }

    if (!this->window_scaling_factor_set(window_scaling_factor))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_SET_WINDOW_SCALING_FACTOR_2);
    }

    invocation.ret();
}

bool DisplayManager::default_style_setHandler(guint32 value)
{
    RETURN_VAL_IF_TRUE(this->default_style_ == DisplayStyle(value), true);

    this->default_style_ = DisplayStyle(value);

    if (this->display_settings_->get_enum(DISPLAY_SCHEMA_STYLE) != int32_t(this->default_style_))
    {
        this->display_settings_->set_enum(DISPLAY_SCHEMA_STYLE, int32_t(this->default_style_));
    }
    return true;
}

bool DisplayManager::primary_setHandler(const Glib::ustring &value)
{
    this->primary_ = value.raw();
    return true;
}

bool DisplayManager::window_scaling_factor_setHandler(gint32 value)
{
    this->window_scaling_factor_ = value;
    return true;
}

void DisplayManager::init()
{
    this->load_settings();
    this->load_monitors();
    this->load_config();

    this->display_settings_->signal_changed().connect(sigc::mem_fun(this, &DisplayManager::display_settings_changed));
    this->xrandr_manager_->signal_resources_changed().connect(sigc::mem_fun(this, &DisplayManager::resources_changed));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 DISPLAY_DBUS_NAME,
                                                 sigc::mem_fun(this, &DisplayManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &DisplayManager::on_name_acquired),
                                                 sigc::mem_fun(this, &DisplayManager::on_name_lost));

    CCErrorCode error_code = CCErrorCode::SUCCESS;
    if (!this->switch_style_and_save(this->default_style_, error_code))
    {
        KLOG_WARNING_DISPLAY("%s.", CC_ERROR2STR(error_code).c_str());
    }

    /* window_scaling_factor的初始化顺序：
       1. 先读取xsettings中的window-scaling-factor属性; (load_settings)
       2. 读取monitor.xml中维护的window-scaling-factor值 （switch_style_and_save）
       3. 如果第2步和第1步的值不相同，则说明在上一次进入会话时用户修改了缩放率，需要在这一次进入会话时生效，
          因此需要将monitor.xml中的缩放率更新到xsettings中的window-scaling-factor属性中*/
    if (this->window_scaling_factor_ != this->xsettings_settings_->get_int(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR))
    {
        this->xsettings_settings_->set_int(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, this->window_scaling_factor_);
    }
}

void DisplayManager::load_settings()
{
    this->default_style_ = DisplayStyle(this->display_settings_->get_enum(DISPLAY_SCHEMA_STYLE));
    this->dynamic_scaling_window_ = this->display_settings_->get_boolean(DISPLAY_SCHEMA_DYNAMIC_SCALING_WINDOW);
    this->window_scaling_factor_ = this->xsettings_settings_->get_int(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR);
    this->max_screen_record_number_ = this->display_settings_->get_int(DISPLAY_SCHEMA_MAX_SCREEN_RECORD_NUMBER);
}

void DisplayManager::load_monitors()
{
    // 加载主显示器
    auto primary_output = this->xrandr_manager_->get_primary_output();
    auto primary_name = primary_output ? primary_output->name : std::string();
    this->primary_set(primary_name);

    // 删除已经不存在的monitor
    for (auto iter = this->monitors_.begin(); iter != this->monitors_.end();)
    {
        auto output = this->xrandr_manager_->get_output(iter->first);
        if (!output || !output->connection)
        {
            iter->second->dbus_unregister();
            this->monitors_.erase(iter++);
        }
        else
        {
            ++iter;
        }
    }

    auto outputs = this->xrandr_manager_->get_connected_outputs();
    for (const auto &output : outputs)
    {
        auto uid = this->xrandr_manager_->gen_uid(output);

        MonitorInfo monitor_info;
        monitor_info.id = output->id;
        monitor_info.uid = uid;
        monitor_info.name = output->name;
        monitor_info.connected = output->connection;
        monitor_info.modes = output->modes;
        monitor_info.npreferred = output->npreferred;

        auto crtc = this->xrandr_manager_->get_crtc(output->crtc);
        if (!crtc)
        {
            monitor_info.enabled = false;
        }
        else
        {
            monitor_info.enabled = true;
            monitor_info.x = crtc->x;
            monitor_info.y = crtc->y;
            monitor_info.rotation = DisplayRotationType(crtc->rotation & ROTATION_ALL_MASK);
            monitor_info.reflect = DisplayReflectType(crtc->rotation & REFLECT_ALL_MASK);
            monitor_info.rotations = this->xrandr_manager_->get_rotations(crtc);
            monitor_info.reflects = this->xrandr_manager_->get_reflects(crtc);
            monitor_info.mode = crtc->mode;
        }

        auto iter = this->monitors_.find(output->id);
        if (iter == this->monitors_.end())
        {
            auto monitor = std::make_shared<DisplayMonitor>(monitor_info);
            this->monitors_.emplace(output->id, monitor);
            monitor->dbus_register();
        }
        else
        {
            iter->second->update(monitor_info);
        }
    }
}

void DisplayManager::load_config()
{
    if (Glib::file_test(this->config_file_path_, Glib::FILE_TEST_EXISTS))
    {
        try
        {
            this->display_config_ = display(this->config_file_path_, xml_schema::Flags::dont_validate);
        }
        catch (const xml_schema::Exception &e)
        {
            KLOG_WARNING_DISPLAY("Failed to load config file: %s: %s", this->config_file_path_.c_str(), e.what());
            this->display_config_ = nullptr;
        }
    }
    else
    {
        KLOG_DEBUG_DISPLAY("Displaymanager load config file %s is not exist.", this->config_file_path_.c_str());
    }
    return;
}

bool DisplayManager::apply_config(CCErrorCode &error_code)
{
    if (!this->display_config_)
    {
        error_code = CCErrorCode::ERROR_DISPLAY_CONFIG_IS_EMPTY;
        return false;
    }

    auto monitors_id = this->get_monitors_uid();
    const auto &screens = this->display_config_->screen();
    bool result = false;

    for (const auto &screen : screens)
    {
        const auto &monitors = screen.monitor();
        auto monitors_config_id = this->get_c_monitors_uid(monitors);
        if (monitors_id == monitors_config_id)
        {
            KLOG_DEBUG_DISPLAY("Match ids and the ids is %s.", monitors_id.c_str());
            if (this->apply_screen_config(screen, error_code))
            {
                result = true;
                break;
            }
        }
    }

    if (!result && error_code == CCErrorCode::SUCCESS)
    {
        error_code = CCErrorCode::ERROR_DISPLAY_CONFIG_ITEM_NOTFOUND;
    }
    return result;
}

bool DisplayManager::apply_screen_config(const ScreenConfigInfo &screen_config, CCErrorCode &error_code)
{
    const auto &c_monitors = screen_config.monitor();

    this->primary_set(screen_config.primary());
    this->window_scaling_factor_set(screen_config.window_scaling_factor());

    for (const auto &c_monitor : c_monitors)
    {
        auto monitor = this->match_best_monitor(c_monitor.uid(), c_monitor.name());

        if (!monitor)
        {
            KLOG_WARNING_DISPLAY("Cannot find monitor for uid=%s, name=%s.",
                                 c_monitor.uid().c_str(),
                                 c_monitor.name().c_str());
            return false;
        }

        /* 一般情况下uid相同时name也是相同的，但是有些特殊情况会出现不一样，这里uid主要是为了唯一标识一台显示器，
           而name是用来区分显示器接口的，比如有一台显示器最开始是接入在HDMI-1，后面改到HDMI-2了，那么在能获取到edid的
           情况下uid是不变的，但是name会发生变化。如果出现name不一样的情况下这里仅仅记录日志，方便后续跟踪问题。*/
        if (c_monitor.name() != monitor->name_get())
        {
            KLOG_DEBUG_DISPLAY("The monitor name is dismatch. config name: %s, monitor name: %s.",
                               c_monitor.name().c_str(),
                               monitor->name_get().c_str());
        }

        if (!c_monitor.enabled())
        {
            monitor->enabled_set(false);
            monitor->x_set(0);
            monitor->y_set(0);
            monitor->rotation_set(uint16_t(DisplayRotationType::DISPLAY_ROTATION_0));
            monitor->reflect_set(uint16_t(DisplayReflectType::DISPLAY_REFLECT_NORMAL));
            monitor->current_mode_set(0);
        }
        else
        {
            // 只有在显示器开启状态下才能取匹配mode，因为显示器关闭状态下c_monitor里面保持的分辨率都是0x0
            auto mode = monitor->match_best_mode(c_monitor.width(), c_monitor.height(), c_monitor.refresh_rate());
            if (!mode)
            {
                KLOG_WARNING_DISPLAY("Cannot match the mode. width: %d, height: %d, refresh: %.2f.",
                                     c_monitor.width(),
                                     c_monitor.height(),
                                     c_monitor.refresh_rate());
                return false;
            }

            monitor->enabled_set(true);
            monitor->x_set(c_monitor.x());
            monitor->y_set(c_monitor.y());
            monitor->rotation_set(uint16_t(DisplayUtil::str_to_rotation(c_monitor.rotation())));
            monitor->reflect_set(uint16_t(DisplayUtil::str_to_reflect(c_monitor.reflect())));
            monitor->current_mode_set(mode->id);
        }
    }

    return true;
}

void DisplayManager::fill_screen_config(ScreenConfigInfo &screen_config)
{
    screen_config.timestamp((uint32_t)time(NULL));
    screen_config.primary(this->primary_);
    screen_config.window_scaling_factor(this->window_scaling_factor_);

    for (auto &iter : this->monitors_)
    {
        auto mode = this->xrandr_manager_->get_mode(iter.second->current_mode_get());

        if (!mode || !iter.second->enabled_get())
        {
            MonitorConfigInfo c_monitor(iter.second->get_uid(),
                                        iter.second->name_get().raw(),
                                        false,
                                        0,
                                        0,
                                        0,
                                        0,
                                        DisplayUtil::rotation_to_str(DisplayRotationType::DISPLAY_ROTATION_0),
                                        DisplayUtil::reflect_to_str(DisplayReflectType::DISPLAY_REFLECT_NORMAL),
                                        0.0);

            screen_config.monitor().push_back(std::move(c_monitor));
        }
        else
        {
            MonitorConfigInfo c_monitor(iter.second->get_uid(),
                                        iter.second->name_get().raw(),
                                        true,
                                        iter.second->x_get(),
                                        iter.second->y_get(),
                                        mode->width,
                                        mode->height,
                                        DisplayUtil::rotation_to_str(DisplayRotationType(iter.second->rotation_get())),
                                        DisplayUtil::reflect_to_str(DisplayReflectType(iter.second->reflect_get())),
                                        mode->refresh_rate);
            screen_config.monitor().push_back(std::move(c_monitor));
        }
    }
}

bool DisplayManager::save_config(CCErrorCode &error_code)
{
    if (!this->display_config_)
    {
        this->display_config_ = std::unique_ptr<DisplayConfigInfo>(new DisplayConfigInfo());
    }

    // 禁止保存没有开启任何显示器的配置，这可能会导致下次进入会话屏幕无法显示
    if (this->get_enabled_monitors().size() == 0)
    {
        KLOG_WARNING_DISPLAY("It is forbidden to save the configuration without any display turned on, "
                             "which may cause the next session screen not to be displayed.");
        error_code = CCErrorCode::ERROR_DISPLAY_NO_ENABLED_MONITOR;
        return false;
    }

    auto monitors_uid = this->get_monitors_uid();
    auto &c_screens = this->display_config_->screen();
    bool matched = false;
    ScreenConfigInfo used_config(0, "", 0);

    this->fill_screen_config(used_config);
    for (auto &c_screen : c_screens)
    {
        auto &c_monitors = c_screen.monitor();
        auto c_monitors_uid = this->get_c_monitors_uid(c_monitors);
        if (monitors_uid == c_monitors_uid)
        {
            c_screen = used_config;
            matched = true;
            break;
        }
    }

    if (!matched)
    {
        this->display_config_->screen().push_back(used_config);
    }

    if (c_screens.size() > this->max_screen_record_number_)
    {
        auto oldest_screen = c_screens.begin();
        for (auto iter = c_screens.begin(); iter != c_screens.end(); iter++)
        {
            if ((*iter).timestamp() < (*oldest_screen).timestamp())
            {
                oldest_screen = iter;
            }
        }

        if (oldest_screen != c_screens.end())
        {
            c_screens.erase(oldest_screen);
        }
    }

    RETURN_VAL_IF_FALSE(this->save_to_file(error_code), false);

    return true;
}

bool DisplayManager::apply(CCErrorCode &error_code)
{
    // 如果使用的是nvidia驱动，当没有接入任何显示器时,会将output的分辨率设置为8x8，导致底部面板不可见且后面无法恢复。
    if (this->get_enabled_monitors().size() == 0)
    {
        KLOG_WARNING_DISPLAY("Cannot find enabled monitor.");
        error_code = CCErrorCode::ERROR_DISPLAY_NO_ENABLED_MONITOR;
        return false;
    }

    if (this->dynamic_scaling_window_)
    {
        // 应用缩放因子
        auto variant_value = Glib::Variant<gint32>::create(this->window_scaling_factor_);
        if (!this->xsettings_settings_->set_value(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, variant_value))
        {
            error_code = CCErrorCode::ERROR_DISPLAY_SET_WINDOW_SCALING_FACTOR_1;
            return false;
        }
    }

    // 应用xrandr
    std::string cmdline = XRANDR_CMD;
    std::shared_ptr<DisplayMonitor> primary_monitor;

    /* 加上auto为了确保屏幕大小和显示器之间的缝隙能够自适应。
       例如在扩展屏模式下拔掉一个显示器后，默认屏幕大小还是两个显示器大小之和，如果不做自适应的话被拔掉的显示器上面的窗口就看不到了
    */
    cmdline.append(" --auto");

    // 如果没有设置主显示器，这里会默认使用第一个遍历到的显示器作为主显示器，因为必须要在已开启的显示器中设置一个主显示器，否则可能出现鼠标键盘操作卡顿情况
    for (const auto &iter : this->monitors_)
    {
        const auto &monitor = iter.second;
        if (!monitor->enabled_get())
        {
            continue;
        }

        if (!primary_monitor || monitor->name_get() == this->primary_)
        {
            primary_monitor = monitor;
        }
    }

    for (const auto &iter : this->monitors_)
    {
        auto tmp = iter.second->generate_cmdline(primary_monitor == iter.second);
        cmdline.push_back(' ');
        cmdline.append(tmp);
    }

    try
    {
        std::string standard_error;
        int32_t exit_status = 0;

        KLOG_INFO_DISPLAY("Cmdline is %s.", cmdline.c_str());
        Glib::spawn_command_line_sync(cmdline, nullptr, &standard_error, &exit_status);

        if (!standard_error.empty() || exit_status != 0)
        {
            error_code = CCErrorCode::ERROR_DISPLAY_EXEC_XRANDR_FAILED;
            KLOG_WARNING_DISPLAY("Failed to run xrandr: %s.", standard_error.c_str());
            return false;
        }
    }
    catch (const Glib::Error &e)
    {
        error_code = CCErrorCode::ERROR_DISPLAY_EXEC_XRANDR_FAILED;
        KLOG_WARNING_DISPLAY("Failed to run xrandr: %s.", e.what().c_str());
        return false;
    }
    return true;
}

bool DisplayManager::switch_style(DisplayStyle style, CCErrorCode &error_code)
{
    KLOG_DEBUG_DISPLAY("Switch style to %s.", this->style_enum2str(style).c_str());
    switch (style)
    {
    case DisplayStyle::DISPLAY_STYLE_MIRRORS:
        RETURN_VAL_IF_FALSE(this->switch_to_mirrors(error_code), false);
        break;
    case DisplayStyle::DISPLAY_STYLE_EXTEND:
        this->switch_to_extend();
        break;
    case DisplayStyle::DISPLAY_STYLE_CUSTOM:
        RETURN_VAL_IF_FALSE(this->switch_to_custom(error_code), false);
        break;
    case DisplayStyle::DISPLAY_STYLE_AUTO:
        this->switch_to_auto();
        break;
    default:
        error_code = CCErrorCode::ERROR_DISPLAY_UNKNOWN_DISPLAY_STYLE_1;
        return false;
    }

    // 因为自定义模式下可能由于参数错误导致设置失败，为了增强鲁棒性，这里再做一次扩展模式的设置尝试
    if (!this->apply(error_code))
    {
        KLOG_WARNING_DISPLAY("The first apply failed: %s, try use extend mode", CC_ERROR2STR(error_code).c_str());
        this->switch_to_extend();
        error_code = CCErrorCode::SUCCESS;
        if (!this->apply(error_code))
        {
            KLOG_WARNING_DISPLAY("The second apply also failed: %s.", CC_ERROR2STR(error_code).c_str());
            return false;
        }
    }
    return true;
}

Glib::ustring DisplayManager::style_enum2str(DisplayStyle style)
{
    Glib::ustring style_info;
    switch (style)
    {
    case DisplayStyle::DISPLAY_STYLE_MIRRORS:
        style_info = "mirrors";
        break;
    case DisplayStyle::DISPLAY_STYLE_EXTEND:
        style_info = "extend";
        break;
    case DisplayStyle::DISPLAY_STYLE_CUSTOM:
        style_info = "custom";
        break;
    case DisplayStyle::DISPLAY_STYLE_AUTO:
        style_info = "auto";
        break;
    default:
        style_info = "";
        break;
    }
    return style_info;
}

bool DisplayManager::switch_style_and_save(DisplayStyle style, CCErrorCode &error_code)
{
    RETURN_VAL_IF_FALSE(this->switch_style(style, error_code), false);
    RETURN_VAL_IF_FALSE(this->save_config(error_code), false);
    return true;
}

bool DisplayManager::switch_to_mirrors(CCErrorCode &error_code)
{
    auto monitors = this->get_connected_monitors();
    auto modes = this->monitors_common_modes(monitors);

    if (modes.size() == 0)
    {
        error_code = CCErrorCode::ERROR_DISPLAY_COMMON_MODE_NOTFOUND;
        return false;
    }

    auto width = modes[0]->width;
    auto height = modes[0]->height;

    for (auto monitor : monitors)
    {
        monitor->enabled_set(true);
        monitor->x_set(0);
        monitor->y_set(0);

        auto match_modes = monitor->get_modes_by_size(width, height);
        if (match_modes.size() > 0)
        {
            monitor->current_mode_set(match_modes[0]->id);
        }
        else
        {
            // 这里理论上不可能执行到,所以这里只打印日志,不返回错误
            KLOG_WARNING_DISPLAY("Cannot match mod %ux%u for monitor %s.", width, height, monitor->name_get().c_str());
        }

        monitor->rotation_set(uint16_t(DisplayRotationType::DISPLAY_ROTATION_0));
        monitor->reflect_set(uint16_t(DisplayReflectType::DISPLAY_REFLECT_NORMAL));
    }

    return true;
}

ModeInfoVec DisplayManager::monitors_common_modes(const DisplayMonitorVec &monitors)
{
    ModeInfoVec result;
    RETURN_VAL_IF_TRUE(monitors.size() == 0, result);

    result = this->xrandr_manager_->get_modes(monitors[0]->modes_get());

    for (uint32_t i = 1; i < monitors.size(); ++i)
    {
        auto monitor = monitors[i];
        auto iter = std::remove_if(result.begin(), result.end(), [monitor](std::shared_ptr<ModeInfo> mode) -> bool
                                   {
                                       auto modes = monitor->get_modes_by_size(mode->width, mode->height);
                                       return (modes.size() == 0); });

        result.erase(iter, result.end());
    }
    return result;
}

void DisplayManager::switch_to_extend()
{
    int32_t startx = 0;
    for (const auto &iter : this->monitors_)
    {
        if (!iter.second->connected_get())
        {
            continue;
        }
        auto best_mode = iter.second->get_best_mode();
        if (!best_mode)
        {
            KLOG_WARNING_DISPLAY("Failed to get best mode for monitor %s.", iter.second->name_get().c_str());
            continue;
        }

        iter.second->enabled_set(true);
        iter.second->x_set(startx);
        iter.second->y_set(0);
        iter.second->current_mode_set(best_mode->id);
        iter.second->rotation_set(uint16_t(DisplayRotationType::DISPLAY_ROTATION_0));
        iter.second->reflect_set(uint16_t(DisplayReflectType::DISPLAY_REFLECT_NORMAL));

        startx += best_mode->width;
    }
}

bool DisplayManager::switch_to_custom(CCErrorCode &error_code)
{
    return this->apply_config(error_code);
}

void DisplayManager::switch_to_auto()
{
    CCErrorCode error_code;

    RETURN_IF_TRUE(this->switch_to_custom(error_code));
    this->switch_to_extend();
}

std::shared_ptr<DisplayMonitor> DisplayManager::get_monitor(uint32_t id)
{
    auto iter = this->monitors_.find(id);
    if (iter != this->monitors_.end())
    {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<DisplayMonitor> DisplayManager::get_monitor_by_uid(const std::string &uid)
{
    for (const auto &iter : this->monitors_)
    {
        if (iter.second->get_uid() == uid)
        {
            return iter.second;
        }
    }
    return nullptr;
}

std::shared_ptr<DisplayMonitor> DisplayManager::get_monitor_by_name(const std::string &name)
{
    for (const auto &iter : this->monitors_)
    {
        if (iter.second->name_get() == name)
        {
            return iter.second;
        }
    }
    return nullptr;
}

std::shared_ptr<DisplayMonitor> DisplayManager::match_best_monitor(const std::string &uid,
                                                                   const std::string &name)
{
    std::shared_ptr<DisplayMonitor> retval;
    for (const auto &iter : this->monitors_)
    {
        if (!retval && iter.second->get_uid() == uid)
        {
            retval = iter.second;
        }

        // 完美匹配则直接退出
        if (iter.second->get_uid() == uid && iter.second->name_get() == name)
        {
            retval = iter.second;
            break;
        }
    }
    return retval;
}

std::string DisplayManager::get_monitors_uid()
{
    std::vector<std::string> result;
    for (const auto &iter : this->monitors_)
    {
        result.push_back(iter.second->get_uid());
    }
    std::sort(result.begin(), result.end(), std::less<std::string>());
    return StrUtils::join(result, MONITOR_JOIN_CHAR);
}

std::string DisplayManager::get_c_monitors_uid(const ScreenConfigInfo::MonitorSequence &monitors)
{
    std::vector<std::string> result;
    for (const auto &monitor : monitors)
    {
        result.push_back(monitor.uid());
    }
    std::sort(result.begin(), result.end(), std::less<std::string>());
    return StrUtils::join(result, MONITOR_JOIN_CHAR);
}

bool DisplayManager::save_to_file(CCErrorCode &error_code)
{
    // 文件不存在则先尝试创建对应的目录
    if (!Glib::file_test(this->config_file_path_, Glib::FILE_TEST_EXISTS))
    {
        auto dirname = Glib::path_get_dirname(this->config_file_path_);
        if (g_mkdir_with_parents(dirname.c_str(),
                                 0775) != 0)
        {
            error_code = CCErrorCode::ERROR_DISPLAY_SAVE_CREATE_FILE_FAILED;
            KLOG_WARNING_DISPLAY("Failed to create directory %s.", dirname.c_str());
            return false;
        }
    }

    try
    {
        std::ofstream ofs(this->config_file_path_, std::ios_base::out);
        display(ofs, *this->display_config_.get());
        ofs.close();
    }
    catch (const xml_schema::Exception &e)
    {
        KLOG_WARNING_DISPLAY("%s", e.what());
        error_code = CCErrorCode::ERROR_DISPLAY_WRITE_CONF_FILE_FAILED;
        return false;
    }
    return true;
}

void DisplayManager::resources_changed()
{
    auto old_monitors_uid = this->get_monitors_uid();
    this->load_monitors();
    auto new_monitors_uid = this->get_monitors_uid();

    auto screen_changed_adaptation = this->display_settings_->get_boolean(SCREEN_CHANGED_ADAPT);

    // 如果uid不相同，说明设备硬件发生了变化，此时需要重新进行设置
    if (screen_changed_adaptation && old_monitors_uid != new_monitors_uid)
    {
        CCErrorCode error_code = CCErrorCode::SUCCESS;
        if (!this->switch_style_and_save(this->default_style_, error_code))
        {
            KLOG_WARNING_DISPLAY("%s", CC_ERROR2STR(error_code).c_str());
        }
    }
    this->MonitorsChanged_signal.emit(true);
}

void DisplayManager::display_settings_changed(const Glib::ustring &key)
{
    KLOG_DEBUG_DISPLAY("The %s settings changed.", key.c_str());

    switch (shash(key.c_str()))
    {
    case CONNECT(DISPLAY_SCHEMA_STYLE, _hash):
    {
        auto style = this->display_settings_->get_enum(key);
        this->default_style_set(style);
    }
    break;
    }
}

void DisplayManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    if (!connect)
    {
        KLOG_WARNING_DISPLAY("Failed to connect dbus with %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, DISPLAY_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_DISPLAY("Register object_path %s fail: %s.", DISPLAY_OBJECT_PATH, e.what().c_str());
    }
}

void DisplayManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_DEBUG_DISPLAY("Success to register dbus name: %s", name.c_str());
}

void DisplayManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_WARNING_DISPLAY("Failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran
