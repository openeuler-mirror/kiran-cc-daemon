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
