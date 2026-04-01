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

#include "xinput-device.h"
#include <xcb/xcb.h>
#include <xcb/xinput.h>
#include <algorithm>
#include "lib/base/base.h"
#include "lib/xcb/xcb-connection.h"

namespace Kiran
{
XInputDevice::XInputDevice(const QString &deviceName,
                           xcb_input_device_info_t *deviceInfo) : m_deviceName(deviceName),
                                                                  m_deviceInfo(deviceInfo)
{
    m_xcbConnection = XcbConnection::getDefault();
}

XInputDevice::~XInputDevice()
{
}

bool XInputDevice::hasProperty(const QString &name)
{
    auto xcbConnection = m_xcbConnection->getConnection();

    auto propertyReply = XCB_REPLY(xcb_intern_atom,
                                   xcbConnection,
                                   true,
                                   name.size(),
                                   name.toLatin1().data());

    RETURN_VAL_IF_TRUE(!propertyReply, false);

    auto reply = XCB_REPLY(xcb_input_get_device_property, xcbConnection,
                           propertyReply->atom,
                           XCB_ATOM_ANY,
                           0,
                           1,
                           m_deviceInfo->device_id,
                           false);
    return (reply && reply->type != XCB_ATOM_NONE);
}

bool XInputDevice::isTouchpad()
{
    // 由于之前项目上设备适配，将特殊PS2Mouse作为触摸板处理(#34878)
    // 导致禁用触摸板功能，会禁用掉psmouse设备，导致vmware虚拟机上鼠标功能失效(VirtualPS/2 VMware VMMouse)
    // 此处特殊处理，避免影响vmware虚拟机上鼠标功能
    if (m_deviceName.contains("VMMouse"))
    {
        return false;
    }

    if (isPSMouse())
    {
        return true;
    }

    auto reply = XCB_REPLY(xcb_intern_atom,
                           m_xcbConnection->getConnection(),
                           true,
                           strlen("TOUCHPAD"),
                           "TOUCHPAD");

    if (!reply || m_deviceInfo->device_type != reply->atom)
    {
        return false;
    }

    if (hasProperty("libinput Tapping Enabled") || hasProperty("Synaptics Off"))
    {
        return true;
    }

    return false;
}

void XInputDevice::setProperty(const QString &name, const QVector<bool> &values)
{
    auto xcbConnection = m_xcbConnection->getConnection();

    KLOG_INFO() << "Change device" << m_deviceName
                << "property which name is" << name
                << "and value is" << values;

    auto propertyReply = XCB_REPLY(xcb_intern_atom,
                                   xcbConnection,
                                   true,
                                   name.size(),
                                   name.toLatin1().data());

    RETURN_IF_TRUE(!propertyReply);

    auto getPropertyReply = XCB_REPLY(xcb_input_get_device_property, xcbConnection,
                                      propertyReply->atom,
                                      XCB_ATOM_INTEGER,
                                      0,
                                      1,
                                      m_deviceInfo->device_id,
                                      false);

    if (getPropertyReply &&
        getPropertyReply->type == XCB_ATOM_INTEGER &&
        getPropertyReply->format == 8 &&
        getPropertyReply->num_items > 0)
    {
        auto oldValues = static_cast<uint8_t *>(xcb_input_get_device_property_items(getPropertyReply.get()));
        if (values.size() > int(getPropertyReply->num_items))
        {
            KLOG_WARNING() << "Ignore the remaining value. the number of property set is" << values.size()
                           << ", the number of real device property is" << getPropertyReply->num_items;
        }

        uint32_t num = std::min(uint32_t(getPropertyReply->num_items), uint32_t(values.size()));
        for (uint32_t i = 0; i < num; ++i)
        {
            oldValues[i] = values[i] ? 1 : 0;
        }

        xcb_input_change_device_property(xcbConnection,
                                         propertyReply->atom,
                                         XCB_ATOM_INTEGER,
                                         m_deviceInfo->device_id,
                                         8,
                                         XCB_PROP_MODE_REPLACE,
                                         getPropertyReply->num_items,
                                         oldValues);
    }
    else
    {
        KLOG_WARNING() << "Failed to change device" << m_deviceName
                       << "property which name is" << name
                       << "and value is" << values;
    }
}

void XInputDevice::setProperty(const QString &name, float value)
{
    auto xcbConnection = m_xcbConnection->getConnection();

    KLOG_INFO() << "Change device" << m_deviceName
                << "property which name is" << name
                << "and value is" << value;

    auto propertyReply = XCB_REPLY(xcb_intern_atom,
                                   xcbConnection,
                                   true,
                                   name.size(),
                                   name.toLatin1().data());

    RETURN_IF_TRUE(!propertyReply);

    auto floatReply = XCB_REPLY(xcb_intern_atom,
                                xcbConnection,
                                true,
                                strlen("FLOAT"),
                                "FLOAT");

    RETURN_IF_TRUE(!floatReply);

    auto getPropertyReply = XCB_REPLY(xcb_input_get_device_property, xcbConnection,
                                      propertyReply->atom,
                                      floatReply->atom,
                                      0,
                                      1,
                                      m_deviceInfo->device_id,
                                      false);

    if (getPropertyReply &&
        getPropertyReply->type == floatReply->atom &&
        getPropertyReply->format == 32 &&
        getPropertyReply->num_items > 0)
    {
        unsigned char datacopy[sizeof(float)];
        memcpy(datacopy, &value, sizeof(float));

        xcb_input_change_device_property(xcbConnection,
                                         propertyReply->atom,
                                         floatReply->atom,
                                         m_deviceInfo->device_id,
                                         32,
                                         XCB_PROP_MODE_REPLACE,
                                         getPropertyReply->num_items,
                                         datacopy);
    }
    else
    {
        KLOG_WARNING() << "Failed to change device" << m_deviceName
                       << "property which name is" << name
                       << "and value is" << value;
    }
}

bool XInputDevice::isPSMouse()
{
    auto reply = XCB_REPLY(xcb_intern_atom,
                           m_xcbConnection->getConnection(),
                           true,
                           strlen("MOUSE"),
                           "MOUSE");

    if (!reply || m_deviceInfo->device_type != reply->atom)
    {
        return false;
    }

    return m_deviceName.contains("PS/2");
}

}  // namespace Kiran
