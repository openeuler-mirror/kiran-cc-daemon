/*
 * @Author       : tangjie02
 * @Date         : 2020-11-02 20:37:27
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-12-01 14:32:51
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/include/touchpad_i.h
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

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