/**
 * @file          /kiran-cc-daemon/plugins/power/wrapper/power-screensaver.h
 * @brief         对screensaver的dbus接口的封装
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "lib/base/base.h"

namespace Kiran
{
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