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

#pragma once

#include <kscreen/types.h>
#include <QDBusContext>
#include <QMap>
#include <QSharedPointer>
#include "display-i.h"
#include "display.hxx"
#include "error-i.h"

class DisplayAdaptor;
class QGSettings;
class QPluginLoader;

namespace KScreen
{
class Config;
class ConfigMonitor;
}  // namespace KScreen

namespace Kiran
{
class DisplayMonitor;

class DisplayManager : public QObject,
                       protected QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(uint default_style READ getDefaultStyle WRITE setDefaultStyle)
    Q_PROPERTY(QString primary READ getPrimary WRITE setPrimary)
    Q_PROPERTY(int window_scaling_factor READ getWindowScalingFactor WRITE setWindowScalingFactor)

public:
    DisplayManager();
    virtual ~DisplayManager();

    static DisplayManager* getInstance() { return m_instance; };

    static void globalInit();

    static void globalDeinit() { delete m_instance; };

    // 目前所有monitor都是已连接的,未连接的暂时未创建DisplayMonitor
    QList<QSharedPointer<DisplayMonitor>> getConnectedMonitors();
    // 获取开启的显示器
    QList<QSharedPointer<DisplayMonitor>> getEnabledMonitors();

public:
    uint getDefaultStyle() const;
    QString getPrimary() const;
    int getWindowScalingFactor() const;

    void setDefaultStyle(uint style);
    void setPrimary(const QString& name);
    void setWindowScalingFactor(int windowScalingFactor);

public Q_SLOTS:
    // 应用之前通过dbus调用做的修改
    void ApplyChanges();
    // 获取所有monitor的object path
    QStringList ListMonitors();
    // 恢复之前通过dbus调用做的修改
    void RestoreChanges();
    // 将之前的修改保存到文件，保存之后无法再恢复到之前的修改状态
    void Save();
    // 设置默认显示模式，默认显示模式会在程序第一次启动或者有连接的显示设备删除和添加时进行启用。
    void SetDefaultStyle(uint style);
    // 设置主显示器
    void SetPrimary(const QString& name);
    // 设置窗口缩放因子
    void SetWindowScalingFactor(int windowScalingFactor);
    // 切换显示模式
    void SwitchStyle(uint style);
Q_SIGNALS:  // SIGNALS
    void MonitorsChanged(bool placeholder);

private:
    // 初始化前准备
    void preInit();
    void init();
    // 加载settings内容
    void loadSettings();
    // 每个已连接的output对应一个monitor对象
    void loadMonitors();
    // 加载配置文件
    void loadConfig();

    // 应用配置到m_monitors中，这里先要根据monitorIDs进行匹配找到对应的ScreenConfigInfo，然后再调用applyScreenConfig。
    bool applyConfig(CCErrorCode& errorCode);
    // 应用配置到monitors_中
    bool applyScreenConfig(const ScreenConfigInfo& screenConfig, CCErrorCode& errorCode);
    // 从monitors_提取参数填充配置
    void fillScreenConfig(ScreenConfigInfo& screenConfig);
    // 保存配置
    bool saveConfig(CCErrorCode& errorCode);
    // 让m_monitors中的参数实际生效，通过libkscreen接口实现
    bool apply(CCErrorCode& errorCode);

    // 切换显示模式
    bool switchStyle(DisplayStyle style, CCErrorCode& errorCode);
    // 切换并保存显示模式
    bool switchStyleAndSave(DisplayStyle style, CCErrorCode& errorCode);
    // 切换到镜像模式
    bool switchToMirrors(CCErrorCode& errorCode);
    // 切换到扩展模式
    void switchToExtend();
    // 切换到自定义模式
    bool switchToCustom(CCErrorCode& errorCode);
    // 切换到自动模式
    void switchToAuto();

    // 获取monitor
    // QSharedPointer<DisplayMonitor> get_monitor(uint32_t id);
    // QSharedPointer<DisplayMonitor> get_monitor_by_uid(const QString& uid);
    QSharedPointer<DisplayMonitor> getMonitorByName(const QString& name);
    // 优先匹配uid，如果有多个uid匹配，则再匹配name
    QSharedPointer<DisplayMonitor> matchBestMonitor(const QString& uid, const QString& name);

    // 将uid进行排序后拼接
    QString getMonitorsUID();
    // 将monitor name进行排序后拼接
    QString getMonitorNames();
    QString getCMonitorsUID(const ScreenConfigInfo::MonitorSequence& monitors);

    // 保存配置到文件
    bool saveToFile(CCErrorCode& errorCode);
    void dumpDisplayConfig();

    void processConfigureChanged();
    void processSettingsChanged(const QString& key);

    QString styleEnum2Str(int style);
    int styleStr2Enum(QString style);

private:
    static DisplayManager* m_instance;
    DisplayAdaptor* m_displayAdaptor;
    // 显示配置文件路径
    QString m_configFilePath;
    // 显示配置内容
    std::unique_ptr<DisplayConfigInfo> m_displayConfig;

    KScreen::ConfigPtr m_currentConfig;
    KScreen::ConfigMonitor* m_configMonitor;

    QGSettings* m_displaySettings;
    QGSettings* m_xsettingsSettings;
    DisplayStyle m_defaultStyle;
    // 主显示器名字
    QString m_primary;
    // 窗口缩放率
    int32_t m_windowScalingFactor;
    // 开启动态缩放窗口
    bool m_dynamicScalingWindow;
    // 可存储屏幕个数最大值
    uint32_t m_maxScreenRecordNumber;

    QMap<uint32_t, QSharedPointer<DisplayMonitor>> m_monitors;
};
}  // namespace Kiran
