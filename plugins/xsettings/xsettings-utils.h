/*
 * @Author       : tangjie02
 * @Date         : 2020-11-24 11:15:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-24 17:04:23
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/xsettings/xsettings-utils.h
 */
#pragma once

#include "lib/base/base.h"
namespace Kiran
{
class XSettingsUtils
{
public:
    XSettingsUtils(){};
    virtual ~XSettingsUtils(){};

    static double get_dpi_from_x_server();
    static int get_window_scale_auto();

    static bool update_user_env_variable(const std::string &variable,
                                         const std::string &value,
                                         std::string &error);

private:
    static double dpi_from_pixels_and_mm(int pixels, int mm);
};
}  // namespace Kiran
