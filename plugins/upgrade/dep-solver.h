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

#include <QFutureWatcher>
#include <QObject>
#include <QStringList>

#include <error-i.h>

namespace Kiran
{
class DnfWrapper;
class DepSolver : public QObject
{
    Q_OBJECT

public:
    explicit DepSolver(QObject *parent = nullptr);
    ~DepSolver();

    CCErrorCode solveDeps(const QStringList &packageIDs);

private:
    struct ResultDetail
    {
        bool success;
        QString pkgDepsJson;
        QString errorMessage;
    };

    ResultDetail doSolveDeps(const QStringList &packageIDs);

signals:
    void solveDepsCompleted(bool success, const QString &pkgDepsJson, const QString &errorMessage);

private:
    DnfWrapper *m_dnfWrapper;

    QFutureWatcher<ResultDetail> m_solveFutureWatcher;
};
}  // namespace Kiran