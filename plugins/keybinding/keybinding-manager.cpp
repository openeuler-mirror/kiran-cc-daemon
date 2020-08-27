/*
 * @Author       : tangjie02
 * @Date         : 2020-08-24 16:20:07
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-26 17:04:24
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/keybinding-manager.cpp
 */

#include "plugins/keybinding/keybinding-manager.h"

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <glib/gi18n.h>

#include "lib/log.h"
#include "lib/str-util.h"
#include "plugins/keybinding/keylist-entries-parser.h"

namespace Kiran
{
#define KEYBINDING_DBUS_NAME "com.unikylin.Kiran.SessionDaemon.Keybinding"
#define KEYBINDING_OBJECT_PATH "/com/unikylin/Kiran/SessionDaemon/Keybinding"

#define KEYBINDING_SCHEMA_ID "com.unikylin.kiran.keybinding"
#define KEYBINDING_SCHEMA_SHORTCUTS "shortcuts"

#define MATECC_KEYBINDINGS_DIR "/usr/share/mate-control-center/keybindings"

KeybindingManager::KeybindingManager() : dbus_connect_id_(0),
                                         object_register_id_(0)
{
    this->keybinding_settings_ = Gio::Settings::create(KEYBINDING_SCHEMA_ID);
}

KeybindingManager::~KeybindingManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }

    if (this->root_window_)
    {
        this->root_window_->remove_filter(&KeybindingManager::window_event, this);
    }
}

KeybindingManager *KeybindingManager::instance_ = nullptr;
void KeybindingManager::global_init()
{
    instance_ = new KeybindingManager();
    instance_->init();
}

void KeybindingManager::init()
{
    this->load_custom_shortcut(this->shortcuts_);
    this->load_system_shortcut(this->shortcuts_);

    auto display = Gdk::Display::get_default();
    // auto xdisplay = GDK_DISPLAY_XDISPLAY(display->gobj());
    this->root_window_ = display->get_default_screen()->get_root_window();

    this->root_window_->add_filter(&KeybindingManager::window_event, this);
    auto event_mask = this->root_window_->get_events();
    this->root_window_->set_events(event_mask | Gdk::KEY_PRESS_MASK);

    this->keybinding_settings_->signal_changed().connect(sigc::mem_fun(this, &KeybindingManager::custom_settings_changed));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 KEYBINDING_DBUS_NAME,
                                                 sigc::mem_fun(this, &KeybindingManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &KeybindingManager::on_name_acquired),
                                                 sigc::mem_fun(this, &KeybindingManager::on_name_lost));
}

void KeybindingManager::load_custom_shortcut(std::map<std::string, std::shared_ptr<ShortCut>> &shortcuts)
{
    SETTINGS_PROFILE("");

    if (this->keybinding_settings_)
    {
        Glib::VariantBase base;
        this->keybinding_settings_->get_value(KEYBINDING_SCHEMA_SHORTCUTS, base);
        if (!base.gobj())
        {
            LOG_WARNING("failed to get shortcuts from settings.");
            return;
        }

        try
        {
            auto v1 = Glib::VariantBase::cast_dynamic<Glib::Variant<std::map<std::string, Glib::VariantBase>>>(base);
            auto v2 = v1.get();
            for (const auto &v2_item : v2)
            {
                auto custom_shortcut = std::make_shared<CustomShortCut>();

                auto v3 = Glib::VariantBase::cast_dynamic<Glib::Variant<ShortCutTuple>>(v2_item.second);

                custom_shortcut->name_ = v3.get_child<std::string>(0);
                custom_shortcut->action_ = v3.get_child<std::string>(1);
                custom_shortcut->key_combination_ = v3.get_child<std::string>(2);
                shortcuts.emplace(v2_item.first, custom_shortcut);
            }
        }
        catch (std::bad_cast &bc)
        {
            LOG_WARNING("failed to load shortcuts.\n", bc.what());
            return;
        }
    }
}

void KeybindingManager::custom_settings_changed(const Glib::ustring &key)
{
    SETTINGS_PROFILE("");
}

void KeybindingManager::load_system_shortcut(std::map<std::string, std::shared_ptr<ShortCut>> &shortcuts)
{
    KeyListEntriesParser parser(MATECC_KEYBINDINGS_DIR);
    std::vector<KeyListEntries> keys;
    std::string err;
    if (!parser.parse(keys, err))
    {
        LOG_WARNING("failed to parse %s: %s.", MATECC_KEYBINDINGS_DIR, err.c_str());
        return;
    }

    for (auto &keylist_entries : keys)
    {
        bindtextdomain(keylist_entries.package.c_str(), KCC_LOCALEDIR);
        bind_textdomain_codeset(keylist_entries.package.c_str(), "UTF-8");
        for (auto &keylist_entry : keylist_entries.entries_)
        {
            auto system_shortcut = std::make_shared<SystemShortCut>(keylist_entries.name,
                                                                    keylist_entry.description,
                                                                    keylist_entries.schema,
                                                                    keylist_entry.name,
                                                                    keylist_entries.package);

            if (!system_shortcut->is_valid())
            {
                LOG_WARNING("the SystemShortCut %s is invalid.", system_shortcut->get_id().c_str());
                continue;
            }

            auto iter = this->shortcuts_.emplace(system_shortcut->get_id(), system_shortcut);
            if (!iter.second)
            {
                LOG_WARNING("exists the same SystemShortCut: %s.", system_shortcut->get_id().c_str());
                continue;
            }
        }
    }
}

GdkFilterReturn KeybindingManager::window_event(GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
    RETURN_VAL_IF_TRUE(event->type != GDK_KEY_PRESS, GDK_FILTER_CONTINUE);

    KeybindingManager *keybinding_manager = (KeybindingManager *)data;
    g_return_val_if_fail(KeybindingManager::get_instance() == keybinding_manager, GDK_FILTER_REMOVE);

    for (const auto &elem : keybinding_manager->shortcuts_)
    {
        if (elem.second->kind_ != SHORTCUT_KIND_CUSTOM)
        {
            continue;
        }

        const auto custom_shortcut = std::dynamic_pointer_cast<CustomShortCut>(elem.second);
        auto key_state = keybinding_manager->get_key_state(custom_shortcut->key_combination_);
        if (key_state == keybinding_manager->get_key_state(event))
        {
            std::vector<std::string> argv;
            try
            {
                argv = Glib::shell_parse_argv(custom_shortcut->action_);
            }
            catch (const Glib::Error &e)
            {
                LOG_WARNING("failed to parse %s: %s.", custom_shortcut->action_.c_str(), e.what().c_str());
                return GDK_FILTER_CONTINUE;
            }

            try
            {
                Glib::spawn_async(std::string(), argv, Glib::SPAWN_SEARCH_PATH);
            }
            catch (const Glib::Error &e)
            {
                LOG_WARNING("failed to exec %s: %s.", custom_shortcut->action_.c_str(), e.what().c_str());
            }

            return GDK_FILTER_REMOVE;
        }
    }

    return GDK_FILTER_CONTINUE;
}

KeyState KeybindingManager::get_key_state(const std::string &key_comb)
{
    KeyState key_state;
    size_t cur_pos = 0;

    while (cur_pos < key_comb.size())
    {
        if (key_comb[cur_pos] == '<')
        {
            auto gt_pos = key_comb.find('>', cur_pos);
            if (gt_pos == std::string::npos)
            {
                return INVALID_KEYSTATE;
            }
            auto token = StrUtil::tolower(key_comb.substr(cur_pos, gt_pos - cur_pos + 1));
            cur_pos = gt_pos + 1;
            switch (shash(token.c_str()))
            {
            case "<release>"_hash:
                key_state.mods |= GDK_RELEASE_MASK;
                break;
            case "<primary>"_hash:
            case "<control>"_hash:
            case "<ctrl>"_hash:
            case "<ctl>"_hash:
                key_state.mods |= GDK_CONTROL_MASK;
                break;
            case "<shift>"_hash:
            case "<shft>"_hash:
                key_state.mods |= GDK_SHIFT_MASK;
                break;
            case "<mod1>"_hash:
            case "<alt>"_hash:
                key_state.mods |= GDK_MOD1_MASK;
                break;
            case "<mod2>"_hash:
                key_state.mods |= GDK_MOD2_MASK;
                break;
            case "<mod3>"_hash:
                key_state.mods |= GDK_MOD3_MASK;
                break;
            case "<mod4>"_hash:
                key_state.mods |= GDK_MOD4_MASK;
                break;
            case "<mod5>"_hash:
                key_state.mods |= GDK_MOD5_MASK;
                break;
            case "<meta>"_hash:
                key_state.mods |= GDK_META_MASK;
                break;
            case "<hyper>"_hash:
                key_state.mods |= GDK_HYPER_MASK;
                break;
            case "<super>"_hash:
                key_state.mods |= GDK_SUPER_MASK;
                break;
            default:
                break;
            }
        }
        else
        {
            auto keyval = gdk_keyval_from_name(key_comb.substr(cur_pos).c_str());
            RETURN_VAL_IF_TRUE(keyval == GDK_KEY_VoidSymbol, INVALID_KEYSTATE);
            key_state.key_symbol = keyval;
            break;
        }
    }
    return key_state;
}

void KeybindingManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    SETTINGS_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        LOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, KEYBINDING_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("register object_path %s fail: %s.", KEYBINDING_OBJECT_PATH, e.what().c_str());
    }
}

void KeybindingManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void KeybindingManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran