/*
 * @Author       : tangjie02
 * @Date         : 2020-09-07 09:52:57
 * @LastEditors  : tangjie02
 * @LastEditTime : 2096-10-15 23:32:38
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

#define DISPLAY_FILE_NAME "display.xml"
#define MONITOR_JOIN_CHAR ","
#define XRANDR_CMD "xrandr"

DisplayManager::DisplayManager(XrandrManager *xrandr_manager) : xrandr_manager_(xrandr_manager),
                                                                dbus_connect_id_(0),
                                                                object_register_id_(0)
{
    this->display_file_path_ = Glib::build_filename(Glib::get_user_config_dir(),
                                                    DISPLAY_CONF_DIR,
                                                    DISPLAY_FILE_NAME);
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

void DisplayManager::ApplyChanges(MethodInvocation &invocation)
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
        // Glib::spawn_command_line_sync(cmdline);
    }
    catch (const Glib::Error &e)
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, "failed to set xrandr: %s.", e.what().c_str());
    }
    invocation.ret();
}

void DisplayManager::ResetChanges(MethodInvocation &invocation)
{
    std::string err;
    if (!this->apply_display_config(err))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, "failed to apply monitors config: %s.", err.c_str());
    }
    invocation.ret();
}

void DisplayManager::Save(MethodInvocation &invocation)
{
    auto monitors_id = this->get_monitors_id();

    auto &c_screens = this->display_config_->screen();
    ScreenConfigInfo *match_screen = NULL;

    for (auto &c_screen : c_screens)
    {
        auto &c_monitors = c_screen.monitor();
        auto c_monitors_id = this->get_monitors_config_id(c_monitors);
        if (monitors_id == c_monitors_id)
        {
            match_screen = &c_screen;
            break;
        }
    }
    if (match_screen)
    {
        for (auto &c_monitor : match_screen->monitor())
        {
            auto monitor = this->get_monitor(c_monitor.uuid());
            if (monitor)
            {
                MonitorBaseInfo base;
                monitor->get(base);
                c_monitor.x(base.x);
                c_monitor.y(base.y);
                c_monitor.width(base.width);
                c_monitor.height(base.height);
                c_monitor.rotation(DisplayUtil::rotation_to_str(base.rotation));
                c_monitor.reflect(DisplayUtil::reflect_to_str(base.reflect));
                c_monitor.refresh_rate(base.refresh_rate);
            }
        }

        try
        {
            std::ofstream ofs(this->display_file_path_);
            display(ofs, *this->display_config_.get());
        }
        catch (const xml_schema::Exception &e)
        {
            DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, "%s", e.what());
        }
    }
    invocation.ret();
}

void DisplayManager::ListOutputs(MethodInvocation &invocation)
{
    // std::vector<Glib::ustring> names;
    // for (const auto& output : this->outputs_)
    // {
    //     names.push_back(output.second->get_name());
    // }
    // invocation.ret(names);
}

void DisplayManager::init()
{
    this->load_monitors();

    std::string err;
    if (!this->load_display_config(err))
    {
        LOG_WARNING("failed to load monitors file: %s.", err.c_str());
    }

    if (!this->apply_display_config(err))
    {
        LOG_WARNING("failed to apply monitors config: %s.", err.c_str());
    }

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 DISPLAY_DBUS_NAME,
                                                 sigc::mem_fun(this, &DisplayManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &DisplayManager::on_name_acquired),
                                                 sigc::mem_fun(this, &DisplayManager::on_name_lost));
}

void DisplayManager::load_monitors()
{
    auto outputs = this->xrandr_manager_->get_connected_outputs();
    for (const auto &output : outputs)
    {
        auto crtc = this->xrandr_manager_->get_crtc(output->crtc);
        if (!crtc)
        {
            LOG_WARNING("failed to get crct for output %s.", output->name.c_str());
            continue;
        }

        auto mode = this->xrandr_manager_->get_mode(crtc->mode);

        if (!mode)
        {
            LOG_WARNING("failed to get mode for output %s.", output->name.c_str());
            continue;
        }

        MonitorInfo monitor_info = {id : output->id,
                                    name : output->name,
                                    connected : output->connection,
                                    x : crtc->x,
                                    y : crtc->y,
                                    width : mode->width,
                                    height : mode->height,
                                    rotation : RotationType(crtc->rotation & ROTATION_ALL_MASK),
                                    reflect : ReflectType(crtc->rotation & REFLECT_ALL_MASK),
                                    refresh_rate : mode->refresh_rate,
                                    rotations : this->xrandr_manager_->get_rotations(crtc),
                                    reflects : this->xrandr_manager_->get_reflects(crtc),
                                    mode : mode->id,
                                    modes : output->modes,
                                    npreferred : output->npreferred};

        auto monitor = std::make_shared<DisplayMonitor>(monitor_info);
        auto uuid = this->xrandr_manager_->gen_uuid(output);
        this->monitors_.emplace(uuid, monitor);
        monitor->dbus_register();
    }
}

bool DisplayManager::load_display_config(std::string &err)
{
    if (Glib::file_test(this->display_file_path_, Glib::FILE_TEST_EXISTS))
    {
        try
        {
            this->display_config_ = display(this->display_file_path_);
        }
        catch (const xml_schema::Exception &e)
        {
            err = e.what();
            return false;
        }
        return true;
    }
    else
    {
        err = fmt::format("file {0} is not exist.", this->display_file_path_);
        return false;
    }
}

bool DisplayManager::apply_display_config(std::string &err)
{
    RETURN_VAL_IF_FALSE(this->display_config_, true);

    auto monitors_id = this->get_monitors_id();

    const auto &screens = this->display_config_->screen();
    for (const auto &screen : screens)
    {
        const auto &monitors = screen.monitor();
        auto monitors_config_id = this->get_monitors_config_id(monitors);
        if (monitors_id == monitors_config_id)
        {
            RETURN_VAL_IF_FALSE(this->apply_screen_config(screen, err), false);
            break;
        }
    }
    return true;
}

bool DisplayManager::apply_screen_config(const ScreenConfigInfo &screen_config, std::string &err)
{
    const auto &c_monitors = screen_config.monitor();

    for (const auto &c_monitor : c_monitors)
    {
        std::string uuid = c_monitor.uuid();
        auto monitor = this->get_monitor(uuid);

        if (monitor)
        {
            MonitorBaseInfo base = {x : c_monitor.x(),
                                    y : c_monitor.y(),
                                    width : c_monitor.width(),
                                    height : c_monitor.height(),
                                    rotation : DisplayUtil::str_to_rotation(c_monitor.rotation()),
                                    reflect : DisplayUtil::str_to_reflect(c_monitor.reflect()),
                                    refresh_rate : c_monitor.refresh_rate()};
            monitor->set(base);
        }
        else
        {
            LOG_WARNING("cannot find monitor for %s.", uuid.c_str());
        }
    }
    return true;
}

std::shared_ptr<DisplayMonitor> DisplayManager::get_monitor(const std::string &uuid)
{
    auto iter = this->monitors_.find(uuid);
    if (iter != this->monitors_.end())
    {
        return iter->second;
    }
    return nullptr;
}

std::string DisplayManager::get_monitors_id()
{
    std::vector<std::string> result;
    for (const auto &monitor : this->monitors_)
    {
        result.push_back(monitor.first);
    }
    std::sort(result.begin(), result.end(), std::less<std::string>());
    return StrUtil::join(result, MONITOR_JOIN_CHAR);
}

std::string DisplayManager::get_monitors_config_id(const ScreenConfigInfo::MonitorSequence &monitors)
{
    std::vector<std::string> result;
    for (const auto &monitor : monitors)
    {
        result.push_back(monitor.uuid());
    }
    std::sort(result.begin(), result.end(), std::less<std::string>());
    return StrUtil::join(result, MONITOR_JOIN_CHAR);
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
