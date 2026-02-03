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

#include <unistd.h>
#include <upgrade-i.h>
#include <QDBusContext>
#include <QDateTime>
#include <QFuture>
#include <QFutureWatcher>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSettings>
#include <QStringList>

class QDBusInterface;
class UpgradeAdaptor;

namespace Kiran
{
class DnfWrapper;
class Scanner;
class DepSolver;
class Installer;
class Configuration;
class UpgradeHistoryDB;
class Manager : public QObject, protected QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(int backend_status READ getBackendStatus WRITE setBackendStatus)
    Q_PROPERTY(QString latest_upgrade_time READ getLatestUpgradeTime WRITE setLatestUpgradeTime)
    Q_PROPERTY(int reminder_interval READ getReminderInterval WRITE setReminderInterval)
    Q_PROPERTY(QString latest_reminder_time READ getLatestReminderTime WRITE setLatestReminderTime)
    Q_PROPERTY(QString latest_scan_time READ getLatestScanTime WRITE setLatestScanTime)
public:
    static Manager *instance() { return m_instance; };
    static void globalInit();
    static void globalDeinit() { delete m_instance; };
    void init();

public:  // PROPERTIES
    // 获取后台状态
    int getBackendStatus();
    // 获取最新升级时间
    QString getLatestUpgradeTime() const;
    // 获取提醒间隔天数
    int getReminderInterval() const;
    // 获取最新提醒时间
    QString getLatestReminderTime() const;
    // 获取最新扫描时间
    QString getLatestScanTime() const;

    //设置后台状态
    void setBackendStatus(int status);
    //设置最新升级时间
    void setLatestUpgradeTime(const QString &latestUpgradeTime);
    //设置提醒间隔天数
    void setReminderInterval(int reminderInterval);
    //设置最新提醒时间至配置文件
    void setLatestReminderTime(const QString &latestReminderTime);
    //设置最新扫描时间至配置文件
    void setLatestScanTime(const QString &latestScanTime);

    /**
     * @brief 扫描可更新包
     */
    void Scan();

    /**
     * @brief 解决包依赖
     */
    void SolveDeps(const QStringList &packageIDs);

    /**
     * @brief 升级包
     */
    void Upgrade(const QStringList &packageIDs);

    /**
     * @brief 获取可更新包Json字符串
     * @return 可更新包Json字符串
     */
    QString GetUpgradePkgsInfo();

    /**
     * @brief 获取升级日志
     * @return 升级日志信息
     * @note 日志包括：升级阶段、正在处理的软件包ID
     */
    QString GetUpgradeLog();

    /**
     * @brief 获取升级历史记录
     * @return JSON格式的升级历史记录数组
     * @note 历史记录包括：升级时间、结果（成功/失败）、错误信息、成功升级的软件包、失败的软件包
     */
    QString GetUpgradeHistory();
    /**
     * @brief 设置提醒间隔天数
     * @param days 提醒间隔天数,从不、一周，一月，三月
     */
    void SetReminderInterval(int reminderInterval);

signals:
    void ScanCompleted(bool success, const QString &errorMessage);
    void SolveDepsCompleted(bool success, const QString &pkgDepsJson, const QString &errorMessage);
    void UpgradeCompleted(bool success, const QString &errorMessage);

    // 升级进度信号
    void UpgradePercentageChanged(uint percentage);
    void UpgradeActionChanged(const QString &action, const QString &actionHint);

    // 升级历史记录信号
    void UpgradeHistoryAdded(const UpgradeHistory &history);

private:
    Manager(QObject *parent = nullptr);
    ~Manager();

    void upgradeAuthenticated(const QDBusMessage &message, const QStringList &packageIDs);
    void setReminderIntAuthenticated(const QDBusMessage &message, int reminderInterval);

    //加载配置
    void loadConfig();

    bool isValidInterval(int reminderInterval);

    template <typename Signal>
    void handleTaskResult(bool success, const QString &errorMessage, Signal signal)
    {
        {
            QMutexLocker locker(&m_statusMutex);
            m_status = BACKEND_STATUS_IDLE;
        }
        emit(this->*signal)(success, errorMessage);
    }

private:
    static Manager *m_instance;
    UpgradeAdaptor *m_adaptor;
    DnfWrapper *m_dnfWrapper;
    Scanner *m_scanner;
    DepSolver *m_depSolver;
    Installer *m_installer;
    UpgradeHistoryDB *m_historyDB;

    // 最新升级时间
    QString m_lastUpgradeTime;
    // 最新提醒时间
    QString m_latestReminderTime;
    // 最新扫描时间
    QString m_latestScanTime;

    // 状态管理
    QMutex m_statusMutex;
    BackendStatus m_status;

    // 提醒间隔天数
    int m_reminderInterval;

    // 配置文件
    Configuration *m_config;

    // 缓存更新周期
    int m_cacheIntvalHours;
};
}  // namespace Kiran