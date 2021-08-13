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
