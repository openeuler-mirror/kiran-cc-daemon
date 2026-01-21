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

#include <glib-object.h>
#include <glib.h>
#include <libdnf/libdnf.h>

#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>
#include <QtConcurrent>

#include "configuration.h"
#include "dep-solver.h"
#include "dnf/dnf-wrapper.h"
#include "installer.h"
#include "lib/base/base.h"
#include "lib/base/polkit-proxy.h"
#include "manager.h"
#include "scanner.h"
#include "upgradeadaptor.h"

namespace Kiran
{
#define AUTH_UPGRADE_UPGRADE "com.kylinsec.kiran.system-daemon.upgrade.upgrade"
#define AUTH_UPGRADE_SET_REMINDER_INTERVAL "com.kylinsec.kiran.system-daemon.upgrade.set-reminder-interval"

// 配置文件路径
#define CONFIG_FILE "/etc/kiran-cc-daemon/system/upgrade/upgrade.conf"

// 缓存配置
#define CACHE_SECTION "cache"
#define CACHE_CONFIG_UPDATE_INTERVAL_HOURS_KEY CACHE_SECTION "/update-interval-hours"
// 提醒配置
#define REMINDER_SECTION "reminder"
#define REMINDER_CONFIG_INTERVAL_KEY REMINDER_SECTION "/interval"
#define REMINDER_CONFIG_LAST_REMINDER_TIME_KEY REMINDER_SECTION "/last-reminder-time"
// 更新配置
#define UPDATE_SECTION "update"
#define UPDATE_CONFIG_LAST_UPDATE_TIME_KEY UPDATE_SECTION "/last-update-time"
// 扫描配置
#define SCAN_SECTION "scan"
#define SCAN_CONFIG_LAST_SCAN_TIME_KEY SCAN_SECTION "/last-scan-time"

#define SEND_PROPERTY_NOTIFY(property, propertyHump)                          \
    QVariantMap changedProperties;                                            \
    changedProperties.insert(QStringLiteral(#property), get##propertyHump()); \
                                                                              \
    QDBusMessage signalMessage = QDBusMessage::createSignal(                  \
        UPGRADE_OBJECT_PATH,                                                  \
        QStringLiteral("org.freedesktop.DBus.Properties"),                    \
        QStringLiteral("PropertiesChanged"));                                 \
                                                                              \
    signalMessage.setArguments({                                              \
        UPGRADE_DBUS_INTERFACE_NAME,                                          \
        changedProperties,                                                    \
        QStringList(),                                                        \
    });                                                                       \
    QDBusConnection::systemBus().send(signalMessage);

Manager::Manager(QObject *parent) : QObject(parent),
                                    m_adaptor(nullptr),
                                    m_scanner(nullptr),
                                    m_depSolver(nullptr),
                                    m_installer(nullptr),
                                    m_status(BACKEND_STATUS_IDLE),
                                    m_reminderInterval(0),
                                    m_config(nullptr),
                                    m_cacheIntvalHours(DEFAULT_CACHE_UPDATE_INTERVAL_HOURS)
{
    // 加载DNF模块
    DnfWrapper::globalInit();
    m_dnfWrapper = DnfWrapper::instance();

    //加载配置
    loadConfig();

    //加载扫描模块
    m_scanner = new Scanner(this);
    connect(m_scanner, &Scanner::scanCompleted, this, [this](bool success, const QString &errorMessage)
            {
                KLOG_INFO(upgrade) << "Scan completed, success: " << success << ", error message: " << errorMessage;

                handleTaskResult(success, errorMessage, &Manager::ScanCompleted);

                //无论扫描是否成功，都更新最新扫描时间
                setLatestScanTime(QDateTime::currentDateTime().toString(DEFAULT_DATE_TIME_FORMAT));
            });

    //加载依赖解析模块
    m_depSolver = new DepSolver(this);
    connect(m_depSolver, &DepSolver::solveDepsCompleted, this, [this](bool success, const QString &pkgDepsJson, const QString &errorMessage)
            {
                KLOG_INFO(upgrade) << "Solve deps completed, success: " << success
                                   << ", pkg deps json: " << pkgDepsJson
                                   << ", error message: " << errorMessage;

                setBackendStatus(BACKEND_STATUS_IDLE);
                emit SolveDepsCompleted(success, pkgDepsJson, errorMessage);
            });

    //加载安装模块
    m_installer = new Installer(this);
    connect(m_installer, &Installer::installCompleted, this, [this](bool success, const QString &errorMessage)
            {
                KLOG_INFO(upgrade) << "Upgrade completed, success: " << success << ", error message: " << errorMessage;

                handleTaskResult(success, errorMessage, &Manager::UpgradeCompleted);

                if (success)
                {
                    //更新最新升级时间
                    setLatestUpgradeTime(QDateTime::currentDateTime().toString(DEFAULT_DATE_TIME_FORMAT));
                }
            });
    connect(m_installer, &Installer::installProgressChanged, this, &Manager::UpgradePercentageChanged);
    connect(m_installer, &Installer::installActionChanged, this, &Manager::UpgradeActionChanged);
}

Manager::~Manager()
{
    if (m_config)
    {
        delete m_config;
        m_config = nullptr;
    }
    if (m_dnfWrapper)
    {
        DnfWrapper::globalDeinit();
        m_dnfWrapper = nullptr;
    }
}

Manager *Manager::m_instance = nullptr;
void Manager::globalInit()
{
    if (!m_instance)
    {
        m_instance = new Manager();
        m_instance->init();
    }
}

void Manager::init()
{
    auto systemConnection = QDBusConnection::systemBus();
    if (!systemConnection.registerService(UPGRADE_DBUS_NAME))
    {
        KLOG_WARNING(upgrade) << "Failed to register dbus name: " << UPGRADE_DBUS_NAME;
        return;
    }

    if (!systemConnection.registerObject(UPGRADE_OBJECT_PATH, this))
    {
        KLOG_ERROR(upgrade) << "Can't register object:" << systemConnection.lastError();
        return;
    }
    m_adaptor = new UpgradeAdaptor(this);
}

void Manager::Scan()
{
    if (getBackendStatus() == BACKEND_STATUS_SCANNING)
    {
        KLOG_WARNING(upgrade) << "Scanner is already in progress";
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_UPGRADE_SCAN_NOT_COMPLETED);
    }

    auto errorCode = m_scanner->scan();
    if (errorCode != CCErrorCode::SUCCESS)
    {
        KLOG_WARNING(upgrade) << "Scan failed: " << errorCode;
        DBUS_ERROR_REPLY_AND_RET(errorCode);
    }

    KLOG_INFO(upgrade) << "Scan system upgrade packages";
    setBackendStatus(BACKEND_STATUS_SCANNING);
    QDBusConnection::systemBus().send(this->message().createReply());
}

void Manager::SolveDeps(const QStringList &packageIDs)
{
    if (getBackendStatus() == BACKEND_STATUS_SOLVING_DEPS)
    {
        KLOG_WARNING(upgrade) << "DepSolver is already in progress";
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_UPGRADE_SOLVE_DEPS_NOT_COMPLETED);
    }

    if (packageIDs.isEmpty())
    {
        KLOG_WARNING(upgrade) << "Package IDs list is empty";
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_UPGRADE_PACKAGE_IDS_EMPTY);
    }

    auto upgradePkgs = m_scanner->getUpgradePkgs();
    if (upgradePkgs.isEmpty())
    {
        KLOG_WARNING(upgrade) << "Upgrade packages is empty";
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_UPGRADE_UPGRADE_PKGS_EMPTY);
    }

    m_depSolver->setUpgradePkgs(upgradePkgs);
    auto errorCode = m_depSolver->solveDeps(packageIDs);
    if (errorCode != CCErrorCode::SUCCESS)
    {
        KLOG_WARNING(upgrade) << "Solve deps failed: " << errorCode;
        DBUS_ERROR_REPLY_AND_RET(errorCode);
    }

    KLOG_INFO(upgrade) << "Solve deps for package IDs: " << packageIDs;
    setBackendStatus(BACKEND_STATUS_SOLVING_DEPS);
    QDBusConnection::systemBus().send(this->message().createReply());
}

CHECK_AUTH_WITH_1ARGS(Manager, Upgrade, upgradeAuthenticated, AUTH_UPGRADE_UPGRADE, const QStringList &);
void Manager::upgradeAuthenticated(const QDBusMessage &message, const QStringList &packageIDs)
{
    if (getBackendStatus() == BACKEND_STATUS_UPGRADING)
    {
        KLOG_WARNING(upgrade) << "Upgrade is already in progress";
        DBUS_ERROR_DELAY_REPLY_AND_RET(CCErrorCode::ERROR_UPGRADE_INSTALL_NOT_COMPLETED);
    }

    if (packageIDs.isEmpty())
    {
        KLOG_WARNING(upgrade) << "Package IDs list is empty";
        DBUS_ERROR_DELAY_REPLY_AND_RET(CCErrorCode::ERROR_UPGRADE_PACKAGE_IDS_EMPTY);
    }

    auto upgradePkgs = m_scanner->getUpgradePkgs();
    if (upgradePkgs.isEmpty())
    {
        KLOG_WARNING(upgrade) << "Upgrade packages is empty";
        DBUS_ERROR_DELAY_REPLY_AND_RET(CCErrorCode::ERROR_UPGRADE_UPGRADE_PKGS_EMPTY);
    }

    m_installer->setUpgradePkgs(upgradePkgs);
    auto errorCode = m_installer->install(packageIDs);
    if (errorCode != CCErrorCode::SUCCESS)
    {
        KLOG_WARNING(upgrade) << "Upgrade failed: " << errorCode;
        DBUS_ERROR_DELAY_REPLY_AND_RET(errorCode);
    }

    KLOG_INFO(upgrade) << "Upgrade package IDs: " << packageIDs;
    setBackendStatus(BACKEND_STATUS_UPGRADING);
    QDBusConnection::systemBus().send(message.createReply());
}

CHECK_AUTH_WITH_1ARGS(Manager, SetReminderInterval, setReminderIntAuthenticated, AUTH_UPGRADE_SET_REMINDER_INTERVAL, int);
void Manager::setReminderIntAuthenticated(const QDBusMessage &message, int reminderInterval)
{
    if (reminderInterval == m_reminderInterval)
    {
        KLOG_WARNING(upgrade) << "Reminder interval is already set to " << reminderInterval;
        QDBusConnection::systemBus().send(message.createReply());
        return;
    }

    if (!isValidInterval(reminderInterval))
    {
        KLOG_WARNING(upgrade) << "Invalid reminder interval value: " << reminderInterval
                              << ", resetting to default: " << DEFAULT_REMINDER_INTERVAL;
        DBUS_ERROR_DELAY_REPLY_AND_RET(CCErrorCode::ERROR_UPGRADE_REMINDER_INTERVAL_INVALID);
    }

    setReminderInterval(reminderInterval);
    QDBusConnection::systemBus().send(message.createReply());
}

void Manager::setReminderInterval(int reminderInterval)
{
    KLOG_INFO(upgrade) << "Set reminder interval to " << reminderInterval;
    m_reminderInterval = reminderInterval;
    //写入配置文件
    if (reminderInterval != m_config->get(REMINDER_CONFIG_INTERVAL_KEY).toInt())
    {
        m_config->set(REMINDER_CONFIG_INTERVAL_KEY, reminderInterval);
    }
    SEND_PROPERTY_NOTIFY(reminder_interval, ReminderInterval)
}

int Manager::getReminderInterval() const
{
    return m_reminderInterval;
}

QString Manager::GetUpgradeLog()
{
    return m_installer->getInstallLog();
}
QString Manager::GetUpgradePkgsInfo()
{
    return m_scanner->getUpgradePkgsJson();
}

int Manager::getBackendStatus()
{
    QMutexLocker locker(&m_statusMutex);
    return static_cast<int>(m_status);
}

void Manager::setBackendStatus(int status)
{
    KLOG_INFO(upgrade) << "Set backend status to " << status;
    QMutexLocker locker(&m_statusMutex);
    m_status = static_cast<BackendStatus>(status);
}

QString Manager::getLatestUpgradeTime() const
{
    return m_lastUpgradeTime;
}
void Manager::setLatestUpgradeTime(const QString &latestUpgradeTime)
{
    KLOG_INFO(upgrade) << "Set latest upgrade time to " << latestUpgradeTime;
    RETURN_IF_TRUE(m_lastUpgradeTime == latestUpgradeTime);

    m_lastUpgradeTime = latestUpgradeTime;
    if (latestUpgradeTime != m_config->get(UPDATE_CONFIG_LAST_UPDATE_TIME_KEY).toString())
    {
        m_config->set(UPDATE_CONFIG_LAST_UPDATE_TIME_KEY, latestUpgradeTime);
    }

    SEND_PROPERTY_NOTIFY(latest_upgrade_time, LatestUpgradeTime)
}

QString Manager::getLatestReminderTime() const
{
    return m_latestReminderTime;
}
void Manager::setLatestReminderTime(const QString &latestReminderTime)
{
    KLOG_INFO(upgrade) << "Set latest reminder time to " << latestReminderTime;
    RETURN_IF_TRUE(m_latestReminderTime == latestReminderTime);

    m_latestReminderTime = latestReminderTime;
    if (latestReminderTime != m_config->get(REMINDER_CONFIG_LAST_REMINDER_TIME_KEY).toString())
    {
        m_config->set(REMINDER_CONFIG_LAST_REMINDER_TIME_KEY, latestReminderTime);
    }

    SEND_PROPERTY_NOTIFY(latest_reminder_time, LatestReminderTime)
}

QString Manager::getLatestScanTime() const
{
    return m_latestScanTime;
}
void Manager::setLatestScanTime(const QString &latestScanTime)
{
    KLOG_INFO(upgrade) << "Set latest scan time to " << latestScanTime;
    RETURN_IF_TRUE(m_latestScanTime == latestScanTime);

    m_latestScanTime = latestScanTime;
    if (latestScanTime != m_config->get(SCAN_CONFIG_LAST_SCAN_TIME_KEY).toString())
    {
        m_config->set(SCAN_CONFIG_LAST_SCAN_TIME_KEY, latestScanTime);
    }

    SEND_PROPERTY_NOTIFY(latest_scan_time, LatestScanTime)
}
void Manager::loadConfig()
{
    m_config = new Configuration(CONFIG_FILE, this);
    m_config->load();

    //获取提醒间隔天数
    auto interval = m_config->get(REMINDER_CONFIG_INTERVAL_KEY, DEFAULT_REMINDER_INTERVAL).toInt();
    if (!isValidInterval(interval))
    {
        KLOG_WARNING(upgrade) << "Invalid reminder interval value: " << interval
                              << ", resetting to default: " << DEFAULT_REMINDER_INTERVAL;
        m_reminderInterval = DEFAULT_REMINDER_INTERVAL;
    }
    else
    {
        m_reminderInterval = interval;
    }
    KLOG_DEBUG(upgrade) << "Reminder interval loaded from config: "
                        << m_reminderInterval << " days";

    //获取最新提醒时间
    m_latestReminderTime = m_config->get(REMINDER_CONFIG_LAST_REMINDER_TIME_KEY, "").toString();
    KLOG_DEBUG(upgrade) << "Latest reminder time loaded from config: "
                        << m_latestReminderTime;

    //获取最新升级时间
    m_lastUpgradeTime = m_config->get(UPDATE_CONFIG_LAST_UPDATE_TIME_KEY, "").toString();
    KLOG_DEBUG(upgrade) << "Latest upgrade time loaded from config: "
                        << m_lastUpgradeTime;

    //获取缓存更新周期
    m_cacheIntvalHours = m_config->get(CACHE_CONFIG_UPDATE_INTERVAL_HOURS_KEY, DEFAULT_CACHE_UPDATE_INTERVAL_HOURS).toInt();
    m_dnfWrapper->setCacheUpdateIntvalHours(m_cacheIntvalHours);
    KLOG_DEBUG(upgrade) << "Cache update interval loaded from config: "
                        << m_cacheIntvalHours << " hours";

    //获取最新扫描时间
    m_latestScanTime = m_config->get(SCAN_CONFIG_LAST_SCAN_TIME_KEY, "").toString();
    KLOG_DEBUG(upgrade) << "Latest scan time loaded from config: "
                        << m_latestScanTime;

    //监听配置变化
    connect(m_config, &Configuration::valueChanged, this, [this](const QString &key, const QVariant &value)
            {
                KLOG_DEBUG(upgrade) << key << "config value changed to " << value;
                if (key == REMINDER_CONFIG_INTERVAL_KEY &&
                    m_reminderInterval != value.toInt())
                {
                    setReminderInterval(value.toInt());
                }
                else if (key == UPDATE_CONFIG_LAST_UPDATE_TIME_KEY &&
                         m_lastUpgradeTime != value.toString())
                {
                    setLatestUpgradeTime(value.toString());
                }
                else if (key == CACHE_CONFIG_UPDATE_INTERVAL_HOURS_KEY &&
                         m_cacheIntvalHours != value.toInt())
                {
                    m_cacheIntvalHours = value.toInt();
                    m_dnfWrapper->setCacheUpdateIntvalHours(value.toInt());
                }
                else if (key == REMINDER_CONFIG_LAST_REMINDER_TIME_KEY &&
                         m_latestReminderTime != value.toString())
                {
                    setLatestReminderTime(value.toString());
                }
                else if (key == SCAN_CONFIG_LAST_SCAN_TIME_KEY &&
                         m_latestScanTime != value.toString())
                {
                    setLatestScanTime(value.toString());
                }
            });
}

bool Manager::isValidInterval(int reminderInterval)
{
    // 检查 reminderInterval 是否是 ReminderInterval 枚举中定义的有效值
    // 有效值：0 (NEVER), 7 (WEEKLY), 30 (MONTHLY), 90 (QUARTERLY)
    return (reminderInterval == REMINDER_INTERVAL_NEVER ||
            reminderInterval == REMINDER_INTERVAL_WEEKLY ||
            reminderInterval == REMINDER_INTERVAL_MONTHLY ||
            reminderInterval == REMINDER_INTERVAL_QUARTERLY);
}
}  // namespace Kiran
