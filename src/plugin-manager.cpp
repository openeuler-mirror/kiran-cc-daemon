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
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "src/plugin-manager.h"
#include <QDBusConnection>
#include <QGSettings>
#include <QPluginLoader>
#include <QSettings>
#include "config.h"
#include "lib/base/base.h"
#include "plugin-i.h"
#include "src/plugin_manager_adaptor.h"

namespace Kiran
{
#define SYSTEM_DAEMON_DBUS_NAME "com.kylinsec.Kiran.SystemDaemon"
#define SYSTEM_DAEMON_OBJECT_PATH "/com/kylinsec/Kiran/SystemDaemon"
#define SYSTEM_DAEMON_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SystemDaemon"

#define SESSION_DAEMON_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon"
#define SESSION_DAEMON_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon"
#define SESSION_DAEMON_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SessionDaemon"

#define SESSION_DAEMON_SCHAME_ID "com.kylinsec.kiran.session-daemon"
#define SESSION_DAEMON_SCHEMA_KEY_ENABLED_PLUGINS "enabled-plugins"
#define SESSION_DAEMON_SCHEMA_KEY_DISABLED_PLUGINS "disabled-plugins"

#define SYSTEM_PLUGIN_CONFIG_NAME "plugin_options"

struct PluginMetaData
{
    PluginMetaData() : enabled(false) {}
    QString id;
    QString description;
    // 是否运行插件
    bool enabled;
};

struct PluginInfo
{
    PluginInfo() : pluginInstance(nullptr) {}

    QSharedPointer<QPluginLoader> loader;
    PluginMetaData metaData;
    // 插件实例化的对外接口对象
    IPlugin* pluginInstance;
};

PluginManager::PluginManager(const QString& pluginDir) : m_pluginDir(pluginDir)
{
    m_adaptor = new PluginManagerAdaptor(this);
}

PluginManager::~PluginManager()
{
    deactivatePlugins();
}

void PluginManager::deactivatePlugins()
{
    for (auto& pluginInfo : m_plugins)
    {
        if (pluginInfo->metaData.enabled)
        {
            deactivatePlugin(pluginInfo);
            pluginInfo->metaData.enabled = false;
        }
    }
}

void PluginManager::ActivatePlugin(const QString& id)
{
    auto pluginInfo = m_plugins.value(id);
    if (!pluginInfo)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_PLUGIN_NOT_EXIST);
    }

    if (pluginInfo->metaData.enabled)
    {
        KLOG_DEBUG() << "Plugin" << id << "already be activated.";
        return;
    }

    activatePlugin(pluginInfo);
    pluginInfo->metaData.enabled = true;
}

void PluginManager::DeactivatePlugin(const QString& id)
{
    auto pluginInfo = m_plugins.value(id);
    if (!pluginInfo)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_PLUGIN_NOT_EXIST);
    }

    if (!pluginInfo->metaData.enabled)
    {
        KLOG_DEBUG() << "Plugin" << id << "already is deactivated.";
        return;
    }

    deactivatePlugin(pluginInfo);
    pluginInfo->metaData.enabled = false;
}

QStringList PluginManager::GetActivatedPlugins()
{
    QStringList retval;
    for (auto iter = m_plugins.begin(); iter != m_plugins.end(); ++iter)
    {
        if (iter.value()->metaData.enabled)
        {
            retval.append(iter.key());
        }
    }
    return retval;
}

QStringList PluginManager::GetPlugins()
{
    return m_plugins.keys();
}

void PluginManager::init()
{
    loadPlugins();
}

void PluginManager::loadPlugins()
{
    QDir dir(m_pluginDir);

    for (auto& fileInfo : dir.entryInfoList(QDir::Files))
    {
        if (!fileInfo.fileName().endsWith(".so"))
        {
            KLOG_INFO() << "Ignore file" << fileInfo.filePath() << ", because it doesn't end with so";
            continue;
        }

        auto pluginInfo = QSharedPointer<PluginInfo>::create();
        pluginInfo->loader = QSharedPointer<QPluginLoader>::create(fileInfo.filePath());
        auto metaDataJson = pluginInfo->loader->metaData();
        metaDataJson = metaDataJson.value("MetaData").toObject();
        pluginInfo->metaData.id = metaDataJson.value("id").toString();
        pluginInfo->metaData.description = metaDataJson.value("description").toString();
        pluginInfo->metaData.enabled = pluginIsEnabled(pluginInfo->metaData.id);
        m_plugins.insert(pluginInfo->metaData.id, pluginInfo);

        if (pluginInfo->metaData.enabled)
        {
            activatePlugin(pluginInfo);
        }
    }
}

bool PluginManager::activatePlugin(QSharedPointer<PluginInfo> pluginInfo)
{
    pluginInfo->pluginInstance = dynamic_cast<IPlugin*>(pluginInfo->loader->instance());
    if (pluginInfo->pluginInstance)
    {
        KLOG_INFO() << "Plugin " << pluginInfo->metaData.id << "is activated.";
        pluginInfo->pluginInstance->activate();
    }
    else
    {
        KLOG_ERROR() << "Failed to instantiate plugin" << pluginInfo->metaData.id << ", error:" << pluginInfo->loader->errorString();
        return false;
    }
    return true;
}

bool PluginManager::deactivatePlugin(QSharedPointer<PluginInfo> pluginInfo)
{
    if (pluginInfo->pluginInstance)
    {
        // 插件自身需要进行清理操作
        pluginInfo->pluginInstance->deactivate();
        KLOG_INFO() << "Plugin " << pluginInfo->metaData.id << "is deactivated.";
    }
    return pluginInfo->loader->unload();
}

SessionPluginManager* SessionPluginManager::m_instance = nullptr;
void SessionPluginManager::globalInit(const QString& pluginDir)
{
    m_instance = new SessionPluginManager(pluginDir);
    m_instance->init();
}

SessionPluginManager::SessionPluginManager(const QString& pluginDir) : PluginManager(pluginDir)
{
    m_sessionSettings = new QGSettings(SESSION_DAEMON_SCHAME_ID, "", this);
    m_enabledPlugins = m_sessionSettings->get(SESSION_DAEMON_SCHEMA_KEY_ENABLED_PLUGINS).toStringList();
    auto disabledPlugins = m_sessionSettings->get(SESSION_DAEMON_SCHEMA_KEY_DISABLED_PLUGINS).toStringList();
    for (auto disabledPlugin : disabledPlugins)
    {
        m_enabledPlugins.removeAll(disabledPlugin);
    }
}

void SessionPluginManager::init()
{
    this->PluginManager::init();

    auto sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerService(SESSION_DAEMON_DBUS_NAME))
    {
        KLOG_WARNING() << "Failed to register dbus name: " << SESSION_DAEMON_DBUS_NAME;
        return;
    }

    if (!sessionConnection.registerObject(SESSION_DAEMON_OBJECT_PATH, SESSION_DAEMON_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR() << "Can't register object:" << sessionConnection.lastError();
        return;
    }
}

bool SessionPluginManager::pluginIsEnabled(const QString& pluginID)
{
    return m_enabledPlugins.contains(pluginID);
}

SystemPluginManager* SystemPluginManager::m_instance = nullptr;
void SystemPluginManager::globalInit(const QString& pluginDir)
{
    m_instance = new SystemPluginManager(pluginDir);
    m_instance->init();
}

SystemPluginManager::SystemPluginManager(const QString& pluginDir) : PluginManager(pluginDir)
{
    auto pluginConfigPath = QString("%1/%2").arg(pluginDir).arg(QString(SYSTEM_PLUGIN_CONFIG_NAME));
    m_systemSettings = new QSettings(pluginConfigPath, QSettings::IniFormat, this);

    for (const auto& groupName : m_systemSettings->childGroups())
    {
        m_systemSettings->beginGroup(groupName);
        auto pluginEnabled = m_systemSettings->value("Available").toBool();
        m_systemSettings->endGroup();

        if (pluginEnabled)
        {
            m_enabledPlugins.push_back(groupName);
        }
    }
}

void SystemPluginManager::init()
{
    this->PluginManager::init();

    auto systemConnection = QDBusConnection::systemBus();
    if (!systemConnection.registerService(SYSTEM_DAEMON_DBUS_NAME))
    {
        KLOG_WARNING() << "Failed to register dbus name: " << SYSTEM_DAEMON_DBUS_NAME;
        return;
    }

    if (!systemConnection.registerObject(SYSTEM_DAEMON_OBJECT_PATH, SYSTEM_DAEMON_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR() << "Can't register object:" << systemConnection.lastError();
        return;
    }
}

bool SystemPluginManager::pluginIsEnabled(const QString& pluginID)
{
    return m_enabledPlugins.contains(pluginID);
}

}  // namespace Kiran