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
#define KCC_KEYBINDINGS_DIR KCC_INSTALL_DATADIR "/keybindings"

SystemShortCuts::SystemShortCuts()
{
}

void SystemShortCuts::init()
{
    KLOG_PROFILE("");
    this->load_system_shortcuts(this->shortcuts_);

    EWMH::get_instance()->signal_wm_window_change().connect(sigc::mem_fun(this, &SystemShortCuts::wm_window_changed));
}

bool SystemShortCuts::modify(const std::string &uid,
                             const std::string &key_combination)
{
    KLOG_PROFILE("Uid: %s keycomb: %s.", uid.c_str(), key_combination.c_str());

    auto shortcut = this->get(uid);
    if (!shortcut)
    {
        KLOG_WARNING("The shortcut %s is not exists.", uid.c_str());
        return false;
    }

    RETURN_VAL_IF_TRUE(shortcut->key_combination == key_combination, true);

    shortcut->key_combination = key_combination;
    shortcut->settings->set_string(shortcut->settings_key, shortcut->key_combination);
    this->shortcut_changed_.emit(shortcut);
    return true;
}

std::shared_ptr<SystemShortCut> SystemShortCuts::get(const std::string &uid)
{
    auto iter = this->shortcuts_.find(uid);
    if (iter != this->shortcuts_.end())
    {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<SystemShortCut> SystemShortCuts::get_by_keycomb(const std::string &keycomb)
{
    KLOG_PROFILE("Keycomb: %s", keycomb.c_str());

    // 禁用快捷键不进行搜索
    RETURN_VAL_IF_TRUE(keycomb == SHORTCUT_KEYCOMB_DISABLE, nullptr);

    for (auto &iter : this->shortcuts_)
    {
        if (iter.second->key_combination == keycomb)
        {
            return iter.second;
        }
    }
    return nullptr;
}

void SystemShortCuts::reset()
{
    // 这里为了避免快捷键出现冲突的情况，首先讲所有快捷键设置为空，然后再设置为默认值

    // 断开所有信号连接
    for (auto &iter : this->shortcuts_)
    {
        iter.second->connection.disconnect();
    }

    // 将所有需要重置的快捷键设置为空
    for (auto &iter : this->shortcuts_)
    {
        auto shortcut = iter.second;
        CONTINUE_IF_TRUE(shortcut->key_combination == shortcut->default_key_combination);
        iter.second->settings->set_string(iter.second->settings_key, Glib::ustring());
    }

    // 将所有需要重置的快捷键设置为默认值
    for (auto &iter : this->shortcuts_)
    {
        auto shortcut = iter.second;
        CONTINUE_IF_TRUE(shortcut->key_combination == shortcut->default_key_combination);

        shortcut->key_combination = shortcut->default_key_combination;
        shortcut->settings->set_string(shortcut->settings_key, shortcut->key_combination);
        this->shortcut_changed_.emit(shortcut);
    }

    // 重新监听所有信号
    for (auto &iter : this->shortcuts_)
    {
        auto shortcut = iter.second;
        shortcut->connection = shortcut->settings->signal_changed(shortcut->settings_key).connect(sigc::bind(sigc::mem_fun(this, &SystemShortCuts::settings_changed), shortcut->uid));
    }
}

void SystemShortCuts::load_system_shortcuts(std::map<std::string, std::shared_ptr<SystemShortCut>> &shortcuts)
{
    KLOG_PROFILE("");

    KeyListEntriesParser parser(KCC_KEYBINDINGS_DIR);
    std::vector<KeyListEntries> keys;
    std::string err;
    if (!parser.parse(keys, err))
    {
        KLOG_WARNING("failed to parse %s: %s.", KCC_KEYBINDINGS_DIR, err.c_str());
        return;
    }

    auto wm_keybindings = EWMH::get_instance()->get_wm_keybindings();

    for (auto &keylist_entries : keys)
    {
        auto &package = keylist_entries.package;

        if (keylist_entries.wm_name.length() > 0 &&
            std::find(wm_keybindings.begin(), wm_keybindings.end(), keylist_entries.wm_name) == wm_keybindings.end())
        {
            KLOG_DEBUG("Cannot match current window manager: %s.", keylist_entries.wm_name.c_str());
            continue;
        }

        // 过滤掉没有翻译文件的配置
        if (package.empty())
        {
            KLOG_WARNING("Filter the keylist entries which name is %s, because the translation file not be found.", keylist_entries.name.c_str());
            continue;
        }

        auto schemas = Gio::Settings::list_schemas();
        if (std::find(schemas.begin(), schemas.end(), keylist_entries.schema) == schemas.end())
        {
            KLOG_WARNING("The schema id '%s' isn't exist", keylist_entries.schema.c_str());
            continue;
        }
        auto system_settings = Gio::Settings::create(keylist_entries.schema);

        bindtextdomain(package.c_str(), KCC_LOCALEDIR);
        bind_textdomain_codeset(package.c_str(), "UTF-8");

        for (auto &keylist_entry : keylist_entries.entries_)
        {
            // 配置文件支持条件语句对快捷键进行过滤
            if (!this->should_show_key(keylist_entry))
            {
                KLOG_DEBUG("The system shortcut should not show. type: %s, name: %s, description: %s.",
                           keylist_entries.name.c_str(),
                           keylist_entry.name.c_str(),
                           keylist_entry.description.c_str());
                continue;
            }

            auto shortcut = std::make_shared<SystemShortCut>();
            Glib::VariantBase default_value;

            shortcut->kind = dgettext(package.c_str(), keylist_entries.name.c_str());
            shortcut->name = dgettext(package.c_str(), keylist_entry.description.c_str());
            shortcut->settings = system_settings;
            shortcut->settings_key = keylist_entry.name;
            shortcut->key_combination = system_settings->get_string(keylist_entry.name);
            // 快捷键默认值
            system_settings->get_default_value(keylist_entry.name, default_value);
            if (default_value.gobj())
            {
                try
                {
                    shortcut->default_key_combination = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(default_value).get().raw();
                }
                catch (const std::exception &e)
                {
                    KLOG_WARNING("%s", e.what());
                }
            }

            if (shortcut->kind.length() == 0 ||
                shortcut->name.length() == 0 ||
                ShortCutHelper::get_keystate(shortcut->key_combination) == INVALID_KEYSTATE)
            {
                KLOG_WARNING("The system shortcut is invalid. kind: %s name: %s keycomb: %s.",
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
                KLOG_WARNING("Exists the same system shortcut, uid: %s schema: %s key: %s.",
                             shortcut->uid.c_str(),
                             keylist_entries.schema.c_str(),
                             keylist_entry.name.c_str());
                continue;
            }
            shortcut->connection = system_settings->signal_changed(keylist_entry.name).connect(sigc::bind(sigc::mem_fun(this, &SystemShortCuts::settings_changed), shortcut->uid));
        }
    }
}

void SystemShortCuts::wm_window_changed()
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

void SystemShortCuts::settings_changed(const Glib::ustring &key, std::string shortcut_uid)
{
    auto shortcut = this->get(shortcut_uid);
    RETURN_IF_FALSE(shortcut);

    if (shortcut)
    {
        auto value = shortcut->settings->get_string(key);
        if (shortcut->key_combination != value &&
            ShortCutHelper::get_keystate(value) != INVALID_KEYSTATE)
        {
            shortcut->key_combination = value;
            this->shortcut_changed_.emit(shortcut);
        }
    }
}

bool SystemShortCuts::should_show_key(const KeyListEntry &entry)
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