/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-control-panel is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     luoqing <luoqing@kylinsec.com.cn>
 */

#pragma once

#include <QDBusArgument>
#include <QList>
#include <QMetaType>
#include <QObject>
#include <QSize>

typedef QList<quint32> quint32List;
Q_DECLARE_METATYPE(quint32List)

typedef QList<quint16> quint16List;
Q_DECLARE_METATYPE(quint16List)

struct DisplayModesStu
{
    DisplayModesStu() : index(0), w(0), h(0), refreshRate(0) {}
    uint32_t index;
    uint32_t w;
    uint32_t h;
    double refreshRate;

    friend QDBusArgument &operator<<(QDBusArgument &argument, const DisplayModesStu &stu)
    {
        argument.beginStructure();
        argument << stu.index << stu.w << stu.h << stu.refreshRate;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, DisplayModesStu &stu)
    {
        argument.beginStructure();
        argument >> stu.index >> stu.w >> stu.h >> stu.refreshRate;
        argument.endStructure();
        return argument;
    }
};

typedef QList<DisplayModesStu> ListDisplayModesStu;

Q_DECLARE_METATYPE(DisplayModesStu)
Q_DECLARE_METATYPE(ListDisplayModesStu)
