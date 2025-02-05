/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
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

#include "xinput-backend.h"
#include <xcb/xcb.h>
#include <xcb/xcbext.h>
#include <xcb/xinput.h>
#include "lib/base/base.h"
#include "lib/xcb/xcb-connection.h"
#include "xinput-device.h"

namespace Kiran
{

XInputBackend::XInputBackend()
{
    m_xcbConnection = XcbConnection::getDefault();
}

bool XInputBackend::isValid()
{
    auto xcbConnection = m_xcbConnection->getConnection();

    xcb_extension_t xcbInputID = {"XInputExtension", 0};

    auto reply = xcb_get_extension_data(xcbConnection, &xcbInputID);
    if (!reply || !reply->present)
    {
        KLOG_WARNING() << "XInput extension is not present on the X server";
        return false;
    }
    return true;
}

QList<QSharedPointer<InputDevice>> XInputBackend::getDevices() const
{
    QList<QSharedPointer<InputDevice>> inputDevices;

    auto reply = XCB_REPLY(xcb_input_list_input_devices, m_xcbConnection->getConnection());
    auto xcbDevices = xcb_input_list_input_devices_devices(reply.get());
    auto xcbDeviceIter = xcb_input_list_input_devices_devices_iterator(reply.get());
    auto xcbDeviceNameIter = xcb_input_list_input_devices_names_iterator(reply.get());
    QStringList inputDeviceNames;

    while (xcbDeviceNameIter.rem > 0 && xcbDeviceIter.rem > 0)
    {
        auto xcbDeviceName = QByteArray((const char*)xcb_str_name(xcbDeviceNameIter.data),
                                        xcb_str_name_length(xcbDeviceNameIter.data));
        auto xcbDevice = xcbDeviceIter.data;
        xcb_str_next(&xcbDeviceNameIter);
        xcb_input_device_info_next(&xcbDeviceIter);

        if (xcbDeviceName == "Virtual core pointer" ||
            xcbDeviceName == "Virtual core keyboard")
        {
            KLOG_DEBUG() << "Ignore device" << xcbDeviceName;
            continue;
        }

        inputDeviceNames.push_back(xcbDeviceName);
        auto inputDevice = qSharedPointerCast<InputDevice>(QSharedPointer<XInputDevice>::create(xcbDeviceName, xcbDevice));
        inputDevices.push_back(inputDevice);
    }
    KLOG_DEBUG() << "Input devices contain" << inputDeviceNames;
    return inputDevices;
}

}  // namespace Kiran
