/**
 * @file          /kiran-cc-daemon/plugins/appearance/theme/appearance-theme.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/appearance/theme/appearance-theme.h"

#include "lib/base/base.h"
#include "xsettings-i.h"

#define MARCO_SCHEMA_ID "org.mate.Marco.general"
#define MARCO_SCHEMA_KEY_THEME "theme"

namespace Kiran
{
AppearanceTheme::AppearanceTheme()
{
    this->xsettings_settings_ = Gio::Settings::create(XSETTINGS_SCHEMA_ID);
    this->marco_settings_ = Gio::Settings::create(MARCO_SCHEMA_ID);
}

void AppearanceTheme::init()
{
    this->theme_monitor_.init();

    for (auto& monitor_info : this->theme_monitor_.get_monitor_infos())
    {
        ThemeParse parse(monitor_info);
        auto theme = parse.parse();
        if (theme)
        {
            // LOG_DEBUG("path: %s type: %d", theme->path.c_str(), theme->type);
            this->add_theme(theme);
        }
    }

    this->theme_monitor_.signal_monitor_event().connect(sigc::mem_fun(this, &AppearanceTheme::on_monitor_event_changed));
}

ThemeInfoVec AppearanceTheme::get_themes_by_type(AppearanceThemeType type)
{
    ThemeInfoVec themes;
    for (auto& iter : this->themes_)
    {
        // map结构是按照优先级排序的，所以如果存在相同类型的主题，只取优先级最高的主题
        if (iter.first.first == type &&
            iter.second.size() > 0)
        {
            themes.push_back(iter.second.begin()->second);
        }
    }
    return themes;
}

std::shared_ptr<ThemeBase> AppearanceTheme::get_theme(ThemeUniqueKey unique_key)
{
    ThemeKey key = std::make_pair(std::get<0>(unique_key), std::get<1>(unique_key));
    auto themes_iter = this->themes_.find(key);
    RETURN_VAL_IF_TRUE(themes_iter == this->themes_.end(), nullptr);

    auto theme_iter = themes_iter->second.find(std::get<2>(unique_key));
    RETURN_VAL_IF_TRUE(theme_iter == themes_iter->second.end(), nullptr);
    return theme_iter->second;
}

std::shared_ptr<ThemeBase> AppearanceTheme::get_theme(ThemeKey key)
{
    auto themes_iter = this->themes_.find(key);
    RETURN_VAL_IF_TRUE(themes_iter == this->themes_.end(), nullptr);
    RETURN_VAL_IF_TRUE(themes_iter->second.size() == 0, nullptr);

    return themes_iter->second.begin()->second;
}

bool AppearanceTheme::set_theme(ThemeKey key, CCErrorCode& error_code)
{
    auto theme = this->get_theme(key);
    if (!theme)
    {
        error_code = CCErrorCode::ERROR_APPEARANCE_THEME_NOT_EXIST;
        return false;
    }

    switch (theme->type)
    {
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_META:
        this->set_meta_theme(std::static_pointer_cast<ThemeMeta>(theme));
        break;
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK:
    {
        if (this->xsettings_settings_ && theme->name.length() > 0)
        {
            this->xsettings_settings_->set_string(XSETTINGS_SCHEMA_NET_THEME_NAME, theme->name);
        }
        break;
    }
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_METACITY:
    {
        if (this->marco_settings_ && theme->name.length() > 0)
        {
            this->marco_settings_->set_string(MARCO_SCHEMA_KEY_THEME, theme->name);
        }
        break;
    }
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_ICON:
    {
        if (this->xsettings_settings_ && theme->name.length() > 0)
        {
            this->xsettings_settings_->set_string(XSETTINGS_SCHEMA_NET_ICON_THEME_NAME, theme->name);
        }
        break;
    }
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_CURSOR:
    {
        if (this->xsettings_settings_ && theme->name.length() > 0)
        {
            this->xsettings_settings_->set_string(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, theme->name);
        }
        break;
    }
    default:
        error_code = CCErrorCode::ERROR_APPEARANCE_THEME_TYPE_UNSUPPORTED;
        return false;
    }
    return true;
}

std::string AppearanceTheme::get_theme(AppearanceThemeType type)
{
    SETTINGS_PROFILE("type: %d.", type);

    switch (type)
    {
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK:
    {
        if (this->xsettings_settings_)
        {
            return this->xsettings_settings_->get_string(XSETTINGS_SCHEMA_NET_THEME_NAME).raw();
        }
        break;
    }
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_METACITY:
    {
        if (this->marco_settings_)
        {
            return this->marco_settings_->get_string(MARCO_SCHEMA_KEY_THEME);
        }
        break;
    }
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_ICON:
    {
        if (this->xsettings_settings_)
        {
            return this->xsettings_settings_->get_string(XSETTINGS_SCHEMA_NET_ICON_THEME_NAME);
        }
        break;
    }
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_CURSOR:
    {
        if (this->xsettings_settings_)
        {
            return this->xsettings_settings_->get_string(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME);
        }
        break;
    }
    default:
        break;
    }
    return std::string();
}

bool AppearanceTheme::add_theme(std::shared_ptr<ThemeBase> theme)
{
    RETURN_VAL_IF_FALSE(theme, false);
    ThemeKey key = std::make_pair(theme->type, theme->name);
    auto iter = this->themes_[key].emplace(theme->priority, theme);
    return iter.second;
}

bool AppearanceTheme::replace_theme(std::shared_ptr<ThemeBase> theme)
{
    RETURN_VAL_IF_FALSE(theme, false);
    ThemeKey key = std::make_pair(theme->type, theme->name);
    this->themes_[key][theme->priority] = theme;
    return true;
}

bool AppearanceTheme::del_theme(ThemeUniqueKey unique_key)
{
    ThemeKey key = std::make_pair(std::get<0>(unique_key), std::get<1>(unique_key));
    auto themes_iter = this->themes_.find(key);
    if (themes_iter != this->themes_.end())
    {
        auto theme_iter = themes_iter->second.find(std::get<2>(unique_key));
        if (theme_iter != themes_iter->second.end())
        {
            themes_iter->second.erase(theme_iter);
            return true;
        }
    }
    // LOG_DEBUG("Not found the theme %s.", key.second);
    return false;
}

void AppearanceTheme::set_meta_theme(std::shared_ptr<ThemeMeta> theme)
{
    if (this->xsettings_settings_)
    {
        if (theme->gtk_theme.length() > 0)
        {
            this->xsettings_settings_->set_string(XSETTINGS_SCHEMA_NET_THEME_NAME, theme->gtk_theme);
        }

        if (theme->icon_theme.length() > 0)
        {
            this->xsettings_settings_->set_string(XSETTINGS_SCHEMA_NET_ICON_THEME_NAME, theme->icon_theme);
        }

        if (theme->cursor_theme.length() > 0)
        {
            this->xsettings_settings_->set_string(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, theme->cursor_theme);
        }
    }

    if (theme->metacity_theme.length() > 0 && this->marco_settings_)
    {
        this->marco_settings_->set_string(MARCO_SCHEMA_KEY_THEME, theme->metacity_theme);
    }
}

void AppearanceTheme::on_monitor_event_changed(std::shared_ptr<ThemeMonitorInfo> monitor_info,
                                               ThemeMonitorEventType event_type)
{
    ThemeParse parse(monitor_info);
    auto base = parse.parse_base();
    RETURN_IF_FALSE(base);

    auto key = std::make_pair(base->type, base->name);
    auto old_theme = this->get_theme(key);
    auto parsed_theme = parse.parse();

    if (parsed_theme)
    {
        this->replace_theme(parsed_theme);
    }
    else
    {
        ThemeUniqueKey unique_key = std::make_tuple(base->type, base->name, monitor_info->get_priority());
        this->del_theme(unique_key);
    }

    // 需要在执行替换或删除操作后再进行获取，因为不确定变动的是否为最高优先级的主题
    auto new_theme = this->get_theme(key);

    if (old_theme && !new_theme)
    {
        this->themes_changed_.emit(key, ThemeEventType::THEME_EVENT_TYPE_DEL);
    }
    else if (!old_theme && new_theme)
    {
        this->themes_changed_.emit(key, ThemeEventType::THEME_EVENT_TYPE_ADD);
    }
    else if (old_theme && new_theme)
    {
        this->themes_changed_.emit(key, ThemeEventType::THEME_EVENT_TYPE_CHG);
    }
}
}  // namespace Kiran