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

struct IdleActionInfo
{
    IdleActionInfo() : idleTimeout(0), action(0) {}
    IdleActionInfo(int idleTimeout, int action)
    {
        this->idleTimeout = idleTimeout;
        this->action = action;
    }

    int idleTimeout;
    int action;

    friend QDBusArgument &operator<<(QDBusArgument &argument, const IdleActionInfo &idleActionInfo)
    {
        argument.beginStructure();
        argument << idleActionInfo.idleTimeout << idleActionInfo.action;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, IdleActionInfo &idleActionInfo)
    {
        argument.beginStructure();
        argument >> idleActionInfo.idleTimeout >> idleActionInfo.action;
        argument.endStructure();
        return argument;
    }
};

Q_DECLARE_METATYPE(IdleActionInfo)
