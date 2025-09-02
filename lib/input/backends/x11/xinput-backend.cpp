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
#include <QCoreApplication>
#include "lib/base/base.h"
#include "lib/xcb/xcb-connection.h"
#include "xinput-device.h"

namespace Kiran
{
XInputBackend::XInputBackend() : m_xinputOpcode(0)
{
    m_xcbConnection = XcbConnection::getDefault();
    
    if (isValid())
    {
        registerXInputEventListener();
        QCoreApplication::instance()->installNativeEventFilter(this);
    }
}

XInputBackend::~XInputBackend()
{
    QCoreApplication::instance()->removeNativeEventFilter(this);
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
    
    m_xinputOpcode = reply->major_opcode;
    return true;
}

QList<QSharedPointer<InputDevice>> XInputBackend::getDevices() const
{
    QList<QSharedPointer<InputDevice>> inputDevices;

    auto reply = XCB_REPLY(xcb_input_list_input_devices, m_xcbConnection->getConnection());
    auto xcbDeviceIter = xcb_input_list_input_devices_devices_iterator(reply.get());
    auto xcbDeviceNameIter = xcb_input_list_input_devices_names_iterator(reply.get());
    QStringList inputDeviceNames;

    while (xcbDeviceNameIter.rem > 0 && xcbDeviceIter.rem > 0)
    {
        auto xcbDeviceName = QByteArray(static_cast<const char*>(xcb_str_name(xcbDeviceNameIter.data)),
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

bool XInputBackend::registerXInputEventListener()
{
    auto connection = m_xcbConnection->getConnection();
    
    auto screen = m_xcbConnection->getDefaultScreen();
    xcb_window_t window = screen->root;
    
    // 注册设备层次结构更改事件
    xcb_input_event_mask_t* event_mask_ptr = static_cast<xcb_input_event_mask_t*>(
        malloc(sizeof(xcb_input_event_mask_t) + sizeof(uint32_t))
    );
    
    event_mask_ptr->deviceid = XCB_INPUT_DEVICE_ALL;
    event_mask_ptr->mask_len = 1;
    uint32_t* mask_ptr = reinterpret_cast<uint32_t*>(event_mask_ptr + 1);
    *mask_ptr = XCB_INPUT_XI_EVENT_MASK_DEVICE_CHANGED | XCB_INPUT_XI_EVENT_MASK_HIERARCHY;
    
    xcb_input_xi_select_events(connection, window, 1, event_mask_ptr);
    free(event_mask_ptr);
    
    xcb_flush(connection);
    
    KLOG_DEBUG() << "Registered XInput event listener";
    return true;
}

bool XInputBackend::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);
    
    // 只关心 xcb 事件
    if (eventType != "xcb_generic_event_t") {
        return false;
    }
    
    xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
    
    // XInput事件的类型从GE到GE+1，其中GE是GenericEvent类型
    if ((event->response_type & ~0x80) == XCB_GE_GENERIC)
    {
        xcb_ge_generic_event_t* ge = reinterpret_cast<xcb_ge_generic_event_t*>(event);
        
        // 检查是否是XInput扩展事件
        if (ge->extension == m_xinputOpcode)
        {
            // 设备层次结构变化事件
            if (ge->event_type == XCB_INPUT_HIERARCHY)
            {
                handleHierarchyEvent(event);
            }
        }
    }
    
    // 不阻止此事件继续传播
    return false;
}

void XInputBackend::handleHierarchyEvent(void* event)
{
    xcb_input_hierarchy_event_t* hierarchy_event = reinterpret_cast<xcb_input_hierarchy_event_t*>(event);
    
    // 检查设备变化类型
    bool deviceChangedFlag = false;
    auto info = xcb_input_hierarchy_infos_iterator(hierarchy_event);
    
    for (int i = 0; i < hierarchy_event->num_infos; i++)
    {
        if (info.data->flags & (XCB_INPUT_HIERARCHY_MASK_SLAVE_ADDED | 
                               XCB_INPUT_HIERARCHY_MASK_SLAVE_REMOVED |
                               XCB_INPUT_HIERARCHY_MASK_DEVICE_ENABLED |
                               XCB_INPUT_HIERARCHY_MASK_DEVICE_DISABLED))
        {
            deviceChangedFlag = true;
            KLOG_DEBUG() << "Device hierarchy changed, device ID:" << info.data->deviceid;
        }
        
        xcb_input_hierarchy_info_next(&info);
    }
    
    if (deviceChangedFlag)
    {
        KLOG_DEBUG() << "Emitting deviceChanged signal";
        Q_EMIT deviceChanged();
    }
}

}  // namespace Kiran
