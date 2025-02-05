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

#pragma once

#include <QDBusContext>
#include <QObject>
#include <QSharedPointer>

class PluginManagerAdaptor;
class QGSettings;
class QSettings;

namespace Kiran
{
class PluginInfo;

class PluginManager : public QObject,
                      protected QDBusContext
{
    Q_OBJECT

public:
    PluginManager(const QString& pluginDir);
    virtual ~PluginManager();

    // 取消激活所有插件
    void deactivatePlugins();

public Q_SLOTS:  // DBUS接口
    void ActivatePlugin(const QString& id);
    void DeactivatePlugin(const QString& id);
    QStringList GetActivatedPlugins();
    QStringList GetPlugins();

protected:
    virtual void init();
    virtual bool pluginIsEnabled(const QString& pluginID) { return true; };

private:
    void loadPlugins();
    // 激活插件
    bool activatePlugin(QSharedPointer<PluginInfo> pluginInfo);
    bool deactivatePlugin(QSharedPointer<PluginInfo> pluginInfo);

private:
    // dbus适配器
    PluginManagerAdaptor* m_adaptor;
    // 插件搜索目录
    QString m_pluginDir;
    // <插件ID，插件信息>
    QMap<QString, QSharedPointer<PluginInfo>> m_plugins;
};

class SessionPluginManager : public PluginManager
{
    Q_OBJECT

public:
    static SessionPluginManager* getInstance() { return m_instance; }
    static void globalInit(const QString& pluginDir);
    static void globalDeinit() { delete m_instance; };

private:
    SessionPluginManager(const QString& pluginDir);
    virtual void init();
    virtual bool pluginIsEnabled(const QString& pluginID);

private:
    static SessionPluginManager* m_instance;
    QGSettings* m_sessionSettings;
    QStringList m_enabledPlugins;
};

class SystemPluginManager : public PluginManager
{
    Q_OBJECT

public:
    static void globalInit(const QString& pluginDir);
    static void globalDeinit() { delete m_instance; };

private:
    SystemPluginManager(const QString& pluginDir);
    virtual void init();
    virtual bool pluginIsEnabled(const QString& pluginID);

private:
    static SystemPluginManager* m_instance;
    QSettings* m_systemSettings;
    QStringList m_enabledPlugins;
};

}  // namespace Kiran
