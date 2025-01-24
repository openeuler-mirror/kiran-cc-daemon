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

#include "power-backlight-interface.h"

typedef struct xcb_connection_t xcb_connection_t;
typedef uint32_t xcb_atom_t;
typedef uint32_t xcb_randr_output_t;

namespace Kiran
{
// 通过Xrandr扩展调节单个显示设备亮度值
class PowerBacklightMonitorX11Atom : public PowerBacklightAbsolute
{
    Q_OBJECT

public:
    PowerBacklightMonitorX11Atom(xcb_atom_t backlightAtom, xcb_randr_output_t output);
    virtual ~PowerBacklightMonitorX11Atom();

    // 设置亮度值
    virtual bool setBrightnessValue(int32_t brightnessValue) override;
    // 获取亮度值
    virtual int32_t getBrightnessValue() override;
    // 获取亮度最大最小值
    virtual bool getBrightnessRange(int32_t &min, int32_t &max) override;

private:
    xcb_connection_t *m_xcbConnection;
    xcb_atom_t m_backlightAtom;
    xcb_randr_output_t m_output;
};

}  // namespace Kiran
