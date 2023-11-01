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

#include "plugins/inputdevices/keyboard/keyboard-manager.h"

#include <X11/XKBlib.h>

#include "keyboard-i.h"
#include "lib/base/base.h"
#include "lib/dbus/dbus.h"
#include "lib/iso/iso-translation.h"
#include "plugins/inputdevices/keyboard/xkb-rules-parser.h"

namespace Kiran
{
#define KEYBOARD_SCHEMA_ID "com.kylinsec.kiran.keyboard"
#define KEYBOARD_SCHEMA_MODIFIER_LOCK_ENABLED "modifier-lock-enabled"
#define KEYBOARD_SCHEMA_CAPSLOCK_TIPS_ENABLED "capslock-tips-enabled"
#define KEYBOARD_SCHEMA_NUMLOCK_TIPS_ENABLED "numlock-tips-enabled"
#define KEYBOARD_SCHEMA_REPEAT_ENABLED "repeat-enabled"
#define KEYBOARD_SCHEMA_REPEAT_DELAY "repeat-delay"
#define KEYBOARD_SCHEMA_REPEAT_INTERVAL "repeat-interval"
#define KEYBOARD_SCHEMA_LAYOUTS "layouts"
#define KEYBOARD_SCHEMA_OPTIONS "options"

#define LAYOUT_JOIN_CHAR ","
#define LAYOUT_MAX_NUMBER 4
#define DEFAULT_LAYOUT "us"
#define SETXKBMAP "setxkbmap"

#define DEFAULT_XKB_RULES_FILE KCC_XKB_BASE "/rules/base.xml"
#define DEFAULT_DESC_DELIMETERS " (,)"

KeyboardManager::KeyboardManager() : dbus_connect_id_(0),
                                     object_register_id_(0),
                                     modifier_lock_enabled_(false),
                                     capslock_tips_enabled_(false),
                                     numlock_tips_enabled_(false),
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
    auto layouts = this->layouts_get();
    if (layouts.size() >= LAYOUT_MAX_NUMBER)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_EXCEED_LIMIT, LAYOUT_MAX_NUMBER);
    }

    if (this->valid_layouts_.find(layout) == this->valid_layouts_.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_INVALID);
    }

    auto iter = std::find(layouts.begin(), layouts.end(), layout);
    if (iter != layouts.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_ALREADY_EXIST);
    }

    layouts.push_back(layout);
    if (!this->layouts_set(layouts))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_SET_FAILED);
    }
    invocation.ret();
}

void KeyboardManager::DelLayout(const Glib::ustring &layout, MethodInvocation &invocation)
{
    auto layouts = this->layouts_get();
    auto iter = std::find(layouts.begin(), layouts.end(), layout);

    if (iter == layouts.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_NOT_EXIST);
    }
    layouts.erase(iter);

    if (!this->layouts_set(layouts))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_UPDATE_FAILED);
    }
    invocation.ret();
}

void KeyboardManager::ApplyLayout(const Glib::ustring &layout, MethodInvocation &invocation)
{
    auto layouts = this->layouts_get();
    auto iter = std::find(layouts.begin(), layouts.end(), layout);
    if (iter == layouts.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_NOT_EXIST);
    }
    layouts.erase(iter);
    layouts.insert(layouts.begin(), layout);
    if (!this->layouts_set(layouts))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_UPDATE_FAILED);
    }
    invocation.ret();
}

void KeyboardManager::GetValidLayouts(MethodInvocation &invocation)
{
    try
    {
        Json::Value values;
        for (auto &iter : this->valid_layouts_)
        {
            Json::Value value;
            value[KEYBOARD_VALID_LAYOUTS_LAYOUT_NAME] = std::string(iter.first);
            value[KEYBOARD_VALID_LAYOUTS_COUNTRY_NAME] = std::string(iter.second);
            values.append(std::move(value));
        }
        auto retval = StrUtils::json2str(values);
        invocation.ret(retval);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING_INPUTDEVICES("%s.", e.what());
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_GET_FAILED);
    }
}

void KeyboardManager::AddLayoutOption(const Glib::ustring &option, MethodInvocation &invocation)
{
    auto options = this->options_;

    auto iter = std::find(options.begin(), options.end(), option);

    if (iter != options.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_OPTION_ALREADY_EXIST);
    }

    options.push_back(option);
    if (!this->options_set(options))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_OPTION_SET_FAILED);
    }
    invocation.ret();
}

void KeyboardManager::DelLayoutOption(const Glib::ustring &option, MethodInvocation &invocation)
{
    auto options = this->options_;

    auto iter = std::find(options.begin(), options.end(), option);

    if (iter == options.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_OPTION_NOT_EXIST);
    }
    options.erase(iter);

    if (!this->options_set(options))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_OPTION_UPDATE_FAILED);
    }
    invocation.ret();
}

void KeyboardManager::ClearLayoutOption(MethodInvocation &invocation)
{
    if (!this->options_set(std::vector<Glib::ustring>()))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_OPTION_CLEAR_FAILED);
    }
    invocation.ret();
}

void KeyboardManager::SwitchCapsLockTips(bool enabled, MethodInvocation &invocation)
{
    if (!this->capslock_tips_enabled_set(enabled))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_SWITCH_CAPSLOCK_TIPS_FAILED);
    }

    invocation.ret();
}

void KeyboardManager::SwitchNumLockTips(bool enabled, MethodInvocation &invocation)
{
    if (!this->numlock_tips_enabled_set(enabled))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_SWITCH_NUMLOCK_TIPS_FAILED);
    }

    invocation.ret();
}

#define AUTO_REPEAT_SET_HANDLER(prop, type1, key, type2)                                                           \
    bool KeyboardManager::prop##_setHandler(type1 value)                                                           \
    {                                                                                                              \
        RETURN_VAL_IF_TRUE(value == this->prop##_, false);                                                         \
        if (this->keyboard_settings_->get_##type2(key) != value)                                                   \
        {                                                                                                          \
            auto value_r = Glib::Variant<std::remove_cv<std::remove_reference<type1>::type>::type>::create(value); \
            if (!this->keyboard_settings_->set_value(key, value_r))                                                \
            {                                                                                                      \
                return false;                                                                                      \
            }                                                                                                      \
        }                                                                                                          \
        this->prop##_ = value;                                                                                     \
        this->set_auto_repeat();                                                                                   \
        return true;                                                                                               \
    }

AUTO_REPEAT_SET_HANDLER(repeat_enabled, bool, KEYBOARD_SCHEMA_REPEAT_ENABLED, boolean);
AUTO_REPEAT_SET_HANDLER(repeat_delay, gint32, KEYBOARD_SCHEMA_REPEAT_DELAY, int);
AUTO_REPEAT_SET_HANDLER(repeat_interval, gint32, KEYBOARD_SCHEMA_REPEAT_INTERVAL, int);

#define KEYBOARD_PROP_SET_HANDLER(prop, type1, key, type2)                                                         \
    bool KeyboardManager::prop##_setHandler(type1 value)                                                           \
    {                                                                                                              \
        RETURN_VAL_IF_TRUE(value == this->prop##_, false);                                                         \
        if (this->keyboard_settings_->get_##type2(key) != value)                                                   \
        {                                                                                                          \
            auto value_r = Glib::Variant<std::remove_cv<std::remove_reference<type1>::type>::type>::create(value); \
            if (!this->keyboard_settings_->set_value(key, value_r))                                                \
            {                                                                                                      \
                return false;                                                                                      \
            }                                                                                                      \
        }                                                                                                          \
        this->prop##_ = value;                                                                                     \
        return true;                                                                                               \
    }

KEYBOARD_PROP_SET_HANDLER(capslock_tips_enabled, bool, KEYBOARD_SCHEMA_CAPSLOCK_TIPS_ENABLED, boolean);
KEYBOARD_PROP_SET_HANDLER(numlock_tips_enabled, bool, KEYBOARD_SCHEMA_NUMLOCK_TIPS_ENABLED, boolean);

bool KeyboardManager::modifier_lock_enabled_setHandler(bool value)
{
    // do nothing
    return true;
}

bool KeyboardManager::layouts_setHandler(const std::vector<Glib::ustring> &value)
{
    auto layouts = value;

    if (layouts.size() > LAYOUT_MAX_NUMBER)
    {
        KLOG_WARNING_INPUTDEVICES("The number of the layouts set has %d. it exceed max layout number(%d). the subsequent layout is ignored.",
                                  layouts.size(),
                                  LAYOUT_MAX_NUMBER);

        layouts.resize(LAYOUT_MAX_NUMBER);
    }

    if (layouts.size() == 0)
    {
        KLOG_WARNING_INPUTDEVICES("Because the user layout list is empty, so set the default layout 'us'.");
        layouts.push_back(DEFAULT_LAYOUT);
    }

    if (this->layouts_.size() == layouts.size() &&
        std::equal(this->layouts_.begin(), this->layouts_.end(), layouts.begin()))
    {
        return false;
    }

    bool result = this->set_layouts(layouts);

    if (result)
    {
        this->layouts_ = layouts;
    }

    this->keyboard_settings_->set_string_array(KEYBOARD_SCHEMA_LAYOUTS, this->layouts_);
    return result;
}

bool KeyboardManager::options_setHandler(const std::vector<Glib::ustring> &value)
{
    if (this->options_.size() == value.size() &&
        std::equal(this->options_.begin(), this->options_.end(), value.begin()))
    {
        return false;
    }

    bool result = this->set_options(value);

    if (result)
    {
        this->options_ = value;
    }

    this->keyboard_settings_->set_string_array(KEYBOARD_SCHEMA_OPTIONS, this->options_);
    return result;
}

void KeyboardManager::init()
{
    this->load_from_settings();
    this->load_xkb_rules();
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
    if (this->keyboard_settings_)
    {
        this->modifier_lock_enabled_ = this->keyboard_settings_->get_boolean(KEYBOARD_SCHEMA_MODIFIER_LOCK_ENABLED);
        this->capslock_tips_enabled_ = this->keyboard_settings_->get_boolean(KEYBOARD_SCHEMA_CAPSLOCK_TIPS_ENABLED);
        this->numlock_tips_enabled_ = this->keyboard_settings_->get_boolean(KEYBOARD_SCHEMA_NUMLOCK_TIPS_ENABLED);
        this->repeat_enabled_ = this->keyboard_settings_->get_boolean(KEYBOARD_SCHEMA_REPEAT_ENABLED);
        this->repeat_delay_ = this->keyboard_settings_->get_int(KEYBOARD_SCHEMA_REPEAT_DELAY);
        this->repeat_interval_ = this->keyboard_settings_->get_int(KEYBOARD_SCHEMA_REPEAT_INTERVAL);
        this->layouts_ = this->keyboard_settings_->get_string_array(KEYBOARD_SCHEMA_LAYOUTS);
        this->options_ = this->keyboard_settings_->get_string_array(KEYBOARD_SCHEMA_OPTIONS);
    }
}

void KeyboardManager::settings_changed(const Glib::ustring &key)
{
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
    case CONNECT(KEYBOARD_SCHEMA_LAYOUTS, _hash):
        this->layouts_set(this->keyboard_settings_->get_string_array(key));
        break;
    case CONNECT(KEYBOARD_SCHEMA_OPTIONS, _hash):
        this->options_set(this->keyboard_settings_->get_string_array(key));
        break;
    case CONNECT(KEYBOARD_SCHEMA_CAPSLOCK_TIPS_ENABLED, _hash):
        this->capslock_tips_enabled_set(this->keyboard_settings_->get_boolean(key));
        break;
    case CONNECT(KEYBOARD_SCHEMA_NUMLOCK_TIPS_ENABLED, _hash):
        this->numlock_tips_enabled_set(this->keyboard_settings_->get_boolean(key));
        break;
    default:
        break;
    }
}

void KeyboardManager::load_xkb_rules()
{
    XkbRulesParser rules_parser(DEFAULT_XKB_RULES_FILE);
    XkbRules xkb_rules;
    std::string err;
    if (!rules_parser.parse(xkb_rules, err))
    {
        KLOG_WARNING_INPUTDEVICES("Failed to parse file %s: %s.", DEFAULT_XKB_RULES_FILE, err.c_str());
        return;
    }

    for (size_t i = 0; i < xkb_rules.layouts.size(); ++i)
    {
        auto &layout_name = xkb_rules.layouts[i].name;
        auto country_name = ISOTranslation::get_instance()->get_locale_country_name(StrUtils::toupper(layout_name));
        if (country_name.length() > 0)
        {
            this->valid_layouts_[layout_name] = country_name;
        }
        else
        {
            this->valid_layouts_[layout_name] = ISOTranslation::get_instance()->get_locale_string(xkb_rules.layouts[i].description, DEFAULT_DESC_DELIMETERS);
        }

        KLOG_DEBUG_INPUTDEVICES("name: %s value: %s.", layout_name.c_str(), this->valid_layouts_[layout_name].c_str());

        for (size_t j = 0; j < xkb_rules.layouts[i].variants.size(); ++j)
        {
            auto layout_variant = layout_name + " " + xkb_rules.layouts[i].variants[j].name;
            auto variant_desc = ISOTranslation::get_instance()->get_locale_string(xkb_rules.layouts[i].variants[j].description, DEFAULT_DESC_DELIMETERS);
            auto desciption = this->valid_layouts_[layout_name] + " " + variant_desc;
            this->valid_layouts_[layout_variant] = desciption;

            KLOG_DEBUG_INPUTDEVICES("name: %s value: %s.", layout_variant.c_str(), desciption.c_str());
        }
    }
}

void KeyboardManager::set_all_props()
{
    this->set_auto_repeat();
    this->set_layouts(this->layouts_);
    this->set_options(this->options_);
}

void KeyboardManager::set_auto_repeat()
{
    KLOG_DEBUG_INPUTDEVICES("Repeat_enabled: %d repeat_delay: %d repeat_interval: %d.",
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
            KLOG_WARNING_INPUTDEVICES("XKeyboard keyboard extensions are unavailable, no way to support keyboard autorepeat rate settings");
        }
    }
    else
    {
        XAutoRepeatOff(GDK_DISPLAY_XDISPLAY(display));
    }
}

bool KeyboardManager::set_layouts(const std::vector<Glib::ustring> &layouts)
{
    std::string join_layouts;
    std::string join_variants;
    for (auto iter = layouts.begin(); iter != layouts.end(); ++iter)
    {
        auto layout_variant = StrUtils::split_with_char(*iter, ' ', true);

        if (layout_variant.size() == 1)
        {
            join_layouts += (layout_variant[0] + LAYOUT_JOIN_CHAR);
            join_variants += LAYOUT_JOIN_CHAR;
        }
        else if (layout_variant.size() == 2)
        {
            join_layouts += (layout_variant[0] + LAYOUT_JOIN_CHAR);
            join_variants += (layout_variant[1] + LAYOUT_JOIN_CHAR);
        }
        else
        {
            KLOG_WARNING_INPUTDEVICES("The format of the layout item '%s' is invalid. it's already ignored", iter->c_str());
        }
    }

    if (join_layouts.length() <= 0)
    {
        join_layouts = DEFAULT_LAYOUT LAYOUT_JOIN_CHAR;
        join_variants = std::string(LAYOUT_JOIN_CHAR);
    }

    auto cmdline = fmt::format("{0} -layout {1} -variant {2}", SETXKBMAP, join_layouts, join_variants);
    std::string err;
    try
    {
        KLOG_DEBUG_INPUTDEVICES("Cmdline: %s.", cmdline.c_str());
        Glib::spawn_command_line_sync(cmdline, nullptr, &err);
    }
    catch (const Glib::Error &e)
    {
        err = e.what().raw();
    }

    if (err.length() > 0)
    {
        KLOG_WARNING_INPUTDEVICES("Failed to set layouts: %s.", err.c_str());
        return false;
    }
    return true;
}

bool KeyboardManager::set_options(const std::vector<Glib::ustring> &options)
{
    try
    {
        Glib::spawn_command_line_sync(SETXKBMAP " -option");
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_INPUTDEVICES("Failed to set options: %s.", e.what().c_str());
        return false;
    }

    std::string join_options;
    for (auto iter = options.begin(); iter != options.end(); ++iter)
    {
        join_options += fmt::format(" -option {0}", *iter);
    }

    RETURN_VAL_IF_TRUE(join_options.length() <= 0, true);

    auto cmdline = fmt::format("{0} {1}", SETXKBMAP, join_options);
    try
    {
        KLOG_DEBUG_INPUTDEVICES("Cmdline: %s.", cmdline.c_str());
        Glib::spawn_command_line_sync(cmdline);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_INPUTDEVICES("Failed to set options: %s.", e.what().c_str());
        return false;
    }
    return true;
}

void KeyboardManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    if (!connect)
    {
        KLOG_WARNING_INPUTDEVICES("Failed to connect dbus with %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, KEYBOARD_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_INPUTDEVICES("Register object_path %s fail: %s.", KEYBOARD_OBJECT_PATH, e.what().c_str());
    }
}
void KeyboardManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_DEBUG_INPUTDEVICES("Success to register dbus name: %s", name.c_str());
}
void KeyboardManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_DEBUG_INPUTDEVICES("Failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran