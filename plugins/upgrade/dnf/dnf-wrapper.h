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

#include <upgrade-i.h>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QSettings>
#include <QSharedPointer>
#include <QString>
#include <QTimer>
#include <QtConcurrent>

// 前向声明 libdnf 类型
struct _DnfContext;
typedef struct _DnfContext DnfContext;
struct _DnfSack;
typedef struct _DnfSack DnfSack;
struct _DnfPackage;
typedef struct _DnfPackage DnfPackage;
namespace libdnf
{
struct Goal;
}
typedef struct libdnf::Goal *HyGoal;

namespace Kiran
{
class DnfWrapper : public QObject
{
    Q_OBJECT

public:
    static DnfWrapper *instance() { return m_instance; };
    static void globalInit();
    static void globalDeinit();

    // 设置缓存更新周期
    void setCacheUpdateIntvalHours(int cacheUpdateIntvalHours);

    /**
     * @brief 获取可更新的包列表信息
     * @return 包列表
     */
    QList<QSharedPointer<::DnfPackage>> getUpgradesPackages(QString &errorMessage);

    /**
     * @brief 安装包
     * @param packages 包列表
     * @return 是否成功
     */
    bool installPackages(const QList<QSharedPointer<::DnfPackage>> &packages, QString &errorMessage);

    /**
     * @brief 解决包依赖
     * @param packages 包列表
     * @return 需要安装的包NVR列表
     */
    QStringList solvePackageDeps(const QList<QSharedPointer<::DnfPackage>> &packages, QString &errorMessage);

    /**
     * @brief 根据包名获取系统当前已安装包的版本
     * @param packageName 包名
     * @param errorMessage 错误信息（如果失败）
     * @return 包的版本信息（EVR格式），如果包未安装则返回空字符串
     */
    QString getInstalledPackageVersion(const QString &packageName, QString &errorMessage);

private:
    explicit DnfWrapper(QObject *parent = nullptr);
    ~DnfWrapper();

    void initDnf();

    bool findAndCreateSack(bool forceUpdate = false);
    /**
     * @brief 获取sack的引用，确保在使用期间不会被释放
     * @param forceUpdate 是否强制更新缓存
     * @return sack的智能指针
     */
    QSharedPointer<::DnfSack> getSackRef(bool forceUpdate = false);

    void updateCache();
    void cacheInvalidate();

    void startUpdateTimer();

    //将DnfStateAction枚举转换为字符串
    QString stateActionToString(int action);

    /**
     * @brief 获取所有仓库数量
     */
    int getAllRepoCount();
    /**
     * @brief 获取sack实际加载的仓库列表
     * @return 仓库列表
     */
    int getLoadedRepoCount();
    /**
     * @brief 检查sack中特定仓库是否存在
     * @param sack sack对象
     * @param repoName 仓库名称（repo ID）
     * @return 如果仓库存在返回true，否则返回false
     */
    bool isRepoLoaded(QSharedPointer<::DnfSack> sack, const QString &repoName);

    /**
     * @brief 从goal中获取所有要安装或升级的软件包ID列表
     * @param goal 依赖解析后的goal对象（智能指针）
     * @param error 错误信息指针
     * @return 包ID列表（NVR格式），已去重
     */
    QStringList getAllPackagesFromGoal(const QSharedPointer<typename std::remove_pointer<HyGoal>::type> &goal);

Q_SIGNALS:
    /**
     * @brief 缓存失效信号
     */
    void invalidate();

    void installPercentageChanged(uint percentage);
    void installActionChanged(const QString &action, const QString &actionHint);

    /**
     * @brief 升级日志信号（传递完整的升级日志信息）
     * @param history 升级历史记录结构体
     */
    void upgradeHistotyReady(const UpgradeHistory &history);

private:
    static DnfWrapper *m_instance;

    ::DnfContext *m_dnfCtx{nullptr};
    QSharedPointer<::DnfSack> m_dnfSack;

    // 判断Sack是否有效，可能在多线程中被修改，需要加锁判断
    bool m_isSackVaild;
    QMutex m_sackMutex;

    // 监听/etc/yum.repos.d目录下的文件变化
    QFileSystemWatcher *m_fileWatcher{nullptr};

    // 监听缓存更新线程状态
    QFutureWatcher<bool> m_cacheFutureWatcher;

    // 用于定期更新缓存
    QTimer *m_updateTimer{nullptr};
    int m_updateIntervalHours;
    QDateTime m_lastUpdateTime;

    // 用于延时刷新缓存
    QTimer *m_reloadCacheTimer{nullptr};

    QStringList m_successPackages;
    QStringList m_failedPackages;
};
}  // namespace Kiran