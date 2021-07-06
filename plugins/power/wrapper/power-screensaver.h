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

#include "lib/base/base.h"

namespace Kiran
{
// 对screensaver的dbus接口的封装
class PowerScreenSaver
{
public:
    PowerScreenSaver();
    virtual ~PowerScreenSaver(){};

    void init();

    // 锁屏
    bool lock();

    // 添加throttle，禁止运行屏保主题。函数返回一个cookie，可以通过cookie移除该throttle
    uint32_t add_throttle(const std::string &reason);

    // 锁屏并添加throttle，如果失败则返回0，否则返回cookie
    uint32_t lock_and_throttle(const std::string &reason);

    // 移除throttle
    bool remove_throttle(uint32_t cookie);

    // 模拟用户激活操作
    bool poke();

private:
    bool check_running();

private:
    Glib::RefPtr<Gio::DBus::Proxy> screensaver_proxy_;
};
}  // namespace Kiran