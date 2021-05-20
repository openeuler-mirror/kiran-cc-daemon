/**
 * @file          /kiran-cc-daemon/plugins/inputdevices/common/xinput-helper.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "plugins/inputdevices/common/device-helper.h"

namespace Kiran
{
class XInputHelper
{
public:
    XInputHelper(){};
    virtual ~XInputHelper(){};

    // 判断是否支持xinput扩展
    static bool supports_xinput_devices();
    // 遍历输入设备
    static void foreach_device(std::function<void(std::shared_ptr<DeviceHelper>)> callback);
};

}  // namespace Kiran