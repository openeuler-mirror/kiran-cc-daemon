/**
 * @file          /kiran-cc-daemon/plugins/power/save/power-save-dpms.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"
//
#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>
#include <gdk/gdkx.h>

namespace Kiran
{
enum PowerDpmsLevel
{
    POWER_DPMS_LEVEL_ON,
    POWER_DPMS_LEVEL_STANDBY,
    POWER_DPMS_LEVEL_SUSPEND,
    POWER_DPMS_LEVEL_OFF,
    POWER_DPMS_LEVEL_UNKNOWN
};

class PowerSaveDpms
{
public:
    PowerSaveDpms();
    virtual ~PowerSaveDpms();

    void init();

    // 设置节能模式
    bool set_level(PowerDpmsLevel level);
    // 获取节能模式
    PowerDpmsLevel get_level();

    // 模式发生变化
    sigc::signal<void, PowerDpmsLevel> &signal_level_changed() { return this->level_changed_; };

private:
    void clear_dpms_timeout();

    CARD16 level_enum2card(PowerDpmsLevel enum_level);
    PowerDpmsLevel level_card2enum(CARD16 card_level);

    bool on_timing_check_level_cb();

private:
    GdkDisplay *display_;
    Display *xdisplay_;

    // dpms对该客户端是否可用
    bool capable_;
    // 缓存上一次获取的节能模式
    PowerDpmsLevel cached_level_;

    sigc::signal<void, PowerDpmsLevel> level_changed_;

    sigc::connection timing_handler_;
};
}  // namespace Kiran