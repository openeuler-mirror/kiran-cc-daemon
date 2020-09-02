/*
 * @Author       : tangjie02
 * @Date         : 2020-08-27 11:06:15
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-01 14:57:29
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/system-shortcut.cpp
 */

#include "plugins/keybinding/system-shortcut.h"

#include <glib/gi18n.h>

#include "lib/log.h"
#include "lib/str-util.h"
#include "plugins/keybinding/keylist-entries-parser.h"

namespace Kiran
{
#define MATECC_KEYBINDINGS_DIR "/usr/share/mate-control-center/keybindings"

SystemShortCutManager::SystemShortCutManager()
{
}

SystemShortCutManager *SystemShortCutManager::instance_ = nullptr;
void SystemShortCutManager::global_init()
{
    instance_ = new SystemShortCutManager();
    instance_->init();
}

CCError SystemShortCutManager::modify(const std::string &uid,
                                      const std::string &key_combination,
                                      std::string &err)
{
    SETTINGS_PROFILE("uid: %s keycomb: %s.", uid.c_str(), key_combination.c_str());

    if (ShortCutHelper::get_keystate(key_combination) == INVALID_KEYSTATE)
    {
        err = fmt::format("the key combination is invalid: {0}", key_combination);
        return CCError::ERROR_INVALID_PARAMETER;
    }

    auto system_shortcut = this->get(uid);
    if (!system_shortcut)
    {
        err = fmt::format("not found uid '{0}'", uid);
        return CCError::ERROR_INVALID_PARAMETER;
    }

    if (system_shortcut->key_combination == key_combination)
    {
        return CCError::SUCCESS;
    }

    system_shortcut->key_combination = key_combination;
    system_shortcut->settings->set_string(system_shortcut->settings_key, system_shortcut->key_combination);
    this->system_shortcut_changed_.emit(uid);
    return CCError::SUCCESS;
}

std::shared_ptr<SystemShortCut> SystemShortCutManager::get(const std::string &uid)
{
    auto iter = this->system_shortcuts_.find(uid);
    if (iter != this->system_shortcuts_.end())
    {
        return iter->second;
    }
    return nullptr;
}

void SystemShortCutManager::init()
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
        auto &package = keylist_entries.package;
        auto system_settings = Gio::Settings::create(keylist_entries.schema);
        if (!system_settings)
        {
            LOG_WARNING("the schema id '%s' isn't exist", keylist_entries.schema.c_str());
            continue;
        }

        bindtextdomain(package.c_str(), KCC_LOCALEDIR);
        bind_textdomain_codeset(package.c_str(), "UTF-8");

        for (auto &keylist_entry : keylist_entries.entries_)
        {
            auto system_shortcut = std::make_shared<SystemShortCut>();
            system_shortcut->kind = dgettext(package.c_str(), keylist_entries.name.c_str());
            system_shortcut->name = dgettext(package.c_str(), keylist_entry.description.c_str());
            system_shortcut->settings = system_settings;
            system_shortcut->settings_key = keylist_entry.name;
            system_shortcut->key_combination = system_settings->get_string(keylist_entry.name);

            if (system_shortcut->kind.length() == 0 ||
                system_shortcut->name.length() == 0 ||
                ShortCutHelper::get_keystate(system_shortcut->key_combination) == INVALID_KEYSTATE)
            {
                LOG_WARNING("the system shortcut is invalid. kind: %s name: %s keycomb: %s.",
                            system_shortcut->kind.c_str(),
                            system_shortcut->name.c_str(),
                            system_shortcut->key_combination.c_str());
                continue;
            }

            auto uid = Glib::Checksum::compute_checksum(Glib::Checksum::CHECKSUM_MD5,
                                                        keylist_entries.schema + "+" + keylist_entry.name);

            auto iter = this->system_shortcuts_.emplace(uid, system_shortcut);
            if (!iter.second)
            {
                LOG_WARNING("exists the same system shortcut, uid: %s schema: %s key: %s.",
                            uid.c_str(),
                            keylist_entries.schema.c_str(),
                            keylist_entry.name.c_str());
                continue;
            }
            system_settings->signal_changed(keylist_entry.name).connect(sigc::bind(sigc::mem_fun(this, &SystemShortCutManager::settings_changed), system_settings));
        }
    }
}

void SystemShortCutManager::settings_changed(const Glib::ustring &key, const Glib::RefPtr<Gio::Settings> settings)
{
    auto uid = Glib::Checksum::compute_checksum(Glib::Checksum::CHECKSUM_MD5,
                                                (settings->property_schema_id().get_value() + "+" + key).raw());

    auto system_shortcut = this->get(uid);

    if (system_shortcut)
    {
        auto value = settings->get_string(key);
        if (system_shortcut->key_combination != value &&
            ShortCutHelper::get_keystate(value) != INVALID_KEYSTATE)
        {
            system_shortcut->key_combination = value;
            this->system_shortcut_changed_.emit(uid);
        }
    }
}
}  // namespace Kiran