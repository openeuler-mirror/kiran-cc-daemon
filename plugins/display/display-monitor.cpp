/*
 * @Author       : tangjie02
 * @Date         : 2020-09-07 11:25:37
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-04 11:23:57
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/display/display-monitor.cpp
 */

#include "plugins/display/display-monitor.h"

#include <glib/gi18n.h>

#include "lib/base/base.h"
#include "plugins/display/display-manager.h"
#include "plugins/display/display-util.h"

namespace Kiran
{
MonitorInfo::MonitorInfo() : id(0),
                             connected(false),
                             enabled(false),
                             x(0),
                             y(0),
                             rotation(DisplayRotationType::DISPLAY_ROTATION_0),
                             reflect(DisplayReflectType::DISPLAY_REFLECT_NORMAL),
                             mode(0),
                             npreferred(0)
{
}

DisplayMonitor::DisplayMonitor(const MonitorInfo &monitor_info) : object_register_id_(0),
                                                                  monitor_info_(monitor_info)
{
}

DisplayMonitor::~DisplayMonitor()
{
    this->dbus_unregister();
}

void DisplayMonitor::update(const MonitorInfo &monitor_info)
{
    this->monitor_info_.uid = monitor_info.uid;
    this->id_set(monitor_info.id);
    this->name_set(monitor_info.name);
    this->connected_set(monitor_info.connected);
    this->enabled_set(monitor_info.enabled);
    this->x_set(monitor_info.x);
    this->y_set(monitor_info.y);
    this->rotation_set(monitor_info.rotation);
    this->reflect_set(monitor_info.reflect);
    this->rotations_set(monitor_info.rotations);
    this->reflects_set(monitor_info.reflects);
    this->current_mode_set(monitor_info.mode);
    this->modes_set(std::vector<uint32_t>(monitor_info.modes.begin(), monitor_info.modes.end()));
    this->npreferred_set(monitor_info.npreferred);
}

void DisplayMonitor::dbus_register()
{
    this->object_path_ = fmt::format(DISPLAY_MONITOR_OBJECT_PATH "{0}", this->monitor_info_.id);
    try
    {
        this->dbus_connect_ = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SESSION);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("failed to get session bus: %s.", e.what().c_str());
        return;
    }

    this->object_register_id_ = this->register_object(this->dbus_connect_, this->object_path_.c_str());
}

void DisplayMonitor::dbus_unregister()
{
    if (this->object_register_id_)
    {
        this->unregister_object();
        this->object_register_id_ = 0;
    }
}

std::shared_ptr<ModeInfo> DisplayMonitor::get_best_mode()
{
    RETURN_VAL_IF_TRUE(this->monitor_info_.modes.size() == 0, nullptr);
    return XrandrManager::get_instance()->get_mode(this->monitor_info_.modes[0]);
}

std::string DisplayMonitor::generate_cmdline(const std::string &priamry)
{
    std::string result;
    std::string tmp;

    if (!this->monitor_info_.enabled)
    {
        result = fmt::format("--output {0} --off", this->monitor_info_.name);
        return result;
    }

    auto mode = XrandrManager::get_instance()->get_mode(this->monitor_info_.mode);

    // 如果mode不存在,说明没有分配crtc,因此使用自动分配模式
    if (!mode)
    {
        result = fmt::format("--output {0} --auto", this->monitor_info_.name);
        return result;
    }

    result = fmt::format("--output {0} --pos {1}x{2} --rotation {3} --reflect {4} --mode {5}x{6} --rate {7} {8}",
                         this->monitor_info_.name,
                         this->monitor_info_.x,
                         this->monitor_info_.y,
                         DisplayUtil::rotation_to_str(this->monitor_info_.rotation),
                         DisplayUtil::reflect_to_str(this->monitor_info_.reflect),
                         mode->width,
                         mode->height,
                         mode->refresh_rate,
                         this->monitor_info_.name == priamry ? "--primary" : "");

    return result;
}

ModeInfoVec DisplayMonitor::get_modes_by_size(uint32_t width, uint32_t height)
{
    ModeInfoVec modes;
    for (const auto &mode_id : this->monitor_info_.modes)
    {
        auto mode = XrandrManager::get_instance()->get_mode(mode_id);
        if (!mode)
        {
            LOG_WARNING("cannot find mode %u.", mode_id);
            continue;
        }
        if (mode->width == width && mode->height == height)
        {
            modes.push_back(mode);
        }
    }
    return modes;
}

std::shared_ptr<ModeInfo> DisplayMonitor::match_best_mode(uint32_t width, uint32_t height, double refresh_rate)
{
    auto modes = this->get_modes_by_size(width, height);

    std::shared_ptr<ModeInfo> match_mode;

    for (auto &mode : modes)
    {
        if (!match_mode ||
            (fabs(match_mode->refresh_rate - refresh_rate) > fabs(mode->refresh_rate - refresh_rate)))
        {
            match_mode = mode;
        }
    }
    return match_mode;
}

std::vector<guint16> DisplayMonitor::rotations_get()
{
    std::vector<guint16> rotations;
    for (const auto &rotation : this->monitor_info_.rotations)
    {
        rotations.push_back(uint16_t(rotation));
    }
    return rotations;
}

std::vector<guint16> DisplayMonitor::reflects_get()
{
    std::vector<guint16> reflects;
    for (const auto &reflect : this->monitor_info_.reflects)
    {
        reflects.push_back(uint16_t(reflect));
    }
    return reflects;
}

std::vector<guint32> DisplayMonitor::modes_get()
{
    std::vector<guint32> result;

    for (const auto &mode : this->monitor_info_.modes)
    {
        result.push_back(mode);
    }
    return result;
}

void DisplayMonitor::Enable(bool enabled, MethodInvocation &invocation)
{
    // 当小于2个显示器开启时，不允许再关闭剩余的显示器
    if (!enabled && DisplayManager::get_instance()->get_enabled_monitors().size() <= 1)
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, _("Cannot disable the monitor, because the number of the enabled monitor is less than 1"));
    }

    this->enabled_set(enabled);
    invocation.ret();
}

void DisplayMonitor::ListModes(MethodInvocation &invocation)
{
    std::vector<std::tuple<guint32, guint32, guint32, double>> result;
    for (const auto &mode_id : this->monitor_info_.modes)
    {
        auto monitor = XrandrManager::get_instance()->get_mode(mode_id);
        if (monitor)
        {
            result.push_back(*monitor.get());
        }
        else
        {
            DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, _("Exist null mode in mode list"));
        }
    }
    invocation.ret(std::move(result));
}

void DisplayMonitor::ListPreferredModes(MethodInvocation &invocation)
{
    std::vector<std::tuple<guint32, guint32, guint32, double>> result;
    for (int i = 0; i < this->monitor_info_.npreferred && i < (int)this->monitor_info_.modes.size(); ++i)
    {
        auto monitor = XrandrManager::get_instance()->get_mode(this->monitor_info_.modes[i]);
        if (monitor)
        {
            result.push_back(*monitor.get());
        }
        else
        {
            DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, _("Exist null mode in preferred mode list"));
        }
    }
    invocation.ret(std::move(result));
}

void DisplayMonitor::GetCurrentMode(MethodInvocation &invocation)
{
    std::tuple<guint32, guint32, guint32, double> result;
    auto monitor = XrandrManager::get_instance()->get_mode(this->monitor_info_.mode);
    if (monitor)
    {
        result = *monitor.get();
        invocation.ret(result);
    }
    else
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, _("The current mode is not exist"));
    }
}

void DisplayMonitor::SetMode(guint32 width, guint32 height, double refresh_rate, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("width: %u, height: %u refresh rate: %f.", width, height, refresh_rate);

    auto mode = this->match_best_mode(width, height, refresh_rate);

    if (!mode)
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_INVALID_PARAMETER, _("Not found match mode."));
    }

    this->current_mode_set(mode->id);
    invocation.ret();
}

void DisplayMonitor::SetModeById(guint32 id, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("mode id: %u.", id);

    if (this->find_index_by_mode_id(id) < 0)
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_INVALID_PARAMETER, _("The mode id {0} is not exist in mode list"), id);
    }

    this->current_mode_set(id);
    invocation.ret();
}

void DisplayMonitor::SetModeBySize(guint32 width, guint32 height, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("width: %u, height: %u.", width, height);

    auto modes = this->get_modes_by_size(width, height);

    if (modes.size() > 0)
    {
        this->current_mode_set(modes[0]->id);
        invocation.ret();
    }
    else
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_INVALID_PARAMETER, _("Not found match mode."));
    }
}

void DisplayMonitor::SetPosition(gint32 x, gint32 y, MethodInvocation &invocation)
{
    this->x_set(x);
    this->y_set(y);
    invocation.ret();
}

void DisplayMonitor::SetRotation(guint16 rotation, MethodInvocation &invocation)
{
    if (this->find_index_by_rotation(DisplayRotationType(rotation)) < 0)
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_INVALID_PARAMETER, _("Unknown rotation type"));
    }
    this->rotation_set(rotation);
    invocation.ret();
}

void DisplayMonitor::SetReflect(guint16 reflect, MethodInvocation &invocation)
{
    if (this->find_index_by_reflect(DisplayReflectType(reflect)) < 0)
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_INVALID_PARAMETER, _("Unknown reflect type"));
    }
    this->reflect_set(reflect);
    invocation.ret();
}

#define SET_SIMPLE_PROP2(prop, type1, type2)                                 \
    bool DisplayMonitor::prop##_setHandler(type1 value)                      \
    {                                                                        \
        RETURN_VAL_IF_TRUE(type1(this->monitor_info_.prop) == value, false); \
        this->monitor_info_.prop = type2(value);                             \
        return true;                                                         \
    }

#define SET_SIMPLE_PROP1(prop, type) SET_SIMPLE_PROP2(prop, type, type)

SET_SIMPLE_PROP1(connected, bool);
SET_SIMPLE_PROP1(enabled, bool);
SET_SIMPLE_PROP1(x, gint32);
SET_SIMPLE_PROP1(y, gint32);
SET_SIMPLE_PROP2(rotation, guint16, DisplayRotationType);
SET_SIMPLE_PROP2(reflect, guint16, DisplayReflectType);
SET_SIMPLE_PROP1(npreferred, gint32);

bool DisplayMonitor::id_setHandler(guint32 value)
{
    // UnSupported
    return false;
}

bool DisplayMonitor::name_setHandler(const Glib::ustring &value)
{
    // UnSupported
    return false;
}

bool DisplayMonitor::rotations_setHandler(const std::vector<guint16> &value)
{
    this->monitor_info_.rotations.clear();
    for (const auto &rotation : value)
    {
        this->monitor_info_.rotations.push_back(DisplayRotationType(rotation));
    }
    return true;
}

bool DisplayMonitor::reflects_setHandler(const std::vector<guint16> &value)
{
    this->monitor_info_.reflects.clear();
    for (const auto &reflect : value)
    {
        this->monitor_info_.reflects.push_back(DisplayReflectType(reflect));
    }
    return true;
}

bool DisplayMonitor::current_mode_setHandler(guint32 value)
{
    this->monitor_info_.mode = value;
    return true;
}

bool DisplayMonitor::modes_setHandler(const std::vector<guint32> &value)
{
    this->monitor_info_.modes.clear();
    for (const auto &elem : value)
    {
        this->monitor_info_.modes.push_back(elem);
    }
    return true;
}

int32_t DisplayMonitor::find_index_by_mode_id(uint32_t mode_id)
{
    for (int32_t i = 0; i < (int32_t)this->monitor_info_.modes.size(); ++i)
    {
        if (this->monitor_info_.modes[i] == mode_id)
        {
            return i;
        }
    }
    return -1;
}

int32_t DisplayMonitor::find_index_by_rotation(DisplayRotationType rotation)
{
    for (int32_t i = 0; i < (int32_t)this->monitor_info_.rotations.size(); ++i)
    {
        if (this->monitor_info_.rotations[i] == rotation)
        {
            return i;
        }
    }
    return -1;
}

int32_t DisplayMonitor::find_index_by_reflect(DisplayReflectType reflect)
{
    for (int32_t i = 0; i < (int32_t)this->monitor_info_.reflects.size(); ++i)
    {
        if (this->monitor_info_.reflects[i] == reflect)
        {
            return i;
        }
    }
    return -1;
}
}  // namespace Kiran
