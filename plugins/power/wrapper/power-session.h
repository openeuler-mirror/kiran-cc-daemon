/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <QObject>

class QDBusMessage;

namespace Kiran
{
// 对SessionManager的dbus接口的封装
class PowerSession : public QObject
{
    Q_OBJECT

public:
    PowerSession();
    virtual ~PowerSession(){};

    void init();

    // 获取空闲状态
    bool getIdle() { return m_isIdle; };
    // 空闲状态是否禁用
    bool getIdleInhibited() { return m_isIdleInhibited; };
    // 挂起状态是否禁用
    bool getSuspendInhibited() { return m_isSuspendInhibited; };

    // 挂机
    bool canSuspend();
    void suspend();
    // 休眠
    bool canHibernate();
    void hibernate();
    // 关机
    bool canShutdown();
    void shutdown();

Q_SIGNALS:
    // 空闲状态发生变化
    void idleStatusChanged(bool);
    // 禁用状态发生变化
    void inhibitorChanged();

private:
    // 获取空闲状态
    uint32_t getStatus();
    // 判断logout, switch-user, suspend和idle的禁用状态
    bool getInhibited(uint32_t flag);

private Q_SLOTS:
    void processInhibitorChanged(const QDBusMessage& message);
    void processPresenceStatusChanged(const QDBusMessage& message);

private:
    bool m_isIdle;
    bool m_isIdleInhibited;
    bool m_isSuspendInhibited;
};
}  // namespace Kiran