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
#include <QFutureWatcher>
#include <QMap>
#include <QObject>
#include <QSharedPointer>

struct _DnfPackage;
typedef struct _DnfPackage DnfPackage;

namespace Kiran
{
class DnfWrapper;
class Installer : public QObject
{
    Q_OBJECT

public:
    explicit Installer(QObject *parent = nullptr);
    ~Installer();

    void setUpgradePkgs(const QMap<QString, QSharedPointer<::DnfPackage>> &upgradePkgs);
    CCErrorCode install(const QStringList &packageIDs);
    QString getInstallLog();

private:
    struct ResultDetail
    {
        bool success;
        QString errorMessage;
    };

    Installer::ResultDetail doInstall(const QStringList &packageIDs);

private slots:
    void handleInstallAction(const QString &action, const QString &actionHint);

signals:
    void installProgressChanged(uint percentage);
    void installActionChanged(const QString &action, const QString &actionHint);
    void installCompleted(bool success, const QString &errorMessage);

private:
    DnfWrapper *m_dnfWrapper;
    QFutureWatcher<ResultDetail> m_installerFutureWatcher;

    QMap<QString, QSharedPointer<::DnfPackage>> m_upgradePkgs;

    QString m_installLog;
    QMutex m_installLogMutex;
};
}  // namespace Kiran