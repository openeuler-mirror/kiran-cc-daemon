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

#include "keybinding-manager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeySequence>
#include "custom-shortcut.h"
#include "keybinding-i.h"
#include "keybindingadaptor.h"
#include "keys/keys-sound.h"
#include "keys/keys-system.h"
#include "keys/keys-touchpad.h"
#include "lib/base/base.h"
#include "system-shortcut.h"

namespace Kiran
{
KeybindingManager::KeybindingManager()
{
    m_adaptor = new KeybindingAdaptor(this);
    m_customShortcuts = QSharedPointer<CustomShortcuts>::create();
    m_systemShortcuts = QSharedPointer<SystemShortcuts>::create();
    m_keysComponents.append({new KeysSound(), new KeysTouchpad(), new KeysSystem()});
}

KeybindingManager::~KeybindingManager()
{
}

KeybindingManager *KeybindingManager::m_instance = nullptr;
void KeybindingManager::globalInit()
{
    m_instance = new KeybindingManager();
    m_instance->init();
}

QString KeybindingManager::AddCustomShortcut(const QString &name, const QString &action, const QString &keyComb)
{
    if (name.isEmpty() || action.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RETVAL(QString(), CCErrorCode::ERROR_ARGUMENT_INVALID);
    }

    if (hasSameKeycomb(QString(), keyComb))
    {
        DBUS_ERROR_REPLY_AND_RETVAL(QString(), CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_ALREADY_EXIST);
    }

    auto customShortcut = QSharedPointer<CustomShortCut>::create(name, action, keyComb);

    if (!m_customShortcuts->add(customShortcut))
    {
        DBUS_ERROR_REPLY_AND_RETVAL(QString(), CCErrorCode::ERROR_CALL_FUNCTION_FAILED);
    }

    QJsonObject jsonObj;
    jsonObj[KEYBINDING_SHORTCUT_JK_UID] = customShortcut->uid;
    jsonObj[KEYBINDING_SHORTCUT_JK_KIND] = tr("Custom");
    jsonObj[KEYBINDING_SHORTCUT_JK_TYPE] = KEYBINDING_SHORTCUT_JK_CUSTOM;
    auto customShortCutJson = QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);

    Q_EMIT Added(customShortCutJson);
    return customShortcut->uid;
}

void KeybindingManager::DeleteCustomShortcut(const QString &uid)
{
    if (!m_customShortcuts->remove(uid))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_CALL_FUNCTION_FAILED);
    }

    QJsonObject jsonObj;
    jsonObj[KEYBINDING_SHORTCUT_JK_UID] = uid;
    jsonObj[KEYBINDING_SHORTCUT_JK_KIND] = tr("Custom");
    jsonObj[KEYBINDING_SHORTCUT_JK_TYPE] = KEYBINDING_SHORTCUT_JK_CUSTOM;
    auto customShortCutJson = QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);

    Q_EMIT Deleted(customShortCutJson);
    return;
}

QString KeybindingManager::GetCustomShortcut(const QString &uid)
{
    auto customShortcut = m_customShortcuts->get(uid);
    if (!customShortcut)
    {
        DBUS_ERROR_REPLY_AND_RETVAL(QString(), CCErrorCode::ERROR_KEYBINDING_CUSTOM_SHORTCUT_NOT_EXIST);
    }

    QJsonObject jsonObj;
    jsonObj[KEYBINDING_SHORTCUT_JK_UID] = uid;
    jsonObj[KEYBINDING_SHORTCUT_JK_KIND] = tr("Custom");
    jsonObj[KEYBINDING_SHORTCUT_JK_NAME] = customShortcut->name;
    jsonObj[KEYBINDING_SHORTCUT_JK_ACTION] = customShortcut->action;
    jsonObj[KEYBINDING_SHORTCUT_JK_KEY_COMBINATION] = customShortcut->keyComb;
    return QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
}

QString KeybindingManager::GetSystemShortcut(const QString &uid)
{
    auto systemShortcut = m_systemShortcuts->get(uid);
    if (!systemShortcut)
    {
        DBUS_ERROR_REPLY_AND_RETVAL(QString(), CCErrorCode::ERROR_KEYBINDING_SYSTEM_SHORTCUT_NOT_EXIST);
    }

    QJsonObject jsonObj;
    jsonObj[KEYBINDING_SHORTCUT_JK_UID] = uid;
    jsonObj[KEYBINDING_SHORTCUT_JK_KIND] = systemShortcut->kind;
    jsonObj[KEYBINDING_SHORTCUT_JK_NAME] = systemShortcut->name;
    jsonObj[KEYBINDING_SHORTCUT_JK_KEY_COMBINATION] = systemShortcut->keyCombination;
    return QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
}

QString KeybindingManager::ListCustomShortcuts()
{
    QJsonObject jsonRoot;
    QJsonArray jsonShortcuts;

    auto customShortcuts = m_customShortcuts->get();
    for (const auto &customShortcut : customShortcuts)
    {
        QJsonObject jsonShortcut;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_UID] = customShortcut->uid;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_NAME] = customShortcut->name;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_ACTION] = customShortcut->action;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_KEY_COMBINATION] = customShortcut->keyComb;
        jsonShortcuts.append(jsonShortcut);
    }
    jsonRoot[KEYBINDING_SHORTCUT_JK_CUSTOM] = jsonShortcuts;

    return QJsonDocument(jsonRoot).toJson(QJsonDocument::Compact);
}

QString KeybindingManager::ListShortcuts()
{
    QJsonObject jsonRoot;
    QJsonArray jsonCustomShortcuts;
    QJsonArray jsonSystemShortcuts;

    auto customShortcuts = m_customShortcuts->get();
    for (const auto &customShortcut : customShortcuts)
    {
        QJsonObject jsonShortcut;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_UID] = customShortcut->uid;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_NAME] = customShortcut->name;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_ACTION] = customShortcut->action;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_KEY_COMBINATION] = customShortcut->keyComb;
        jsonCustomShortcuts.append(jsonShortcut);
    }
    jsonRoot[KEYBINDING_SHORTCUT_JK_CUSTOM] = jsonCustomShortcuts;

    auto systemShortcuts = m_systemShortcuts->get();
    for (const auto &systemShortcut : systemShortcuts)
    {
        QJsonObject jsonShortcut;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_UID] = systemShortcut->uid;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_KIND] = systemShortcut->kind;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_NAME] = systemShortcut->name;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_KEY_COMBINATION] = systemShortcut->keyCombination;
        jsonSystemShortcuts.append(jsonShortcut);
    }
    jsonRoot[KEYBINDING_SHORTCUT_JK_SYSTEM] = jsonSystemShortcuts;

    return QJsonDocument(jsonRoot).toJson(QJsonDocument::Compact);
}

QString KeybindingManager::ListSystemShortcuts()
{
    QJsonObject jsonRoot;
    QJsonArray jsonShortcuts;

    auto systemShortcuts = m_systemShortcuts->get();
    for (const auto &systemShortcut : systemShortcuts)
    {
        QJsonObject jsonShortcut;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_UID] = systemShortcut->uid;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_KIND] = systemShortcut->kind;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_NAME] = systemShortcut->name;
        jsonShortcut[KEYBINDING_SHORTCUT_JK_KEY_COMBINATION] = systemShortcut->keyCombination;
        jsonShortcuts.append(jsonShortcut);
    }
    jsonRoot[KEYBINDING_SHORTCUT_JK_SYSTEM] = jsonShortcuts;
    return QJsonDocument(jsonRoot).toJson(QJsonDocument::Compact);
}

void KeybindingManager::KeybindingManager::ModifyCustomShortcut(const QString &uid,
                                                                const QString &name,
                                                                const QString &action,
                                                                const QString &keyComb)
{
    if (name.isEmpty() || action.isEmpty())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ARGUMENT_INVALID);
    }

    if (hasSameKeycomb(uid, keyComb))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_ALREADY_EXIST);
    }

    auto customShortcut = QSharedPointer<CustomShortCut>::create(uid, name, action, keyComb);
    if (!m_customShortcuts->modify(customShortcut))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_CALL_FUNCTION_FAILED);
    }

    QJsonObject jsonObj;
    jsonObj[KEYBINDING_SHORTCUT_JK_UID] = uid;
    jsonObj[KEYBINDING_SHORTCUT_JK_KIND] = tr("Custom");
    jsonObj[KEYBINDING_SHORTCUT_JK_TYPE] = KEYBINDING_SHORTCUT_JK_CUSTOM;
    auto customShortCutJson = QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);

    Q_EMIT Changed(customShortCutJson);
    return;
}

void KeybindingManager::ModifySystemShortcut(const QString &uid, const QString &keyComb)
{
    if (hasSameKeycomb(uid, keyComb))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBINDING_SYSTEM_KEYCOMB_ALREADY_EXIST);
    }

    if (!m_systemShortcuts->modify(uid, keyComb))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_CALL_FUNCTION_FAILED);
    }
}

void KeybindingManager::ResetShortcuts()
{
    m_systemShortcuts->reset();
}

void KeybindingManager::init()
{
    m_customShortcuts->init();
    m_systemShortcuts->init();

    for (auto keysComponent : m_keysComponents)
    {
        keysComponent->init();
    }

    auto sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerService(KEYBINDING_DBUS_NAME))
    {
        KLOG_WARNING(audio) << "Failed to register dbus name: " << KEYBINDING_DBUS_NAME;
        return;
    }

    if (!sessionConnection.registerObject(KEYBINDING_OBJECT_PATH, KEYBINDING_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR(audio) << "Can't register object:" << sessionConnection.lastError();
        return;
    }

    connect(m_systemShortcuts.data(), &SystemShortcuts::shortcutAdded, this, &KeybindingManager::processSystemShortcutAdded);
    connect(m_systemShortcuts.data(), &SystemShortcuts::shortcutDeleted, this, &KeybindingManager::processSystemShortcutDeleted);
    connect(m_systemShortcuts.data(), &SystemShortcuts::shortcutChanged, this, &KeybindingManager::processSystemShortcutChanged);
}

bool KeybindingManager::hasSameKeycomb(const QString &uid, const QString &keyComb)
{
    auto customShortcut = m_customShortcuts->getByKeyComb(keyComb);
    RETURN_VAL_IF_TRUE(customShortcut && customShortcut->uid != uid, true);

    auto systemShortcut = m_systemShortcuts->getByKeycomb(keyComb);
    RETURN_VAL_IF_TRUE(systemShortcut && systemShortcut->uid != uid, true);

    return false;
}

void KeybindingManager::processSystemShortcutAdded(QSharedPointer<SystemShortcut> systemShortcut)
{
    if (systemShortcut)
    {
        QJsonObject jsonObj;
        jsonObj[KEYBINDING_SHORTCUT_JK_UID] = systemShortcut->uid;
        jsonObj[KEYBINDING_SHORTCUT_JK_KIND] = systemShortcut->kind;
        jsonObj[KEYBINDING_SHORTCUT_JK_TYPE] = KEYBINDING_SHORTCUT_JK_SYSTEM;

        auto systemShortcutStr = QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
        Q_EMIT Added(systemShortcutStr);
    }
}

void KeybindingManager::processSystemShortcutDeleted(QSharedPointer<SystemShortcut> systemShortcut)
{
    if (systemShortcut)
    {
        QJsonObject jsonObj;
        jsonObj[KEYBINDING_SHORTCUT_JK_UID] = systemShortcut->uid;
        jsonObj[KEYBINDING_SHORTCUT_JK_KIND] = systemShortcut->kind;
        jsonObj[KEYBINDING_SHORTCUT_JK_TYPE] = KEYBINDING_SHORTCUT_JK_SYSTEM;

        auto systemShortcutStr = QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
        Q_EMIT Deleted(systemShortcutStr);
    }
}

void KeybindingManager::processSystemShortcutChanged(QSharedPointer<SystemShortcut> systemShortcut)
{
    if (systemShortcut)
    {
        QJsonObject jsonObj;
        jsonObj[KEYBINDING_SHORTCUT_JK_UID] = systemShortcut->uid;
        jsonObj[KEYBINDING_SHORTCUT_JK_KIND] = systemShortcut->kind;
        jsonObj[KEYBINDING_SHORTCUT_JK_TYPE] = KEYBINDING_SHORTCUT_JK_SYSTEM;

        auto systemShortcutStr = QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
        Q_EMIT Changed(systemShortcutStr);
    }
}

}  // namespace Kiran