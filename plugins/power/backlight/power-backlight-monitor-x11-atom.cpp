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

#include "power-backlight-monitor-x11-atom.h"
#include <xcb/randr.h>
#include <xcb/xcb.h>

namespace Kiran
{
PowerBacklightMonitorX11Atom::PowerBacklightMonitorX11Atom(xcb_atom_t backlightAtom,
                                                           xcb_randr_output_t output) : m_backlightAtom(backlightAtom),
                                                                                        m_output(output)
{
    m_xcbConnection = xcb_connect(NULL, NULL);
}

PowerBacklightMonitorX11Atom::~PowerBacklightMonitorX11Atom()
{
    if (m_xcbConnection)
    {
        xcb_disconnect(m_xcbConnection);
    }
}

bool PowerBacklightMonitorX11Atom::setBrightnessValue(int32_t brightnessValue)
{
    auto cookie = xcb_randr_change_output_property(m_xcbConnection,
                                                   m_output,
                                                   m_backlightAtom,
                                                   XCB_ATOM_INTEGER,
                                                   32,
                                                   XCB_PROP_MODE_REPLACE,
                                                   1,
                                                   (const void *)&brightnessValue);
    auto error = xcb_request_check(m_xcbConnection, cookie);
    if (error)
    {
        KLOG_WARNING(power) << "Failed to set brightness value, xcb error:" << error->error_code;
        free(error);
        return false;
    }
    return true;
}

int32_t PowerBacklightMonitorX11Atom::getBrightnessValue()
{
    RETURN_VAL_IF_TRUE(m_backlightAtom == XCB_ATOM_NONE, -1);

    int32_t result = -1;

    auto reply = XCB_REPLY(xcb_randr_get_output_property,
                           m_xcbConnection,
                           m_output,
                           m_backlightAtom,
                           XCB_ATOM_ANY,
                           0,
                           4,
                           false,
                           false);

    if (reply && reply->type == XCB_ATOM_INTEGER && reply->num_items == 1 && reply->format == 32)
    {
        memcpy(&result, xcb_randr_get_output_property_data(reply.get()), sizeof(int32_t));
    }
    else
    {
        KLOG_WARNING(power) << "The data of the brightness proerty is incorrect.";
    }

    return result;
}

bool PowerBacklightMonitorX11Atom::getBrightnessRange(int32_t &min, int32_t &max)
{
    auto reply = XCB_REPLY(xcb_randr_query_output_property,
                           m_xcbConnection,
                           m_output, m_backlightAtom);

    if (!reply)
    {
        KLOG_WARNING(power) << "Could not get output property for" << m_output;
        return false;
    }

    if (!reply->range || xcb_randr_query_output_property_valid_values_length(reply.get()) != 2)
    {
        KLOG_WARNING(power) << "The values isn't a range";
        return false;
    }

    auto values = xcb_randr_query_output_property_valid_values(reply.get());
    min = values[0];
    max = values[1];
    return true;
}

}  // namespace Kiran
