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

/*
kglobalaccel的快捷键包含了当前生效快捷键和默认快捷键，提供的几个接口区别：
  KGlobalAccel::self()->setShortcut：只修改当前生效的快捷键。如果传入的flag是Autoloading时，只有第一次初始化时设置才生效。
  KGlobalAccelInterface::setForeignShortcutKeys：跟KGlobalAccel::self()->setShortcut传入NoAutoloading参数时的效果一样，强制覆盖当前快捷键。
  KGlobalAccel::self()->setGlobalShortcut：修改当前快捷键和默认快捷键。当前快捷键只有第一次设置生效，默认快捷键每次都会覆盖。
*/

SystemShortcuts::SystemShortcuts()
{
    m_globalAccelInterface = new KGlobalAccelInterface(QStringLiteral("org.kde.kglobalaccel"),
                                                       QStringLiteral("/kglobalaccel"),
                                                       QDBusConnection::sessionBus(),
                                                       this);

    qDBusRegisterMetaType<KGlobalShortcutInfo>();
    qDBusRegisterMetaType<QList<KGlobalShortcutInfo>>();
}

void SystemShortcuts::init()
{
    /* 这里只初始化MATE的系统快捷键，因为KDE的系统快捷键依赖kglobalaccel，
       kglobalaccel并没有提供component增加/删除/变化信号，所以只能实时更新。*/
    initMateShortcuts();
}

bool SystemShortcuts::modify(const QString &uid, const QString &keyCombination)
{
    auto mixShortcuts = getMixShortcuts();
    auto shortcutMix = mixShortcuts.value(uid);
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
            KLOG_INFO(keybinding) << "Modify shortcut" << shortcutMix->mate.name
                                  << "from" << shortcutMix->mate.keyCombination
                                  << "to" << keyCombination;

            shortcutMix->mate.keyCombination = keyCombination;
            shortcutMix->mate.gsettings->set(shortcutMix->mate.settingsKey, keyCombination);
            Q_EMIT shortcutChanged(mix2common(shortcutMix));
        }
        break;
    }
    case SystemShortcutDesktopType::SYSTEM_SHORTCUT_DESKTOP_TYPE_KDE:
    {
        if (keys2GtkStr(shortcutMix->kde.keys) != keyCombination)
        {
            KLOG_INFO(keybinding) << "Modify shortcut" << shortcutMix->kde.friendlyName
                                  << "from" << keys2GtkStr(shortcutMix->kde.keys)
                                  << "to" << keyCombination;

            shortcutMix->kde.keys = (QList<QKeySequence>() << KeybindingUtils::keyCombGtk2Qt(keyCombination));
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
    auto mixShortcuts = getMixShortcuts();
    auto shortcutMix = mixShortcuts.value(uid);
    return mix2common(shortcutMix);
}

QSharedPointer<SystemShortcut> SystemShortcuts::getByKeycomb(const QString &keycomb)
{
    RETURN_VAL_IF_FALSE(KeybindingUtils::isValidKeySequence(keycomb), nullptr);

    auto mixShortcuts = getMixShortcuts();
    for (const auto &mixShortcut : mixShortcuts)
    {
        auto shortcut = mix2common(mixShortcut);
        if (shortcut->keyCombination.compare(keycomb, Qt::CaseInsensitive) == 0)
        {
            return shortcut;
        }
    }
    return nullptr;
}

QList<QSharedPointer<SystemShortcut>> SystemShortcuts::get()
{
    QList<QSharedPointer<SystemShortcut>> shortcuts;
    auto mixShortcuts = getMixShortcuts();

    for (const auto &mixShortcut : mixShortcuts)
    {
        auto shortcut = mix2common(mixShortcut);
        if (shortcut)
        {
            shortcuts.push_back(shortcut);
        }
    }

    return shortcuts;
}

void SystemShortcuts::reset()
{
    auto mixShortcuts = getMixShortcuts();

    // 将所有需要重置的快捷键设置为默认值
    for (auto &mixShortcut : mixShortcuts)
    {
        switch (mixShortcut->desktopType)
        {
        case SystemShortcutDesktopType::SYSTEM_SHORTCUT_DESKTOP_TYPE_MATE:
        {
            auto gsettings = mixShortcut->mate.gsettings;
            gsettings->reset(mixShortcut->mate.settingsKey);

            auto newKeyCombination = gsettings->get(mixShortcut->mate.settingsKey).toString();
            if (newKeyCombination != mixShortcut->mate.keyCombination)
            {
                mixShortcut->mate.keyCombination = newKeyCombination;
                Q_EMIT shortcutChanged(mix2common(mixShortcut));
            }
            break;
        }
        case SystemShortcutDesktopType::SYSTEM_SHORTCUT_DESKTOP_TYPE_KDE:
        {
            auto actionID = buildActionId(mixShortcut->kde);
            if (mixShortcut->kde.defaultKeys != mixShortcut->kde.keys)
            {
                m_globalAccelInterface->setForeignShortcutKeys(actionID, mixShortcut->kde.defaultKeys);
                mixShortcut->kde.keys = mixShortcut->kde.defaultKeys;
                Q_EMIT shortcutChanged(mix2common(mixShortcut));
            }
            break;
        }
        default:
            KLOG_WARNING(keybinding) << "Unknown desktop type" << mixShortcut->desktopType;
        }
    }
}

void SystemShortcuts::initMateShortcuts()
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

    KLOG_INFO(keybinding) << "The window manager keybindings are" << wmKeybindings;

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

            auto shortcutMix = QSharedPointer<MixSystemShortcut>::create();
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
                                         << "and nam is" << shortcutMix->mate.name
                                         << "and keycomb is" << shortcutMix->mate.keyCombination;
                continue;
            }

            auto uidData = QString("%1:%2").arg(keylistEntries.schema, keylistEntry.name);
            shortcutMix->uid = QCryptographicHash::hash(uidData.toUtf8(), QCryptographicHash::Md5).toHex();

            if (m_mateShortcuts.contains(shortcutMix->uid))
            {
                KLOG_WARNING(keybinding) << "Exists the same system shortcut. uid is" << shortcutMix->uid
                                         << "and schema is" << keylistEntries.schema
                                         << "and key is" << keylistEntry.name;
                continue;
            }
            m_mateShortcuts.insert(shortcutMix->uid, shortcutMix);
        }
    }

    for (auto iter = m_gsettingsSet.begin(); iter != m_gsettingsSet.end(); ++iter)
    {
        connect(iter.value(), &QGSettings::changed,
                std::bind(&SystemShortcuts::processSettingsChanged, this, std::placeholders::_1, iter.key()));
    }
}

QMap<QString, QSharedPointer<MixSystemShortcut>> SystemShortcuts::getMixShortcuts()
{
    auto mixShortcuts = getKShortcuts();

    for (auto mateShortcut : m_mateShortcuts)
    {
        if (mixShortcuts.contains(mateShortcut->uid))
        {
            KLOG_WARNING(keybinding) << "Mate and Kde have same uid" << mateShortcut->uid
                                     << ", ignore mate shortcut";
            continue;
        }
        mixShortcuts.insert(mateShortcut->uid, mateShortcut);
    }
    return mixShortcuts;
}

QMap<QString, QSharedPointer<MixSystemShortcut>> SystemShortcuts::getKShortcuts()
{
    if (!m_globalAccelInterface->isValid())
    {
        KLOG_WARNING(keybinding) << "Failed to communicate with global shortcuts daemon";
        return QMap<QString, QSharedPointer<MixSystemShortcut>>();
    }

    QMap<QString, QSharedPointer<MixSystemShortcut>> kSystemShortcuts;
    QList<KGlobalShortcutInfo> globalShortcutInfos;

    auto componentPathsReply = m_globalAccelInterface->allComponents();
    componentPathsReply.waitForFinished();
    if (componentPathsReply.isError())
    {
        KLOG_WARNING(keybinding) << "Failed to get all components:" << componentPathsReply.error();
        return QMap<QString, QSharedPointer<MixSystemShortcut>>();
    }

    auto componentPaths = componentPathsReply.value();
    for (const auto &componentPath : componentPaths)
    {
        const QString path = componentPath.path();
        KGlobalAccelComponentInterface component(m_globalAccelInterface->service(), path, m_globalAccelInterface->connection());

        // 过滤自定义快捷键
        auto componentName = component.uniqueName();
        if (componentName.compare("custom", Qt::CaseInsensitive) == 0)
        {
            continue;
        }

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
        auto shortcutMix = QSharedPointer<MixSystemShortcut>::create();

        auto data = QString("%1:%2").arg(globalShortcutInfo.componentUniqueName()).arg(globalShortcutInfo.uniqueName());
        shortcutMix->uid = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Algorithm::Md5).toHex();
        shortcutMix->desktopType = SystemShortcutDesktopType::SYSTEM_SHORTCUT_DESKTOP_TYPE_KDE;
        shortcutMix->kde.componentUniqueName = globalShortcutInfo.componentUniqueName();
        shortcutMix->kde.componentFriendlyName = globalShortcutInfo.componentFriendlyName();
        shortcutMix->kde.uniqueName = globalShortcutInfo.uniqueName();
        shortcutMix->kde.friendlyName = globalShortcutInfo.friendlyName();
        shortcutMix->kde.keys = globalShortcutInfo.keys();
        shortcutMix->kde.defaultKeys = globalShortcutInfo.defaultKeys();
        kSystemShortcuts.insert(shortcutMix->uid, shortcutMix);

        KLOG_DEBUG(keybinding) << "Loaded shortcut:" << shortcutMix->uid
                               << shortcutMix->kde.componentUniqueName
                               << shortcutMix->kde.componentFriendlyName
                               << shortcutMix->kde.uniqueName
                               << shortcutMix->kde.friendlyName
                               << keys2GtkStr(shortcutMix->kde.keys)
                               << keys2GtkStr(shortcutMix->kde.defaultKeys);
    }

    return kSystemShortcuts;
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

QSharedPointer<SystemShortcut> SystemShortcuts::mix2common(QSharedPointer<MixSystemShortcut> shortcutMix)
{
    RETURN_VAL_IF_FALSE(shortcutMix, nullptr);

    switch (shortcutMix->desktopType)
    {
    case SystemShortcutDesktopType::SYSTEM_SHORTCUT_DESKTOP_TYPE_MATE:
    {
        return QSharedPointer<SystemShortcut>(new SystemShortcut{.uid = shortcutMix->uid,
                                                                 .kind = shortcutMix->mate.kind,
                                                                 .name = shortcutMix->mate.name,
                                                                 .keyCombination = shortcutMix->mate.keyCombination});
    }
    case SystemShortcutDesktopType::SYSTEM_SHORTCUT_DESKTOP_TYPE_KDE:
    {
        auto keyCombination = keys2GtkStr(shortcutMix->kde.keys);
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

QString SystemShortcuts::keys2GtkStr(const QList<QKeySequence> &keys)
{
    if (keys.size() == 0)
    {
        return QString();
    }
    // Key_Super_L键需要做一下特殊处理，否则前端界面显示是乱码
    auto keySequence0 = keys.at(0);
    if (keySequence0 == Qt::Key_Super_L)
    {
        return QString("Super");
    }
    return KeybindingUtils::keyCombQt2Gtk(keySequence0.toString());
}

QStringList SystemShortcuts::buildActionId(const KDESystemShortcut &systemShortcutKDE)
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
    auto shortcut = m_mateShortcuts.value(key);
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