/*
 * @Author       : tangjie02
 * @Date         : 2020-12-01 10:15:50
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-12-01 16:13:14
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/appearance/appearance-manager.cpp
 */
#include "plugins/appearance/appearance-manager.h"

#include <glib/gi18n.h>

namespace Kiran
{
AppearanceManager::AppearanceManager()
{
    this->font_ = std::make_shared<AppearanceFont>();
}

AppearanceManager* AppearanceManager::instance_ = nullptr;
void AppearanceManager::global_init()
{
    instance_ = new AppearanceManager();
    instance_->init();
}

void AppearanceManager::init()
{
}

void AppearanceManager::GetFont(gint32 type, MethodInvocation& invocation)
{
    if (type < 0 || type >= int32_t(AppearanceFontType::APPEARANCE_FONT_TYPE_LAST))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, _("The font type is invalid"));
    }
    invocation.ret(this->font_->get_font(AppearanceFontType(type)));
}

void AppearanceManager::SetFont(gint32 type, const Glib::ustring& font_name, MethodInvocation& invocation)
{
    if (type < 0 || type >= int32_t(AppearanceFontType::APPEARANCE_FONT_TYPE_LAST))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, _("The font type is invalid"));
    }

    if (!this->font_->set_font(AppearanceFontType(type), font_name))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, _("The font type is unsupported"));
    }
    invocation.ret();
}

}  // namespace  Kiran
