/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 */

#include "keys-screenshot.h"
#include <glib.h>
#include <KGlobalAccel>
#include <QGSettings>
#include <QKeySequence>
#include <QProcess>
#include "keybinding-i.h"
#include "lib/base/base.h"

namespace Kiran
{
#define ACTION_NAME_WINDOW_SCREENSHOT "windowScreenshot"
#define ACTION_NAME_FULLSCREEN_SCREENSHOT "fullscreenScreenshot"
#define ACTION_NAME_AREA_SCREENSHOT "areaScreenshot"

KeysScreenshot::KeysScreenshot() : KeysComponent("Screenshot", tr("Screenshot")),
                                   m_syncingFromSettings(false),
                                   m_syncingFromKGlobalAccel(false)
{
    m_actionSchemaKeyMap.insert(ACTION_NAME_WINDOW_SCREENSHOT, KEYS_SCHEMA_WINDOW_SCREENSHOT);
    m_actionSchemaKeyMap.insert(ACTION_NAME_FULLSCREEN_SCREENSHOT, KEYS_SCHEMA_FULLSCREEN_SCREENSHOT);
    m_actionSchemaKeyMap.insert(ACTION_NAME_AREA_SCREENSHOT, KEYS_SCHEMA_AREA_SCREENSHOT);

    for (auto iter = m_actionSchemaKeyMap.constBegin(); iter != m_actionSchemaKeyMap.constEnd(); ++iter)
    {
        m_schemaActionMap.insert(iter.value(), iter.key());
    }
}

void KeysScreenshot::init()
{
    registerConfigurableShortcuts(false);

    connect(m_settings, &QGSettings::changed, this, &KeysScreenshot::processSettingsChanged);
    connect(this, &KeysComponent::shortcutChanged, this, &KeysScreenshot::processKGlobalShortcutChanged);
}

void KeysScreenshot::registerConfigurableShortcuts(bool forceUpdate)
{
    for (auto iter = m_actionSchemaKeyMap.constBegin(); iter != m_actionSchemaKeyMap.constEnd(); ++iter)
    {
        registerConfigurableShortCut(iter.value(), forceUpdate);
    }
}

void KeysScreenshot::registerConfigurableShortCut(const QString &schemaKey, bool forceUpdate)
{
    auto actionName = actionBySchemaKey(schemaKey);
    RETURN_IF_TRUE(actionName.isEmpty());

    auto shortcut = m_settings->get(schemaKey).toString();
    registerShortCut(shortcut, actionName, displayNameByAction(actionName), true, forceUpdate);
}

void KeysScreenshot::processSettingsChanged(const QString &schemaKey)
{
    if (m_syncingFromKGlobalAccel)
    {
        return;
    }

    auto actionName = actionBySchemaKey(schemaKey);
    RETURN_IF_TRUE(actionName.isEmpty());

    auto shortcut = m_settings->get(schemaKey).toString();
    auto keySequenceList = KGlobalAccel::self()->globalShortcut(getComponentName(), actionName);
    auto currentShortcut = keySequenceList.isEmpty() ? QStringLiteral("") : keySequenceList.first().toString();
    RETURN_IF_TRUE(currentShortcut == shortcut);

    KLOG_INFO(keybinding) << "Syncing shortcut for"
                          << schemaKey << "from gsettings to KGlobalAccel, "
                          << "new shortcut is" << shortcut;

    m_syncingFromSettings = true;
    registerConfigurableShortCut(schemaKey, true);
    m_syncingFromSettings = false;
}

void KeysScreenshot::processKGlobalShortcutChanged(const QString &actionUnique)
{
    if (m_syncingFromSettings)
    {
        return;
    }

    auto schemaKey = schemaKeyByAction(actionUnique);
    if (schemaKey.isEmpty())
    {
        return;
    }

    auto keySequenceList = KGlobalAccel::self()->globalShortcut(getComponentName(), actionUnique);
    auto shortcut = keySequenceList.isEmpty() ? QStringLiteral("") : keySequenceList.first().toString();
    auto currentShortcut = m_settings->get(schemaKey).toString();
    RETURN_IF_TRUE(currentShortcut == shortcut);

    KLOG_INFO(keybinding) << "Syncing shortcut for"
                          << schemaKey << "from KGlobalAccel to gsettings, "
                          << "old shortcut is" << currentShortcut << "and "
                          << "new shortcut is" << shortcut;

    m_syncingFromKGlobalAccel = true;
    m_settings->set(schemaKey, shortcut);
    m_syncingFromKGlobalAccel = false;
}

QString KeysScreenshot::schemaKeyByAction(const QString &actionUnique) const
{
    return m_actionSchemaKeyMap.value(actionUnique);
}

QString KeysScreenshot::actionBySchemaKey(const QString &schemaKey) const
{
    return m_schemaActionMap.value(schemaKey);
}

QString KeysScreenshot::displayNameByAction(const QString &actionUnique) const
{
    switch (shash(actionUnique.toUtf8().data()))
    {
    case CONNECT(ACTION_NAME_WINDOW_SCREENSHOT, _hash):
        return tr("Window screenshot");
    case CONNECT(ACTION_NAME_FULLSCREEN_SCREENSHOT, _hash):
        return tr("Fullscreen screenshot");
    case CONNECT(ACTION_NAME_AREA_SCREENSHOT, _hash):
        return tr("Area screenshot");
    default:
        break;
    }

    return QString();
}

QString KeysScreenshot::commandSchemaKeyByAction(const QString &actionUnique) const
{
    switch (shash(actionUnique.toUtf8().data()))
    {
    case CONNECT(ACTION_NAME_WINDOW_SCREENSHOT, _hash):
        return KEYS_SCHEMA_WINDOW_SCREENSHOT_COMMAND;
    case CONNECT(ACTION_NAME_FULLSCREEN_SCREENSHOT, _hash):
        return KEYS_SCHEMA_FULLSCREEN_SCREENSHOT_COMMAND;
    case CONNECT(ACTION_NAME_AREA_SCREENSHOT, _hash):
        return KEYS_SCHEMA_AREA_SCREENSHOT_COMMAND;
    default:
        break;
    }

    return QString();
}

void KeysScreenshot::executeScreenshotCommand(const QString &commandSchemaKey)
{
    auto command = m_settings->get(commandSchemaKey).toString().trimmed();
    if (command.isEmpty())
    {
        KLOG_WARNING(keybinding) << "Screenshot command for" << commandSchemaKey << "is empty.";
        return;
    }

    gchar **argv = nullptr;
    int argc = 0;
    GError *error = nullptr;

    g_shell_parse_argv(command.toStdString().c_str(), &argc, &argv, &error);
    if (error)
    {
        KLOG_WARNING(keybinding) << "Parse screenshot command" << command << "failed, error is" << error->message;
        g_error_free(error);
        return;
    }

    if (argc <= 0)
    {
        KLOG_WARNING(keybinding) << "The screenshot command" << command << "is invalid.";
        return;
    }

    SCOPE_EXIT(
        {
            if (argv)
            {
                g_strfreev(argv);
            }
        });

    auto program = QString(argv[0]);
    QStringList arguments;
    for (auto i = 1; i < argc; ++i)
    {
        arguments.append(argv[i]);
    }

    KLOG_INFO(keybinding) << "Execute screenshot command" << command;

    if (!QProcess::startDetached(program, arguments))
    {
        KLOG_WARNING(keybinding) << "Execute screenshot command" << command << "failed.";
    }
}

void KeysScreenshot::triggerShortCut(const QString &name)
{
    KLOG_INFO(keybinding) << "Execute command for shortcut" << name;

    auto commandSchemaKey = commandSchemaKeyByAction(name);
    if (commandSchemaKey.isEmpty())
    {
        return;
    }

    executeScreenshotCommand(commandSchemaKey);
}

}  // namespace Kiran
