/**
 * Copyright (c) 2025 ~ 2026 KylinSec Co., Ltd.
 * kiran-control-panel is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#pragma once

#include <QDBusArgument>
#include <QList>
#include <QMetaType>
#include <QObject>
#include <QSize>

struct DBusZoneInfo
{
    DBusZoneInfo() : gmt(0) {}
    QString name;
    QString localName;
    qlonglong gmt;

    friend QDBusArgument &operator<<(QDBusArgument &argument, const DBusZoneInfo &zoneInfo)
    {
        argument.beginStructure();
        argument << zoneInfo.name << zoneInfo.localName << zoneInfo.gmt;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, DBusZoneInfo &zoneInfo)
    {
        argument.beginStructure();
        argument >> zoneInfo.name >> zoneInfo.localName >> zoneInfo.gmt;
        argument.endStructure();
        return argument;
    }
};

typedef QList<DBusZoneInfo> DBusZoneInfos;

Q_DECLARE_METATYPE(DBusZoneInfo)
Q_DECLARE_METATYPE(DBusZoneInfos)
