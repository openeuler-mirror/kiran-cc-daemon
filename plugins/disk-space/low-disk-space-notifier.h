/**
 * Copyright (c) 2020 ~ 2026 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:    gaobo <gaobo@kylinsec.com.cn>
 */

#pragma once

#include <QObject>
#include <QSharedPointer>
#include <QString>

struct _NotifyNotification;
typedef struct _NotifyNotification NotifyNotification;

namespace Kiran
{

/** Low-disk alert via libnotify; m_notification uses QSharedPointer + g_object_unref */
class LowDiskSpaceNotifier : public QObject
{
    Q_OBJECT

public:
    explicit LowDiskSpaceNotifier(QObject* parent = nullptr);

    bool show(bool showAnalyze, qint64 freeBytes, const QString& volumeName, const QString& mountPath);

    bool hasActiveNotification() const;

signals:
    void response(bool launchedAnalyzer, const QString& mountPath);

private slots:
    void emitResponse(bool launchedAnalyzer);

private:
    static void onNotificationClosed(NotifyNotification* notification, void* userData);
    static void onNotificationAction(NotifyNotification* notification, char* action, void* userData);
    bool ensureNotifyInit();

    QSharedPointer<NotifyNotification> m_notification;
    QString m_mountPath;
    bool m_responseSent = false;
};

}  // namespace Kiran
