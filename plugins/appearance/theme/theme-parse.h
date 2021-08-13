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
#include "plugins/appearance/theme/theme-monitor.h"

namespace Kiran
{
struct ThemeBase
{
    // 主题类型
    AppearanceThemeType type;
    // 主题名
    std::string name;
    // 主题加载优先级
    int32_t priority;
    // 主题路径
    std::string path;
};

struct ThemeMeta : public ThemeBase
{
    std::string gtk_theme;
    std::string metacity_theme;
    std::string icon_theme;
    std::string cursor_theme;
};

using ThemeInfoVec = std::vector<std::shared_ptr<ThemeBase>>;

class ThemeParse
{
public:
    ThemeParse(std::shared_ptr<ThemeMonitorInfo> monitor_info);
    virtual ~ThemeParse(){};

    std::shared_ptr<ThemeBase> parse();
    std::shared_ptr<ThemeBase> parse_base();

private:
    std::shared_ptr<ThemeBase> parse_meta();
    std::shared_ptr<ThemeBase> parse_gtk();
    std::shared_ptr<ThemeBase> parse_metacity();
    std::shared_ptr<ThemeBase> parse_icon();
    std::shared_ptr<ThemeBase> parse_cursor();

    std::shared_ptr<ThemeBase> file_base(std::shared_ptr<ThemeBase> theme_base, AppearanceThemeType type);
    std::string get_theme_path(const std::string& path, AppearanceThemeType type);

private:
    std::shared_ptr<ThemeMonitorInfo> monitor_info_;

    static std::map<ThemeMonitorType, AppearanceThemeType> monitor2theme_;
};
}  // namespace Kiran