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

#include "appearance-i.h"
#include "lib/base/base.h"

namespace Kiran
{
class AppearanceFont
{
public:
    AppearanceFont();
    virtual ~AppearanceFont(){};

    void init();

    std::string get_font(AppearanceFontType type);
    bool set_font(AppearanceFontType type, const std::string& font);

private:
    Glib::RefPtr<Gio::Settings> xsettings_settings_;
    Glib::RefPtr<Gio::Settings> interface_settings_;
    Glib::RefPtr<Gio::Settings> marco_settings_;
    Glib::RefPtr<Gio::Settings> caja_settings_;
};
}  // namespace  Kiran
