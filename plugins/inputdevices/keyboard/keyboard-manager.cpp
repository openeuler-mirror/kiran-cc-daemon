/*
 * @Author       : tangjie02
 * @Date         : 2020-08-12 16:25:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 15:20:47
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/inputdevices/keyboard/keyboard-manager.cpp
 */
#include "plugins/inputdevices/keyboard/keyboard-manager.h"

#include <X11/XKBlib.h>

#include "keyboard_i.h"
#include "lib/base/base.h"
#include "lib/dbus/dbus.h"
#include "lib/iso/iso-translation.h"
#include "plugins/inputdevices/keyboard/xkb-rules-parser.h"

namespace Kiran
{
#define KEYBOARD_SCHEMA_ID "com.kylinsec.kiran.keyboard"
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
    SETTINGS_PROFILE("layout: %s.", layout.c_str());

    auto layouts = this->layouts_;

    if (layouts.size() >= LAYOUT_MAX_NUMBER)
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_EXCEED_LIMIT,
                                 "the number of the layout can't exceeds {0}. current number: {1}",
                                 LAYOUT_MAX_NUMBER,
                                 layouts.size());
    }

    if (this->valid_layouts_.find(layout) == this->valid_layouts_.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_INVALID_PARAMETER,
                                 "the layout '{0}' is invalid.",
                                 layout);
    }

    auto iter = std::find(layouts.begin(), layouts.end(), layout);

    if (iter != layouts.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_INVALID_PARAMETER,
                                 "the layout '{0}' already is exist in user layout list.",
                                 layout);
    }

    layouts.push_back(layout);
    if (!this->layouts_set(layouts))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_UNKNOWN, "failed to set the layout.");
    }
    invocation.ret();
}

void KeyboardManager::DelLayout(const Glib::ustring &layout, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("layout: %s.", layout.c_str());

    auto layouts = this->layouts_;

    auto iter = std::find(layouts.begin(), layouts.end(), layout);

    if (iter == layouts.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_INVALID_PARAMETER,
                                 "the layout '{0}' is no exist in user layout list.",
                                 layout);
    }
    layouts.erase(iter);

    if (!this->layouts_set(layouts))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_UNKNOWN, "failed to set the layout.");
    }
    invocation.ret();
}

void KeyboardManager::GetValidLayouts(MethodInvocation &invocation)
{
    invocation.ret(this->valid_layouts_);
}

void KeyboardManager::AddLayoutOption(const Glib::ustring &option, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("option: %s.", option.c_str());

    auto options = this->options_;

    auto iter = std::find(options.begin(), options.end(), option);

    if (iter != options.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_INVALID_PARAMETER,
                                 "the option '{0}' already is exist in user option list.",
                                 option);
    }

    options.push_back(option);
    if (!this->options_set(options))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_UNKNOWN, "failed to set the option.");
    }
    invocation.ret();
}

void KeyboardManager::DelLayoutOption(const Glib::ustring &option, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("option: %s.", option.c_str());

    auto options = this->options_;

    auto iter = std::find(options.begin(), options.end(), option);

    if (iter == options.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_INVALID_PARAMETER,
                                 "the option '{0}' is no exist in user option list.",
                                 option);
    }
    options.erase(iter);

    if (!this->options_set(options))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_UNKNOWN, "failed to set the option.");
    }
    invocation.ret();
}

void KeyboardManager::ClearLayoutOption(MethodInvocation &invocation)
{
    if (!this->options_set(std::vector<Glib::ustring>()))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_UNKNOWN, "failed to set the option.");
    }
    invocation.ret();
}

#define AUTO_REPEAT_SET_HANDLER(prop, type1, key, type2)                                                           \
    bool KeyboardManager::prop##_setHandler(type1 value)                                                           \
    {                                                                                                              \
        SETTINGS_PROFILE("value: %s.", fmt::format("{0}", value).c_str());                                         \
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

bool KeyboardManager::layouts_setHandler(const std::vector<Glib::ustring> &value)
{
    SETTINGS_PROFILE("value: %s.", StrUtils::join(value, ",").c_str());

    auto layouts = value;

    if (layouts.size() > LAYOUT_MAX_NUMBER)
    {
        LOG_WARNING("the number of the layouts set has %d. it exceed max layout number(%d). the subsequent layout is ignored.",
                    layouts.size(),
                    LAYOUT_MAX_NUMBER);

        layouts.resize(LAYOUT_MAX_NUMBER);
    }

    if (layouts.size() == 0)
    {
        LOG_WARNING("because the user layout list is empty, so set the default layout 'us'.");
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
    SETTINGS_PROFILE("value: %s.", StrUtils::join(value, ",").c_str());
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
    SETTINGS_PROFILE("");

    if (this->keyboard_settings_)
    {
        this->repeat_enabled_ = this->keyboard_settings_->get_boolean(KEYBOARD_SCHEMA_REPEAT_ENABLED);
        this->repeat_delay_ = this->keyboard_settings_->get_int(KEYBOARD_SCHEMA_REPEAT_DELAY);
        this->repeat_interval_ = this->keyboard_settings_->get_int(KEYBOARD_SCHEMA_REPEAT_INTERVAL);
        this->layouts_ = this->keyboard_settings_->get_string_array(KEYBOARD_SCHEMA_LAYOUTS);
        this->options_ = this->keyboard_settings_->get_string_array(KEYBOARD_SCHEMA_OPTIONS);
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
    case CONNECT(KEYBOARD_SCHEMA_LAYOUTS, _hash):
        this->layouts_set(this->keyboard_settings_->get_string_array(key));
        break;
    case CONNECT(KEYBOARD_SCHEMA_OPTIONS, _hash):
        this->options_set(this->keyboard_settings_->get_string_array(key));
        break;
    default:
        break;
    }
}

void KeyboardManager::load_xkb_rules()
{
    SETTINGS_PROFILE("");

    XkbRulesParser rules_parser(DEFAULT_XKB_RULES_FILE);
    XkbRules xkb_rules;
    std::string err;
    if (!rules_parser.parse(xkb_rules, err))
    {
        LOG_WARNING("failed to parse file %s: %s.", DEFAULT_XKB_RULES_FILE, err.c_str());
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

        // LOG_DEBUG("name: %s value: %s.", layout_name.c_str(), this->valid_layouts_[layout_name].c_str());

        for (size_t j = 0; j < xkb_rules.layouts[i].variants.size(); ++j)
        {
            auto layout_variant = layout_name + " " + xkb_rules.layouts[i].variants[j].name;
            auto variant_desc = ISOTranslation::get_instance()->get_locale_string(xkb_rules.layouts[i].variants[j].description, DEFAULT_DESC_DELIMETERS);
            auto desciption = this->valid_layouts_[layout_name] + " " + variant_desc;
            this->valid_layouts_[layout_variant] = desciption;

            // LOG_DEBUG("name: %s value: %s.", layout_variant.c_str(), desciption.c_str());
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

bool KeyboardManager::set_layouts(const std::vector<Glib::ustring> &layouts)
{
    SETTINGS_PROFILE("layouts: %s.", StrUtils::join(layouts, ";").c_str());

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
            LOG_WARNING("the format of the layout item '%s' is invalid. it's already ignored", iter->c_str());
        }
    }

    if (join_layouts.length() <= 0)
    {
        join_layouts = DEFAULT_LAYOUT LAYOUT_JOIN_CHAR;
        join_variants = LAYOUT_JOIN_CHAR;
    }

    auto cmdline = fmt::format("{0} -layout {1} -variant {2}", SETXKBMAP, join_layouts, join_variants);
    std::string err;
    try
    {
        LOG_DEBUG("cmdline: %s.", cmdline.c_str());
        Glib::spawn_command_line_sync(cmdline, nullptr, &err);
    }
    catch (const Glib::Error &e)
    {
        err = e.what().raw();
    }

    if (err.length() > 0)
    {
        LOG_WARNING("failed to set layouts: %s.", err.c_str());
        return false;
    }
    return true;
}

bool KeyboardManager::set_options(const std::vector<Glib::ustring> &options)
{
    SETTINGS_PROFILE("options: %s.", StrUtils::join(options, ";").c_str());

    try
    {
        Glib::spawn_command_line_sync(SETXKBMAP " -option");
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("failed to set options: %s.", e.what().c_str());
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
        LOG_DEBUG("cmdline: %s.", cmdline.c_str());
        Glib::spawn_command_line_sync(cmdline);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("failed to set options: %s.", e.what().c_str());
        return false;
    }
    return true;
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