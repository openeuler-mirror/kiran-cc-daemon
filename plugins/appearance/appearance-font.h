/*
 * @Author       : tangjie02
 * @Date         : 2020-12-01 11:26:30
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-12-01 15:26:58
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/appearance/appearance-font.h
 */
#pragma once

#include "appearance_i.h"
#include "lib/base/base.h"

namespace Kiran
{
class AppearanceFont
{
public:
    AppearanceFont();
    virtual ~AppearanceFont(){};

    std::string get_font(AppearanceFontType type);
    bool set_font(AppearanceFontType type, const std::string& font_name);

private:
    Glib::RefPtr<Gio::Settings> xsettings_settings_;
    Glib::RefPtr<Gio::Settings> interface_settings_;
    Glib::RefPtr<Gio::Settings> marco_settings_;
    Glib::RefPtr<Gio::Settings> caja_settings_;
};
}  // namespace  Kiran
