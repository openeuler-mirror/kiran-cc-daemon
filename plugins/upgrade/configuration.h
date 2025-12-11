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

#pragma once

#include <QFileSystemWatcher>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QTimer>
#include <QVariant>

namespace Kiran
{
/**
 * @brief 配置文件管理类
 * @details 提供配置文件的加载、读取、设置和监听功能
 */
class Configuration : public QObject
{
    Q_OBJECT

public:
    Configuration(const QString &configFile, QObject *parent = nullptr);
    ~Configuration();

    /**
     * @brief 加载配置文件
     * @return 是否成功加载
     */
    bool load();

    /**
     * @brief 获取配置值
     * @param key 配置键，格式为 "section/key" 或 "section/key" (使用完整路径)
     * @param defaultValue 默认值
     * @return 配置值
     */
    QVariant get(const QString &key, const QVariant &defaultValue = QVariant()) const;

    /**
     * @brief 设置配置值
     * @param key 配置键，格式为 "section/key"
     * @param value 配置值
     * @return 是否成功设置
     */
    bool set(const QString &key, const QVariant &value);

    /**
     * @brief 同步配置到文件
     */
    void sync();

    /**
     * @brief 获取所有配置键
     * @return 所有配置键列表
     */
    QStringList allKeys() const;

    /**
     * @brief 检查配置键是否存在
     * @param key 配置键
     * @return 是否存在
     */
    bool contains(const QString &key) const;

Q_SIGNALS:
    /**
     * @brief 配置值变化信号
     * @param key 变化的配置键
     * @param value 新的配置值
     */
    void valueChanged(const QString &key, const QVariant &value);

private Q_SLOTS:
    /**
     * @brief 启动延迟重载定时器
     * @param path 文件路径
     */
    void startReloadTimer(const QString &path);

    /**
     * @brief 重新加载配置并检测变化
     */
    void reloadConfig();

private:
    /**
     * @brief 检测配置变化并发送信号
     */
    void detectChanges();

private:
    QSettings *m_settings;                     // 配置对象
    QFileSystemWatcher *m_fileWatcher;         // 文件监听器
    QTimer *m_reloadTimer;                     // 延迟重载定时器
    QMap<QString, QVariant> m_previousValues;  // 上一次的配置值，用于检测变化
    QString m_configFilePath;                  // 配置文件路径
};

}  // namespace Kiran
