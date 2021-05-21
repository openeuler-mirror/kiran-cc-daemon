/*
 * @Author       : tangjie02
 * @Date         : 2020-12-01 11:26:30
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-12-02 15:57:55
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/appearance/font/appearance-font.h
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
