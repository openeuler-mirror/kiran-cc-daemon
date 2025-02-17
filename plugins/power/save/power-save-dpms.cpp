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

#include "power-save-dpms.h"
#include <xcb/dpms.h>
#ifdef WITH_DPMS_KDE
#include <KScreenDpms/Dpms>
#endif
#include <QGuiApplication>
#include <QThread>
#include "lib/base/base.h"
#include "lib/xcb/xcb-connection.h"

namespace Kiran
{
#define POWER_DPMS_TIMING_CHECK 10

#ifdef WITH_DPMS_KDE

PowerSaveDpmsKDE::PowerSaveDpmsKDE(QObject *parent) : PowerSaveDpms(parent)
{
    m_dpms = new KScreen::Dpms(this);
}
void PowerSaveDpmsKDE::setLevel(PowerDpmsLevel level)
{
    switch (level)
    {
    case PowerDpmsLevel::POWER_DPMS_LEVEL_ON:
        m_dpms->switchMode(KScreen::Dpms::Mode::On);
        break;
    case PowerDpmsLevel::POWER_DPMS_LEVEL_STANDBY:
        m_dpms->switchMode(KScreen::Dpms::Mode::Standby);
        break;
    case PowerDpmsLevel::POWER_DPMS_LEVEL_SUSPEND:
        m_dpms->switchMode(KScreen::Dpms::Mode::Suspend);
        break;
    case PowerDpmsLevel::POWER_DPMS_LEVEL_OFF:
        m_dpms->switchMode(KScreen::Dpms::Mode::Off);
        break;
    default:
        KLOG_WARNING(power) << "Unknown power level" << level;
        break;
    }

    QGuiApplication::sync();
}

#endif

PowerSaveDpmsX11::PowerSaveDpmsX11(QObject *parent) : PowerSaveDpms(parent),
                                                      m_capable(false)
{
    auto xcbConnection = XcbConnection::getDefault();
    auto xcbConnectionRaw = xcbConnection->getConnection();

    xcb_prefetch_extension_data(xcbConnectionRaw, &xcb_dpms_id);
    auto *extension = xcb_get_extension_data(xcbConnectionRaw, &xcb_dpms_id);
    if (!extension || !extension->present)
    {
        KLOG_WARNING(power) << "DPMS extension not available";
        return;
    }

    auto capableReply = XCB_REPLY(xcb_dpms_capable, xcbConnectionRaw);
    if (!capableReply || !capableReply->capable)
    {
        KLOG_WARNING(power) << "DPMS is not capable.";
        return;
    }

    m_capable = true;
    xcb_dpms_set_timeouts(xcbConnectionRaw, 0, 0, 0);
}

void PowerSaveDpmsX11::setLevel(PowerDpmsLevel level)
{
    auto xcbConnection = XcbConnection::getDefault();
    auto xcbConnectionRaw = xcbConnection->getConnection();

    RETURN_IF_FALSE(m_capable);

    auto dpmsInfoReply = XCB_REPLY(xcb_dpms_info, xcbConnectionRaw);

    if (!dpmsInfoReply)
    {
        KLOG_WARNING(power) << "Failed to query DPMS state.";
        return;
    }

    // dpms功能未开启，执行开启操作
    if (!dpmsInfoReply->state)
    {
        KLOG_INFO(power) << "DPMS is defaultly not enabled so enable it now";
        xcb_dpms_enable(xcbConnectionRaw);
    }

    // 这里添加延迟，为了避免息屏后按键触发其它事件导致的亮屏 Fixed #24589
    QThread::usleep(1000 * 100);

    xcb_dpms_force_level(xcbConnectionRaw, levelK2Xcb(level));
    xcb_flush(xcbConnectionRaw);
}

uint16_t PowerSaveDpmsX11::levelK2Xcb(PowerDpmsLevel level)
{
    switch (level)
    {
    case PowerDpmsLevel::POWER_DPMS_LEVEL_ON:
        return XCB_DPMS_DPMS_MODE_ON;
    case PowerDpmsLevel::POWER_DPMS_LEVEL_STANDBY:
        return XCB_DPMS_DPMS_MODE_STANDBY;
    case PowerDpmsLevel::POWER_DPMS_LEVEL_SUSPEND:
        return XCB_DPMS_DPMS_MODE_SUSPEND;
    case PowerDpmsLevel::POWER_DPMS_LEVEL_OFF:
        return XCB_DPMS_DPMS_MODE_OFF;
    default:
        break;
    }
    return XCB_DPMS_DPMS_MODE_ON;
}

PowerDpmsLevel PowerSaveDpmsX11::levelXcb2K(uint16_t level)
{
    switch (level)
    {
    case XCB_DPMS_DPMS_MODE_ON:
        return PowerDpmsLevel::POWER_DPMS_LEVEL_ON;
    case XCB_DPMS_DPMS_MODE_STANDBY:
        return PowerDpmsLevel::POWER_DPMS_LEVEL_STANDBY;
    case XCB_DPMS_DPMS_MODE_SUSPEND:
        return PowerDpmsLevel::POWER_DPMS_LEVEL_SUSPEND;
    case XCB_DPMS_DPMS_MODE_OFF:
        return PowerDpmsLevel::POWER_DPMS_LEVEL_OFF;
    default:
        break;
    }
    return PowerDpmsLevel::POWER_DPMS_LEVEL_UNKNOWN;
}

}  // namespace Kiran