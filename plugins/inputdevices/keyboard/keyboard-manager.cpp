/*
 * @Author       : tangjie02
 * @Date         : 2020-08-12 16:25:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-13 10:08:43
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/inputdevices/keyboard/keyboard-manager.cpp
 */
#include "plugins/inputdevices/keyboard/keyboard-manager.h"

#include <X11/XKBlib.h>

#include "lib/log.h"

namespace Kiran
{
#define KEYBOARD_DBUS_NAME "com.unikylin.Kiran.SessionDaemon.Keyboard"
#define KEYBOARD_OBJECT_PATH "/com/unikylin/Kiran/SessionDaemon/Keyboard"

#define KEYBOARD_SCHEMA_ID "com.unikylin.kiran.keyboard"
#define KEYBOARD_SCHEMA_REPEAT_ENABLED "repeat-enabled"
#define KEYBOARD_SCHEMA_REPEAT_DELAY "repeat-delay"
#define KEYBOARD_SCHEMA_REPEAT_INTERVAL "repeat-interval"

KeyboardManager::KeyboardManager() : dbus_connect_id_(0),
                                     object_register_id_(0),
                                     repeat_enabled_(true),
                                     repeat_delay_(500),
                                     repeat_interval_(30)
{
    this->keyboard_settings_ = Gio::Settings::create(KEYBOARD_SCHEMA_ID);
}

KeyboardManager::~KeyboardManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

KeyboardManager *KeyboardManager::instance_ = nullptr;
void KeyboardManager::global_init()
{
    instance_ = new KeyboardManager();
    instance_->init();
}

void KeyboardManager::AddLayout(const Glib::ustring &layout, MethodInvocation &invocation)
{
}
void KeyboardManager::DelLayout(const Glib::ustring &layout, MethodInvocation &invocation)
{
}
void KeyboardManager::AddLayoutOption(const Glib::ustring &option, MethodInvocation &invocation)
{
}

void KeyboardManager::DelLayoutOption(const Glib::ustring &option, MethodInvocation &invocation)
{
}

void KeyboardManager::ClearLayoutOption(MethodInvocation &invocation)
{
}

#define AUTO_REPEAT_SET_HANDLER(prop, type, key, type2)                                \
    bool KeyboardManager::prop##_setHandler(type value)                                \
    {                                                                                  \
        SETTINGS_PROFILE("");                                                          \
        RETURN_VAL_IF_TRUE(value == this->prop##_, false);                             \
        if (g_settings_get_##type2(this->keyboard_settings_->gobj(), key) != value)    \
        {                                                                              \
            if (!g_settings_set_##type2(this->keyboard_settings_->gobj(), key, value)) \
            {                                                                          \
                return false;                                                          \
            }                                                                          \
        }                                                                              \
        this->prop##_ = value;                                                         \
        this->set_auto_repeat();                                                       \
        return true;                                                                   \
    }

AUTO_REPEAT_SET_HANDLER(repeat_enabled, bool, KEYBOARD_SCHEMA_REPEAT_ENABLED, boolean);
AUTO_REPEAT_SET_HANDLER(repeat_delay, gint32, KEYBOARD_SCHEMA_REPEAT_DELAY, int);
AUTO_REPEAT_SET_HANDLER(repeat_interval, gint32, KEYBOARD_SCHEMA_REPEAT_INTERVAL, int);

bool KeyboardManager::model_setHandler(const Glib::ustring &value)
{
    return true;
}

bool KeyboardManager::layouts_setHandler(const std::vector<Glib::ustring> &value)
{
    return true;
}

bool KeyboardManager::options_setHandler(const std::vector<Glib::ustring> &value)
{
    return true;
}

void KeyboardManager::init()
{
    this->load_from_settings();
    this->set_all_props();
    this->keyboard_settings_->signal_changed().connect(sigc::mem_fun(this, &KeyboardManager::settings_changed));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 KEYBOARD_DBUS_NAME,
                                                 sigc::mem_fun(this, &KeyboardManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &KeyboardManager::on_name_acquired),
                                                 sigc::mem_fun(this, &KeyboardManager::on_name_lost));
}

void KeyboardManager::load_from_settings()
{
    SETTINGS_PROFILE("");

    if (this->keyboard_settings_)
    {
        this->repeat_enabled_ = this->keyboard_settings_->get_boolean(KEYBOARD_SCHEMA_REPEAT_ENABLED);
        this->repeat_delay_ = this->keyboard_settings_->get_int(KEYBOARD_SCHEMA_REPEAT_DELAY);
        this->repeat_interval_ = this->keyboard_settings_->get_int(KEYBOARD_SCHEMA_REPEAT_INTERVAL);
    }
}

void KeyboardManager::settings_changed(const Glib::ustring &key)
{
    SETTINGS_PROFILE("key: %s.", key.c_str());

    switch (shash(key.c_str()))
    {
    case CONNECT(KEYBOARD_SCHEMA_REPEAT_ENABLED, _hash):
        this->repeat_enabled_set(this->keyboard_settings_->get_boolean(key));
        break;
    case CONNECT(KEYBOARD_SCHEMA_REPEAT_DELAY, _hash):
        this->repeat_delay_set(this->keyboard_settings_->get_int(key));
        break;
    case CONNECT(KEYBOARD_SCHEMA_REPEAT_INTERVAL, _hash):
        this->repeat_interval_set(this->keyboard_settings_->get_int(key));
        break;
    default:
        break;
    }
}

void KeyboardManager::set_all_props()
{
    this->set_auto_repeat();
}

void KeyboardManager::set_auto_repeat()
{
    SETTINGS_PROFILE("repeat_enabled: %d repeat_delay: %d repeat_interval: %d.",
                     this->repeat_enabled_,
                     this->repeat_delay_,
                     this->repeat_interval_);

    auto display = gdk_display_get_default();

    if (this->repeat_enabled_)
    {
        XAutoRepeatOn(GDK_DISPLAY_XDISPLAY(display));

        auto ret = XkbSetAutoRepeatRate(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()),
                                        XkbUseCoreKbd,
                                        this->repeat_delay_,
                                        this->repeat_interval_);

        if (!ret)
        {
            LOG_WARNING("XKeyboard keyboard extensions are unavailable, no way to support keyboard autorepeat rate settings");
        }
    }
    else
    {
        XAutoRepeatOff(GDK_DISPLAY_XDISPLAY(display));
    }
}

void KeyboardManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    SETTINGS_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        LOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, KEYBOARD_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("register object_path %s fail: %s.", KEYBOARD_OBJECT_PATH, e.what().c_str());
    }
}
void KeyboardManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}
void KeyboardManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran