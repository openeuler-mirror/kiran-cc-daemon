/*
 * @Author       : tangjie02
 * @Date         : 2020-08-10 09:18:32
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-10 09:26:07
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/inputdevices/common/xinput-helper.h
 */

#pragma once

namespace Kiran
{
class XInputHelper
{
public:
    XInputHelper(){};
    virtual ~XInputHelper(){};

    // 判断是否支持xinput扩展
    static bool supports_xinput_devices();
};

}  // namespace Kiran