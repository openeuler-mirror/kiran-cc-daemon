/**
 * @file          /kiran-cc-daemon/plugins/power/wrapper/power-login1.h
 * @brief         对systemd-login的dbus封装
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

namespace Kiran
{
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