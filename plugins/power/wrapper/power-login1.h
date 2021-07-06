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
// 对systemd-login的dbus封装
class PowerLogin1
{
public:
    PowerLogin1();
    virtual ~PowerLogin1(){};

    void init();

    // 挂机
    bool suspend();
    // 休眠
    bool hibernate();
    // 关机
    bool shutdown();

private:
    Glib::RefPtr<Gio::DBus::Proxy> login1_proxy_;
};
}  // namespace Kiran