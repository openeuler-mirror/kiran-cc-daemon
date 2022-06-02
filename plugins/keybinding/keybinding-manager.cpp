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

#include "plugins/keybinding/keybinding-manager.h"

#include "keybinding-i.h"

namespace Kiran
{
KeybindingManager::KeybindingManager() : dbus_connect_id_(0),
                                         object_register_id_(0)
{
    this->custom_shortcuts_ = std::make_shared<CustomShortCuts>();
    this->system_shortcuts_ = std::make_shared<SystemShortCuts>();
}

KeybindingManager::~KeybindingManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

KeybindingManager *KeybindingManager::instance_ = nullptr;
void KeybindingManager::global_init()
{
    instance_ = new KeybindingManager();
    instance_->init();
}

void KeybindingManager::AddCustomShortcut(const Glib::ustring &name,
                                          const Glib::ustring &action,
                                          const Glib::ustring &key_combination,
                                          MethodInvocation &invocation)
{
    KLOG_PROFILE("");

    if (name.empty() || action.empty())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ARGUMENT_INVALID);
    }

    if (ShortCutHelper::get_keystate(key_combination) == INVALID_KEYSTATE)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_INVALID);
    }

    if (this->has_same_keycomb(std::string(), key_combination))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_ALREADY_EXIST);
    }

    auto custom_shortcut = std::make_shared<CustomShortCut>(name, action, key_combination);

    if (!this->custom_shortcuts_->add(custom_shortcut))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_CALL_FUNCTION_FAILED);
    }
    else
    {
        Json::Value values;
        values[KEYBINDING_SHORTCUT_JK_UID] = std::string(custom_shortcut->uid);
        values[KEYBINDING_SHORTCUT_JK_KIND] = std::string(CUSTOM_SHORTCUT_KIND);
        std::string signal_val = StrUtils::json2str(values);

        invocation.ret(custom_shortcut->uid);
        this->Added_signal.emit(Glib::ustring(signal_val));
    }
}

void KeybindingManager::ModifyCustomShortcut(const Glib::ustring &uid,
                                             const Glib::ustring &name,
                                             const Glib::ustring &action,
                                             const Glib::ustring &key_combination,
                                             MethodInvocation &invocation)
{
    KLOG_PROFILE("");

    if (name.empty() || action.empty())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ARGUMENT_INVALID);
    }

    if (ShortCutHelper::get_keystate(key_combination) == INVALID_KEYSTATE)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_INVALID);
    }

    if (this->has_same_keycomb(uid, key_combination))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_ALREADY_EXIST);
    }

    auto custom_shortcut = std::make_shared<CustomShortCut>(uid, name, action, key_combination);
    if (!this->custom_shortcuts_->modify(custom_shortcut))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_CALL_FUNCTION_FAILED);
    }
    else
    {
        Json::Value values;
        values[KEYBINDING_SHORTCUT_JK_UID] = std::string(uid);
        values[KEYBINDING_SHORTCUT_JK_KIND] = std::string(CUSTOM_SHORTCUT_KIND);
        std::string signal_val = StrUtils::json2str(values);

        invocation.ret();
        this->Changed_signal.emit(Glib::ustring(signal_val));
    }
}

void KeybindingManager::DeleteCustomShortcut(const Glib::ustring &uid, MethodInvocation &invocation)
{
    KLOG_PROFILE("");
    if (!this->custom_shortcuts_->remove(uid))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_CALL_FUNCTION_FAILED);
    }
    else
    {
        Json::Value values;
        values[KEYBINDING_SHORTCUT_JK_UID] = std::string(uid);
        values[KEYBINDING_SHORTCUT_JK_KIND] = std::string(CUSTOM_SHORTCUT_KIND);
        std::string signal_val = StrUtils::json2str(values);

        invocation.ret();
        this->Deleted_signal.emit(Glib::ustring(signal_val));
    }
}

void KeybindingManager::GetCustomShortcut(const Glib::ustring &uid, MethodInvocation &invocation)
{
    KLOG_PROFILE("");

    try
    {
        Json::Value values;

        auto custom_shortcut = this->custom_shortcuts_->get(uid.raw());
        if (!custom_shortcut)
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBINDING_CUSTOM_SHORTCUT_NOT_EXIST);
        }
        values[KEYBINDING_SHORTCUT_JK_UID] = std::string(uid);
        values[KEYBINDING_SHORTCUT_JK_NAME] = custom_shortcut->name;
        values[KEYBINDING_SHORTCUT_JK_ACTION] = custom_shortcut->action;
        values[KEYBINDING_SHORTCUT_JK_KEY_COMBINATION] = custom_shortcut->key_combination;

        auto retval = StrUtils::json2str(values);
        invocation.ret(retval);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_JSON_WRITE_EXCEPTION);
    }
}

void KeybindingManager::ListCustomShortcuts(MethodInvocation &invocation)
{
    KLOG_PROFILE("");

    try
    {
        Json::Value root;
        Json::Value values;

        auto custom_shortcuts = this->custom_shortcuts_->get();
        for (const auto &shortcut : custom_shortcuts)
        {
            values[KEYBINDING_SHORTCUT_JK_UID] = std::string(shortcut.first);
            values[KEYBINDING_SHORTCUT_JK_NAME] = shortcut.second->name;
            values[KEYBINDING_SHORTCUT_JK_ACTION] = shortcut.second->action;
            values[KEYBINDING_SHORTCUT_JK_KEY_COMBINATION] = shortcut.second->key_combination;

            root[KEYBINDING_SHORTCUT_JK_CUSTOM].append(values);
        }

        auto retval = StrUtils::json2str(root);
        invocation.ret(retval);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_JSON_WRITE_EXCEPTION);
    }
}

void KeybindingManager::ModifySystemShortcut(const Glib::ustring &uid,
                                             const Glib::ustring &key_combination,
                                             MethodInvocation &invocation)
{
    KLOG_PROFILE("");

    if (ShortCutHelper::get_keystate(key_combination) == INVALID_KEYSTATE)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBINDING_SYSTEM_KEYCOMB_INVALID);
    }

    if (this->has_same_keycomb(uid, key_combination))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBINDING_SYSTEM_KEYCOMB_ALREADY_EXIST);
    }

    if (!this->system_shortcuts_->modify(uid.raw(), key_combination.raw()))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_CALL_FUNCTION_FAILED);
    }
    invocation.ret();
}

void KeybindingManager::GetSystemShortcut(const Glib::ustring &uid, MethodInvocation &invocation)
{
    KLOG_PROFILE("");

    try
    {
        Json::Value values;

        auto system_shortcut = this->system_shortcuts_->get(uid);
        if (!system_shortcut)
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBINDING_SYSTEM_SHORTCUT_NOT_EXIST);
        }

        values[KEYBINDING_SHORTCUT_JK_UID] = std::string(uid);
        values[KEYBINDING_SHORTCUT_JK_KIND] = system_shortcut->kind;
        values[KEYBINDING_SHORTCUT_JK_NAME] = system_shortcut->name;
        values[KEYBINDING_SHORTCUT_JK_KEY_COMBINATION] = system_shortcut->key_combination;

        auto retval = StrUtils::json2str(values);
        invocation.ret(retval);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_JSON_WRITE_EXCEPTION);
    }
}

void KeybindingManager::ListSystemShortcuts(MethodInvocation &invocation)
{
    KLOG_PROFILE("");

    try
    {
        Json::Value root;
        Json::Value values;

        auto system_shortcuts = this->system_shortcuts_->get();
        for (const auto &shortcut : system_shortcuts)
        {
            values[KEYBINDING_SHORTCUT_JK_UID] = std::string(shortcut.first);
            values[KEYBINDING_SHORTCUT_JK_KIND] = shortcut.second->kind;
            values[KEYBINDING_SHORTCUT_JK_NAME] = shortcut.second->name;
            values[KEYBINDING_SHORTCUT_JK_KEY_COMBINATION] = shortcut.second->key_combination;

            root[KEYBINDING_SHORTCUT_JK_SYSTEM].append(values);
        }

        auto retval = StrUtils::json2str(root);
        invocation.ret(retval);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_JSON_WRITE_EXCEPTION);
    }
}

void KeybindingManager::ListShortcuts(MethodInvocation &invocation)
{
    KLOG_PROFILE("");

    try
    {
        Json::Value root;

        auto custom_shortcuts = this->custom_shortcuts_->get();
        for (const auto &shortcut : custom_shortcuts)
        {
            Json::Value values;
            values[KEYBINDING_SHORTCUT_JK_UID] = std::string(shortcut.first);
            values[KEYBINDING_SHORTCUT_JK_NAME] = shortcut.second->name;
            values[KEYBINDING_SHORTCUT_JK_ACTION] = shortcut.second->action;
            values[KEYBINDING_SHORTCUT_JK_KEY_COMBINATION] = shortcut.second->key_combination;

            root[KEYBINDING_SHORTCUT_JK_CUSTOM].append(values);
        }

        auto system_shortcuts = this->system_shortcuts_->get();
        for (const auto &shortcut : system_shortcuts)
        {
            Json::Value values;
            values[KEYBINDING_SHORTCUT_JK_UID] = std::string(shortcut.first);
            values[KEYBINDING_SHORTCUT_JK_KIND] = shortcut.second->kind;
            values[KEYBINDING_SHORTCUT_JK_NAME] = shortcut.second->name;
            values[KEYBINDING_SHORTCUT_JK_KEY_COMBINATION] = shortcut.second->key_combination;

            root[KEYBINDING_SHORTCUT_JK_SYSTEM].append(values);
        }

        auto retval = StrUtils::json2str(root);
        invocation.ret(retval);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_JSON_WRITE_EXCEPTION);
    }
}

void KeybindingManager::ResetShortcuts(MethodInvocation &invocation)
{
    KLOG_PROFILE("");
    this->system_shortcuts_->reset();
    invocation.ret();
}

void KeybindingManager::init()
{
    this->custom_shortcuts_->init();
    this->system_shortcuts_->init();

    this->system_shortcuts_->signal_shortcut_added().connect(sigc::mem_fun(this, &KeybindingManager::system_shortcut_added));
    this->system_shortcuts_->signal_shortcut_deleted().connect(sigc::mem_fun(this, &KeybindingManager::system_shortcut_deleted));
    this->system_shortcuts_->signal_shortcut_changed().connect(sigc::mem_fun(this, &KeybindingManager::system_shortcut_changed));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 KEYBINDING_DBUS_NAME,
                                                 sigc::mem_fun(this, &KeybindingManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &KeybindingManager::on_name_acquired),
                                                 sigc::mem_fun(this, &KeybindingManager::on_name_lost));
}

bool KeybindingManager::has_same_keycomb(const std::string &uid, const std::string &key_combination)
{
    auto custom_shortcut = this->custom_shortcuts_->get_by_keycomb(key_combination);
    RETURN_VAL_IF_TRUE(custom_shortcut && custom_shortcut->uid != uid, true);

    auto system_shortcut = this->system_shortcuts_->get_by_keycomb(key_combination);
    RETURN_VAL_IF_TRUE(system_shortcut && system_shortcut->uid != uid, true);

    return false;
}

void KeybindingManager::system_shortcut_added(std::shared_ptr<SystemShortCut> system_shortcut)
{
    if (system_shortcut)
    {
        Json::Value values;
        values[KEYBINDING_SHORTCUT_JK_UID] = system_shortcut->uid;
        values[KEYBINDING_SHORTCUT_JK_KIND] = system_shortcut->kind;
        std::string signal_val = StrUtils::json2str(values);

        this->Added_signal.emit(Glib::ustring(signal_val));
    }
}

void KeybindingManager::system_shortcut_deleted(std::shared_ptr<SystemShortCut> system_shortcut)
{
    if (system_shortcut)
    {
        Json::Value values;
        values[KEYBINDING_SHORTCUT_JK_UID] = system_shortcut->uid;
        values[KEYBINDING_SHORTCUT_JK_KIND] = system_shortcut->kind;
        std::string signal_val = StrUtils::json2str(values);

        this->Deleted_signal.emit(Glib::ustring(signal_val));
    }
}

void KeybindingManager::system_shortcut_changed(std::shared_ptr<SystemShortCut> system_shortcut)
{
    if (system_shortcut)
    {
        Json::Value values;
        values[KEYBINDING_SHORTCUT_JK_UID] = system_shortcut->uid;
        values[KEYBINDING_SHORTCUT_JK_KIND] = system_shortcut->kind;
        std::string signal_val = StrUtils::json2str(values);

        this->Changed_signal.emit(Glib::ustring(signal_val));
    }
}

void KeybindingManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        KLOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, KEYBINDING_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("register object_path %s fail: %s.", KEYBINDING_OBJECT_PATH, e.what().c_str());
    }
}

void KeybindingManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void KeybindingManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_DEBUG("failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran