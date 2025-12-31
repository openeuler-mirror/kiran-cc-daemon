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

private:
    explicit DnfWrapper(QObject *parent = nullptr);
    ~DnfWrapper();

    void initDnf();

    bool findAndCreateSack();
    /**
     * @brief 获取sack的引用，确保在使用期间不会被释放
     * @return sack的智能指针
     */
    QSharedPointer<::DnfSack> getSackRef();

    void updateCache();
    void cacheInvalidate();

    void startUpdateTimer();

    //将DnfStateAction枚举转换为字符串
    QString stateActionToString(int action);

Q_SIGNALS:
    void cacheUpdated(bool success);

    void installPercentageChanged(uint percentage);
    void installActionChanged(const QString &action, const QString &actionHint);

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
    int m_updateIntervalHours{24};
    QDateTime m_lastUpdateTime;

    // 用于延时刷新缓存
    QTimer *m_reloadCacheTimer{nullptr};
};
}  // namespace Kiran