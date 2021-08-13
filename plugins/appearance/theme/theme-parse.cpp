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

#include "plugins/appearance/theme/theme-parse.h"

#include <gtkmm.h>
namespace Kiran
{
#define META_GROUP_NAME "X-GNOME-Metatheme"

std::map<ThemeMonitorType, AppearanceThemeType> ThemeParse::monitor2theme_ = {
    {ThemeMonitorType::THEME_MONITOR_TYPE_META, APPEARANCE_THEME_TYPE_META},
    {ThemeMonitorType::THEME_MONITOR_TYPE_GTK, APPEARANCE_THEME_TYPE_GTK},
    {ThemeMonitorType::THEME_MONITOR_TYPE_METACITY, APPEARANCE_THEME_TYPE_METACITY},
    {ThemeMonitorType::THEME_MONITOR_TYPE_ICON, APPEARANCE_THEME_TYPE_ICON},
    {ThemeMonitorType::THEME_MONITOR_TYPE_CURSOR, APPEARANCE_THEME_TYPE_CURSOR}};

ThemeParse::ThemeParse(std::shared_ptr<ThemeMonitorInfo> monitor_info) : monitor_info_(monitor_info)
{
}

std::shared_ptr<ThemeBase> ThemeParse::parse()
{
    switch (this->monitor_info_->get_type())
    {
    case ThemeMonitorType::THEME_MONITOR_TYPE_META:
        return this->parse_meta();
    case ThemeMonitorType::THEME_MONITOR_TYPE_GTK:
        return this->parse_gtk();
    case ThemeMonitorType::THEME_MONITOR_TYPE_METACITY:
        return this->parse_metacity();
    case ThemeMonitorType::THEME_MONITOR_TYPE_ICON:
        return this->parse_icon();
    case ThemeMonitorType::THEME_MONITOR_TYPE_CURSOR:
        return this->parse_cursor();
        break;
    default:
        return nullptr;
    }
}

std::shared_ptr<ThemeBase> ThemeParse::parse_base()
{
    auto iter = ThemeParse::monitor2theme_.find(this->monitor_info_->get_type());
    RETURN_VAL_IF_TRUE(iter == ThemeParse::monitor2theme_.end(), nullptr);

    auto base = std::make_shared<ThemeBase>();
    base->type = iter->second;
    base->priority = this->monitor_info_->get_priority();
    base->path = this->get_theme_path(this->monitor_info_->get_path(), base->type);
    base->name = Glib::path_get_basename(base->path);
    return base;
}

std::shared_ptr<ThemeBase> ThemeParse::parse_meta()
{
    auto index_file = Glib::build_filename(std::vector<std::string>{this->monitor_info_->get_path(), "index.theme"});
    RETURN_VAL_IF_FALSE(Glib::file_test(index_file, Glib::FILE_TEST_IS_REGULAR), nullptr);

    auto meta = std::make_shared<ThemeMeta>();
    this->file_base(meta, AppearanceThemeType::APPEARANCE_THEME_TYPE_META);

    // Gtk/Metacity/Icon三个主题必须设置，否则解析失败；Cursor主题可选

    Glib::KeyFile key_file;
    try
    {
        key_file.load_from_file(index_file);
        meta->gtk_theme = key_file.get_string(META_GROUP_NAME, "GtkTheme");
        meta->metacity_theme = key_file.get_string(META_GROUP_NAME, "MetacityTheme");
        meta->icon_theme = key_file.get_string(META_GROUP_NAME, "IconTheme");
    }
    catch (const Glib::Error& e)
    {
        KLOG_DEBUG("%s", e.what().c_str());
        return nullptr;
    }

    IGNORE_EXCEPTION(meta->cursor_theme = key_file.get_string(META_GROUP_NAME, "CursorTheme"));

    return meta;
}

std::shared_ptr<ThemeBase> ThemeParse::parse_gtk()
{
    std::string css_file;
    if (gtk_get_major_version() == GTK2_MAJOR)
    {
        css_file = Glib::build_filename(std::vector<std::string>{this->monitor_info_->get_path(), "gtkrc"});
    }
    else
    {
        css_file = Glib::build_filename(std::vector<std::string>{this->monitor_info_->get_path(), "gtk.css"});
    }
    RETURN_VAL_IF_FALSE(Glib::file_test(css_file, Glib::FILE_TEST_IS_REGULAR), nullptr);

    std::shared_ptr<ThemeBase> base = std::make_shared<ThemeBase>();
    return this->file_base(base, AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK);
}

std::shared_ptr<ThemeBase> ThemeParse::parse_metacity()
{
    auto theme_1_file = Glib::build_filename(std::vector<std::string>{this->monitor_info_->get_path(), "metacity-theme-1.xml"});
    auto theme_2_file = Glib::build_filename(std::vector<std::string>{this->monitor_info_->get_path(), "metacity-theme-1.xml"});
    auto theme_3_file = Glib::build_filename(std::vector<std::string>{this->monitor_info_->get_path(), "metacity-theme-3.xml"});

    if (!Glib::file_test(theme_1_file, Glib::FILE_TEST_IS_REGULAR) &&
        !Glib::file_test(theme_2_file, Glib::FILE_TEST_IS_REGULAR) &&
        !Glib::file_test(theme_3_file, Glib::FILE_TEST_IS_REGULAR))
    {
        return nullptr;
    }

    std::shared_ptr<ThemeBase> base = std::make_shared<ThemeBase>();
    return this->file_base(base, AppearanceThemeType::APPEARANCE_THEME_TYPE_METACITY);
}

std::shared_ptr<ThemeBase> ThemeParse::parse_icon()
{
    auto index_file = Glib::build_filename(std::vector<std::string>{this->monitor_info_->get_path(), "index.theme"});
    RETURN_VAL_IF_FALSE(Glib::file_test(index_file, Glib::FILE_TEST_IS_REGULAR), nullptr);

    std::shared_ptr<ThemeBase> base = std::make_shared<ThemeBase>();
    return this->file_base(base, AppearanceThemeType::APPEARANCE_THEME_TYPE_ICON);
}

std::shared_ptr<ThemeBase> ThemeParse::parse_cursor()
{
    auto left_ptr_file = Glib::build_filename(std::vector<std::string>{this->monitor_info_->get_path(), "left_ptr"});
    RETURN_VAL_IF_FALSE(Glib::file_test(left_ptr_file, Glib::FILE_TEST_IS_REGULAR), nullptr);

    std::shared_ptr<ThemeBase> base = std::make_shared<ThemeBase>();
    return this->file_base(base, AppearanceThemeType::APPEARANCE_THEME_TYPE_CURSOR);
}

std::shared_ptr<ThemeBase> ThemeParse::file_base(std::shared_ptr<ThemeBase> theme_base, AppearanceThemeType type)
{
    theme_base->type = type;
    theme_base->priority = this->monitor_info_->get_priority();
    theme_base->path = this->get_theme_path(this->monitor_info_->get_path(), theme_base->type);
    theme_base->name = Glib::path_get_basename(theme_base->path);
    return theme_base;
}

std::string ThemeParse::get_theme_path(const std::string& monitor_path, AppearanceThemeType type)
{
    if (type == APPEARANCE_THEME_TYPE_META ||
        type == APPEARANCE_THEME_TYPE_ICON)
    {
        return monitor_path;
    }
    else
    {
        return Glib::path_get_dirname(monitor_path);
    }
}
}  // namespace Kiran
