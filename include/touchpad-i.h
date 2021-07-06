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

#ifdef __cplusplus
extern "C"
{
#endif

#define TOUCHPAD_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon.TouchPad"
#define TOUCHPAD_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/TouchPad"
#define TOUCHPAD_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SessionDaemon.TouchPad"

    enum TouchPadClickMethod
    {

        TOUCHPAD_CLICK_METHOD_BUTTON_AREAS,
        TOUCHPAD_CLICK_METHOD_CLICK_FINGER,
    };

    enum TouchPadScrollMethod
    {
        TOUCHPAD_SCROLL_METHOD_TWO_FINGER,
        TOUCHPAD_SCROLL_METHOD_EDGE,
        TOUCHPAD_SCROLL_METHOD_BUTTON,
    };

#ifdef __cplusplus
}
#endif