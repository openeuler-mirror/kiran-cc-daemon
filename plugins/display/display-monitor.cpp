/*
 * @Author       : tangjie02
 * @Date         : 2020-09-07 11:25:37
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-07 11:56:03
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/display/display-output.cpp
 */

#include "plugins/display/display-monitor.h"

#include "lib/base/base.h"
#include "plugins/display/display-util.h"

namespace Kiran
{
#define DISPLAY_MONITOR_OBJECT_PATH "/com/unikylin/Kiran/SessionDaemon/Display/Monitor"

DisplayMonitor::DisplayMonitor(const MonitorInfo &monitor_info) : object_register_id_(0),
                                                                  monitor_info_(monitor_info)
{
}

DisplayMonitor::~DisplayMonitor()
{
    this->dbus_unregister();
}

void DisplayMonitor::dbus_register()
{
    this->object_path_ = fmt::format(DISPLAY_MONITOR_OBJECT_PATH "/{0}", this->monitor_info_.id);
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

void DisplayMonitor::set(const MonitorBaseInfo &base)
{
    this->x_set(base.x);
    this->y_set(base.y);
    this->width_set(base.width);
    this->height_set(base.height);
    this->rotation_set(uint16_t(base.rotation));
    this->reflect_set(uint16_t(base.reflect));
    this->refresh_rate_set(base.refresh_rate);
}

void DisplayMonitor::get(MonitorBaseInfo &base)
{
    base.x = this->x_get();
    base.y = this->y_get();
    base.width = this->width_get();
    base.height = this->height_get();
    base.rotation = RotationType(this->rotation_get());
    base.reflect = ReflectType(this->reflect_get());
    base.refresh_rate = this->refresh_rate_get();
}

std::string DisplayMonitor::generate_cmdline()
{
    std::string result;
    std::string tmp;

    tmp = fmt::format("--output {0} --pos {1}x{2} -rotation {3} --reflect {4}",
                      this->monitor_info_.name,
                      this->monitor_info_.x,
                      this->monitor_info_.y,
                      DisplayUtil::rotation_to_str(this->monitor_info_.rotation),
                      DisplayUtil::reflect_to_str(this->monitor_info_.reflect));
    result.append(std::move(tmp));

    auto mode = XrandrManager::get_instance()->get_mode(this->monitor_info_.mode);
    if (mode)
    {
        tmp = fmt::format(" --mode {0}x{1} --rate {2}",
                          mode->width,
                          mode->height,
                          mode->refresh_rate);
        result.append(std::move(tmp));
    }

    return result;
}

void DisplayMonitor::SetMode(guint32 index, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("index: %u.", index);

    if (index >= this->monitor_info_.modes.size())
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_INVALID_PARAMETER, "the index must be less than %u.", this->monitor_info_.modes.size());
    }

    this->current_mode_set(this->monitor_info_.modes[index]);
    invocation.ret();
}

void DisplayMonitor::SetModeBySize(guint16 width, guint16 height, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("width: %u, height: %u.", width, height);

    RRMode match_mode_id = 0;

    for (const auto &mode_id : this->monitor_info_.modes)
    {
        auto mode = XrandrManager::get_instance()->get_mode(mode_id);

        if (mode &&
            mode->width == width &&
            mode->height == height)
        {
            match_mode_id = mode_id;
            break;
        }
    }

    if (match_mode_id)
    {
        this->current_mode_set(match_mode_id);
        invocation.ret();
    }
    else
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_INVALID_PARAMETER, "cannot match mode %ux%u.", width, height);
    }
}

void DisplayMonitor::SetPosition(gint32 x, gint32 y, MethodInvocation &invocation)
{
    this->x_set(x);
    this->y_set(y);
    invocation.ret();
}

void DisplayMonitor::SetReflect(guint32 index, MethodInvocation &invocation)
{
    if (index > this->monitor_info_.reflects.size())
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_INVALID_PARAMETER, "the index must be less than %u.", this->monitor_info_.reflects.size());
    }
    this->reflect_set(uint16_t(this->monitor_info_.reflects[index]));
    invocation.ret();
}

void DisplayMonitor::SetRotation(guint32 index, MethodInvocation &invocation)
{
    if (index > this->monitor_info_.rotations.size())
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_INVALID_PARAMETER, "the index must be less than %u.", this->monitor_info_.rotations.size());
    }
    this->rotation_set(uint16_t(this->monitor_info_.rotations[index]));
    invocation.ret();
}

void DisplayMonitor::SetRefreshRate(double refresh_rate, MethodInvocation &invocation)
{
    this->reflect_set(refresh_rate);
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

SET_SIMPLE_PROP1(x, gint32);
SET_SIMPLE_PROP1(y, gint32);
SET_SIMPLE_PROP1(width, guint32);
SET_SIMPLE_PROP1(height, guint32);
SET_SIMPLE_PROP2(rotation, guint16, RotationType);
SET_SIMPLE_PROP2(reflect, guint16, ReflectType);
SET_SIMPLE_PROP1(refresh_rate, double);
SET_SIMPLE_PROP1(npreferred, gint32);

bool DisplayMonitor::rotations_setHandler(const std::vector<guint16> &value)
{
    this->monitor_info_.rotations.clear();
    for (const auto &rotation : value)
    {
        this->monitor_info_.rotations.push_back(RotationType(rotation));
    }
    return true;
}

bool DisplayMonitor::reflects_setHandler(const std::vector<guint16> &value)
{
    this->monitor_info_.reflects.clear();
    for (const auto &reflect : value)
    {
        this->monitor_info_.reflects.push_back(ReflectType(reflect));
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

}  // namespace Kiran
