/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 10:09:05
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-20 09:57:35
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/inputdevices/mouse/mouse-manager.cpp
 */

#include "plugins/inputdevices/mouse/mouse-manager.h"

#include "lib/helper.h"
#include "lib/log.h"
#include "plugins/inputdevices/common/xinput-helper.h"

namespace Kiran
{
#define MOUSE_DBUS_NAME "com.unikylin.Kiran.SessionDaemon.Mouse"
#define MOUSE_OBJECT_PATH "/com/unikylin/Kiran/SessionDaemon/Mouse"

#define MOUSE_SCHEMA_ID "com.unikylin.kiran.mouse"
#define MOUSE_SCHEMA_LEFT_HANDED "left-handed"
#define MOUSE_SCHEMA_MOTION_ACCELERATION "motion-acceleration"
#define MOUSE_SCHEMA_MIDDLE_EMULATION_ENABLED "middle-emulation-enabled"
#define MOUSE_SCHEMA_NATURAL_SCROLL "natural-scroll"

#define MOUSE_PROP_LEFT_HANDED "libinput Left Handed Enabled"
#define MOUSE_PROP_ACCEL_SPEED "libinput Accel Speed"
#define MOUSE_PROP_MIDDLE_EMULATION_ENABLED "libinput Middle Emulation Enabled"
#define MOUSE_PROP_NATURAL_SCROLL "libinput Natural Scrolling Enabled"

MouseManager::MouseManager() : dbus_connect_id_(0),
                               object_register_id_(0),
                               left_handed_(false),
                               motion_acceleration_(0),
                               middle_emulation_enabled_(false),
                               natural_scroll_(false)
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
    this->left_handed_set(false);
    this->motion_acceleration_set(0);
    this->middle_emulation_enabled_set(false);
    this->natural_scroll_set(false);
}

#define PROP_SET_HANDLER(prop, type, key, type2)                                    \
    bool MouseManager::prop##_setHandler(type value)                                \
    {                                                                               \
        SETTINGS_PROFILE("value: %s.", fmt::format("{0}", value).c_str());          \
        RETURN_VAL_IF_TRUE(value == this->prop##_, false);                          \
        if (g_settings_get_##type2(this->mouse_settings_->gobj(), key) != value)    \
        {                                                                           \
            if (!g_settings_set_##type2(this->mouse_settings_->gobj(), key, value)) \
            {                                                                       \
                return false;                                                       \
            }                                                                       \
        }                                                                           \
        this->prop##_ = value;                                                      \
        this->set_##prop##_to_devices();                                            \
        return true;                                                                \
    }

PROP_SET_HANDLER(left_handed, bool, MOUSE_SCHEMA_LEFT_HANDED, boolean);
PROP_SET_HANDLER(motion_acceleration, double, MOUSE_SCHEMA_MOTION_ACCELERATION, double);
PROP_SET_HANDLER(middle_emulation_enabled, bool, MOUSE_SCHEMA_MIDDLE_EMULATION_ENABLED, boolean);
PROP_SET_HANDLER(natural_scroll, bool, MOUSE_SCHEMA_NATURAL_SCROLL, boolean);

bool MouseManager::double_click_setHandler(gint32 value)
{
    return true;
}

void MouseManager::init()
{
    SETTINGS_PROFILE("");

    if (!XInputHelper::supports_xinput_devices())
    {
        LOG_WARNING("XInput is not supported, not applying any settings.");
        return;
    }

    this->load_from_settings();
    this->set_all_props_to_devices();

    this->mouse_settings_->signal_changed().connect(sigc::mem_fun(this, &MouseManager::settings_changed));

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
        this->middle_emulation_enabled_ = this->mouse_settings_->get_boolean(MOUSE_SCHEMA_MIDDLE_EMULATION_ENABLED);
        this->natural_scroll_ = this->mouse_settings_->get_boolean(MOUSE_SCHEMA_NATURAL_SCROLL);
    }
}

void MouseManager::settings_changed(const Glib::ustring &key)
{
    SETTINGS_PROFILE("key: %s.", key.c_str());

    switch (shash(key.c_str()))
    {
    case CONNECT(MOUSE_SCHEMA_LEFT_HANDED, _hash):
        this->left_handed_set(this->mouse_settings_->get_boolean(key));
        break;
    case CONNECT(MOUSE_SCHEMA_MOTION_ACCELERATION, _hash):
        this->motion_acceleration_set(this->mouse_settings_->get_double(key));
        break;
    case CONNECT(MOUSE_SCHEMA_MIDDLE_EMULATION_ENABLED, _hash):
        this->middle_emulation_enabled_set(this->mouse_settings_->get_boolean(key));
        break;
    case CONNECT(MOUSE_SCHEMA_NATURAL_SCROLL, _hash):
        this->natural_scroll_set(this->mouse_settings_->get_boolean(key));
        break;
    default:
        break;
    }
}

void MouseManager::set_all_props_to_devices()
{
    this->set_left_handed_to_devices();
    this->set_motion_acceleration_to_devices();
    this->set_middle_emulation_enabled_to_devices();
    this->set_natural_scroll_to_devices();
}

#define SET_PROP_TO_DEVICES(set_device_fun)                                               \
    void MouseManager::set_device_fun##s()                                                \
    {                                                                                     \
        SETTINGS_PROFILE("");                                                             \
        int32_t n_devices = 0;                                                            \
        auto display = gdk_display_get_default();                                         \
        g_return_if_fail(display != NULL);                                                \
        auto devices_info = XListInputDevices(GDK_DISPLAY_XDISPLAY(display), &n_devices); \
                                                                                          \
        for (auto i = 0; i < n_devices; i++)                                              \
        {                                                                                 \
            auto device_helper = std::make_shared<DeviceHelper>(&devices_info[i]);        \
            if (device_helper)                                                            \
            {                                                                             \
                set_device_fun(device_helper);                                            \
            }                                                                             \
        }                                                                                 \
                                                                                          \
        if (devices_info != NULL)                                                         \
        {                                                                                 \
            XFreeDeviceList(devices_info);                                                \
        }                                                                                 \
    }

SET_PROP_TO_DEVICES(set_left_handed_to_device);
SET_PROP_TO_DEVICES(set_motion_acceleration_to_device);
SET_PROP_TO_DEVICES(set_middle_emulation_enabled_to_device);
SET_PROP_TO_DEVICES(set_natural_scroll_to_device);

void MouseManager::set_left_handed_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("device_name: %s.", device_helper->get_device_name().c_str());

    if (device_helper->has_property(MOUSE_PROP_LEFT_HANDED) &&
        !device_helper->is_touchpad())
    {
        device_helper->set_property(MOUSE_PROP_LEFT_HANDED, std::vector<bool>{this->left_handed_});
    }
}

void MouseManager::set_motion_acceleration_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("device_name: %s.", device_helper->get_device_name().c_str());

    if (device_helper->has_property(MOUSE_PROP_ACCEL_SPEED) &&
        !device_helper->is_touchpad())
    {
        device_helper->set_property(MOUSE_PROP_ACCEL_SPEED, (float)this->motion_acceleration_);
    }
}

void MouseManager::set_middle_emulation_enabled_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("device_name: %s.", device_helper->get_device_name().c_str());

    if (device_helper->has_property(MOUSE_PROP_MIDDLE_EMULATION_ENABLED) &&
        !device_helper->is_touchpad())
    {
        device_helper->set_property(MOUSE_PROP_MIDDLE_EMULATION_ENABLED, std::vector<bool>{this->middle_emulation_enabled_});
    }
}

void MouseManager::set_natural_scroll_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("device_name: %s.", device_helper->get_device_name().c_str());

    if (device_helper->has_property(MOUSE_PROP_NATURAL_SCROLL) &&
        !device_helper->is_touchpad())
    {
        device_helper->set_property(MOUSE_PROP_NATURAL_SCROLL, std::vector<bool>{this->natural_scroll_});
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