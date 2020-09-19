/*
 * @Author       : tangjie02
 * @Date         : 2020-09-07 09:52:57
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-16 08:54:30
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/display/display-manager.cpp
 */

#include "plugins/display/display-manager.h"

#include <fstream>

#include "lib/base/base.h"
#include "plugins/display/display-util.h"
namespace Kiran
{
#define DISPLAY_DBUS_NAME "com.unikylin.Kiran.SessionDaemon.Display"
#define DISPLAY_OBJECT_PATH "/com/unikylin/Kiran/SessionDaemon/Display"

#define DISPLAY_SCHEMA_ID "com.unikylin.kiran.display"
#define DISPLAY_SCHEMA_ID_MODE "display-mode"

#define DISPLAY_FILE_NAME "display.xml"
#define MONITOR_JOIN_CHAR ","
#define XRANDR_CMD "xrandr"

DisplayManager::DisplayManager(XrandrManager *xrandr_manager) : xrandr_manager_(xrandr_manager),
                                                                mode_(DisplayMode::EXTEND),
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

void DisplayManager::SwitchMode(guint32 mode, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("mode: %u.", mode);

    std::string err;
    if (!this->switch_mode(DisplayMode(mode), err))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, "failed to switch mode: {0}", err);
    }
    invocation.ret();
}

void DisplayManager::ApplyChanges(MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");
    std::string err;
    if (!this->apply(err))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, "failed to set xrandr: {0}.", err);
    }
    invocation.ret();
}

void DisplayManager::ResetChanges(MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");

    std::string err;
    if (!this->apply_config(err))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, "failed to apply monitors config: {0}.", err);
    }
    invocation.ret();
}

void DisplayManager::Save(MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");

    if (!this->display_config_)
    {
        this->display_config_ = std::unique_ptr<DisplayConfigInfo>(new DisplayConfigInfo());
    }

    auto monitors_uid = this->get_monitors_uid();
    auto &c_screens = this->display_config_->screen();
    ScreenConfigInfo *match_screen = NULL;

    for (auto &c_screen : c_screens)
    {
        auto &c_monitors = c_screen.monitor();
        auto c_monitors_uid = this->get_c_monitors_uid(c_monitors);
        if (monitors_uid == c_monitors_uid)
        {
            match_screen = &c_screen;
            break;
        }
    }

    if (match_screen)
    {
        for (auto &c_monitor : match_screen->monitor())
        {
            auto monitor = this->get_monitor_by_uid(c_monitor.uid());
            if (!monitor)
            {
                DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, "failed to get monitor for {0}.", c_monitor.uid().c_str());
            }
            MonitorBaseInfo base;
            monitor->get(base);

            auto mode = this->xrandr_manager_->get_mode(base.mode);
            if (!mode)
            {
                DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, "failed to get mode for monitor: {0}.", c_monitor.uid().c_str());
            }
            c_monitor.x(base.x);
            c_monitor.y(base.y);
            c_monitor.width(mode->width);
            c_monitor.height(mode->height);
            c_monitor.rotation(DisplayUtil::rotation_to_str(base.rotation));
            c_monitor.reflect(DisplayUtil::reflect_to_str(base.reflect));
            c_monitor.refresh_rate(mode->refresh_rate);
        }
    }
    else
    {
        ScreenConfigInfo new_screen("no", "");

        for (auto &iter : this->monitors_)
        {
            MonitorBaseInfo base;
            iter.second->get(base);

            auto mode = this->xrandr_manager_->get_mode(base.mode);
            if (!mode)
            {
                DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, "failed to get mode for monitor: {0}.", iter.second->get_uid());
            }

            MonitorConfigInfo c_monitor(iter.second->get_uid(),
                                        base.x,
                                        base.y,
                                        mode->width,
                                        mode->height,
                                        DisplayUtil::rotation_to_str(base.rotation),
                                        DisplayUtil::reflect_to_str(base.reflect),
                                        mode->refresh_rate);

            new_screen.monitor().push_back(std::move(c_monitor));
        }

        this->display_config_->screen().push_back(new_screen);
    }

    std::string err;
    if (!this->save_to_file(err))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, "cannot save to file: {0}.", err);
    }

    invocation.ret();
}

bool DisplayManager::mode_setHandler(guint32 value)
{
    RETURN_VAL_IF_TRUE(this->mode_ == DisplayMode(value), true);

    this->mode_ = DisplayMode(value);

    if (this->display_settings_->get_enum(DISPLAY_SCHEMA_ID_MODE) != int32_t(this->mode_))
    {
        this->display_settings_->set_enum(DISPLAY_SCHEMA_ID_MODE, int32_t(this->mode_));
    }
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
    if (!this->switch_mode(this->mode_, err))
    {
        LOG_WARNING("failed to switch mode to %u.", uint32_t(this->mode_));
    }
}

void DisplayManager::load_settings()
{
    SETTINGS_PROFILE("settings: %p.", this->display_settings_.get());

    if (this->display_settings_)
    {
        this->mode_ = DisplayMode(this->display_settings_->get_enum(DISPLAY_SCHEMA_ID_MODE));
    }
}

void DisplayManager::load_monitors()
{
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
            monitor_info.rotation = RotationType(crtc->rotation & ROTATION_ALL_MASK);
            monitor_info.reflect = ReflectType(crtc->rotation & REFLECT_ALL_MASK);
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
        LOG_DEBUG("file {0} is not exist.", this->config_file_path_);
    }
    return;
}

bool DisplayManager::apply_config(std::string &err)
{
    SETTINGS_PROFILE("");

    if (!this->display_config_)
    {
        err = fmt::format("the config is empty.");
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
            LOG_DEBUG("match ids: %s.", monitors_id.c_str());
            if (this->apply_screen_config(screen, err))
            {
                result = true;
                break;
            }
        }
    }
    if (!result)
    {
        err = fmt::format("failed to apply config: %s.", err.c_str());
    }
    return result;
}

bool DisplayManager::apply_screen_config(const ScreenConfigInfo &screen_config, std::string &err)
{
    const auto &c_monitors = screen_config.monitor();

    for (const auto &c_monitor : c_monitors)
    {
        std::string uid = c_monitor.uid();
        auto monitor = this->get_monitor_by_uid(uid);

        if (!monitor)
        {
            LOG_WARNING("cannot find monitor for %s.", uid.c_str());
            continue;
        }

        auto mode = monitor->match_best_mode(c_monitor.width(), c_monitor.height(), c_monitor.refresh_rate());

        if (!mode)
        {
            LOG_WARNING("cannot find mode for %s.", uid.c_str());
        }

        MonitorBaseInfo base = {x : c_monitor.x(),
                                y : c_monitor.y(),
                                rotation : DisplayUtil::str_to_rotation(c_monitor.rotation()),
                                reflect : DisplayUtil::str_to_reflect(c_monitor.reflect()),
                                mode : mode->id};
        monitor->set(base);
    }

    RETURN_VAL_IF_FALSE(this->apply(err), false);
    return true;
}

bool DisplayManager::apply(std::string &err)
{
    std::string cmdline = XRANDR_CMD;

    for (const auto &monitor : this->monitors_)
    {
        auto tmp = monitor.second->generate_cmdline();
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

bool DisplayManager::switch_mode(DisplayMode mode, std::string &err)
{
    SETTINGS_PROFILE("mode: %u.", uint32_t(mode));
    switch (mode)
    {
    case DisplayMode::MIRRORS:
        RETURN_VAL_IF_FALSE(this->switch_to_mirrors(err), false);
        break;
    case DisplayMode::EXTEND:
        RETURN_VAL_IF_FALSE(this->switch_to_extend(err), false);
        break;
    case DisplayMode::CUSTOM:
        RETURN_VAL_IF_FALSE(this->switch_to_custom(err), false);
        break;
    case DisplayMode::AUTO:
        RETURN_VAL_IF_FALSE(this->switch_to_auto(err), false);
        break;
    default:
        err = fmt::format("unknown mode: {0}.", uint32_t(mode));
        return false;
    }

    RETURN_VAL_IF_FALSE(this->apply(err), false);
    this->mode_set(uint32_t(mode));
    return true;
}

bool DisplayManager::switch_to_mirrors(std::string &err)
{
    SETTINGS_PROFILE("");

    auto monitors = this->get_connected_monitors();
    auto modes = this->monitors_common_modes(monitors);

    if (modes.size() == 0)
    {
        err = "cannot find common mode for all enabled monitors.";
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

        monitor->rotation_set(uint16_t(RotationType::ROTATION_0));
        monitor->reflect_set(uint16_t(ReflectType::REFLECT_NORMAL));
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
        iter.second->rotation_set(uint16_t(RotationType::ROTATION_0));
        iter.second->reflect_set(uint16_t(ReflectType::REFLECT_NORMAL));

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

    err = fmt::format("failed to set auto mode.");
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

std::string DisplayManager::get_monitors_uid()
{
    std::vector<std::string> result;
    for (const auto &iter : this->monitors_)
    {
        result.push_back(iter.second->get_uid());
    }
    std::sort(result.begin(), result.end(), std::less<std::string>());
    return StrUtil::join(result, MONITOR_JOIN_CHAR);
}

std::string DisplayManager::get_c_monitors_uid(const ScreenConfigInfo::MonitorSequence &monitors)
{
    std::vector<std::string> result;
    for (const auto &monitor : monitors)
    {
        result.push_back(monitor.uid());
    }
    std::sort(result.begin(), result.end(), std::less<std::string>());
    return StrUtil::join(result, MONITOR_JOIN_CHAR);
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
            err = fmt::format("failed to create directory {0}.", dirname);
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
        err = e.what();
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
        if (!this->switch_mode(this->mode_, err))
        {
            LOG_WARNING("failed to switch mode: %s", err.c_str());
        }
    }
    this->MonitorsChanged_signal.emit(true);
}

void DisplayManager::settings_changed(const Glib::ustring &key)
{
    SETTINGS_PROFILE("key: %s.", key.c_str());

    switch (shash(key.c_str()))
    {
    case CONNECT(DISPLAY_SCHEMA_ID_MODE, _hash):
    {
        auto mode = this->display_settings_->get_enum(key);
        this->mode_set(mode);
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
