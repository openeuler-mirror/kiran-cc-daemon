/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 10:09:05
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-10 11:08:52
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/inputdevices/touchpad/touchpad-manager.cpp
 */

#include "plugins/inputdevices/touchpad/touchpad-manager.h"

#include "lib/helper.h"
#include "lib/log.h"
#include "plugins/inputdevices/common/xinput-helper.h"

namespace Kiran
{
#define TOUCHPAD_DBUS_NAME "com.unikylin.Kiran.SessionDaemon.TouchPad"
#define TOUCHPAD_OBJECT_PATH "/com/unikylin/Kiran/SessionDaemon/TouchPad"

#define TOUCHPAD_SCHEMA_ID "com.unikylin.kiran.touchpad"
#define TOUCHPAD_SCHEMA_LEFT_HANDED "left-handed"
#define TOUCHPAD_SCHEMA_DISABLE_WHILE_TYPING "disable-while-typing"
#define TOUCHPAD_SCHEMA_TAP_TO_CLICK "tap-to-click"
#define TOUCHPAD_SCHEMA_CLICK_METHOD "click-method"
#define TOUCHPAD_SCHEMA_SCROLL_METHOD "scroll-method"
#define TOUCHPAD_SCHEMA_NATURAL_SCROLL "natural-scroll"
#define TOUCHPAD_SCHEMA_TOUCHPAD_ENABLED "touchpad-enabled"
#define TOUCHPAD_SCHEMA_MOTION_ACCELERATION "motion-acceleration"

#define TOUCHPAD_PROP_LEFT_HANDED "libinput Left Handed Enabled"
#define TOUCHPAD_PROP_DISABLE_WHILE_TYPING "libinput Disable While Typing Enabled"
#define TOUCHPAD_PROP_TAPPING_ENABLED "libinput Tapping Enabled"
#define TOUCHPAD_PROP_CLICK_METHOD "libinput Click Method Enabled"
#define TOUCHPAD_PROP_SCROLL_METHOD "libinput Scroll Method Enabled"
#define TOUCHPAD_PROP_NATURAL_SCROLL "libinput Natural Scrolling Enabled"
#define TOUCHPAD_PROP_DEVICE_ENABLED "Device Enabled"
#define TOUCHPAD_PROP_ACCEL_SPEED "libinput Accel Speed"

TouchPadManager::TouchPadManager() : dbus_connect_id_(0),
                                     object_register_id_(0),
                                     left_handed_(false),
                                     disable_while_typing_(false),
                                     tap_to_click_(true),
                                     click_method_(0),
                                     scroll_method_(0),
                                     natural_scroll_(false),
                                     touchpad_enabled_(true),
                                     motion_acceleration_(0)
{
    this->touchpad_settings_ = Gio::Settings::create(TOUCHPAD_SCHEMA_ID);
}

TouchPadManager::~TouchPadManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

TouchPadManager *TouchPadManager::instance_ = nullptr;
void TouchPadManager::global_init()
{
    instance_ = new TouchPadManager();
    instance_->init();
}

void TouchPadManager::Reset(MethodInvocation &invocation)
{
}

#define PROP_SET_HANDLER(prop, type, key, set_fun)        \
    bool TouchPadManager::prop##_setHandler(type value)   \
    {                                                     \
        SETTINGS_PROFILE("");                             \
        RETURN_VAL_IF_TRUE(value == this->prop##_, true); \
                                                          \
        this->prop##_ = value;                            \
        this->touchpad_settings_->set_fun(key, value);    \
        this->set_##prop##_to_devices();                  \
        return true;                                      \
    }

PROP_SET_HANDLER(left_handed, bool, "left-handed", set_boolean);
PROP_SET_HANDLER(disable_while_typing, bool, "disable-while-typing", set_boolean);
PROP_SET_HANDLER(tap_to_click, bool, "tap-to-click", set_boolean);
PROP_SET_HANDLER(click_method, gint32, "click-method", set_int);
PROP_SET_HANDLER(scroll_method, gint32, "scroll-method", set_int);
PROP_SET_HANDLER(natural_scroll, bool, "natural-scroll", set_boolean);
PROP_SET_HANDLER(touchpad_enabled, bool, "touchpad-enabled", set_boolean);
PROP_SET_HANDLER(motion_acceleration, double, "motion-acceleration", set_double);

void TouchPadManager::init()
{
    SETTINGS_PROFILE("");

    if (!XInputHelper::supports_xinput_devices())
    {
        LOG_WARNING("XInput is not supported, not applying any settings.");
        return;
    }

    this->load_from_settings();
    this->set_all_prop_to_devices();

    this->touchpad_settings_->signal_changed().connect(sigc::mem_fun(this, &TouchPadManager::settings_changed));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 TOUCHPAD_DBUS_NAME,
                                                 sigc::mem_fun(this, &TouchPadManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &TouchPadManager::on_name_acquired),
                                                 sigc::mem_fun(this, &TouchPadManager::on_name_lost));
}

void TouchPadManager::load_from_settings()
{
    SETTINGS_PROFILE("");

    if (this->touchpad_settings_)
    {
        this->left_handed_ = this->touchpad_settings_->get_boolean(TOUCHPAD_SCHEMA_LEFT_HANDED);
        this->disable_while_typing_ = this->touchpad_settings_->get_boolean(TOUCHPAD_SCHEMA_DISABLE_WHILE_TYPING);
        this->tap_to_click_ = this->touchpad_settings_->get_boolean(TOUCHPAD_SCHEMA_TAP_TO_CLICK);
        this->click_method_ = this->touchpad_settings_->get_int(TOUCHPAD_SCHEMA_CLICK_METHOD);
        this->scroll_method_ = this->touchpad_settings_->get_int(TOUCHPAD_SCHEMA_SCROLL_METHOD);
        this->natural_scroll_ = this->touchpad_settings_->get_boolean(TOUCHPAD_SCHEMA_NATURAL_SCROLL);
        this->touchpad_enabled_ = this->touchpad_settings_->get_boolean(TOUCHPAD_SCHEMA_TOUCHPAD_ENABLED);
        this->motion_acceleration_ = this->touchpad_settings_->get_double(TOUCHPAD_SCHEMA_MOTION_ACCELERATION);
    }
}

void TouchPadManager::settings_changed(const Glib::ustring &key)
{
    SETTINGS_PROFILE("key: %s.", key.c_str());

    switch (shash(key.c_str()))
    {
    case "left-handed"_hash:
        this->left_handed_set(this->touchpad_settings_->get_boolean(key));
        break;
    case "disable-while-typing"_hash:
        this->disable_while_typing_set(this->touchpad_settings_->get_boolean(key));
        break;
    case "tap-to-click"_hash:
        this->tap_to_click_set(this->touchpad_settings_->get_boolean(key));
        break;
    case "click-method"_hash:
        this->click_method_set(this->touchpad_settings_->get_int(key));
        break;
    case "scroll-method"_hash:
        this->scroll_method_set(this->touchpad_settings_->get_int(key));
        break;
    case "natural-scroll"_hash:
        this->natural_scroll_set(this->touchpad_settings_->get_boolean(key));
        break;
    case "touchpad-enabled"_hash:
        this->touchpad_enabled_set(this->touchpad_settings_->get_boolean(key));
        break;
    case "motion-acceleration"_hash:
        this->motion_acceleration_set(this->touchpad_settings_->get_double(key));
        break;
    default:
        break;
    }
}

void TouchPadManager::set_all_prop_to_devices()
{
    this->set_left_handed_to_devices();
    this->set_disable_while_typing_to_devices();
    this->set_tap_to_click_to_devices();
    this->set_click_method_to_devices();
    this->set_scroll_method_to_devices();
    this->set_natural_scroll_to_devices();
    this->set_touchpad_enabled_to_devices();
    this->set_motion_acceleration_to_devices();
}

#define SET_PROP_TO_DEVICES(set_device_fun)                                                                 \
    void TouchPadManager::set_device_fun##s()                                                               \
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
SET_PROP_TO_DEVICES(set_disable_while_typing_to_device);
SET_PROP_TO_DEVICES(set_tap_to_click_to_device);
SET_PROP_TO_DEVICES(set_click_method_to_device);
SET_PROP_TO_DEVICES(set_scroll_method_to_device);
SET_PROP_TO_DEVICES(set_natural_scroll_to_device);
SET_PROP_TO_DEVICES(set_touchpad_enabled_to_device);
SET_PROP_TO_DEVICES(set_motion_acceleration_to_device);

void TouchPadManager::set_left_handed_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("device_name: %s.", device_helper->get_device_name().c_str());

    if (device_helper->has_property(TOUCHPAD_PROP_LEFT_HANDED) &&
        device_helper->is_touchpad())
    {
        device_helper->set_property(TOUCHPAD_PROP_LEFT_HANDED, std::vector<bool>{this->left_handed_});
    }
}

void TouchPadManager::set_disable_while_typing_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("device_name: %s.", device_helper->get_device_name().c_str());

    if (device_helper->has_property(TOUCHPAD_PROP_DISABLE_WHILE_TYPING) &&
        device_helper->is_touchpad())
    {
        device_helper->set_property(TOUCHPAD_PROP_DISABLE_WHILE_TYPING, std::vector<bool>{this->disable_while_typing_});
    }
}

void TouchPadManager::set_tap_to_click_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("device_name: %s.", device_helper->get_device_name().c_str());

    if (device_helper->has_property(TOUCHPAD_PROP_TAPPING_ENABLED) &&
        device_helper->is_touchpad())
    {
        device_helper->set_property(TOUCHPAD_PROP_TAPPING_ENABLED, std::vector<bool>{this->tap_to_click_});
    }
}

void TouchPadManager::set_click_method_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("device_name: %s.", device_helper->get_device_name().c_str());

    if (device_helper->has_property(TOUCHPAD_PROP_CLICK_METHOD) &&
        device_helper->is_touchpad())
    {
        switch (this->click_method_)
        {
        case int32_t(ClickMethod::BUTTON_AREAS):
            device_helper->set_property(TOUCHPAD_PROP_CLICK_METHOD, std::vector<bool>{true, false});
            break;
        case int32_t(ClickMethod::CLICK_FINGER):
            device_helper->set_property(TOUCHPAD_PROP_CLICK_METHOD, std::vector<bool>{false, true});
            break;
        default:
            LOG_WARNING("unknow click methods: %d.", this->click_method_);
            break;
        }
    }
}

void TouchPadManager::set_scroll_method_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("device_name: %s.", device_helper->get_device_name().c_str());

    if (device_helper->has_property(TOUCHPAD_PROP_SCROLL_METHOD) &&
        device_helper->is_touchpad())
    {
        switch (this->scroll_method_)
        {
        case int32_t(ScrollMethod::TWO_FINGER):
            device_helper->set_property(TOUCHPAD_PROP_SCROLL_METHOD, std::vector<bool>{true, false, false});
            break;
        case int32_t(ScrollMethod::EDGE):
            device_helper->set_property(TOUCHPAD_PROP_SCROLL_METHOD, std::vector<bool>{false, true, false});
            break;
        case int32_t(ScrollMethod::BUTTON):
            device_helper->set_property(TOUCHPAD_PROP_SCROLL_METHOD, std::vector<bool>{false, false, true});
            break;
        default:
            LOG_WARNING("unknow scroll methods: %d.", this->scroll_method_);
            break;
        }
    }
}

void TouchPadManager::set_natural_scroll_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("device_name: %s.", device_helper->get_device_name().c_str());

    if (device_helper->has_property(TOUCHPAD_PROP_NATURAL_SCROLL) &&
        device_helper->is_touchpad())
    {
        device_helper->set_property(TOUCHPAD_PROP_NATURAL_SCROLL, std::vector<bool>{this->natural_scroll_});
    }
}

void TouchPadManager::set_touchpad_enabled_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("device_name: %s.", device_helper->get_device_name().c_str());

    if (device_helper->has_property(TOUCHPAD_PROP_DEVICE_ENABLED) &&
        device_helper->is_touchpad())
    {
        device_helper->set_property(TOUCHPAD_PROP_DEVICE_ENABLED, std::vector<bool>{this->touchpad_enabled_});
    }
}

void TouchPadManager::set_motion_acceleration_to_device(std::shared_ptr<DeviceHelper> device_helper)
{
    SETTINGS_PROFILE("device_name: %s.", device_helper->get_device_name().c_str());

    if (device_helper->has_property(TOUCHPAD_PROP_ACCEL_SPEED) &&
        device_helper->is_touchpad())
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
        device_helper->set_property(TOUCHPAD_PROP_ACCEL_SPEED, motion_accel);
    }
}

void TouchPadManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    SETTINGS_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        LOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, TOUCHPAD_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("register object_path %s fail: %s.", TOUCHPAD_OBJECT_PATH, e.what().c_str());
    }
}

void TouchPadManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void TouchPadManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_WARNING("failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran