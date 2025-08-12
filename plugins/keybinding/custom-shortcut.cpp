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

#include "custom-shortcut.h"
#include <glib.h>
#include <KActionCollection>
#include <KGlobalAccel>
#include <QAction>
#include <QProcess>
#include <QRandomGenerator>
#include <QSettings>
#include <QStandardPaths>
#include "config.h"
#include "keybinding-utils.h"
#include "lib/base/base.h"

namespace Kiran
{
#define KEYBINDING_CONF_DIR "kylinsec/" PROJECT_NAME "/keybinding"
#define CUSTOM_SHORTCUT_FILE "custom_shortcut.ini"
#define CUSTOM_KEYFILE_NAME "name"
#define CUSTOM_KEYFILE_ACTION "action"
#define CUSTOM_KEYFILE_KEYCOMB "key_combination"
#define GEN_ID_MAX_NUM 5
#define SAVE_TIMEOUT_MILLISECONDS 500

CustomShortcuts::CustomShortcuts()
{
    auto configLocation = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    m_confFilePath = QString("%1/%2/%3")
                         .arg(configLocation)
                         .arg(KEYBINDING_CONF_DIR)
                         .arg(CUSTOM_SHORTCUT_FILE);

    m_settings = new QSettings(m_confFilePath, QSettings::IniFormat, this);
    m_actionCollection = new KActionCollection(this);
    m_actionCollection->setComponentName("Custom");
    m_actionCollection->setComponentDisplayName(tr("Custom"));
}

CustomShortcuts::~CustomShortcuts()
{
}

void CustomShortcuts::init()
{
    auto groups = m_settings->childGroups();
    for (const auto &group : groups)
    {
        auto shortcut = get(group);
        registerShortcut(shortcut);
    }
}

bool CustomShortcuts::add(QSharedPointer<CustomShortCut> shortcut)
{
    KLOG_INFO(keybinding) << "Add custom shortcut, name is"
                          << shortcut->name
                          << "action is" << shortcut->action
                          << "keyComb is" << shortcut->keyComb;

    RETURN_VAL_IF_FALSE(checkValid(shortcut), false);

    shortcut->uid = genUid();
    if (shortcut->uid.size() == 0)
    {
        KLOG_WARNING(keybinding) << "Cannot generate unique ID for custom shortcut.";
        return false;
    }

    m_settings->beginGroup(shortcut->uid);
    m_settings->setValue(CUSTOM_KEYFILE_NAME, shortcut->name);
    m_settings->setValue(CUSTOM_KEYFILE_ACTION, shortcut->action);
    m_settings->setValue(CUSTOM_KEYFILE_KEYCOMB, shortcut->keyComb);
    m_settings->endGroup();
    m_settings->sync();

    registerShortcut(shortcut);
    return true;
}

bool CustomShortcuts::modify(QSharedPointer<CustomShortCut> shortcut)
{
    KLOG_INFO(keybinding) << "Modify custom shortcut, name is"
                          << shortcut->name
                          << "action is" << shortcut->action
                          << "keyComb is" << shortcut->keyComb;

    RETURN_VAL_IF_FALSE(checkValid(shortcut), false);

    if (!exists(shortcut->uid))
    {
        KLOG_WARNING(keybinding) << "The shortcut" << shortcut->uid << "is not exists.";
        return false;
    }

    auto key = QString("%1/%2").arg(shortcut->uid).arg(CUSTOM_KEYFILE_KEYCOMB);
    auto oldKeyComb = m_settings->value(key).toString();

    if (oldKeyComb != shortcut->keyComb)
    {
        registerShortcut(shortcut);
    }
    m_settings->setValue(key, shortcut->keyComb);
    return true;
}

bool CustomShortcuts::remove(const QString &uid)
{
    KLOG_INFO(keybinding) << "Remove custom shortcut key by" << uid;

    auto shortcut = get(uid);
    if (!shortcut)
    {
        KLOG_WARNING(keybinding) << "The keycomb" << uid << "is not exists.";
        return false;
    }

    unregisterShortcut(shortcut);
    m_settings->remove(uid);
    m_settings->sync();
    return true;
}

QSharedPointer<CustomShortCut> CustomShortcuts::get(const QString &uid)
{
    auto groups = m_settings->childGroups();
    if (!groups.contains(uid))
    {
        return nullptr;
    }

    m_settings->beginGroup(uid);

    auto shortcut = QSharedPointer<CustomShortCut>::create();
    shortcut->uid = uid;
    shortcut->name = m_settings->value(CUSTOM_KEYFILE_NAME).toString();
    shortcut->action = m_settings->value(CUSTOM_KEYFILE_ACTION).toString();
    shortcut->keyComb = m_settings->value(CUSTOM_KEYFILE_KEYCOMB).toString();

    m_settings->endGroup();

    return shortcut;
}

QSharedPointer<CustomShortCut> CustomShortcuts::getByKeyComb(const QString &keyComb)
{
    auto groups = m_settings->childGroups();
    for (auto &group : groups)
    {
        auto shortcut = get(group);
        if (shortcut->keyComb == keyComb)
        {
            return shortcut;
        }
    }
    return nullptr;
}

QMap<QString, QSharedPointer<CustomShortCut>> CustomShortcuts::get()
{
    QMap<QString, QSharedPointer<CustomShortCut>> shortcuts;

    auto groups = m_settings->childGroups();
    for (const auto &group : groups)
    {
        auto shortcut = get(group);
        if (shortcut)
        {
            shortcuts.insert(group, shortcut);
        }
    }
    return shortcuts;
}

bool CustomShortcuts::exists(const QString &uid)
{
    auto groups = m_settings->childGroups();
    return groups.contains(uid);
}

QString CustomShortcuts::genUid()
{
    auto groups = m_settings->childGroups();
    for (int i = 0; i < GEN_ID_MAX_NUM; ++i)
    {
        quint32 value = QRandomGenerator::global()->generate();
        auto randGroup = QString("Custom%1").arg(value);
        if (!groups.contains(randGroup))
        {
            return randGroup;
        }
    }
    return QString();
}

bool CustomShortcuts::checkValid(QSharedPointer<CustomShortCut> shortcut)
{
    if (shortcut->name.length() == 0 ||
        shortcut->action.length() == 0)
    {
        KLOG_WARNING(keybinding) << "The name or action is null string";
        return false;
    }

    if (!KeybindingUtils::isValidKeySequence(shortcut->keyComb))
    {
        return false;
    }
    return true;
}

void CustomShortcuts::registerShortcut(QSharedPointer<CustomShortCut> shortcut)
{
    auto customAction = m_actionCollection->action(shortcut->uid);
    if (customAction == nullptr)
    {
        customAction = m_actionCollection->addAction(shortcut->uid);
        customAction->setText(shortcut->name);
        customAction->setData(shortcut->uid);
        connect(customAction,
                &QAction::triggered,
                std::bind(&CustomShortcuts::triggerAction, this, std::placeholders::_1, customAction));
    }

    KGlobalAccel::self()->setShortcut(customAction, QList<QKeySequence>() << shortcut->keyComb, KGlobalAccel::NoAutoloading);
}

void CustomShortcuts::unregisterShortcut(QSharedPointer<CustomShortCut> shortcut)
{
    auto customAction = m_actionCollection->action(shortcut->uid);
    if (customAction == nullptr)
    {
        KLOG_WARNING(keybinding) << "Not exists shortcut" << shortcut->uid;
        return;
    }
    KGlobalAccel::self()->removeAllShortcuts(customAction);
    customAction->disconnect();
    m_actionCollection->takeAction(customAction);
    delete customAction;
}

void CustomShortcuts::triggerAction(bool checked, QAction *action)
{
    auto uid = action->data().toString();
    auto shortcut = get(uid);

    char **argv = nullptr;
    int argc = 0;
    GError *error = nullptr;

    g_shell_parse_argv(shortcut->action.toStdString().c_str(), &argc, &argv, &error);
    if (error)
    {
        KLOG_WARNING(keybinding) << "Parse action" << shortcut->action << "failed, error is " << error->message;
        g_error_free(error);
        return;
    }

    if (argc <= 0)
    {
        KLOG_WARNING(keybinding) << "The action" << shortcut->action << "is invalid.";
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

    KLOG_INFO(keybinding) << "Execute custom shortcut command" << shortcut->action;

    if (!QProcess::startDetached(program, arguments))
    {
        KLOG_WARNING(keybinding) << "Execute command" << shortcut->action << "failed.";
        return;
    }
}

}  // namespace Kiran