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