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

#include <xcb/xinput.h>
#include <QSharedPointer>
#include <QVector>
#include "../../input-device.h"

namespace Kiran
{
class XcbConnection;

class XInputDevice : public InputDevice
{
    Q_OBJECT

public:
    XInputDevice() = delete;
    XInputDevice(const QString &deviceName, xcb_input_device_info_t *deviceInfo);
    virtual ~XInputDevice();
    // 判断设备是否存在指定的属性名
    bool hasProperty(const QString &name) override;
    // 判断设备是否为触摸板
    bool isTouchpad() override;
    // 设置属性值
    void setProperty(const QString &name, const QVector<bool> &values) override;
    void setProperty(const QString &name, float value) override;

    operator bool() const
    {
        return (m_deviceInfo != NULL);
    }

private:
    // 判断设备是否为psmouse
    bool isPSMouse();

private:
    QSharedPointer<XcbConnection> m_xcbConnection;
    QString m_deviceName;
    xcb_input_device_info_t *m_deviceInfo;
};
}  // namespace Kiran