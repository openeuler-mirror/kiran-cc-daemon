/*
 * @Author       : tangjie02
 * @Date         : 2020-09-07 09:52:57
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-04 13:54:18
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/display/display-manager.cpp
 */

#include "plugins/display/display-manager.h"

#include <glib/gi18n.h>

#include <fstream>

#include "lib/base/base.h"
#include "plugins/display/display-util.h"
namespace Kiran
{
#define DISPLAY_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon.Display"
#define DISPLAY_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Display"

#define DISPLAY_SCHEMA_ID "com.kylinsec.kiran.display"
#define DISPLAY_SCHEMA_ID_STYLE "display-style"

#define DISPLAY_FILE_NAME "display.xml"
#define MONITOR_JOIN_CHAR ","
#define XRANDR_CMD "xrandr"

DisplayManager::DisplayManager(XrandrManager *xrandr_manager) : xrandr_manager_(xrandr_manager),
                                                                default_style_(DisplayStyle::DISPLAY_STYLE_EXTEND),
                                                                dbus_connect_id_(0),
                                                                object_register_id_(0)
{
    this->config_file_path_ = Glib::build_filename(Glib::get_user_config_dir(),
                                                   DISPLAY_CONF_DIR,
                                                   DISPLAY_FILE_NAME);

    this->display_settings_ = Gio::Settings::create(DISPLAY_SCHEMA_ID);
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
    SETTINGS_PROFILE("");

    std::vector<Glib::ustring> object_paths;
    for (const auto &iter : this->monitors_)
    {
        object_paths.push_back(iter.second->get_object_path());
    }
    invocation.ret(object_paths);
}

void DisplayManager::SwitchStyle(guint32 style, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("style: %u.", style);

    std::string err;
    if (!this->switch_style(DisplayStyle(style), err))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, err);
    }
    invocation.ret();
}

void DisplayManager::SetDefaultStyle(guint32 style, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("style: %u", style);
    this->default_style_set(style);
    invocation.ret();
}

void DisplayManager::ApplyChanges(MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");
    std::string err;
    if (!this->apply(err))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, _("Apply failed: {0}"), err);
    }
    invocation.ret();
}

void DisplayManager::RestoreChanges(MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");

    std::string err;
    if (!this->apply_config(err))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, "Restore failed: {0}", err);
    }
    invocation.ret();
}

void DisplayManager::SetPrimary(const Glib::ustring &name, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("name: %s.", name.c_str());

    if (name.length() > 0 && !this->get_monitor_by_name(name))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, _("Not found the primary monitor"));
    }
    this->primary_set(name);
    invocation.ret();
}

void DisplayManager::Save(MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");
    std::string err;

    if (!this->save_config(err))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, err.c_str());
    }

    invocation.ret();
}

bool DisplayManager::default_style_setHandler(guint32 value)
{
    RETURN_VAL_IF_TRUE(this->default_style_ == DisplayStyle(value), true);

    this->default_style_ = DisplayStyle(value);

    if (this->display_settings_->get_enum(DISPLAY_SCHEMA_ID_STYLE) != int32_t(this->default_style_))
    {
        this->display_settings_->set_enum(DISPLAY_SCHEMA_ID_STYLE, int32_t(this->default_style_));
    }
    return true;
}

bool DisplayManager::primary_setHandler(const Glib::ustring &value)
{
    this->primary_ = value.raw();
    return true;
}

void DisplayManager::init()
{
    SETTINGS_PROFILE("");

    this->load_settings();
    this->load_monitors();
    this->load_config();

    this->display_settings_->signal_changed().connect(sigc::mem_fun(this, &DisplayManager::settings_changed));
    this->xrandr_manager_->signal_resources_changed().connect(sigc::mem_fun(this, &DisplayManager::resources_changed));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 DISPLAY_DBUS_NAME,
                                                 sigc::mem_fun(this, &DisplayManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &DisplayManager::on_name_acquired),
                                                 sigc::mem_fun(this, &DisplayManager::on_name_lost));

    std::string err;
    if (!this->switch_style_and_save(this->default_style_, err))
    {
        LOG_WARNING("%s");
    }
}

void DisplayManager::load_settings()
{
    SETTINGS_PROFILE("settings: %p.", this->display_settings_.get());

    if (this->display_settings_)
    {
        this->default_style_ = DisplayStyle(this->display_settings_->get_enum(DISPLAY_SCHEMA_ID_STYLE));
    }
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
            LOG_WARNING("failed to load file: %s: %s", this->config_file_path_.c_str(), e.what());
            this->display_config_ = nullptr;
        }
    }
    else
    {
        LOG_DEBUG("file %s is not exist.", this->config_file_path_);
    }
    return;
}

bool DisplayManager::apply_config(std::string &err)
{
    SETTINGS_PROFILE("");

    if (!this->display_config_)
    {
        err = _("The config is empty");
        return false;
    }

    auto monitors_id = this->get_monitors_uid();
    const auto &screens = this->display_config_->screen();
    bool result = false;
    std::string err2;

    for (const auto &screen : screens)
    {
        const auto &monitors = screen.monitor();
        auto monitors_config_id = this->get_c_monitors_uid(monitors);
        if (monitors_id == monitors_config_id)
        {
            LOG_DEBUG("match ids: %s.", monitors_id.c_str());
            if (this->apply_screen_config(screen, err2))
            {
                result = true;
                break;
            }
        }
    }
    if (!result)
    {
        err = _("Not found match configuration");
    }
    return result;
}

bool DisplayManager::apply_screen_config(const ScreenConfigInfo &screen_config, std::string &err)
{
    const auto &c_monitors = screen_config.monitor();

    this->primary_set(screen_config.primary());

    for (const auto &c_monitor : c_monitors)
    {
        std::string uid = c_monitor.uid();
        auto monitor = this->get_monitor_by_uid(uid);

        if (!monitor)
        {
            LOG_WARNING("cannot find monitor for %s.", uid.c_str());
            continue;
        }

        if (c_monitor.name() != monitor->name_get())
        {
            LOG_WARNING("the name is mismatch. config name: %s, monitor name: %s.",
                        c_monitor.name().c_str(),
                        monitor->name_get().c_str());
            continue;
        }

        auto mode = monitor->match_best_mode(c_monitor.width(), c_monitor.height(), c_monitor.refresh_rate());

        if (!c_monitor.enabled() || !mode)
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
            monitor->enabled_set(true);
            monitor->x_set(c_monitor.x());
            monitor->y_set(c_monitor.y());
            monitor->rotation_set(uint16_t(DisplayUtil::str_to_rotation(c_monitor.rotation())));
            monitor->reflect_set(uint16_t(DisplayUtil::str_to_reflect(c_monitor.reflect())));
            monitor->current_mode_set(mode->id);
        }
    }

    RETURN_VAL_IF_FALSE(this->apply(err), false);
    return true;
}

void DisplayManager::fill_screen_config(ScreenConfigInfo &screen_config)
{
    screen_config.primary(this->primary_);

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

bool DisplayManager::save_config(std::string &err)
{
    if (!this->display_config_)
    {
        this->display_config_ = std::unique_ptr<DisplayConfigInfo>(new DisplayConfigInfo());
    }

    auto monitors_uid = this->get_monitors_uid();
    auto &c_screens = this->display_config_->screen();
    bool matched = false;
    ScreenConfigInfo used_config("");

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

    RETURN_VAL_IF_FALSE(this->save_to_file(err), false);

    return true;
}

bool DisplayManager::apply(std::string &err)
{
    std::string cmdline = XRANDR_CMD;

    auto primary_monitor = this->get_monitor_by_name(this->primary_);

    if (!primary_monitor)
    {
        cmdline.append(" --noprimary");
    }

    for (const auto &monitor : this->monitors_)
    {
        auto tmp = monitor.second->generate_cmdline(this->primary_);
        cmdline.push_back(' ');
        cmdline.append(tmp);
    }

    try
    {
        LOG_DEBUG("cmdline: %s.", cmdline.c_str());
        Glib::spawn_command_line_sync(cmdline);
    }
    catch (const Glib::Error &e)
    {
        err = e.what().raw();
        return false;
    }
    return true;
}

bool DisplayManager::switch_style(DisplayStyle style, std::string &err)
{
    SETTINGS_PROFILE("style: %u.", uint32_t(style));
    switch (style)
    {
    case DisplayStyle::DISPLAY_STYLE_MIRRORS:
        RETURN_VAL_IF_FALSE(this->switch_to_mirrors(err), false);
        break;
    case DisplayStyle::DISPLAY_STYLE_EXTEND:
        RETURN_VAL_IF_FALSE(this->switch_to_extend(err), false);
        break;
    case DisplayStyle::DISPLAY_STYLE_CUSTOM:
        RETURN_VAL_IF_FALSE(this->switch_to_custom(err), false);
        break;
    case DisplayStyle::DISPLAY_STYLE_AUTO:
        RETURN_VAL_IF_FALSE(this->switch_to_auto(err), false);
        break;
    default:
        err = fmt::format("unknown style: {0}.", uint32_t(style));
        return false;
    }

    RETURN_VAL_IF_FALSE(this->apply(err), false);
    return true;
}

bool DisplayManager::switch_style_and_save(DisplayStyle style, std::string &err)
{
    RETURN_VAL_IF_FALSE(this->switch_style(style, err), false);
    RETURN_VAL_IF_FALSE(this->save_config(err), false);
    return true;
}

bool DisplayManager::switch_to_mirrors(std::string &err)
{
    SETTINGS_PROFILE("");

    auto monitors = this->get_connected_monitors();
    auto modes = this->monitors_common_modes(monitors);

    if (modes.size() == 0)
    {
        err = _("Cannot find common mode for all enabled monitors");
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
            LOG_WARNING("cannot match mod %ux%u for monitor %s.", width, height, monitor->name_get().c_str());
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
        auto iter = std::remove_if(result.begin(), result.end(), [monitor](std::shared_ptr<ModeInfo> mode) -> bool {
            auto modes = monitor->get_modes_by_size(mode->width, mode->height);
            return (modes.size() == 0);
        });

        result.erase(iter, result.end());
    }
    return result;
}

bool DisplayManager::switch_to_extend(std::string &err)
{
    SETTINGS_PROFILE("");

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
            LOG_WARNING("failed to get best mode for monitor %s.", iter.second->name_get().c_str());
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

    return true;
}

bool DisplayManager::switch_to_custom(std::string &err)
{
    SETTINGS_PROFILE("");

    return this->apply_config(err);
}

bool DisplayManager::switch_to_auto(std::string &err)
{
    SETTINGS_PROFILE("");

    RETURN_VAL_IF_TRUE(this->switch_to_custom(err), true);
    LOG_DEBUG("%s", err.c_str());

    RETURN_VAL_IF_TRUE(this->switch_to_extend(err), true);
    LOG_DEBUG("%s", err.c_str());

    RETURN_VAL_IF_TRUE(this->switch_to_mirrors(err), true);
    LOG_DEBUG("%s", err.c_str());

    err = _("Auto mode is set failed");
    return false;
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

bool DisplayManager::save_to_file(std::string &err)
{
    // 文件不存在则先尝试创建对应的目录
    if (!Glib::file_test(this->config_file_path_, Glib::FILE_TEST_EXISTS))
    {
        auto dirname = Glib::path_get_dirname(this->config_file_path_);
        if (g_mkdir_with_parents(dirname.c_str(),
                                 0775) != 0)
        {
            err = fmt::format(_("Failed to create directory {0}"), dirname);
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
        LOG_WARNING("%s", e.what());
        err = fmt::format(_("Write to file {0} failed"), this->config_file_path_);
        return false;
    }
    return true;
}

void DisplayManager::resources_changed()
{
    SETTINGS_PROFILE("");
    auto old_monitor_num = this->monitors_.size();
    auto new_monitor_num = this->xrandr_manager_->get_connected_outputs().size();

    this->load_monitors();

    // 如果连接设备的数量不相等，说明有设备被删除或者新设备加入
    // 因此重新设置参数
    if (old_monitor_num != new_monitor_num)
    {
        std::string err;
        if (!this->switch_style_and_save(this->default_style_, err))
        {
            LOG_WARNING("%s", err.c_str());
        }
    }
    this->MonitorsChanged_signal.emit(true);
}

void DisplayManager::settings_changed(const Glib::ustring &key)
{
    SETTINGS_PROFILE("key: %s.", key.c_str());

    switch (shash(key.c_str()))
    {
    case CONNECT(DISPLAY_SCHEMA_ID_STYLE, _hash):
    {
        auto style = this->display_settings_->get_enum(key);
        this->default_style_set(style);
    }
    break;
    }
}

void DisplayManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    SETTINGS_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        LOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, DISPLAY_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("register object_path %s fail: %s.", DISPLAY_OBJECT_PATH, e.what().c_str());
    }
}

void DisplayManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void DisplayManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_WARNING("failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran
