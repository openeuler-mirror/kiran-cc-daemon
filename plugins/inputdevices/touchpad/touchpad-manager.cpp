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

#include "plugins/inputdevices/touchpad/touchpad-manager.h"

#include "lib/base/base.h"
#include "plugins/inputdevices/common/xinput-helper.h"
#include "touchpad-i.h"

namespace Kiran
{
#define X_HASH(X) CONNECT(X, _hash)

#define TOUCHPAD_SCHEMA_ID "com.kylinsec.kiran.touchpad"
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
                                     has_touchpad_(false),
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
    this->left_handed_set(false);
    this->disable_while_typing_set(false);
    this->tap_to_click_set(true);
    this->click_method_set(0);
    this->scroll_method_set(0);
    this->natural_scroll_set(false);
    this->touchpad_enabled_set(true);
    this->motion_acceleration_set(0);
}

#define PROP_SET_HANDLER(prop, type, key, type2)                                       \
    bool TouchPadManager::prop##_setHandler(type value)                                \
    {                                                                                  \
        KLOG_PROFILE("value: %s.", fmt::format("{0}", value).c_str());                 \
        RETURN_VAL_IF_TRUE(value == this->prop##_, false);                             \
        if (g_settings_get_##type2(this->touchpad_settings_->gobj(), key) != value)    \
        {                                                                              \
            if (!g_settings_set_##type2(this->touchpad_settings_->gobj(), key, value)) \
            {                                                                          \
                return false;                                                          \
            }                                                                          \
        }                                                                              \
        this->prop##_ = value;                                                         \
        this->set_##prop##_to_devices();                                               \
        return true;                                                                   \
    }

PROP_SET_HANDLER(left_handed, bool, TOUCHPAD_SCHEMA_LEFT_HANDED, boolean);
PROP_SET_HANDLER(disable_while_typing, bool, TOUCHPAD_SCHEMA_DISABLE_WHILE_TYPING, boolean);
PROP_SET_HANDLER(tap_to_click, bool, TOUCHPAD_SCHEMA_TAP_TO_CLICK, boolean);
PROP_SET_HANDLER(click_method, gint32, TOUCHPAD_SCHEMA_CLICK_METHOD, int);
PROP_SET_HANDLER(scroll_method, gint32, TOUCHPAD_SCHEMA_SCROLL_METHOD, int);
PROP_SET_HANDLER(natural_scroll, bool, TOUCHPAD_SCHEMA_NATURAL_SCROLL, boolean);
PROP_SET_HANDLER(touchpad_enabled, bool, TOUCHPAD_SCHEMA_TOUCHPAD_ENABLED, boolean);
PROP_SET_HANDLER(motion_acceleration, double, TOUCHPAD_SCHEMA_MOTION_ACCELERATION, double);

void TouchPadManager::init()
{
    KLOG_PROFILE("");

    if (!XInputHelper::supports_xinput_devices())
    {
        KLOG_WARNING("XInput is not supported, not applying any settings.");
        return;
    }

    XInputHelper::foreach_device([this](std::shared_ptr<DeviceHelper> device_helper) {
        if (device_helper->is_touchpad())
        {
            this->has_touchpad_ = true;
        }
    });

    this->load_from_settings();
    this->set_all_props_to_devices();

    this->touchpad_settings_->signal_changed().connect(sigc::mem_fun(this, &TouchPadManager::settings_changed));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 TOUCHPAD_DBUS_NAME,
                                                 sigc::mem_fun(this, &TouchPadManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &TouchPadManager::on_name_acquired),
                                                 sigc::mem_fun(this, &TouchPadManager::on_name_lost));
}

void TouchPadManager::load_from_settings()
{
    KLOG_PROFILE("");

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
    KLOG_PROFILE("key: %s.", key.c_str());

    switch (shash(key.c_str()))
    {
    case CONNECT(TOUCHPAD_SCHEMA_LEFT_HANDED, _hash):
        this->left_handed_set(this->touchpad_settings_->get_boolean(key));
        break;
    case CONNECT(TOUCHPAD_SCHEMA_DISABLE_WHILE_TYPING, _hash):
        this->disable_while_typing_set(this->touchpad_settings_->get_boolean(key));
        break;
    case CONNECT(TOUCHPAD_SCHEMA_TAP_TO_CLICK, _hash):
        this->tap_to_click_set(this->touchpad_settings_->get_boolean(key));
        break;
    case CONNECT(TOUCHPAD_SCHEMA_CLICK_METHOD, _hash):
        this->click_method_set(this->touchpad_settings_->get_int(key));
        break;
    case CONNECT(TOUCHPAD_SCHEMA_SCROLL_METHOD, _hash):
        this->scroll_method_set(this->touchpad_settings_->get_int(key));
        break;
    case CONNECT(TOUCHPAD_SCHEMA_NATURAL_SCROLL, _hash):
        this->natural_scroll_set(this->touchpad_settings_->get_boolean(key));
        break;
    case CONNECT(TOUCHPAD_SCHEMA_TOUCHPAD_ENABLED, _hash):
        this->touchpad_enabled_set(this->touchpad_settings_->get_boolean(key));
        break;
    case CONNECT(TOUCHPAD_SCHEMA_MOTION_ACCELERATION, _hash):
        this->motion_acceleration_set(this->touchpad_settings_->get_double(key));
        break;
    default:
        break;
    }
}

void TouchPadManager::set_all_props_to_devices()
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

void TouchPadManager::set_left_handed_to_devices()
{
    KLOG_PROFILE("");

    XInputHelper::foreach_device([this](std::shared_ptr<DeviceHelper> device_helper) {
        if (device_helper->has_property(TOUCHPAD_PROP_LEFT_HANDED) &&
            device_helper->is_touchpad())
        {
            device_helper->set_property(TOUCHPAD_PROP_LEFT_HANDED, std::vector<bool>{this->left_handed_});
        }
    });
}

void TouchPadManager::set_disable_while_typing_to_devices()
{
    KLOG_PROFILE("");

    XInputHelper::foreach_device([this](std::shared_ptr<DeviceHelper> device_helper) {
        if (device_helper->has_property(TOUCHPAD_PROP_DISABLE_WHILE_TYPING) &&
            device_helper->is_touchpad())
        {
            device_helper->set_property(TOUCHPAD_PROP_DISABLE_WHILE_TYPING, std::vector<bool>{this->disable_while_typing_});
        }
    });
}

void TouchPadManager::set_tap_to_click_to_devices()
{
    KLOG_PROFILE("");

    XInputHelper::foreach_device([this](std::shared_ptr<DeviceHelper> device_helper) {
        if (device_helper->has_property(TOUCHPAD_PROP_TAPPING_ENABLED) &&
            device_helper->is_touchpad())
        {
            device_helper->set_property(TOUCHPAD_PROP_TAPPING_ENABLED, std::vector<bool>{this->tap_to_click_});
        }
    });
}

void TouchPadManager::set_click_method_to_devices()
{
    KLOG_PROFILE("");

    XInputHelper::foreach_device([this](std::shared_ptr<DeviceHelper> device_helper) {
        if (device_helper->has_property(TOUCHPAD_PROP_CLICK_METHOD) &&
            device_helper->is_touchpad())
        {
            switch (this->click_method_)
            {
            case int32_t(TouchPadClickMethod::TOUCHPAD_CLICK_METHOD_BUTTON_AREAS):
                device_helper->set_property(TOUCHPAD_PROP_CLICK_METHOD, std::vector<bool>{true, false});
                break;
            case int32_t(TouchPadClickMethod::TOUCHPAD_CLICK_METHOD_CLICK_FINGER):
                device_helper->set_property(TOUCHPAD_PROP_CLICK_METHOD, std::vector<bool>{false, true});
                break;
            default:
                KLOG_WARNING("unknow click methods: %d.", this->click_method_);
                break;
            }
        }
    });
}

void TouchPadManager::set_scroll_method_to_devices()
{
    KLOG_PROFILE("");

    XInputHelper::foreach_device([this](std::shared_ptr<DeviceHelper> device_helper) {
        if (device_helper->has_property(TOUCHPAD_PROP_SCROLL_METHOD) &&
            device_helper->is_touchpad())
        {
            switch (this->scroll_method_)
            {
            case int32_t(TouchPadScrollMethod::TOUCHPAD_SCROLL_METHOD_TWO_FINGER):
                device_helper->set_property(TOUCHPAD_PROP_SCROLL_METHOD, std::vector<bool>{true, false, false});
                break;
            case int32_t(TouchPadScrollMethod::TOUCHPAD_SCROLL_METHOD_EDGE):
                device_helper->set_property(TOUCHPAD_PROP_SCROLL_METHOD, std::vector<bool>{false, true, false});
                break;
            case int32_t(TouchPadScrollMethod::TOUCHPAD_SCROLL_METHOD_BUTTON):
                device_helper->set_property(TOUCHPAD_PROP_SCROLL_METHOD, std::vector<bool>{false, false, true});
                break;
            default:
                KLOG_WARNING("unknow scroll methods: %d.", this->scroll_method_);
                break;
            }
        }
    });
}

void TouchPadManager::set_natural_scroll_to_devices()
{
    KLOG_PROFILE("");

    XInputHelper::foreach_device([this](std::shared_ptr<DeviceHelper> device_helper) {
        if (device_helper->has_property(TOUCHPAD_PROP_NATURAL_SCROLL) &&
            device_helper->is_touchpad())
        {
            device_helper->set_property(TOUCHPAD_PROP_NATURAL_SCROLL, std::vector<bool>{this->natural_scroll_});
        }
    });
}

void TouchPadManager::set_touchpad_enabled_to_devices()
{
    KLOG_PROFILE("");

    XInputHelper::foreach_device([this](std::shared_ptr<DeviceHelper> device_helper) {
        if (device_helper->has_property(TOUCHPAD_PROP_DEVICE_ENABLED) &&
            device_helper->is_touchpad())
        {
            device_helper->set_property(TOUCHPAD_PROP_DEVICE_ENABLED, std::vector<bool>{this->touchpad_enabled_});
        }
    });
}

void TouchPadManager::set_motion_acceleration_to_devices()
{
    KLOG_PROFILE("");

    XInputHelper::foreach_device([this](std::shared_ptr<DeviceHelper> device_helper) {
        if (device_helper->has_property(TOUCHPAD_PROP_ACCEL_SPEED) &&
            device_helper->is_touchpad())
        {
            device_helper->set_property(TOUCHPAD_PROP_ACCEL_SPEED, (float)this->motion_acceleration_);
        }
    });
}

void TouchPadManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        KLOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, TOUCHPAD_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("register object_path %s fail: %s.", TOUCHPAD_OBJECT_PATH, e.what().c_str());
    }
}

void TouchPadManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void TouchPadManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_WARNING("failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran