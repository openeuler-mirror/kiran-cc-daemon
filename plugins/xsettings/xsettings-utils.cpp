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

#include "xsettings-utils.h"
#include <X11/Xatom.h>
#include <glib.h>
#include <xcb/xcb.h>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QGuiApplication>
#include <QRect>
#include <QScreen>
#include "lib/xcb/xcb-connection.h"
#include "xsettings-common.h"

namespace Kiran
{
double XSettingsUtils::getDPIFromXServer()
{
    auto xcbConnection = XcbConnection::getDefault();
    auto defaultScreen = xcbConnection->getDefaultScreen();

    double dpi = DPI_FALLBACK;

    if (defaultScreen != NULL)
    {
        double widthDPI;
        double heightDPI;

        widthDPI = XSettingsUtils::dpiFromPixelsAndMm(defaultScreen->width_in_pixels, defaultScreen->width_in_millimeters);
        heightDPI = XSettingsUtils::dpiFromPixelsAndMm(defaultScreen->height_in_pixels, defaultScreen->height_in_millimeters);

        if (widthDPI < DPI_LOW_REASONABLE_VALUE ||
            widthDPI > DPI_HIGH_REASONABLE_VALUE ||
            heightDPI < DPI_LOW_REASONABLE_VALUE ||
            heightDPI > DPI_HIGH_REASONABLE_VALUE)
        {
            dpi = DPI_FALLBACK;
        }
        else
        {
            dpi = (widthDPI + heightDPI) / 2.0;
        }
    }

    return dpi;
}

/* Auto-detect the most appropriate scale factor for the primary monitor.
 * A lot of this code is shamelessly copied and adapted from Linux Mint/Cinnamon.
 */
int XSettingsUtils::getWindowScaleAuto()
{
    // 默认值不缩放
    int windowScale = 1;
    auto primaryScreen = QGuiApplication::primaryScreen();
    auto geometry = primaryScreen->geometry();
    auto physicalSize = primaryScreen->physicalSize();

    RETURN_VAL_IF_TRUE(geometry.height() < HIDPI_MIN_HEIGHT, 1);

    /* Some monitors/TV encode the aspect ratio (16/9 or 16/10) instead of the physical size */
    if ((physicalSize.width() == 160 && physicalSize.height() == 90) ||
        (physicalSize.width() == 160 && physicalSize.height() == 100) ||
        (physicalSize.width() == 16 && physicalSize.height() == 9) ||
        (physicalSize.width() == 16 && physicalSize.height() == 10))
        return 1;

    if (physicalSize.width() > 0 && physicalSize.height() > 0)
    {
        auto dpiX = (double)geometry.width() / (physicalSize.width() / 25.4);
        auto dpiY = (double)geometry.height() / (physicalSize.height() / 25.4);
        /* We don't completely trust these values so both must be high, and never pick
         * higher ratio than 2 automatically */
        if (dpiX > HIDPI_LIMIT && dpiY > HIDPI_LIMIT)
            windowScale = 2;
    }

    return windowScale;
}

bool XSettingsUtils::updateUserEnvVariable(const QString &variable, const QString &value)
{
    auto sendMessage = QDBusMessage::createMethodCall("org.gnome.SessionManager",
                                                      "/org/gnome/SessionManager",
                                                      "org.gnome.SessionManager",
                                                      "Setenv");

    sendMessage << variable << value;

    auto replyMessage = QDBusConnection::sessionBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(xsettings) << "Call Setenv return error:" << replyMessage.errorMessage();
        return false;
    }

    return true;
}

double XSettingsUtils::dpiFromPixelsAndMm(int pixels, int mm)
{
    double dpi;

    if (mm >= 1)
        dpi = pixels / (mm / 25.4);
    else
        dpi = 0;

    return dpi;
}

double XSettingsUtils::formatScaleDPI(int scale, double dpi)
{
    return double(CLAMP(dpi * scale, DPI_LOW_REASONABLE_VALUE, DPI_HIGH_REASONABLE_VALUE));
}

}  // namespace  Kiran
