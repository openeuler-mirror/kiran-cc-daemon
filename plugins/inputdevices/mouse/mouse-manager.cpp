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

#include "plugins/inputdevices/mouse/mouse-manager.h"

#include "lib/base/base.h"
#include "mouse-i.h"
#include "plugins/inputdevices/common/xinput-helper.h"

namespace Kiran
{
#define MOUSE_SCHEMA_ID "com.kylinsec.kiran.mouse"
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

void MouseManager::init()
{
    if (!XInputHelper::supports_xinput_devices())
    {
        KLOG_WARNING_INPUTDEVICES("XInput is not supported, not applying any settings.");
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
    KLOG_DEBUG_INPUTDEVICES("Load from settings.");

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
    KLOG_DEBUG_INPUTDEVICES("The %s settings changed.", key.c_str());

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

void MouseManager::set_left_handed_to_devices()
{
    XInputHelper::foreach_device([this](std::shared_ptr<DeviceHelper> device_helper) {
        if (device_helper->has_property(MOUSE_PROP_LEFT_HANDED) &&
            !device_helper->is_touchpad())
        {
            device_helper->set_property(MOUSE_PROP_LEFT_HANDED, std::vector<bool>{this->left_handed_});
        }
    });
}

void MouseManager::set_motion_acceleration_to_devices()
{
    XInputHelper::foreach_device([this](std::shared_ptr<DeviceHelper> device_helper) {
        if (device_helper->has_property(MOUSE_PROP_ACCEL_SPEED) &&
            !device_helper->is_touchpad())
        {
            device_helper->set_property(MOUSE_PROP_ACCEL_SPEED, (float)this->motion_acceleration_);
        }
    });
}

void MouseManager::set_middle_emulation_enabled_to_devices()
{
    XInputHelper::foreach_device([this](std::shared_ptr<DeviceHelper> device_helper) {
        if (device_helper->has_property(MOUSE_PROP_MIDDLE_EMULATION_ENABLED) &&
            !device_helper->is_touchpad())
        {
            device_helper->set_property(MOUSE_PROP_MIDDLE_EMULATION_ENABLED, std::vector<bool>{this->middle_emulation_enabled_});
        }
    });
}

void MouseManager::set_natural_scroll_to_devices()
{
    XInputHelper::foreach_device([this](std::shared_ptr<DeviceHelper> device_helper) {
        if (device_helper->has_property(MOUSE_PROP_NATURAL_SCROLL) &&
            !device_helper->is_touchpad())
        {
            device_helper->set_property(MOUSE_PROP_NATURAL_SCROLL, std::vector<bool>{this->natural_scroll_});
        }
    });
}

void MouseManager::foreach_device(std::function<void(std::shared_ptr<DeviceHelper>)> callback)
{
    int32_t n_devices = 0;
    auto devices_info = XListInputDevices(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), &n_devices);

    for (auto i = 0; i < n_devices; i++)
    {
        if (strcmp(devices_info[i].name, "Virtual core pointer") == 0 ||
            strcmp(devices_info[i].name, "Virtual core keyboard") == 0)
        {
            KLOG_DEBUG_INPUTDEVICES("Ignore device: %s.", devices_info[i].name);
            continue;
        }
        auto device_helper = std::make_shared<DeviceHelper>(&devices_info[i]);
        if (device_helper)
        {
            callback(device_helper);
        }
    }

    if (devices_info != NULL)
    {
        XFreeDeviceList(devices_info);
    }
}

void MouseManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    if (!connect)
    {
        KLOG_WARNING_INPUTDEVICES("Failed to connect dbus with %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, MOUSE_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_INPUTDEVICES("Register object_path %s fail: %s.", MOUSE_OBJECT_PATH, e.what().c_str());
    }
}

void MouseManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_DEBUG_INPUTDEVICES("Success to register dbus name: %s", name.c_str());
}

void MouseManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_WARNING_INPUTDEVICES("Failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran