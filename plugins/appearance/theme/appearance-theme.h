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

#include <queue>

#include "plugins/appearance/theme/theme-monitor.h"
#include "plugins/appearance/theme/theme-parse.h"

namespace Kiran
{
enum ThemeEventType
{
    // 主题添加
    THEME_EVENT_TYPE_ADD,
    // 主题删除
    THEME_EVENT_TYPE_DEL,
    // 主题修改
    THEME_EVENT_TYPE_CHG,
};

// 主题类型和主题名
using ThemeKey = std::pair<int32_t, std::string>;
// 主题类型/主题名/优先级
using ThemeUniqueKey = std::tuple<int32_t, std::string, int32_t>;
// 这里需要指定按照std::less方式排序，优先级'值'越小的排在前面，这样可以直接通过map.begin()->second获取优先级最高的主题
using ThemeKeyValue = std::map<int32_t, std::shared_ptr<ThemeBase>, std::less<int32_t>>;

class AppearanceTheme
{
public:
    AppearanceTheme();
    virtual ~AppearanceTheme(){};

    void init();

    // 获取指定类型的主题列表
    ThemeInfoVec get_themes_by_type(AppearanceThemeType type);
    std::shared_ptr<ThemeBase> get_theme(ThemeUniqueKey unique_key);
    // 如果存在多个类型和名字相同的主题，则返回优先级最高的主题
    std::shared_ptr<ThemeBase> get_theme(ThemeKey key);

    // 设置指定类型(meta/gtk/metacity...)的主题，如果主题不存在则返回失败，否则修改gsettings使主题生效
    bool set_theme(ThemeKey key, CCErrorCode& error_code);
    // 获取指定类型的主题名，直接从gsettings中读取
    std::string get_theme(AppearanceThemeType type);

    sigc::signal<void, ThemeKey> signal_theme_changed() { return this->theme_changed_; };

private:
    bool add_theme(std::shared_ptr<ThemeBase> theme);
    bool replace_theme(std::shared_ptr<ThemeBase> theme);
    bool del_theme(ThemeUniqueKey unique_key);

    void set_meta_theme(std::shared_ptr<ThemeMeta> theme);
    void set_gtk_theme(const std::string& theme_name);
    void set_icon_theme(const std::string& theme_name);
    void set_cursor_theme(const std::string& theme_name);
    void set_metacity_theme(const std::string& theme_name);

    // xsettings插件的settings项发生变化处理
    void on_xsettings_settings_changed(const Glib::ustring& key);
    void on_monitor_event_changed(std::shared_ptr<ThemeMonitorInfo> monitor_info, ThemeMonitorEventType event_type);

private:
    static AppearanceTheme* instance_;

    ThemeMonitor theme_monitor_;

    std::map<ThemeKey, ThemeKeyValue> themes_;

    // 某个类型的主题设置发生了变化
    sigc::signal<void, ThemeKey> theme_changed_;
    // 某个类型的主题列表发生了变化
    sigc::signal<void, ThemeKey, ThemeEventType> themes_changed_;

    Glib::RefPtr<Gio::Settings> xsettings_settings_;
    Glib::RefPtr<Gio::Settings> marco_settings_;
};
}  // namespace Kiran