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

#include "power-idle-timer.h"
#include <QTimer>
#include "../wrapper/power-session.h"
#include "../wrapper/power-wrapper-manager.h"
#include "lib/base/base.h"

namespace Kiran
{
PowerIdleTimer::PowerIdleTimer(QObject *parent) : QObject(parent),
                                                  m_mode(PowerIdleMode::POWER_IDLE_MODE_NORMAL),
                                                  m_blankTimeout(0),
                                                  m_sleepTimeout(0)
{
    m_session = PowerWrapperManager::getInstance()->getDefaultSession();
    m_blankTimer = new QTimer(this);
    m_sleepTimer = new QTimer(this);
}

PowerIdleTimer::~PowerIdleTimer()
{
}

void PowerIdleTimer::init()
{
    connect(m_session.get(), &PowerSession::idleStatusChanged, this, &PowerIdleTimer::processSessionIdleStatusChanged);
    connect(m_session.get(), &PowerSession::inhibitorChanged, this, &PowerIdleTimer::processInhibitorChanged);
    connect(m_blankTimer, &QTimer::timeout, this, &PowerIdleTimer::processBlankTimeout);
    connect(m_sleepTimer, &QTimer::timeout, this, &PowerIdleTimer::processSleepTimeout);
}

bool PowerIdleTimer::setIdleTimeout(PowerIdleMode mode, uint32_t timeout)
{
    KLOG_DEBUG(power) << "Set idle timeout to" << timeout << "for mode" << PowerIdleTimer::idleModeEnum2Str(mode);

    switch (mode)
    {
    case PowerIdleMode::POWER_IDLE_MODE_DIM:
        // 已不支持该功能，空闲信号通过会话管理信号监控，当收到空闲信号时根据gsettings开关决定显示器是否变暗
        return false;
    case PowerIdleMode::POWER_IDLE_MODE_BLANK:
        return setBlankTimeout(timeout);
    case PowerIdleMode::POWER_IDLE_MODE_SLEEP:
        return setSleepTimeout(timeout);
    default:
        break;
    }
    return false;
}

QString PowerIdleTimer::idleModeEnum2Str(PowerIdleMode mode)
{
    QString modeInfo;
    switch (mode)
    {
    case PowerIdleMode::POWER_IDLE_MODE_NORMAL:
        modeInfo = "normal";
        break;
    case PowerIdleMode::POWER_IDLE_MODE_DIM:
        modeInfo = "dim";
        break;
    case PowerIdleMode::POWER_IDLE_MODE_BLANK:
        modeInfo = "blank";
        break;
    case PowerIdleMode::POWER_IDLE_MODE_SLEEP:
        modeInfo = "sleep";
        break;
    default:
        modeInfo = "";
        break;
    }
    return modeInfo;
}

bool PowerIdleTimer::setBlankTimeout(uint32_t timeout)
{
    m_blankTimeout = timeout;
    return true;
}

bool PowerIdleTimer::setSleepTimeout(uint32_t timeout)
{
    m_sleepTimeout = timeout;
    return true;
}

void PowerIdleTimer::updateMode()
{
    auto isIdle = m_session->getIdle();
    auto idleInhibit = m_session->getIdleInhibited();

    KLOG_INFO(power) << "Current system is" << (isIdle ? "idle." : "not idle.") << "idle inhibit is" << idleInhibit;

    // 如果为未空闲状态，或者禁止空闲时操作，则不进行节能处理
    if (!isIdle || idleInhibit)
    {
        switchMode(PowerIdleMode::POWER_IDLE_MODE_NORMAL);
        m_blankTimer->stop();
        m_sleepTimer->stop();
        return;
    }

    // 计算机空闲超时后首先进入显示器变暗状态
    if (m_mode == PowerIdleMode::POWER_IDLE_MODE_NORMAL)
    {
        switchMode(PowerIdleMode::POWER_IDLE_MODE_DIM);
    }

    // 设置空闲一定时间后显示器/计算机进入节能模式的定时器
    if (!m_blankTimer->isActive() && m_blankTimeout > 0)
    {
        m_blankTimer->start(m_blankTimeout * 1000);
    }

    auto isSuspendInhibit = m_session->getSuspendInhibited();
    if (isSuspendInhibit)
    {
        m_sleepTimer->stop();
    }
    else if (!m_sleepTimer->isActive() &&
             m_sleepTimeout > 0)
    {
        m_sleepTimer->start(m_sleepTimeout * 1000);
    }
}

void PowerIdleTimer::switchMode(PowerIdleMode mode)
{
    if (m_mode != mode)
    {
        m_mode = mode;
        Q_EMIT modeChanged(mode);
    }
}

void PowerIdleTimer::processBlankTimeout()
{
    // 如果已经进入计算机节能模式，则不再发送显示设备节能信号
    if (m_mode >= PowerIdleMode::POWER_IDLE_MODE_BLANK)
    {
        KLOG_DEBUG(power) << "Ignore blank timeout, because of current mode is" << m_mode;
    }
    else
    {
        switchMode(PowerIdleMode::POWER_IDLE_MODE_BLANK);
    }

    m_blankTimer->stop();
}

void PowerIdleTimer::processSleepTimeout()
{
    switchMode(PowerIdleMode::POWER_IDLE_MODE_SLEEP);
    m_sleepTimer->stop();
}

void PowerIdleTimer::processSessionIdleStatusChanged(bool isIdle)
{
    KLOG_INFO(power) << "Session idle status changed, need to update mode.";
    updateMode();
}

void PowerIdleTimer::processInhibitorChanged()
{
    KLOG_INFO(power) << "Session inhibitor changed, need to update mode.";
    updateMode();
}

}  // namespace Kiran