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

    // 禁止systemd-login1对电源、休眠、挂起按键和合上盖子进行操作。
    int32_t inhibit(const std::string &what);

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