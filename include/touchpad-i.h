/**
 * @file          /kiran-cc-daemon/include/touchpad_i.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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