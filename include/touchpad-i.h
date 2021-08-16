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