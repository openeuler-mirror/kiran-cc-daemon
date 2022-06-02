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

#include "plugins/appearance/font/appearance-font.h"

#include "xsettings-i.h"

namespace Kiran
{
#define MARCO_SCHEMA_ID "org.mate.Marco.general"
#define MARCO_SCHAME_KEY_TITLEBAR_FONT "titlebar-font"

#define CAJA_SCHEMA_ID "org.mate.caja.desktop"
#define CAJA_SCHEMA_KEY_FONT "font"

#define INTERFACE_SCHEMA_ID "org.mate.interface"
#define INTERFACE_KEY_DOCUMENT_FONT_NAME "document-font-name"
#define INTERFACE_KEY_MONOSPACE_FONT_NAME "monospace-font-name"

AppearanceFont::AppearanceFont()
{
    this->xsettings_settings_ = Gio::Settings::create(XSETTINGS_SCHEMA_ID);
    this->interface_settings_ = Gio::Settings::create(INTERFACE_SCHEMA_ID);
    this->marco_settings_ = Gio::Settings::create(MARCO_SCHEMA_ID);
    this->caja_settings_ = Gio::Settings::create(CAJA_SCHEMA_ID);
}

void AppearanceFont::init()
{
}

std::string AppearanceFont::get_font(AppearanceFontType type)
{
    switch (type)
    {
    case APPEARANCE_FONT_TYPE_APPLICATION:
    {
        RETURN_VAL_IF_FALSE(this->xsettings_settings_, std::string());
        return this->xsettings_settings_->get_string(XSETTINGS_SCHEMA_GTK_FONT_NAME).raw();
    }
    case APPEARANCE_FONT_TYPE_DOCUMENT:
    {
        RETURN_VAL_IF_FALSE(this->interface_settings_, std::string());
        return this->interface_settings_->get_string(INTERFACE_KEY_DOCUMENT_FONT_NAME).raw();
    }
    case APPEARANCE_FONT_TYPE_DESKTOP:
    {
        RETURN_VAL_IF_FALSE(this->caja_settings_, std::string());
        return this->caja_settings_->get_string(CAJA_SCHEMA_KEY_FONT).raw();
    }
    case APPEARANCE_FONT_TYPE_WINDOW_TITLE:
    {
        RETURN_VAL_IF_FALSE(this->marco_settings_, std::string());
        return this->marco_settings_->get_string(MARCO_SCHAME_KEY_TITLEBAR_FONT).raw();
    }
    case APPEARANCE_FONT_TYPE_MONOSPACE:
    {
        RETURN_VAL_IF_FALSE(this->interface_settings_, std::string());
        return this->interface_settings_->get_string(INTERFACE_KEY_MONOSPACE_FONT_NAME).raw();
    }
    default:
        return std::string();
    }
}

bool AppearanceFont::set_font(AppearanceFontType type, const std::string& font)
{
    KLOG_PROFILE("type: %d.", type);

    switch (type)
    {
    case APPEARANCE_FONT_TYPE_APPLICATION:
    {
        RETURN_VAL_IF_FALSE(this->xsettings_settings_, false);
        this->xsettings_settings_->set_string(XSETTINGS_SCHEMA_GTK_FONT_NAME, font);
        break;
    }
    case APPEARANCE_FONT_TYPE_DOCUMENT:
    {
        RETURN_VAL_IF_FALSE(this->interface_settings_, false);
        this->interface_settings_->set_string(INTERFACE_KEY_DOCUMENT_FONT_NAME, font);
        break;
    }
    case APPEARANCE_FONT_TYPE_DESKTOP:
    {
        RETURN_VAL_IF_FALSE(this->caja_settings_, false);
        this->caja_settings_->set_string(CAJA_SCHEMA_KEY_FONT, font);
        break;
    }
    case APPEARANCE_FONT_TYPE_WINDOW_TITLE:
    {
        RETURN_VAL_IF_FALSE(this->marco_settings_, false);
        this->marco_settings_->set_string(MARCO_SCHAME_KEY_TITLEBAR_FONT, font);
        break;
    }
    case APPEARANCE_FONT_TYPE_MONOSPACE:
    {
        RETURN_VAL_IF_FALSE(this->interface_settings_, false);
        this->interface_settings_->set_string(INTERFACE_KEY_MONOSPACE_FONT_NAME, font);
        break;
    }
    default:
        return false;
    }
    return true;
}
}  // namespace Kiran
