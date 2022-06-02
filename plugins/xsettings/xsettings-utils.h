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
