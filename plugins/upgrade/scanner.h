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

#include <error-i.h>
#include <upgrade-i.h>
#include <QFuture>
#include <QFutureWatcher>
#include <QJsonObject>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSharedPointer>
#include <QString>

struct _DnfPackage;
typedef struct _DnfPackage DnfPackage;

namespace Kiran
{
class DnfWrapper;
class Scanner : public QObject
{
    Q_OBJECT

public:
    explicit Scanner(QObject *parent = nullptr);
    ~Scanner();

    //扫描更新
    CCErrorCode scan();
    QString getUpgradePkgsJson();
    QMap<QString, QSharedPointer<::DnfPackage>> getUpgradePkgs();

private:
    struct ResultDetail
    {
        bool success;
        QString errorMessage;
    };

    Scanner::ResultDetail doScan();
    QString formatSize(quint64 size);
    QJsonObject getPackageAdvisoryInfo(QSharedPointer<::DnfPackage> pkg);
    void clearUpgradePkgs();

    //将DnfAdvisoryKind枚举转换为AdvisoryKindFlag
    AdvisoryKindFlag advisoryKindToFlag(int kind);

signals:
    void scanCompleted(bool success, const QString &errorMessage);

private:
    DnfWrapper *m_dnfWrapper;
    QFutureWatcher<ResultDetail> m_scanFutureWatcher;

    // 可更新包
    QMap<QString, QSharedPointer<::DnfPackage>> m_upgradePkgs;
    QMutex m_upgradePkgsMutex;
};
}  // namespace Kiran