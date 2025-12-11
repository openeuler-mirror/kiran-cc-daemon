/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#include "configuration.h"

#include <qt5-log-i.h>
#include <upgrade-i.h>
#include "lib/base/log.h"

namespace Kiran
{
Configuration::Configuration(const QString &configFile, QObject *parent)
    : QObject(parent),
      m_settings(nullptr),
      m_fileWatcher(nullptr),
      m_reloadTimer(nullptr),
      m_configFilePath(configFile)
{
    // 初始化 QSettings
    m_settings = new QSettings(m_configFilePath, QSettings::IniFormat, this);

    // 初始化文件监听器
    m_fileWatcher = new QFileSystemWatcher(this);

    // 初始化延迟重载定时器
    m_reloadTimer = new QTimer(this);
    m_reloadTimer->setSingleShot(true);
    m_reloadTimer->setInterval(100);  // 延迟100ms处理，避免频繁触发

    // 连接信号槽
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &Configuration::startReloadTimer);
    connect(m_reloadTimer, &QTimer::timeout, this, &Configuration::reloadConfig);
}

Configuration::~Configuration()
{
    m_settings->sync();
}

bool Configuration::load()
{
    // 同步配置
    m_settings->sync();

    // 保存当前所有配置值，用于后续检测变化
    QStringList keys = m_settings->allKeys();
    m_previousValues.clear();
    for (const QString &key : keys)
    {
        m_previousValues[key] = m_settings->value(key);
    }

    // 添加文件监听
    if (!m_fileWatcher->files().contains(m_configFilePath))
    {
        if (m_fileWatcher->addPath(m_configFilePath))
        {
            KLOG_DEBUG(upgrade) << "Start monitoring config file:" << m_configFilePath;
        }
        else
        {
            KLOG_WARNING(upgrade) << "Failed to add config file to watcher:" << m_configFilePath;
        }
    }

    KLOG_INFO(upgrade) << "Configuration loaded successfully, total keys num:" << keys.size() << "keys:" << keys;
    return true;
}

QVariant Configuration::get(const QString &key, const QVariant &defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

bool Configuration::set(const QString &key, const QVariant &value)
{
    // 保存旧值
    QVariant oldValue = m_settings->value(key);

    // 设置新值
    m_settings->setValue(key, value);

    // 同步到文件
    m_settings->sync();

    // 更新 previousValues
    m_previousValues[key] = value;

    return true;
}

void Configuration::sync()
{
    m_settings->sync();
}

QStringList Configuration::allKeys() const
{
    return m_settings->allKeys();
}

bool Configuration::contains(const QString &key) const
{
    return m_settings->contains(key);
}

void Configuration::startReloadTimer(const QString &path)
{
    KLOG_INFO(upgrade) << "Config file changed:" << path;

    // 使用定时器延迟处理，避免频繁触发
    if (!m_reloadTimer->isActive())
    {
        m_reloadTimer->start();
    }
}

void Configuration::reloadConfig()
{
    m_reloadTimer->stop();

    KLOG_INFO(upgrade) << "Reloading config file";

    // 同步配置文件
    m_settings->sync();

    // 检测变化
    detectChanges();

    // 重新添加监听，因为某些编辑器会删除原文件再创建新文件，导致监听被移除
    if (!m_fileWatcher->files().contains(m_configFilePath))
    {
        if (m_fileWatcher->addPath(m_configFilePath))
        {
            KLOG_DEBUG(upgrade) << "Re-added config file to watcher:" << m_configFilePath;
        }
        else
        {
            KLOG_WARNING(upgrade) << "Failed to re-add config file to watcher:" << m_configFilePath;
        }
    }
}

void Configuration::detectChanges()
{
    // 获取当前所有配置键
    QStringList currentKeys = m_settings->allKeys();

    // 检测新增或修改的配置
    for (const QString &key : currentKeys)
    {
        QVariant currentValue = m_settings->value(key);
        QVariant previousValue = m_previousValues.value(key);

        // 如果值发生变化，发送信号
        if (previousValue != currentValue)
        {
            KLOG_INFO(upgrade) << "Configuration value changed:" << key
                               << "from" << previousValue << "to" << currentValue;
            emit valueChanged(key, currentValue);
        }

        // 更新 previousValues
        m_previousValues[key] = currentValue;
    }

    // 检测删除的配置（如果之前存在但现在不存在）
    QStringList previousKeys = m_previousValues.keys();
    for (const QString &key : previousKeys)
    {
        if (!currentKeys.contains(key))
        {
            KLOG_INFO(upgrade) << "Configuration key removed:" << key;
            // 发送空值表示配置被删除
            emit valueChanged(key, QVariant());
            m_previousValues.remove(key);
        }
    }
}
}  // namespace Kiran
