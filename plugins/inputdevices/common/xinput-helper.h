/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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