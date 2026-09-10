/**
 * Copyright (c) 2026 ~ 2027 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 */

#include "test/appearance/test-appearance-manager.h"
#include "lib/base/base.h"

#include <appearance-i.h>
#include <cstdlib>

namespace Kiran
{

void AppearanceManagerProxy::SetUp()
{
    const char *session_bus = getenv("DBUS_SESSION_BUS_ADDRESS");
    if (session_bus == nullptr || *session_bus == '\0')
    {
        GTEST_SKIP() << "DBUS_SESSION_BUS_ADDRESS is not set";
    }

    try
    {
        this->appearance_proxy_ = SessionDaemon::AppearanceProxy::createForBus_sync(
            Gio::DBus::BUS_TYPE_SESSION,
            Gio::DBus::PROXY_FLAGS_NONE,
            APPEARANCE_DBUS_NAME,
            APPEARANCE_OBJECT_PATH);
    }
    catch (...)
    {
        GTEST_SKIP() << "session bus is not available";
    }

    auto dbus_proxy = this->appearance_proxy_ ? this->appearance_proxy_->dbusProxy() : Glib::RefPtr<Gio::DBus::Proxy>();
    if (!dbus_proxy || dbus_proxy->get_name_owner().empty())
    {
        GTEST_SKIP() << "appearance daemon is not running";
    }

    ASSERT_NE(!this->appearance_proxy_, true);

    this->original_themes_[APPEARANCE_THEME_TYPE_GTK] = this->appearance_proxy_->GetTheme_sync(APPEARANCE_THEME_TYPE_GTK);
    this->original_themes_[APPEARANCE_THEME_TYPE_METACITY] = this->appearance_proxy_->GetTheme_sync(APPEARANCE_THEME_TYPE_METACITY);
    this->original_themes_[APPEARANCE_THEME_TYPE_ICON] = this->appearance_proxy_->GetTheme_sync(APPEARANCE_THEME_TYPE_ICON);
    this->original_themes_[APPEARANCE_THEME_TYPE_CURSOR] = this->appearance_proxy_->GetTheme_sync(APPEARANCE_THEME_TYPE_CURSOR);

    this->original_fonts_[APPEARANCE_FONT_TYPE_APPLICATION] = this->appearance_proxy_->GetFont_sync(APPEARANCE_FONT_TYPE_APPLICATION);
    this->original_fonts_[APPEARANCE_FONT_TYPE_DOCUMENT] = this->appearance_proxy_->GetFont_sync(APPEARANCE_FONT_TYPE_DOCUMENT);
    this->original_fonts_[APPEARANCE_FONT_TYPE_DESKTOP] = this->appearance_proxy_->GetFont_sync(APPEARANCE_FONT_TYPE_DESKTOP);
    this->original_fonts_[APPEARANCE_FONT_TYPE_WINDOW_TITLE] = this->appearance_proxy_->GetFont_sync(APPEARANCE_FONT_TYPE_WINDOW_TITLE);
    this->original_fonts_[APPEARANCE_FONT_TYPE_MONOSPACE] = this->appearance_proxy_->GetFont_sync(APPEARANCE_FONT_TYPE_MONOSPACE);

    this->original_desktop_background_ = this->appearance_proxy_->desktop_background_get();
    this->original_lockscreen_background_ = this->appearance_proxy_->lock_screen_background_get();

    this->snapshot_valid_ = true;
}

void AppearanceManagerProxy::TearDown()
{
    if (!this->snapshot_valid_ || !this->appearance_proxy_)
    {
        return;
    }

    IGNORE_EXCEPTION(
        {
            for (const auto &iter : this->original_themes_)
            {
                this->appearance_proxy_->SetTheme_sync(iter.first, iter.second);
            }
            for (const auto &iter : this->original_fonts_)
            {
                this->appearance_proxy_->SetFont_sync(iter.first, iter.second);
            }
            this->appearance_proxy_->SetDesktopBackground_sync(this->original_desktop_background_);
            this->appearance_proxy_->SetLockScreenBackground_sync(this->original_lockscreen_background_);
        });
}

TEST_F(AppearanceManagerProxy, SetTheme)
{
    ASSERT_NO_THROW(this->appearance_proxy_->SetTheme_sync(APPEARANCE_THEME_TYPE_GTK, "Kiran-white"));
    auto gtkTheme = this->appearance_proxy_->GetTheme_sync(APPEARANCE_THEME_TYPE_GTK);
    ASSERT_EQ(gtkTheme, "Kiran-white");

    ASSERT_NO_THROW(this->appearance_proxy_->SetTheme_sync(APPEARANCE_THEME_TYPE_METACITY, "Kiran-white"));
    auto metaTheme = this->appearance_proxy_->GetTheme_sync(APPEARANCE_THEME_TYPE_METACITY);
    ASSERT_EQ(metaTheme, "Kiran-white");

    ASSERT_NO_THROW(this->appearance_proxy_->SetTheme_sync(APPEARANCE_THEME_TYPE_ICON, "Summer"));
    auto iconTheme = this->appearance_proxy_->GetTheme_sync(APPEARANCE_THEME_TYPE_ICON);
    ASSERT_EQ(iconTheme, "Summer");

    ASSERT_NO_THROW(this->appearance_proxy_->SetTheme_sync(APPEARANCE_THEME_TYPE_CURSOR, "Kiran"));
    auto cursorTheme = this->appearance_proxy_->GetTheme_sync(APPEARANCE_THEME_TYPE_CURSOR);
    ASSERT_EQ(cursorTheme, "Kiran");
}

TEST_F(AppearanceManagerProxy, SetFont)
{
    ASSERT_NO_THROW(this->appearance_proxy_->SetFont_sync(APPEARANCE_FONT_TYPE_APPLICATION, "Noto Sans CJK SC 10"));
    auto appFont = this->appearance_proxy_->GetFont_sync(APPEARANCE_FONT_TYPE_APPLICATION);
    ASSERT_EQ(appFont, "Noto Sans CJK SC 10");

    ASSERT_NO_THROW(this->appearance_proxy_->SetFont_sync(APPEARANCE_FONT_TYPE_DOCUMENT, "Noto Sans CJK SC 11"));
    auto docFont = this->appearance_proxy_->GetFont_sync(APPEARANCE_FONT_TYPE_DOCUMENT);
    ASSERT_EQ(docFont, "Noto Sans CJK SC 11");

    ASSERT_NO_THROW(this->appearance_proxy_->SetFont_sync(APPEARANCE_FONT_TYPE_DESKTOP, "Noto Sans CJK SC 12"));
    auto desktopFont = this->appearance_proxy_->GetFont_sync(APPEARANCE_FONT_TYPE_DESKTOP);
    ASSERT_EQ(desktopFont, "Noto Sans CJK SC 12");

    ASSERT_NO_THROW(this->appearance_proxy_->SetFont_sync(APPEARANCE_FONT_TYPE_WINDOW_TITLE, "Noto Sans CJK SC 13"));
    auto titleFont = this->appearance_proxy_->GetFont_sync(APPEARANCE_FONT_TYPE_WINDOW_TITLE);
    ASSERT_EQ(titleFont, "Noto Sans CJK SC 13");

    ASSERT_NO_THROW(this->appearance_proxy_->SetFont_sync(APPEARANCE_FONT_TYPE_MONOSPACE, "Monospace 12"));
    auto monoFont = this->appearance_proxy_->GetFont_sync(APPEARANCE_FONT_TYPE_MONOSPACE);
    ASSERT_EQ(monoFont, "Monospace 12");
}

TEST_F(AppearanceManagerProxy, Background)
{
    ASSERT_NO_THROW(this->appearance_proxy_->SetDesktopBackground_sync("test.jpg"));
    ASSERT_NO_THROW(this->appearance_proxy_->SetLockScreenBackground_sync("test.jpg"));

    auto live_proxy = SessionDaemon::AppearanceProxy::createForBus_sync(
        Gio::DBus::BUS_TYPE_SESSION,
        Gio::DBus::PROXY_FLAGS_NONE,
        APPEARANCE_DBUS_NAME,
        APPEARANCE_OBJECT_PATH);
    ASSERT_NE(!live_proxy, true);
    ASSERT_EQ(live_proxy->desktop_background_get(), "test.jpg");
    ASSERT_EQ(live_proxy->lock_screen_background_get(), "test.jpg");
}

}  // namespace Kiran
