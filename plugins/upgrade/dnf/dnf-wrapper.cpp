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
#include <solv/pool.h>
#include <solv/repo.h>

#include "dnf-wrapper.h"
#include "lib/base/base.h"

#include <qt5-log-i.h>
#include <QElapsedTimer>

typedef ::DnfPackage DnfPackage;
typedef ::DnfSack DnfSack;
typedef ::DnfContext DnfContext;

typedef void (*percentageChangedCBType)(DnfState *,
                                        uint,
                                        gpointer);
typedef void (*actionChangedTypeCBType)(DnfState *,
                                        DnfStateAction,
                                        const char *,
                                        gpointer);

// DNF 目录定义
#define DNF_CACHE_DIR "/var/cache/dnf"
#define DNF_SOLV_DIR "/var/cache/dnf"
#define DNF_REPO_DIR "/etc/yum.repos.d"
#define DNF_LOG_PATH "/var/log/kylinsec/kiran-control-panel/dnf-log/"

namespace Kiran
{
DnfWrapper::DnfWrapper(QObject *parent)
    : QObject(parent),
      m_dnfCtx(nullptr),
      m_isSackVaild(false),
      m_updateIntervalHours(0),
      m_lastUpdateTime()
{
    // 初始化dnf
    initDnf();

    //初始化延时刷新定时器
    m_reloadCacheTimer = new QTimer(this);
    m_reloadCacheTimer->setInterval(500);
    m_reloadCacheTimer->setSingleShot(true);
    connect(m_reloadCacheTimer, &QTimer::timeout, this, [this]()
            {
                KLOG_DEBUG(upgrade) << "Reload cache triggered......";
                updateCache();
            });

    connect(&m_cacheFutureWatcher, &QFutureWatcher<bool>::finished, this, [this]()
            {
                m_lastUpdateTime = QDateTime::currentDateTime();

                bool success = m_cacheFutureWatcher.result();
                if (success)
                {
                    KLOG_DEBUG(upgrade) << "Cache update completed successfully";
                }
                else
                {
                    KLOG_WARNING(upgrade) << "Cache update failed";
                }

                emit cacheUpdated(success);
                // 重新启动更新定时器
                startUpdateTimer();
            });

    //初始化缓存更新定时器
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, [this]()
            {
                KLOG_DEBUG(upgrade) << "Periodic cache update triggered";
                if (!m_cacheFutureWatcher.isRunning())
                {
                    cacheInvalidate();
                }
            });
}

DnfWrapper::~DnfWrapper()
{
    if (m_dnfCtx)
    {
        g_object_unref(m_dnfCtx);
        m_dnfCtx = nullptr;
    }

    if (m_updateTimer)
    {
        m_updateTimer->stop();
    }
}

DnfWrapper *DnfWrapper::m_instance = nullptr;
void DnfWrapper::globalInit()
{
    if (!m_instance)
    {
        m_instance = new DnfWrapper();
    }
}

void DnfWrapper::globalDeinit()
{
    if (m_instance)
    {
        delete m_instance;
        m_instance = nullptr;
    }
}
void DnfWrapper::initDnf()
{
    m_dnfCtx = dnf_context_new();
    dnf_context_set_cache_dir(m_dnfCtx, DNF_CACHE_DIR);
    dnf_context_set_solv_dir(m_dnfCtx, DNF_SOLV_DIR);
    dnf_context_set_repo_dir(m_dnfCtx, DNF_REPO_DIR);
    dnf_context_set_rpm_verbosity(m_dnfCtx, "info");
    g_autoptr(GError) error = nullptr;
    if (!dnf_context_setup(m_dnfCtx, nullptr, &error))
    {
        KLOG_ERROR(upgrade) << "Failed to init dnf context! error message: "
                            << error->message;
        g_object_unref(m_dnfCtx);
        m_dnfCtx = nullptr;
        return;
    }

    // 监听libdnf缓存失效信号
    typedef void (*dnfCacheInvalidateCBType)(::DnfContext *, const gchar *, gpointer);
    auto callback = [](::DnfContext *ctx, const gchar *message, gpointer user_data)
    {
        Q_UNUSED(ctx);
        auto *self = static_cast<DnfWrapper *>(user_data);
        KLOG_DEBUG(upgrade) << "cache invalidate because:" << message;
        if (self)
        {
            self->cacheInvalidate();
        }
    };
    g_signal_connect(m_dnfCtx, "invalidate",
                     G_CALLBACK(static_cast<dnfCacheInvalidateCBType>(callback)),
                     this);

    // 监控仓库配置目录
    m_fileWatcher = new QFileSystemWatcher(this);
    m_fileWatcher->addPath(DNF_REPO_DIR);
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged, this,
            [this](const QString &path)
            {
                KLOG_DEBUG(upgrade) << "Repository directory changed: " << path;
                cacheInvalidate();
            });
}

void DnfWrapper::setCacheUpdateIntvalHours(int cacheUpdateIntvalHours)
{
    KLOG_DEBUG(upgrade) << "set cache update intval hours from" << m_updateIntervalHours
                        << "to" << cacheUpdateIntvalHours;
    RETURN_IF_TRUE(m_updateIntervalHours == cacheUpdateIntvalHours);

    if (cacheUpdateIntvalHours < 1)
    {
        KLOG_WARNING(upgrade) << "Invalid update interval in config file, using default: "
                              << DEFAULT_CACHE_UPDATE_INTERVAL_HOURS << " hours";
        cacheUpdateIntvalHours = DEFAULT_CACHE_UPDATE_INTERVAL_HOURS;
    }
    m_updateIntervalHours = cacheUpdateIntvalHours;
    // 启动缓存更新定时器
    startUpdateTimer();
}

void DnfWrapper::updateCache()
{
    if (m_cacheFutureWatcher.isRunning())
    {
        KLOG_DEBUG(upgrade) << "Cache update already in progress, skipping";
        return;
    }

    KLOG_DEBUG(upgrade) << "Starting cache update";
    m_cacheFutureWatcher.setFuture(QtConcurrent::run(this, &DnfWrapper::findAndCreateSack, true));
}

void DnfWrapper::cacheInvalidate()
{
    {
        QMutexLocker locker(&m_sackMutex);
        m_isSackVaild = false;
    }

    m_reloadCacheTimer->start();
}

void DnfWrapper::startUpdateTimer()
{
    // 计算距离下次更新的时间
    qint64 secondsUntilUpdate = m_updateIntervalHours * 3600;
    if (m_lastUpdateTime.isValid())
    {
        qint64 secondsSinceUpdate = m_lastUpdateTime.secsTo(QDateTime::currentDateTime());
        if (secondsSinceUpdate < secondsUntilUpdate)
        {
            secondsUntilUpdate -= secondsSinceUpdate;
        }
        else
        {
            // 已经超过更新周期，立即触发更新
            secondsUntilUpdate = 0;
        }
    }
    else
    {
        // 时间不合法，立即触发更新
        secondsUntilUpdate = 0;
    }

    if (secondsUntilUpdate > 0)
    {
        KLOG_DEBUG(upgrade) << "Next cache update in " << secondsUntilUpdate
                            << " seconds";
        m_updateTimer->start(secondsUntilUpdate * 1000);
    }
    else
    {
        KLOG_DEBUG(upgrade) << "Update cache right now.";
        // 立即触发更新
        QTimer::singleShot(0, this, [this]()
                           { cacheInvalidate(); });
    }
}

bool DnfWrapper::findAndCreateSack(bool forceUpdate)
{
    if (!m_dnfCtx)
    {
        KLOG_DEBUG(upgrade) << "Dnf context is not initialized, please initialize it first";
        return false;
    }

    if (forceUpdate)
    {
        m_isSackVaild = false;
    }

    {
        QMutexLocker locker(&m_sackMutex);
        // 若缓存有效并且Sack有效，则直接返回
        if (m_isSackVaild && !m_dnfSack.isNull())
        {
            KLOG_DEBUG(upgrade) << "Sack is valid, no need to create it";
            return true;
        }

        m_isSackVaild = false;
    }

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    // 创建 DnfSack 并加载仓库
    auto sack = QSharedPointer<::DnfSack>(dnf_sack_new(),
                                          [](::DnfSack *sack)
                                          { g_object_unref(sack); });

    dnf_sack_set_cachedir(sack.data(), dnf_context_get_solv_dir(m_dnfCtx));
    dnf_sack_set_rootdir(sack.data(), dnf_context_get_install_root(m_dnfCtx));

    g_autoptr(GError) error = nullptr;
    if (!dnf_sack_setup(sack.data(), DNF_SACK_SETUP_FLAG_NONE, &error))
    {
        KLOG_ERROR(upgrade) << "Failed to setup sack! error message: " << error->message;
        return false;
    }

    //加载系统仓库，必须要加载系统仓库才能比较出可更新包
    if (!dnf_sack_load_system_repo(sack.data(), NULL, DNF_SACK_LOAD_FLAG_BUILD_CACHE,
                                   &error))
    {
        KLOG_ERROR(upgrade) << "Failed to load system repo! error message: "
                            << error->message;
        return false;
    }
    KLOG_INFO(upgrade) << "Load system repo successfully.";

    // 获取所有仓库
    int loadedRepoCount = 0;
    g_autoptr(DnfRepoLoader) repoLoader = dnf_repo_loader_new(m_dnfCtx);
    GPtrArray *repos = dnf_repo_loader_get_repos(repoLoader, &error);
    if (!repos)
    {
        KLOG_ERROR(upgrade) << "Failed to get repos! error message: " << error->message;
        return false;
    }

    // 加载每个仓库到 Sack
    for (uint i = 0; i < repos->len; i++)
    {
        auto repo = static_cast<::DnfRepo *>(g_ptr_array_index(repos, i));

        //过滤”禁用源“
        auto enabled = dnf_repo_get_enabled(repo);
        if (enabled == DNF_REPO_ENABLED_NONE)
        {
            KLOG_DEBUG(upgrade) << "repo" << dnf_repo_get_id(repo) << "is disabled";
            continue;
        }

        g_autoptr(DnfState) dnfState = dnf_state_new();
        g_clear_error(&error);

        auto ret = dnf_sack_add_repo(
            sack.data(), repo, 0,
            static_cast<DnfSackAddFlags>(DNF_SACK_ADD_FLAG_NONE |
                                         DNF_SACK_ADD_FLAG_FILELISTS |
                                         DNF_SACK_ADD_FLAG_UPDATEINFO),
            dnfState, &error);
        if (!ret)
        {
            KLOG_WARNING(upgrade) << "Failed to load repo " << dnf_repo_get_id(repo)
                                  << " error message: " << error->message;
            continue;
        }

        // 验证仓库是否真的被加载到sack中
        // 因为断网且非必须源时dnf_sack_add_repo会返回true但实际未加载数据
        auto repoId = QString::fromUtf8(dnf_repo_get_id(repo));
        if (!isRepoLoaded(sack, repoId))
        {
            KLOG_WARNING(upgrade) << "Repo " << repoId
                                  << " was not actually loaded into sack (possibly due to network issues)";
            continue;
        }

        loadedRepoCount++;
    }

    // 若没有加载任何仓库，则认为缓存失败
    if (loadedRepoCount == 0)
    {
        KLOG_ERROR(upgrade) << "No repo loaded into sack.";
        return false;
    }
    KLOG_INFO(upgrade) << "Loaded " << loadedRepoCount << " repos into sack successfully.";

    /* set up the sack for packages that should only ever be installed, never
   * updated */
    dnf_sack_set_installonly(sack.data(),
                             dnf_context_get_installonly_pkgs(m_dnfCtx));
    /* set the installonly limit one higher than usual to avoid removing any
   * kernels during system upgrades */
    dnf_sack_set_installonly_limit(
        sack.data(), dnf_context_get_installonly_limit(m_dnfCtx) + 1);

    g_ptr_array_unref(repos);
    g_clear_error(&error);

    // 所有操作完成后，在锁保护下更新 m_dnfSack
    {
        QMutexLocker locker(&m_sackMutex);
        m_dnfSack.swap(sack);
        m_isSackVaild = true;
    }

    KLOG_DEBUG(upgrade) << "Cache update completed successfully!"
                        << "finished in" << elapsedTimer.elapsed() << "ms";
    return true;
}

QSharedPointer<::DnfSack> DnfWrapper::getSackRef(bool forceUpdate)
{
    // 确保sack可用
    if (!findAndCreateSack(forceUpdate))
    {
        KLOG_ERROR(upgrade) << "Failed to create sack.";
        return QSharedPointer<::DnfSack>();
    }

    // 在锁保护下获取sack引用，确保在使用期间不会被释放
    {
        QMutexLocker locker(&m_sackMutex);
        if (!m_dnfSack.isNull() && m_isSackVaild)
        {
            return m_dnfSack;
        }
        else
        {
            KLOG_ERROR(upgrade) << "Sack is null or invalid after findAndCreateSack";
            return QSharedPointer<::DnfSack>();
        }
    }
}

QList<QSharedPointer<::DnfPackage>> DnfWrapper::getUpgradesPackages(QString &errorMessage)
{
    QList<QSharedPointer<::DnfPackage>> ret;

    if (!m_dnfCtx)
    {
        errorMessage = tr("Dnf context is not initialized, please initialize it first.");
        return ret;
    }

    // 检查仓库数量和数据库中加载的仓库数量是否一致，如果不一致则强制更新缓存
    bool forceUpdate = false;
    auto loadedRepoCount = getLoadedRepoCount();
    auto allRepoCount = getAllRepoCount();
    KLOG_DEBUG(upgrade) << "Loaded repo number:" << loadedRepoCount
                        << "all repo number:" << allRepoCount;
    if (loadedRepoCount < allRepoCount)
    {
        //强制更新缓存
        forceUpdate = true;
        KLOG_INFO(upgrade) << "loaded repo number != all repo number, force update cache.";
    }

    // 获取sack引用
    auto sack = getSackRef(forceUpdate);
    if (sack.isNull())
    {
        errorMessage = tr("Failed to update cache.");
        return ret;
    }

    QMap<QString, QSharedPointer<::DnfPackage>> uniquePackages;  // 用于去重，key: 包名-架构, value: 包对象

    HyQuery hyQuery = hy_query_create(sack.data());
    hy_query_filter_upgrades(hyQuery, TRUE);
    hy_query_filter_latest(hyQuery, TRUE);  // 过滤出最新版本的包
    GPtrArray *pkgList = hy_query_run(hyQuery);

    // 若多个源中存在同一个包，同一个版本，则需要去重
    for (uint i = 0; i < pkgList->len; i++)
    {
        ::DnfPackage *pkg = (::DnfPackage *)g_ptr_array_index(pkgList, i);
        QString pkgName = QString(dnf_package_get_name(pkg));
        QString pkgArch = QString(dnf_package_get_arch(pkg));
        QString pkgKey = QString("%1-%2").arg(pkgName).arg(pkgArch);

        // 如果包不存在，或者当前包版本更新，则更新
        if (!uniquePackages.contains(pkgKey))
        {
            // 增加引用计数，因为我们要返回这个对象
            uniquePackages[pkgKey] = QSharedPointer<::DnfPackage>(g_object_ref(pkg),
                                                                  [](::DnfPackage *pkg)
                                                                  { g_object_unref(pkg); });
        }
        else
        {
            // 比较版本，保留较新的版本
            auto existingPkg = uniquePackages[pkgKey];
            if (!existingPkg.isNull() && dnf_package_evr_cmp(pkg, existingPkg.data()) > 0)
            {
                uniquePackages[pkgKey] = QSharedPointer<::DnfPackage>(g_object_ref(pkg),
                                                                      [](::DnfPackage *pkg)
                                                                      { g_object_unref(pkg); });
            }
        }
    }
    g_ptr_array_free(pkgList, FALSE);
    hy_query_free(hyQuery);

    return uniquePackages.values();
}

bool DnfWrapper::installPackages(const QList<QSharedPointer<::DnfPackage>> &packages, QString &errorMessage)
{
    if (!packages.size())
    {
        errorMessage = tr("No packages to install.");
        return false;
    }

    if (!m_dnfCtx)
    {
        errorMessage = tr("Dnf context is not initialized, please initialize it first.");
        return false;
    }

    // 获取sack引用
    auto sack = getSackRef();
    if (sack.isNull())
    {
        errorMessage = tr("Failed to get sack.");
        return false;
    }

    g_autoptr(DnfState) state = dnf_state_new();
    g_autoptr(GError) error = nullptr;
    auto goal = QSharedPointer<typename std::remove_pointer<HyGoal>::type>(
        hy_goal_create(sack.data()), &hy_goal_free);
    g_autoptr(DnfTransaction) transaction = dnf_transaction_new(m_dnfCtx);
    dnf_transaction_set_repos(transaction, dnf_context_get_repos(m_dnfCtx));

    auto percentageChangedCB = [](DnfState *, uint percentage, gpointer user_data)
    {
        KLOG_DEBUG(upgrade) << "percentage" << percentage;
        auto *self = static_cast<DnfWrapper *>(user_data);
        if (self)
        {
            emit self->installPercentageChanged(percentage);
        }
    };

    auto actionChangedTypeCB = [](DnfState *, DnfStateAction action,
                                  const char *actionHint,
                                  gpointer user_data)
    {
        auto *self = static_cast<DnfWrapper *>(user_data);
        if (!self)
        {
            return;
        }

        KLOG_DEBUG(upgrade) << "action: " << self->stateActionToString(action)
                            << " actionHint: " << actionHint;

        if (action == DNF_STATE_ACTION_UNKNOWN)
        {
            return;
        }

        auto actionStr = self->stateActionToString(action);
        auto actionHintStr = QString(actionHint);
        emit self->installActionChanged(actionStr, actionHintStr);
    };

    g_signal_connect(
        state, "percentage-changed",
        G_CALLBACK(static_cast<percentageChangedCBType>(percentageChangedCB)),
        this);
    g_signal_connect(
        state, "action-changed",
        G_CALLBACK(static_cast<actionChangedTypeCBType>(actionChangedTypeCB)),
        this);

    if (!dnf_state_set_steps(state, &error, 5, /* depsolve */
                             10,               /* download */
                             85,               /* commit else */
                             -1))
    {
        errorMessage = tr("Failed to init transaction state! error message: %1").arg(error->message);
        return false;
    }

    // 添加要安装的包
    for (auto pkg : packages)
    {
        hy_goal_install(goal.data(), pkg.data());
    }
    dnf_transaction_set_flags(transaction, DNF_TRANSACTION_FLAG_NONE);
    dnf_state_set_allow_cancel(state, false);

    // 解析依赖
    KLOG_DEBUG(upgrade) << "Depsolve started";
    if (!dnf_transaction_depsolve(transaction, goal.data(),
                                  dnf_state_get_child(state), &error) ||
        !dnf_state_done(state, &error))
    {
        errorMessage = tr("Failed to solve dep, error message: %1").arg(error->message);
        return false;
    }
    KLOG_DEBUG(upgrade) << "Depsolve finished";

    // 下载包
    KLOG_DEBUG(upgrade) << "Download started";
    if (!dnf_transaction_download(transaction, dnf_state_get_child(state),
                                  &error) ||
        !dnf_state_done(state, &error))
    {
        errorMessage = tr("Failed to download, error message: %1").arg(error->message);
        return false;
    }
    KLOG_DEBUG(upgrade) << "Download finished";

    // 输出本次有变动的相关包
    if (!hy_goal_write_debugdata(goal.data(), DNF_LOG_PATH, &error))
    {
        KLOG_WARNING(upgrade) << "Failed to write debugdata, error message: "
                              << error->message;
    }

    // 提交事务
    KLOG_DEBUG(upgrade) << "Commit started";
    if (!dnf_transaction_commit(transaction, goal.data(),
                                dnf_state_get_child(state), &error) ||
        !dnf_state_done(state, &error))
    {
        errorMessage = tr("Failed to commit, error message: %1").arg(error->message);
        return false;
    }
    KLOG_DEBUG(upgrade) << "Commit finished";
    return true;
}

QStringList DnfWrapper::solvePackageDeps(const QList<QSharedPointer<::DnfPackage>> &packages, QString &errorMessage)
{
    QStringList ret;

    if (!packages.size())
    {
        errorMessage = tr("No packages to solve dependencies.");
        return ret;
    }

    if (!m_dnfCtx)
    {
        errorMessage = tr("Dnf context is not initialized, please initialize it first.");
        return ret;
    }

    // 获取sack引用
    auto sack = getSackRef();
    if (sack.isNull())
    {
        errorMessage = tr("Failed to get sack.");
        return ret;
    }

    QMap<QString, ::DnfPackage *> addedDeps;  // 用于去重，key: 包名, value: 包对象

    g_autoptr(GError) error = nullptr;
    auto goal = QSharedPointer<typename std::remove_pointer<HyGoal>::type>(
        hy_goal_create(sack.data()), &hy_goal_free);
    g_autoptr(DnfTransaction) transaction = dnf_transaction_new(m_dnfCtx);
    dnf_transaction_set_repos(transaction, dnf_context_get_repos(m_dnfCtx));

    // 添加要安装的包
    for (auto pkg : packages)
    {
        hy_goal_install(goal.data(), pkg.data());
    }

    // 解析依赖
    dnf_transaction_set_flags(transaction, DNF_TRANSACTION_FLAG_NONE);
    if (!dnf_transaction_depsolve(transaction, goal.data(), nullptr, &error))
    {
        errorMessage = tr("Failed to solve dep, error message: %1").arg(error->message);
        return ret;
    }

    // 获取需要安装的包（包括未安装的依赖）
    g_autoptr(GPtrArray) installs = hy_goal_list_installs(goal.data(), &error);
    if (installs && installs->len > 0)
    {
        for (uint i = 0; i < installs->len; i++)
        {
            auto depPkg = (::DnfPackage *)g_ptr_array_index(installs, i);
            QString depName = QString(dnf_package_get_name(depPkg));
            QString depVersion = QString(dnf_package_get_evr(depPkg));

            // 使用包名作为key去重，如果已存在则更新为最新版本
            if (!addedDeps.contains(depName) ||
                dnf_package_evr_cmp(depPkg, addedDeps[depName]) > 0)
            {
                addedDeps[depName] = depPkg;
                KLOG_DEBUG(upgrade) << " installs added deps packages, depName: " << depName
                                    << ", depVersion: " << depVersion;
            }
        }
    }

    // 获取需要升级的包（包括依赖的升级）
    // 这样可以获取到已安装但有更新的依赖包
    g_autoptr(GPtrArray) upgrades = hy_goal_list_upgrades(goal.data(), &error);
    if (upgrades && upgrades->len > 0)
    {
        for (uint i = 0; i < upgrades->len; i++)
        {
            auto depPkg = (::DnfPackage *)g_ptr_array_index(upgrades, i);
            QString depName = QString(dnf_package_get_name(depPkg));
            QString depVersion = QString(dnf_package_get_evr(depPkg));

            // 使用包名作为key去重，如果已存在则更新为最新版本
            if (!addedDeps.contains(depName) ||
                dnf_package_evr_cmp(depPkg, addedDeps[depName]) > 0)
            {
                addedDeps[depName] = depPkg;
                KLOG_DEBUG(upgrade) << " upgrades added deps packages, depName: " << depName
                                    << ", depVersion: " << depVersion;
            }
        }
    }

    // 将去重后的依赖包添加到返回列表
    for (auto pkg : addedDeps.values())
    {
        ret.append(QString::fromUtf8(dnf_package_get_package_id(pkg)));
    }
    return ret;
}

QString DnfWrapper::getInstalledPackageVersion(const QString &packageName, QString &errorMessage)
{
    if (packageName.isEmpty())
    {
        errorMessage = tr("Package name is empty.");
        return QString();
    }

    if (!m_dnfCtx)
    {
        errorMessage = tr("Dnf context is not initialized, please initialize it first.");
        return QString();
    }

    // 获取sack引用
    auto sack = getSackRef();
    if (sack.isNull())
    {
        errorMessage = tr("Failed to get sack.");
        return QString();
    }

    HyQuery hyQuery = hy_query_create(sack.data());
    // 按包名过滤
    hy_query_filter(hyQuery, HY_PKG_NAME, HY_EQ, packageName.toUtf8());
    GPtrArray *pkgList = hy_query_run(hyQuery);

    QString version;
    if (pkgList && pkgList->len > 0)
    {
        // 获取第一个匹配的包
        ::DnfPackage *pkg = static_cast<::DnfPackage *>(g_ptr_array_index(pkgList, 0));
        version = QString(dnf_package_get_evr(pkg));
    }
    else
    {
        errorMessage = tr("Package %1 is not installed.").arg(packageName);
    }

    g_ptr_array_free(pkgList, FALSE);
    hy_query_free(hyQuery);

    return version;
}

QString DnfWrapper::stateActionToString(int action)
{
    switch (static_cast<DnfStateAction>(action))
    {
    case DNF_STATE_ACTION_UNKNOWN:
        return tr("Unknown");
    case DNF_STATE_ACTION_DOWNLOAD_PACKAGES:
        return tr("Downloading packages");
    case DNF_STATE_ACTION_DOWNLOAD_METADATA:
        return tr("Downloading metadata");
    case DNF_STATE_ACTION_LOADING_CACHE:
        return tr("Loading cache");
    case DNF_STATE_ACTION_TEST_COMMIT:
        return tr("Testing transaction");
    case DNF_STATE_ACTION_REQUEST:
        return tr("Requesting data");
    case DNF_STATE_ACTION_REMOVE:
        return tr("Removing packages");
    case DNF_STATE_ACTION_INSTALL:
        return tr("Installing packages");
    case DNF_STATE_ACTION_UPDATE:
        return tr("Updating packages");
    case DNF_STATE_ACTION_CLEANUP:
        return tr("Cleaning packages");
    case DNF_STATE_ACTION_OBSOLETE:
        return tr("Obsoleting packages");
    case DNF_STATE_ACTION_REINSTALL:
        return tr("Reinstalling packages");
    case DNF_STATE_ACTION_DOWNGRADE:
        return tr("Downgrading packages");
    case DNF_STATE_ACTION_QUERY:
        return tr("Querying for results");
    default:
        return tr("Unknown action");
    }
}

int DnfWrapper::getLoadedRepoCount()
{
    int loadedRepoCount = 0;

    if (!m_dnfCtx)
    {
        return loadedRepoCount;
    }

    Pool *pool = nullptr;
    int repoid = 0;
    Repo *repo = nullptr;

    {
        QMutexLocker locker(&m_sackMutex);
        if (m_dnfSack.isNull())
        {
            return loadedRepoCount;
        }

        pool = dnf_sack_get_pool(m_dnfSack.data());
    }

    FOR_REPOS(repoid, repo)
    {
        if (repo && repo->name)
        {
            loadedRepoCount++;
        }
    }
    return loadedRepoCount;
}

bool DnfWrapper::isRepoLoaded(QSharedPointer<::DnfSack> sack, const QString &repoName)
{
    if (sack.isNull() || repoName.isEmpty())
    {
        return false;
    }

    Pool *pool = dnf_sack_get_pool(sack.data());
    int repoid;
    Repo *repo;

    auto repoNameCStr = repoName.toUtf8().constData();
    FOR_REPOS(repoid, repo)
    {
        if (repo && repo->name && !strcmp(repo->name, repoNameCStr))
        {
            KLOG_DEBUG(upgrade) << "Repository " << repoName << " added in sack";
            return true;
        }
    }

    KLOG_DEBUG(upgrade) << "Repository " << repoName << " not added in sack";
    return false;
}

int DnfWrapper::getAllRepoCount()
{
    int enabledRepo = 0;

    if (!m_dnfCtx)
    {
        return enabledRepo;
    }

    g_autoptr(GError) error = nullptr;
    g_autoptr(DnfRepoLoader) repoLoader = dnf_repo_loader_new(m_dnfCtx);
    GPtrArray *repos = dnf_repo_loader_get_repos(repoLoader, &error);
    if (!repos)
    {
        KLOG_ERROR(upgrade) << "Failed to get repos! error message: " << error->message;
        return enabledRepo;
    }
    for (uint i = 0; i < repos->len; i++)
    {
        auto repo = static_cast<::DnfRepo *>(g_ptr_array_index(repos, i));

        //过滤”禁用源“
        auto enabled = dnf_repo_get_enabled(repo);
        if (enabled == DNF_REPO_ENABLED_NONE)
        {
            continue;
        }
        enabledRepo++;
    }
    g_ptr_array_unref(repos);
    return enabledRepo;
}
}  // namespace Kiran