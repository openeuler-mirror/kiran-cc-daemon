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
#include <QMetaType>
#include <QSet>

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
typedef void (*packageProgressChangedCBType)(DnfState *,
                                             const gchar *,
                                             DnfStateAction,
                                             guint,
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
      m_isSackVaild(false)
{
    initDnf();
}

DnfWrapper::~DnfWrapper()
{
    if (m_dnfCtx)
    {
        g_object_unref(m_dnfCtx);
        m_dnfCtx = nullptr;
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

    connect(this, &DnfWrapper::invalidate, this, &DnfWrapper::cacheInvalidate, Qt::QueuedConnection);
    // 监听libdnf缓存失效信号
    typedef void (*dnfCacheInvalidateCBType)(::DnfContext *, const gchar *, gpointer);
    auto callback = [](::DnfContext *ctx, const gchar *message, gpointer user_data)
    {
        Q_UNUSED(ctx);
        auto *self = static_cast<DnfWrapper *>(user_data);
        KLOG_INFO(upgrade) << "cache invalidate because:" << message;
        if (self)
        {
            // 转发信号，使其在qt线程中执行
            emit self->invalidate();
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
                KLOG_INFO(upgrade) << "cache invalidate because: repository directory changed: " << path;
                cacheInvalidate();
            });
}

void DnfWrapper::cacheInvalidate()
{
    QMutexLocker locker(&m_sackMutex);
    m_isSackVaild = false;
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
    g_autoptr(GPtrArray) repos = dnf_repo_loader_get_repos(dnf_context_get_repo_loader(m_dnfCtx), &error);
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

        KLOG_INFO(upgrade) << "Loaded remote repo: " << dnf_repo_get_id(repo);
        loadedRepoCount++;
    }

    KLOG_INFO(upgrade) << "Loaded " << loadedRepoCount << "remote repos into sack.";
    // 若没有加载任何remote仓库，仅告警
    if (loadedRepoCount == 0)
    {
        KLOG_WARNING(upgrade) << "No remote repo loaded into sack.";
    }

    /* set up the sack for packages that should only ever be installed, never
   * updated */
    dnf_sack_set_installonly(sack.data(),
                             dnf_context_get_installonly_pkgs(m_dnfCtx));
    /* set the installonly limit one higher than usual to avoid removing any
   * kernels during system upgrades */
    dnf_sack_set_installonly_limit(
        sack.data(), dnf_context_get_installonly_limit(m_dnfCtx) + 1);

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

    // 检查repo总数和数据库中加载的repo数量是否一致，如果不一致则强制更新缓存
    bool forceUpdate = false;
    auto loadedRepoCount = getLoadedRepoCount();
    auto allRepoCount = getAllRepoCount();
    KLOG_INFO(upgrade) << "Loaded repo number:" << loadedRepoCount
                       << ",all repo number:" << allRepoCount;
    if (loadedRepoCount < allRepoCount)
    {
        //强制更新缓存
        forceUpdate = true;
        KLOG_INFO(upgrade) << "loaded repo number < all repo number, force update cache.";
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
    if (!pkgList)
    {
        errorMessage = tr("Failed to query upgrades package list.");
        hy_query_free(hyQuery);
        return ret;
    }

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
    if (pkgList)
    {
        g_ptr_array_free(pkgList, FALSE);
    }
    hy_query_free(hyQuery);

    return uniquePackages.values();
}

static bool parsePackageId4(const QString &packageId,
                            QString &name,
                            QString &evr,
                            QString &arch,
                            QString &repo)
{
    // libdnf/packagekit style: name;evr;arch;repo
    const auto parts = packageId.split(';');
    if (parts.size() < 4)
    {
        return false;
    }
    name = parts.value(0);
    evr = parts.value(1);
    arch = parts.value(2);
    repo = parts.value(3);
    return !(name.isEmpty() || evr.isEmpty() || arch.isEmpty() || repo.isEmpty());
}

QList<QSharedPointer<::DnfPackage>> DnfWrapper::resolvePackagesByIds(const QSharedPointer<::DnfSack> &sack,
                                                                     const QStringList &packageIDs,
                                                                     QString &errorMessage)
{
    QList<QSharedPointer<::DnfPackage>> ret;
    if (sack.isNull())
    {
        errorMessage = tr("Sack is null.");
        return ret;
    }

    for (const auto &packageId : packageIDs)
    {
        QString name, evr, arch, repo;
        if (!parsePackageId4(packageId, name, evr, arch, repo))
        {
            KLOG_WARNING(upgrade) << "Invalid package id (expected name;evr;arch;repo), skipped:" << packageId;
            continue;
        }

        HyQuery q = hy_query_create(sack.data());
        const QByteArray nameBa = name.toUtf8();
        const QByteArray evrBa = evr.toUtf8();
        const QByteArray archBa = arch.toUtf8();
        const QByteArray repoBa = repo.toUtf8();
        hy_query_filter(q, HY_PKG_NAME, HY_EQ, nameBa.constData());
        hy_query_filter(q, HY_PKG_EVR, HY_EQ, evrBa.constData());
        hy_query_filter(q, HY_PKG_ARCH, HY_EQ, archBa.constData());
        hy_query_filter(q, HY_PKG_REPONAME, HY_EQ, repoBa.constData());

        GPtrArray *pkgList = hy_query_run(q);
        hy_query_free(q);

        if (!pkgList || pkgList->len == 0)
        {
            if (pkgList)
            {
                g_ptr_array_free(pkgList, FALSE);
            }
            KLOG_WARNING(upgrade) << "Package not found for id, skipped:" << packageId;
            continue;
        }

        ::DnfPackage *pkg = nullptr;
        if (pkgList->len != 1)
        {
            KLOG_WARNING(upgrade) << "Multiple packages matched for id, skipped:" << packageId
                                  << "candidates:" << (pkgList ? pkgList->len : 0);
            g_ptr_array_free(pkgList, FALSE);
            continue;
        }
        pkg = static_cast<::DnfPackage *>(g_ptr_array_index(pkgList, 0));

        ret.append(QSharedPointer<::DnfPackage>(static_cast<::DnfPackage *>(g_object_ref(pkg)),
                                                [](::DnfPackage *p)
                                                { g_object_unref(p); }));
        g_ptr_array_free(pkgList, FALSE);
    }

    return ret;
}

bool DnfWrapper::installPackagesByIds(const QStringList &packageIDs, QString &errorMessage)
{
    if (packageIDs.isEmpty())
    {
        errorMessage = tr("No packages to install.");
        return false;
    }
    if (!m_dnfCtx)
    {
        errorMessage = tr("Dnf context is not initialized, please initialize it first.");
        return false;
    }

    auto sack = getSackRef();
    if (sack.isNull())
    {
        errorMessage = tr("Failed to get sack.");
        return false;
    }

    auto pkgs = resolvePackagesByIds(sack, packageIDs, errorMessage);
    if (!errorMessage.isEmpty())
    {
        return false;
    }
    if (pkgs.isEmpty())
    {
        errorMessage = tr("No matching packages found for the given package IDs.");
        return false;
    }

    // sack/goal/transaction 同生命周期（同一作用域内持有）
    return installPackagesInSack(sack, pkgs, errorMessage);
}

bool DnfWrapper::installPackagesInSack(const QSharedPointer<::DnfSack> &sack,
                                       const QList<QSharedPointer<::DnfPackage>> &packages,
                                       QString &errorMessage)
{
    QStringList plannedPackages;
    m_failedPackages.clear();
    m_successPackages.clear();

    // 记录升级开始时间
    QDateTime upgradeTime = QDateTime::currentDateTime();

    // 发送升级日志信号函数
    auto sendUpgradeHistory = [this, upgradeTime](UpgradeResult result, const QString &errMsg)
    {
        UpgradeHistory history(upgradeTime.toString(DEFAULT_DATE_TIME_FORMAT),
                               result,
                               errMsg,
                               m_successPackages,
                               m_failedPackages);
        emit upgradeHistotyReady(history);
    };

    // 发送升级历史信号，初始状态为未知
    sendUpgradeHistory(UpgradeResult::UPGRADE_RESULT_UNKNOWN, "");

    // 默认软件包安装都失败
    for (auto pkg : packages)
    {
        m_failedPackages.append(dnf_package_get_package_id(pkg.data()));
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

    // 跟踪包安装进度的回调
    auto packageProgressChangedCB = [](DnfState *state,
                                       const gchar *packageId,
                                       DnfStateAction action,
                                       guint percentage,
                                       gpointer user_data)
    {
        auto *self = static_cast<DnfWrapper *>(user_data);
        if (!self || !packageId)
        {
            return;
        }
        if (action != DNF_STATE_ACTION_INSTALL && action != DNF_STATE_ACTION_UPDATE)
        {
            return;
        }
        if (percentage < 100)
        {
            return;
        }
        QString pkgId = QString::fromUtf8(packageId);
        {
            // 本次事务开始时默认都标记为失败；当包安装/升级完成时，将其标记为成功并从失败列表移除
            if (self->m_failedPackages.contains(pkgId))
            {
                self->m_failedPackages.removeAll(pkgId);
            }
            if (!self->m_successPackages.contains(pkgId))
            {
                self->m_successPackages.append(pkgId);
            }
        }
        KLOG_DEBUG(upgrade) << "Package installation completed: " << pkgId;
    };

    g_signal_connect(
        state, "percentage-changed",
        G_CALLBACK(static_cast<percentageChangedCBType>(percentageChangedCB)),
        this);
    g_signal_connect(
        state, "action-changed",
        G_CALLBACK(static_cast<actionChangedTypeCBType>(actionChangedTypeCB)),
        this);
    g_signal_connect(
        state, "package-progress-changed",
        G_CALLBACK(static_cast<packageProgressChangedCBType>(packageProgressChangedCB)),
        this);

    if (!dnf_state_set_steps(state, &error, 5, /* depsolve */
                             10,               /* download */
                             85,               /* commit else */
                             -1))
    {
        errorMessage = tr("Failed to init transaction state! error message: %1").arg(error->message);
        sendUpgradeHistory(UpgradeResult::UPGRADE_RESULT_FAILED, errorMessage);
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
        sendUpgradeHistory(UpgradeResult::UPGRADE_RESULT_FAILED, errorMessage);
        return false;
    }
    KLOG_DEBUG(upgrade) << "Depsolve finished";

    // 获取计划安装和升级的包列表
    plannedPackages = getAllPackagesFromGoal(goal);
    KLOG_DEBUG(upgrade) << "Planned packages count (installs + upgrades): " << plannedPackages.size();

    // 下载包
    KLOG_DEBUG(upgrade) << "Download started";
    if (!dnf_transaction_download(transaction, dnf_state_get_child(state),
                                  &error) ||
        !dnf_state_done(state, &error))
    {
        errorMessage = tr("Failed to download, error message: %1").arg(error->message);
        sendUpgradeHistory(UpgradeResult::UPGRADE_RESULT_FAILED, errorMessage);
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
    bool commitSuccess = false;
    if (!dnf_transaction_commit(transaction, goal.data(),
                                dnf_state_get_child(state), &error) ||
        !dnf_state_done(state, &error))
    {
        errorMessage = tr("Failed to commit, error message: %1").arg(error->message);
        commitSuccess = false;
    }
    else
    {
        commitSuccess = true;
    }
    KLOG_DEBUG(upgrade) << "Commit finished";

    // 计算成功/失败的包列表
    m_failedPackages.clear();
    if (commitSuccess)
    {
        // commit成功，所有计划安装的包都标记为成功
        m_successPackages = plannedPackages;
        KLOG_DEBUG(upgrade) << "Commit succeeded, all " << m_successPackages.size() << " packages marked as success";
    }
    else
    {
        // commit失败，使用通过 package-progress-changed 信号跟踪到的成功安装的包
        // 其余包标记为失败
        for (const QString &packageId : plannedPackages)
        {
            if (!m_successPackages.contains(packageId))
            {
                m_failedPackages.append(packageId);
            }
        }
        KLOG_DEBUG(upgrade) << "Commit failed, "
                            << m_successPackages.size() << " packages installed successfully, "
                            << m_failedPackages.size() << " packages failed";
    }

    // 发送升级日志信号
    sendUpgradeHistory(commitSuccess ? UpgradeResult::UPGRADE_RESULT_SUCCESS : UpgradeResult::UPGRADE_RESULT_FAILED, errorMessage);

    return commitSuccess;
}

QStringList DnfWrapper::solvePackageDepsByIds(const QStringList &packageIDs, QString &errorMessage)
{
    if (packageIDs.isEmpty())
    {
        errorMessage = tr("No packages to solve dependencies.");
        return QStringList();
    }
    if (!m_dnfCtx)
    {
        errorMessage = tr("Dnf context is not initialized, please initialize it first.");
        return QStringList();
    }

    auto sack = getSackRef();
    if (sack.isNull())
    {
        errorMessage = tr("Failed to get sack.");
        return QStringList();
    }

    auto pkgs = resolvePackagesByIds(sack, packageIDs, errorMessage);
    if (!errorMessage.isEmpty())
    {
        return QStringList();
    }
    if (pkgs.isEmpty())
    {
        errorMessage = tr("No matching packages found for the given package IDs.");
        return QStringList();
    }

    // sack/goal/transaction 同生命周期（同一作用域内持有）
    return solvePackageDepsInSack(sack, pkgs, errorMessage);
}
QStringList DnfWrapper::solvePackageDepsInSack(const QSharedPointer<::DnfSack> &sack,
                                               const QList<QSharedPointer<::DnfPackage>> &packages,
                                               QString &errorMessage)
{
    QStringList ret;

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

    // 获取所有要安装或升级的包
    ret = getAllPackagesFromGoal(goal);
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
    const QByteArray nameBa = packageName.toUtf8();
    hy_query_filter(hyQuery, HY_PKG_NAME, HY_EQ, nameBa.constData());
    GPtrArray *pkgList = hy_query_run(hyQuery);

    QString version;
    if (pkgList && pkgList->len > 0)
    {
        // 获取第一个匹配的包
        ::DnfPackage *pkg = static_cast<::DnfPackage *>(g_ptr_array_index(pkgList, 0));
        if (pkg)
        {
            version = QString(dnf_package_get_evr(pkg));
        }
    }
    else
    {
        errorMessage = tr("Package %1 is not installed.").arg(packageName);
    }

    if (pkgList)
    {
        g_ptr_array_free(pkgList, FALSE);
    }
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

// 获取sack实际加载的所有remote repo仓库数量，去掉@System仓库。
int DnfWrapper::getLoadedRepoCount()
{
    QSharedPointer<::DnfSack> sack;
    {
        QMutexLocker locker(&m_sackMutex);
        // 仅统计当前“已生成且有效”的 sack，不触发重建
        if (!m_isSackVaild || m_dnfSack.isNull())
        {
            return 0;
        }
        sack = m_dnfSack;
    }

    int loadedRepoCount = 0;
    Pool *pool = dnf_sack_get_pool(sack.data());
    int repoid = 0;
    Repo *repo = nullptr;

    FOR_REPOS(repoid, repo)
    {
        if (repo && repo->name)
        {
            KLOG_DEBUG(upgrade) << "Loaded repo: " << repo->name;

            if (strcmp(repo->name, HY_SYSTEM_REPO_NAME) == 0)
            {
                continue;
            }
            loadedRepoCount++;
        }
    }
    return loadedRepoCount;
}

// 获取所有remote repo数量
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

bool DnfWrapper::isRepoLoaded(QSharedPointer<::DnfSack> sack, const QString &repoName)
{
    if (sack.isNull() || repoName.isEmpty())
    {
        return false;
    }

    Pool *pool = dnf_sack_get_pool(sack.data());
    int repoid;
    Repo *repo;

    auto repoNameBytes = repoName.toUtf8();
    auto repoNameCStr = repoNameBytes.constData();
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

QStringList DnfWrapper::getAllPackagesFromGoal(const QSharedPointer<typename std::remove_pointer<HyGoal>::type> &goal)
{
    g_autoptr(GError) error = nullptr;
    QStringList ret;
    // 按 NVR（name-evra）去重：多个源中同一包 NVR 相同则只保留一条
    QSet<QString> nvrKeySet;

    auto processPackageArray = [&](GPtrArray *pkgArray)
    {
        if (!pkgArray)
            return;
        for (guint i = 0; i < pkgArray->len; i++)
        {
            DnfPackage *pkg = static_cast<DnfPackage *>(g_ptr_array_index(pkgArray, i));
            if (!pkg)
                continue;
            const gchar *packageId = dnf_package_get_package_id(pkg);
            if (!packageId)
                continue;
            const gchar *name = dnf_package_get_name(pkg);
            const gchar *evr = dnf_package_get_evr(pkg);
            const gchar *arch = dnf_package_get_arch(pkg);
            if (!name || !evr || !arch)
                continue;
            auto nvrKey = QString("%1-%2.%3")
                              .arg(QString::fromUtf8(name),
                                   QString::fromUtf8(evr),
                                   QString::fromUtf8(arch));
            if (nvrKeySet.contains(nvrKey))
                continue;
            nvrKeySet.insert(nvrKey);
            ret.append(QString::fromUtf8(packageId));
        }
    };

    // 获取需要安装的包（包括未安装的依赖）
    g_autoptr(GPtrArray) installs = hy_goal_list_installs(goal.data(), &error);
    processPackageArray(installs);

    // 获取需要升级的包（包括依赖的升级）
    g_autoptr(GPtrArray) upgrades = hy_goal_list_upgrades(goal.data(), &error);
    processPackageArray(upgrades);

    return ret;
}
}  // namespace Kiran