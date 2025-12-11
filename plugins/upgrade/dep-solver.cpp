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
#include "dnf/dnf-wrapper.h"

#include <kiran-log/qt5-log-i.h>
#include <qt5-log-i.h>
#include <QFuture>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrent>

#include "dep-solver.h"
#include "lib/base/log.h"

namespace Kiran
{
DepSolver::DepSolver(QObject *parent) : QObject(parent)
{
    m_dnfWrapper = DnfWrapper::instance();
    connect(&m_solveFutureWatcher, &QFutureWatcher<ResultDetail>::finished, this, [this]()
            { emit solveDepsCompleted(m_solveFutureWatcher.result().success,
                                      m_solveFutureWatcher.result().pkgDepsJson,
                                      m_solveFutureWatcher.result().errorMessage); });
}

DepSolver::~DepSolver()
{
    if (m_solveFutureWatcher.isRunning())
    {
        m_solveFutureWatcher.waitForFinished();
    }

    m_upgradePkgs.clear();
}

void DepSolver::setUpgradePkgs(const QMap<QString, QSharedPointer<::DnfPackage>> &upgradePkgs)
{
    m_upgradePkgs = upgradePkgs;
}

CCErrorCode DepSolver::solveDeps(const QStringList &packageIDs)
{
    if (m_solveFutureWatcher.isRunning())
    {
        KLOG_WARNING(upgrade) << "Solve deps is already in progress";
        return CCErrorCode::ERROR_UPGRADE_SOLVE_DEPS_NOT_COMPLETED;
    }

    if (packageIDs.isEmpty())
    {
        KLOG_ERROR(upgrade) << "Package IDs list is empty";
        return CCErrorCode::ERROR_UPGRADE_PACKAGE_IDS_EMPTY;
    }

    m_solveFutureWatcher.setFuture(QtConcurrent::run(this, &DepSolver::doSolveDeps, packageIDs));
    return CCErrorCode::SUCCESS;
}

DepSolver::ResultDetail DepSolver::doSolveDeps(const QStringList &packageIDs)
{
    KLOG_DEBUG(upgrade) << "Starting to solve dependencies for" << packageIDs;

    auto makePkgDepsJson = [](const QStringList &requestPackages, const QStringList &deps) -> QString
    {
        QJsonObject obj;
        obj["request_packages"] = QJsonArray::fromStringList(requestPackages);
        obj["dependency_packages"] = QJsonArray::fromStringList(deps);
        return QJsonDocument(obj).toJson(QJsonDocument::Compact);
    };

    QList<QSharedPointer<::DnfPackage>> targetPackages;

    for (const QString &packageID : packageIDs)
    {
        auto pkg = m_upgradePkgs.value(packageID);
        if (!pkg.isNull())
        {
            targetPackages.append(pkg);
        }
    }

    if (targetPackages.isEmpty())
    {
        KLOG_WARNING(upgrade) << "No matching packages found for the given package IDs";
        return ResultDetail{false, makePkgDepsJson(packageIDs, QStringList()), tr("No matching packages found for the given package IDs")};
    }

    // 解决依赖
    QString errorMessage;
    QStringList deps = m_dnfWrapper->solvePackageDeps(targetPackages, errorMessage);
    if (!errorMessage.isEmpty())
    {
        KLOG_WARNING(upgrade) << "Failed to solve dependencies. " << errorMessage;
        return ResultDetail{false, makePkgDepsJson(packageIDs, QStringList()), errorMessage};
    }

    // 无依赖包,正常情况
    if (deps.isEmpty())
    {
        KLOG_WARNING(upgrade) << "No dependency packages found. " << errorMessage;
        return ResultDetail{true, makePkgDepsJson(packageIDs, QStringList()), QString()};
    }

    KLOG_DEBUG(upgrade) << "Dependencies solved, found" << deps.size() << "dependency packages";
    return ResultDetail{true, makePkgDepsJson(packageIDs, deps), QString()};
}
}  // namespace Kiran