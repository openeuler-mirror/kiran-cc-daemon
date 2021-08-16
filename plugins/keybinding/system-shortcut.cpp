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

#include "plugins/keybinding/system-shortcut.h"

#include <config.h>
#include <glib/gi18n.h>

#include "lib/display/EWMH.h"

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

bool SystemShortCutManager::modify(const std::string &uid,
                                   const std::string &key_combination,
                                   CCErrorCode &error_code)
{
    KLOG_PROFILE("uid: %s keycomb: %s.", uid.c_str(), key_combination.c_str());

    if (ShortCutHelper::get_keystate(key_combination) == INVALID_KEYSTATE)
    {
        error_code = CCErrorCode::ERROR_KEYBINDING_SYSTEM_KEYCOMB_INVALID_1;
        return false;
    }

    auto shortcut = this->get(uid);
    if (!shortcut)
    {
        error_code = CCErrorCode::ERROR_KEYBINDING_SYSTEM_SHORTCUT_NOT_EXIST_1;
        return false;
    }
    RETURN_VAL_IF_TRUE(shortcut->key_combination == key_combination, true);

    shortcut->key_combination = key_combination;
    shortcut->settings->set_string(shortcut->settings_key, shortcut->key_combination);
    this->shortcut_changed_.emit(shortcut);
    return true;
}

std::shared_ptr<SystemShortCut> SystemShortCutManager::get(const std::string &uid)
{
    auto iter = this->shortcuts_.find(uid);
    if (iter != this->shortcuts_.end())
    {
        return iter->second;
    }
    return nullptr;
}

void SystemShortCutManager::init()
{
    KLOG_PROFILE("");
    this->load_system_shortcuts(this->shortcuts_);

    EWMH::get_instance()->signal_wm_window_change().connect(sigc::mem_fun(this, &SystemShortCutManager::wm_window_changed));
}

void SystemShortCutManager::load_system_shortcuts(std::map<std::string, std::shared_ptr<SystemShortCut>> &shortcuts)
{
    KLOG_PROFILE("");

    KeyListEntriesParser parser(MATECC_KEYBINDINGS_DIR);
    std::vector<KeyListEntries> keys;
    std::string err;
    if (!parser.parse(keys, err))
    {
        KLOG_WARNING("failed to parse %s: %s.", MATECC_KEYBINDINGS_DIR, err.c_str());
        return;
    }

    auto wm_keybindings = EWMH::get_instance()->get_wm_keybindings();

    for (auto &keylist_entries : keys)
    {
        auto &package = keylist_entries.package;
        auto system_settings = Gio::Settings::create(keylist_entries.schema);
        if (!system_settings)
        {
            KLOG_WARNING("the schema id '%s' isn't exist", keylist_entries.schema.c_str());
            continue;
        }

        if (keylist_entries.wm_name.length() > 0 &&
            std::find(wm_keybindings.begin(), wm_keybindings.end(), keylist_entries.wm_name) == wm_keybindings.end())
        {
            KLOG_DEBUG("cannot match current window manager: %s.", keylist_entries.wm_name.c_str());
            continue;
        }

        bindtextdomain(package.c_str(), KCC_LOCALEDIR);
        bind_textdomain_codeset(package.c_str(), "UTF-8");

        for (auto &keylist_entry : keylist_entries.entries_)
        {
            if (!this->should_show_key(keylist_entry))
            {
                KLOG_DEBUG("the system shortcut should not show. type: %s, name: %s, description: %s.",
                           keylist_entries.name.c_str(),
                           keylist_entry.name.c_str(),
                           keylist_entry.description.c_str());
                continue;
            }

            auto shortcut = std::make_shared<SystemShortCut>();
            shortcut->kind = dgettext(package.c_str(), keylist_entries.name.c_str());
            shortcut->name = dgettext(package.c_str(), keylist_entry.description.c_str());
            shortcut->settings = system_settings;
            shortcut->settings_key = keylist_entry.name;
            shortcut->key_combination = system_settings->get_string(keylist_entry.name);

            if (shortcut->kind.length() == 0 ||
                shortcut->name.length() == 0 ||
                ShortCutHelper::get_keystate(shortcut->key_combination) == INVALID_KEYSTATE)
            {
                KLOG_WARNING("the system shortcut is invalid. kind: %s name: %s keycomb: %s.",
                             shortcut->kind.c_str(),
                             shortcut->name.c_str(),
                             shortcut->key_combination.c_str());
                continue;
            }

            shortcut->uid = Glib::Checksum::compute_checksum(Glib::Checksum::CHECKSUM_MD5,
                                                             keylist_entries.schema + "+" + keylist_entry.name);

            auto iter = shortcuts.emplace(shortcut->uid, shortcut);
            if (!iter.second)
            {
                KLOG_WARNING("exists the same system shortcut, uid: %s schema: %s key: %s.",
                             shortcut->uid.c_str(),
                             keylist_entries.schema.c_str(),
                             keylist_entry.name.c_str());
                continue;
            }
            system_settings->signal_changed(keylist_entry.name).connect(sigc::bind(sigc::mem_fun(this, &SystemShortCutManager::settings_changed), system_settings));
        }
    }
}

void SystemShortCutManager::wm_window_changed()
{
    auto old_shortcuts = std::move(this->shortcuts_);
    this->load_system_shortcuts(this->shortcuts_);

    // 查找新增和修改的快捷键
    for (auto &shortcut : this->shortcuts_)
    {
        auto iter = old_shortcuts.find(shortcut.first);
        if (iter == old_shortcuts.end())
        {
            this->shortcut_added_.emit(shortcut.second);
        }
        else if (iter->second->kind != shortcut.second->kind ||
                 iter->second->name != shortcut.second->name ||
                 iter->second->key_combination != shortcut.second->key_combination)
        {
            this->shortcut_changed_.emit(shortcut.second);
        }
    }

    // 查找删除的快捷键
    for (auto &shortcut : old_shortcuts)
    {
        auto iter = this->shortcuts_.find(shortcut.first);
        if (iter == this->shortcuts_.end())
        {
            this->shortcut_deleted_.emit(shortcut.second);
        }
    }
}

void SystemShortCutManager::settings_changed(const Glib::ustring &key, const Glib::RefPtr<Gio::Settings> settings)
{
    auto uid = Glib::Checksum::compute_checksum(Glib::Checksum::CHECKSUM_MD5,
                                                (settings->property_schema_id().get_value() + "+" + key).raw());

    auto shortcut = this->get(uid);

    if (shortcut)
    {
        auto value = settings->get_string(key);
        if (shortcut->key_combination != value &&
            ShortCutHelper::get_keystate(value) != INVALID_KEYSTATE)
        {
            shortcut->key_combination = value;
            this->shortcut_changed_.emit(shortcut);
        }
    }
}

bool SystemShortCutManager::should_show_key(const KeyListEntry &entry)
{
    RETURN_VAL_IF_TRUE(entry.comparison.length() == 0, true);
    RETURN_VAL_IF_TRUE(entry.key.length() == 0, false);
    RETURN_VAL_IF_TRUE(entry.schema.length() == 0, false);

    auto settings = Gio::Settings::create(entry.schema);
    auto valuel = settings->get_int(entry.key);
    auto valuer = std::stoll(entry.value);

    switch (shash(entry.comparison.c_str()))
    {
    case "gt"_hash:
        return valuel > valuer;
    case "lt"_hash:
        return valuel < valuer;
    case "eq"_hash:
        return valuel == valuer;
    default:
        return false;
    }

    return false;
}
}  // namespace Kiran