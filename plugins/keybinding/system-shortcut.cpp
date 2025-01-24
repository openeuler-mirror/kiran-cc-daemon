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

#include "system-shortcut.h"
#include <kglobalaccel_component_interface.h>
#include <kglobalaccel_interface.h>
#include <libintl.h>
#include <KGlobalAccel>
#include <QCryptographicHash>
#include <QGSettings>
#include <QGuiApplication>
#include <functional>
#include "config.h"
#include "keybinding-utils.h"
#include "keylist-entries-parser.h"
#include "lib/base/base.h"
#include "lib/xcb/EWMH.h"

namespace Kiran
{
#define KCC_KEYBINDINGS_DIR KCD_INSTALL_DATADIR "/keybindings"

SystemShortcuts::SystemShortcuts()
{
    m_globalAccelInterface = new KGlobalAccelInterface(QStringLiteral("org.kde.kglobalaccel"),
                                                       QStringLiteral("/kglobalaccel"),
                                                       QDBusConnection::sessionBus(),
                                                       this);
    if (!m_globalAccelInterface->isValid())
    {
        KLOG_WARNING(keybinding) << "Failed to communicate with global shortcuts daemon";
    }

    qDBusRegisterMetaType<KGlobalShortcutInfo>();
    qDBusRegisterMetaType<QList<KGlobalShortcutInfo>>();
}

void SystemShortcuts::init()
{
    initKSystemShortcuts();
    initMarcoSystemShortcuts();
}

bool SystemShortcuts::modify(const QString &uid, const QString &keyCombination)
{
    auto shortcutMix = m_shortcutMixs.value(uid);
    if (!shortcutMix)
    {
        KLOG_WARNING(keybinding) << "The shortcut" << uid << "is not exists";
        return false;
    }

    switch (shortcutMix->desktopType)
    {
    case SystemShortcutDesktopType::SYSTEM_SHORTCUT_DESKTOP_TYPE_MATE:
    {
        if (shortcutMix->mate.keyCombination != keyCombination)
        {
            shortcutMix->mate.keyCombination = keyCombination;
            shortcutMix->mate.gsettings->set(shortcutMix->mate.settingsKey, keyCombination);
            Q_EMIT shortcutChanged(mix2common(shortcutMix));
        }
        break;
    }
    case SystemShortcutDesktopType::SYSTEM_SHORTCUT_DESKTOP_TYPE_KDE:
    {
        if (keys2Str(shortcutMix->kde.keys) != keyCombination)
        {
            shortcutMix->kde.keys = (QList<QKeySequence>() << keyCombination);
            auto actionID = buildActionId(shortcutMix->kde);
            m_globalAccelInterface->setForeignShortcutKeys(actionID, shortcutMix->kde.keys);
            Q_EMIT shortcutChanged(mix2common(shortcutMix));
        }
        break;
    }
    default:
        KLOG_WARNING(keybinding) << "Unknown desktop type" << shortcutMix->desktopType;
        return false;
    }

    return true;
}

QSharedPointer<SystemShortcut> SystemShortcuts::get(const QString &uid)
{
    auto shortcutMix = m_shortcutMixs.value(uid);
    return mix2common(shortcutMix);
}

QSharedPointer<SystemShortcut> SystemShortcuts::getByKeycomb(const QString &keycomb)
{
    RETURN_VAL_IF_FALSE(KeybindingUtils::isValidKeySequence(keycomb), nullptr);

    for (const auto &shortcutMix : m_shortcutMixs)
    {
        auto shortcut = mix2common(shortcutMix);
        if (shortcut->keyCombination == keycomb)
        {
            return shortcut;
        }
    }
    return nullptr;
}

QList<QSharedPointer<SystemShortcut>> SystemShortcuts::get()
{
    QList<QSharedPointer<SystemShortcut>> shortcuts;

    for (const auto &shortcutMix : m_shortcutMixs)
    {
        auto shortcut = mix2common(shortcutMix);
        if (shortcut)
        {
            shortcuts.push_back(shortcut);
        }
    }

    return shortcuts;
}

void SystemShortcuts::reset()
{
    // 将所有需要重置的快捷键设置为默认值
    for (auto &shortcutMix : m_shortcutMixs)
    {
        switch (shortcutMix->desktopType)
        {
        case SystemShortcutDesktopType::SYSTEM_SHORTCUT_DESKTOP_TYPE_MATE:
        {
            auto gsettings = shortcutMix->mate.gsettings;
            gsettings->reset(shortcutMix->mate.settingsKey);

            auto newKeyCombination = gsettings->get(shortcutMix->mate.settingsKey).toString();
            if (newKeyCombination != shortcutMix->mate.keyCombination)
            {
                shortcutMix->mate.keyCombination = newKeyCombination;
                Q_EMIT shortcutChanged(mix2common(shortcutMix));
            }
            break;
        }
        case SystemShortcutDesktopType::SYSTEM_SHORTCUT_DESKTOP_TYPE_KDE:
        {
            auto actionID = buildActionId(shortcutMix->kde);
            if (shortcutMix->kde.defaultKeys != shortcutMix->kde.keys)
            {
                m_globalAccelInterface->setForeignShortcutKeys(actionID, shortcutMix->kde.defaultKeys);
                shortcutMix->kde.keys = shortcutMix->kde.defaultKeys;
                Q_EMIT shortcutChanged(mix2common(shortcutMix));
            }
            break;
        }
        default:
            KLOG_WARNING(keybinding) << "Unknown desktop type" << shortcutMix->desktopType;
        }
    }
}

void SystemShortcuts::initKSystemShortcuts()
{
    if (!m_globalAccelInterface->isValid())
    {
        return;
    }

    QList<KGlobalShortcutInfo> globalShortcutInfos;

    auto componentPathsReply = m_globalAccelInterface->allComponents();
    componentPathsReply.waitForFinished();
    if (componentPathsReply.isError())
    {
        KLOG_WARNING(keybinding) << "Failed to get all components:" << componentPathsReply.error();
        return;
    }

    auto componentPaths = componentPathsReply.value();
    for (const auto &componentPath : componentPaths)
    {
        const QString path = componentPath.path();
        KGlobalAccelComponentInterface component(m_globalAccelInterface->service(), path, m_globalAccelInterface->connection());

        auto componentShortcutInfosReply = component.allShortcutInfos();
        componentShortcutInfosReply.waitForFinished();
        if (componentShortcutInfosReply.isError())
        {
            KLOG_WARNING(keybinding) << "Failed to get all shortcuts for" << path << ":" << componentShortcutInfosReply.error();
            continue;
        }

        auto componentShortcutInfos = componentShortcutInfosReply.value();
        globalShortcutInfos.append(componentShortcutInfos);
    }

    for (const auto &globalShortcutInfo : globalShortcutInfos)
    {
        auto shortcutMix = QSharedPointer<SystemShortcutMix>::create();

        auto data = QString("%1:%2").arg(globalShortcutInfo.componentUniqueName()).arg(globalShortcutInfo.uniqueName());
        shortcutMix->uid = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Algorithm::Md5).toHex();
        shortcutMix->desktopType = SystemShortcutDesktopType::SYSTEM_SHORTCUT_DESKTOP_TYPE_KDE;
        shortcutMix->kde.componentUniqueName = globalShortcutInfo.componentUniqueName();
        shortcutMix->kde.componentFriendlyName = globalShortcutInfo.componentFriendlyName();
        shortcutMix->kde.uniqueName = globalShortcutInfo.uniqueName();
        shortcutMix->kde.friendlyName = globalShortcutInfo.friendlyName();
        shortcutMix->kde.keys = globalShortcutInfo.keys();
        shortcutMix->kde.defaultKeys = globalShortcutInfo.defaultKeys();
        m_shortcutMixs.insert(shortcutMix->uid, shortcutMix);
    }
}

void SystemShortcuts::initMarcoSystemShortcuts()
{
    RETURN_IF_TRUE(qGuiApp->platformName() != QLatin1String("xcb"))

    KeyListEntriesParser parser(KCC_KEYBINDINGS_DIR);
    QVector<KeyListEntries> keys;
    QString error;
    if (!parser.parse(keys, error))
    {
        KLOG_WARNING(keybinding) << "Failed to parse" << KCC_KEYBINDINGS_DIR << ", error is" << error;
        return;
    }

    auto wmKeybindings = EWMH::getDefault()->getWmKeybindings();

    for (auto &keylistEntries : keys)
    {
        auto &package = keylistEntries.package;

        if (keylistEntries.wmName.length() > 0 && !wmKeybindings.contains(keylistEntries.wmName))
        {
            KLOG_INFO(keybinding) << keylistEntries.wmName << "is not current window manager, ignore it's keybindings";
            continue;
        }

        // 过滤掉没有翻译文件的配置
        if (package.isEmpty())
        {
            KLOG_WARNING(keybinding) << "Filter the keylist entries which name is" << keylistEntries.name << ", because the translation file not be found.";
            continue;
        }

        if (!QGSettings::isSchemaInstalled(keylistEntries.schema.toUtf8()))
        {
            KLOG_WARNING(keybinding) << "The schema id" << keylistEntries.schema << "isn't exist";
            continue;
        }

        auto systemSettings = m_gsettingsSet.value(keylistEntries.schema.toUtf8());
        if (!systemSettings)
        {
            systemSettings = new QGSettings(keylistEntries.schema.toUtf8(), "", this);
            m_gsettingsSet.insert(keylistEntries.schema.toUtf8(), systemSettings);
        }

        bindtextdomain(package.toUtf8().data(), KCC_LOCALEDIR);
        bind_textdomain_codeset(package.toUtf8().data(), "UTF-8");

        for (auto &keylistEntry : keylistEntries.entries)
        {
            // 配置文件支持条件语句对快捷键进行过滤
            if (!shouldShowKey(keylistEntry))
            {
                KLOG_INFO(keybinding) << "The system shortcut should not show. type is" << keylistEntries.name
                                      << ", name is" << keylistEntry.name
                                      << ", description is" << keylistEntry.description;

                continue;
            }

            auto shortcutMix = QSharedPointer<SystemShortcutMix>::create();
            shortcutMix->desktopType = SystemShortcutDesktopType::SYSTEM_SHORTCUT_DESKTOP_TYPE_MATE;

            shortcutMix->mate.kind = dgettext(package.toUtf8().data(), keylistEntries.name.toUtf8().data());
            shortcutMix->mate.name = dgettext(package.toUtf8().data(), keylistEntry.description.toUtf8().data());
            shortcutMix->mate.gsettings = systemSettings;
            shortcutMix->mate.settingsKey = keylistEntry.name;
            shortcutMix->mate.keyCombination = systemSettings->get(keylistEntry.name).toString();

            if (shortcutMix->mate.kind.length() == 0 ||
                shortcutMix->mate.name.length() == 0 ||
                !KeybindingUtils::isValidKeySequence(shortcutMix->mate.keyCombination))
            {
                KLOG_WARNING(keybinding) << "The system shortcut is invalid. kind is" << shortcutMix->mate.kind
                                         << ", name is" << shortcutMix->mate.name
                                         << ", keycomb is" << shortcutMix->mate.keyCombination;
                continue;
            }

            auto uidData = QString("%1:%2").arg(keylistEntries.schema, keylistEntry.name);
            shortcutMix->uid = QCryptographicHash::hash(uidData.toUtf8(), QCryptographicHash::Md5).toHex();

            auto iter = m_shortcutMixs.insert(shortcutMix->uid, shortcutMix);
            if (m_shortcutMixs.contains(shortcutMix->uid))
            {
                KLOG_WARNING(keybinding) << "Exists the same system shortcut. uid is" << shortcutMix->uid
                                         << "schema is" << keylistEntries.schema
                                         << "key is" << keylistEntry.name;
                continue;
            }
        }
    }

    for (auto iter = m_gsettingsSet.begin(); iter != m_gsettingsSet.end(); ++iter)
    {
        connect(iter.value(), &QGSettings::changed,
                std::bind(&SystemShortcuts::processSettingsChanged, this, std::placeholders::_1, iter.key()));
    }
}

bool SystemShortcuts::shouldShowKey(const KeyListEntry &entry)
{
    RETURN_VAL_IF_TRUE(entry.comparison.length() == 0, true);
    RETURN_VAL_IF_TRUE(entry.key.length() == 0, false);
    RETURN_VAL_IF_TRUE(entry.schema.length() == 0, false);

    QGSettings gsettings(entry.schema.toUtf8());
    auto valuel = gsettings.get(entry.key).toInt();
    auto valuer = entry.value.toLongLong();

    switch (shash(entry.comparison.toUtf8().data()))
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

QSharedPointer<SystemShortcut> SystemShortcuts::mix2common(QSharedPointer<SystemShortcutMix> shortcutMix)
{
    RETURN_VAL_IF_FALSE(shortcutMix, nullptr);

    switch (shortcutMix->desktopType)
    {
    case SystemShortcutDesktopType::SYSTEM_SHORTCUT_DESKTOP_TYPE_MATE:
        return QSharedPointer<SystemShortcut>(new SystemShortcut{.uid = shortcutMix->uid,
                                                                 .kind = shortcutMix->mate.kind,
                                                                 .name = shortcutMix->mate.name,
                                                                 .keyCombination = shortcutMix->mate.keyCombination});
    case SystemShortcutDesktopType::SYSTEM_SHORTCUT_DESKTOP_TYPE_KDE:
    {
        auto keyCombination = keys2Str(shortcutMix->kde.keys);
        return QSharedPointer<SystemShortcut>(new SystemShortcut{.uid = shortcutMix->uid,
                                                                 .kind = shortcutMix->kde.componentFriendlyName,
                                                                 .name = shortcutMix->kde.friendlyName,
                                                                 .keyCombination = keyCombination});
    }
    default:
        KLOG_WARNING(keybinding) << "Unknown desktop type" << shortcutMix->desktopType;
        break;
    }
    return nullptr;
}

QString SystemShortcuts::keys2Str(const QList<QKeySequence> &keys)
{
    return keys.size() == 0 ? QString() : keys.at(0).toString();
}

QStringList SystemShortcuts::buildActionId(const SystemShortcutKDE &systemShortcutKDE)
{
    QStringList actionId{"", "", "", ""};
    actionId[KGlobalAccel::ComponentUnique] = systemShortcutKDE.componentUniqueName;
    actionId[KGlobalAccel::ComponentFriendly] = systemShortcutKDE.componentFriendlyName;
    actionId[KGlobalAccel::ActionUnique] = systemShortcutKDE.uniqueName;
    actionId[KGlobalAccel::ActionFriendly] = systemShortcutKDE.friendlyName;
    return actionId;
}

void SystemShortcuts::processSettingsChanged(const QString &key, const QString &schemaID)
{
    auto uidData = QString("%1:%2").arg(schemaID, key);
    auto uid = QCryptographicHash::hash(uidData.toUtf8(), QCryptographicHash::Md5).toHex();
    auto shortcut = m_shortcutMixs.value(key);
    RETURN_IF_FALSE(shortcut);

    if (shortcut)
    {
        auto value = shortcut->mate.gsettings->get(key).toString();
        if (shortcut->mate.keyCombination != value && KeybindingUtils::isValidKeySequence(value))
        {
            shortcut->mate.keyCombination = value;
            Q_EMIT shortcutChanged(mix2common(shortcut));
        }
    }
}

}  // namespace Kiran