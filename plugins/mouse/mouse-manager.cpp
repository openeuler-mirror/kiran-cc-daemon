/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 10:09:05
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-06 15:28:52
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/mouse/mouse-manager.cpp
 */

#include "plugins/mouse/mouse-manager.h"

#include "lib/helper.h"
#include "lib/log.h"

namespace Kiran
{
#define MOUSE_DBUS_NAME "com.unikylin.Kiran.SessionDaemon.Mouse"
#define MOUSE_OBJECT_PATH "/com/unikylin/Kiran/SessionDaemon/Mouse"

#define MOUSE_SCHEMA_ID "com.unikylin.kiran.mouse"
#define MOUSE_SCHEMA_LEFT_HANDED "left-handed"
#define MOUSE_SCHEMA_MOTION_ACCELERATION "motion-acceleration"
#define MOUSE_SCHEMA_DRAG_THRESHOLD "drag-threshold"
#define MOUSE_SCHEMA_MIDDLE_EMULATION_ENABLED "middle-emulation-enabled"

#define MOUSE_PROP_LEFT_HANDED "libinput Left Handed Enabled"
#define MOUSE_PROP_ACCEL_SPEED "libinput Accel Speed"
#define MOUSE_PROP_MIDDLE_EMULATION_ENABLED "libinput Middle Emulation Enabled"

MouseManager::MouseManager() : dbus_connect_id_(0),
                               object_register_id_(0),
                               left_handed_(false),
                               motion_acceleration_(0),
                               middle_emulation_enabled_(false)
{
    this->mouse_settings_ = Gio::Settings::create(MOUSE_SCHEMA_ID);
}

MouseManager::~MouseManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

MouseManager *MouseManager::instance_ = nullptr;
void MouseManager::global_init()
{
    instance_ = new MouseManager();
    instance_->init();
}

void MouseManager::Reset(MethodInvocation &invocation)
{
}

#define PROP_SET_HANDLER(prop, type, key, set_fun)        \
    bool MouseManager::prop##_setHandler(type value)      \
    {                                                     \
        RETURN_VAL_IF_TRUE(value == this->prop##_, true); \
                                                          \
        this->prop##_ = value;                            \
        this->mouse_settings_->set_fun(key, value);       \
        this->set_##prop##_to_devices();                  \
        return true;                                      \
    }

PROP_SET_HANDLER(left_handed, bool, "left-handed", set_boolean);
PROP_SET_HANDLER(motion_acceleration, double, "motion-acceleration", set_double);
PROP_SET_HANDLER(middle_emulation_enabled, bool, "middle-emulation-enabled", set_boolean);

bool MouseManager::double_click_setHandler(gint32 value)
{
    return true;
}

void MouseManager::init()
{
    SETTINGS_PROFILE("");

    this->load_from_settings();

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 MOUSE_DBUS_NAME,
                                                 sigc::mem_fun(this, &MouseManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &MouseManager::on_name_acquired),
                                                 sigc::mem_fun(this, &MouseManager::on_name_lost));
}

void MouseManager::load_from_settings()
{
    SETTINGS_PROFILE("");

    if (this->mouse_settings_)
    {
        this->left_handed_ = this->mouse_settings_->get_boolean(MOUSE_SCHEMA_LEFT_HANDED);
        this->motion_acceleration_ = this->mouse_settings_->get_double(MOUSE_SCHEMA_MOTION_ACCELERATION);
    }
}

void MouseManager::settings_changed(const Glib::ustring &key)
{
    switch (shash(key.c_str()))
    {
    case "left-handed"_hash:
        this->left_handed_set(this->mouse_settings_->get_boolean(MOUSE_SCHEMA_LEFT_HANDED));
        break;
    case "motion-acceleration"_hash:
        this->motion_acceleration_set(this->mouse_settings_->get_double(MOUSE_SCHEMA_MOTION_ACCELERATION));
        break;
    case "middle-emulation-enabled"_hash:
        this->middle_emulation_enabled_set(this->mouse_settings_->get_boolean(MOUSE_SCHEMA_MIDDLE_EMULATION_ENABLED));
        break;
    }
}

#define SET_PROP_TO_DEVICES(set_device_fun)                                                                 \
    void MouseManager::set_device_fun##s()                                                                  \
    {                                                                                                       \
        SETTINGS_PROFILE("");                                                                               \
        int32_t n_devices = 0;                                                                              \
        auto devices_info = XListInputDevices(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), &n_devices); \
                                                                                                            \
        for (auto i = 0; i < n_devices; i++)                                                                \
        {                                                                                                   \
            auto device_helper = std::make_shared<DeviceHelper>(&devices_info[i]);                          \
            if (device_helper)                                                                              \
            {                                                                                               \
                set_device_fun(device_helper);                                                              \
            }                                                                                               \
        }                                                                                                   \
                                                                                                            \
        if (devices_info != NULL)                                                                           \
        {                                                                                                   \
            XFreeDeviceList(devices_info);                                                                  \
        }                                                                                                   \
    }

SET_PROP_TO_DEVICES(set_left_handed_to_device);
SET_PROP_TO_DEVICES(set_motion_acceleration_to_device);
SET_PROP_TO_DEVICES(set_middle_emulation_enabled_to_device);

void MouseManager::set_left_handed_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("");

    if (device_helper->has_property(MOUSE_PROP_LEFT_HANDED) &&
        !device_helper->is_touchpad())
    {
        device_helper->set_property(MOUSE_PROP_LEFT_HANDED, this->left_handed_);
    }
}

void MouseManager::set_motion_acceleration_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("");

    if (device_helper->has_property(MOUSE_PROP_ACCEL_SPEED) &&
        !device_helper->is_touchpad())
    {
        float motion_accel = 0;
        if (this->motion_acceleration_ < 1 || this->motion_acceleration_ > 10)
        {
            motion_accel = 0;
        }
        else
        {
            motion_accel = (this->motion_acceleration_ - 1) * 2 / 9 - 1;
        }
        device_helper->set_property(MOUSE_PROP_ACCEL_SPEED, motion_accel);
    }
}

void MouseManager::set_middle_emulation_enabled_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("");

    if (device_helper->has_property(MOUSE_PROP_MIDDLE_EMULATION_ENABLED) &&
        !device_helper->is_touchpad())
    {
        device_helper->set_property(MOUSE_PROP_MIDDLE_EMULATION_ENABLED, this->middle_emulation_enabled_);
    }
}

void MouseManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    SETTINGS_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        LOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, MOUSE_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("register object_path %s fail: %s.", MOUSE_OBJECT_PATH, e.what().c_str());
    }
}

void MouseManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void MouseManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_WARNING("failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran