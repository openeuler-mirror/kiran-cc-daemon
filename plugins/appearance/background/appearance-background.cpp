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

#include "appearance-background.h"
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <QApplication>
#include <QGSettings>
#include <QImage>
#include <QPainter>
#include <QScreen>
#include <QTimer>
#include "appearance-i.h"
#include "lib/base/base.h"
#include "lib/xcb/xcb-connection.h"

namespace Kiran
{
#define MATE_BACKGROUND_SCHEMA_ID "org.mate.background"
#define MATE_BACKGROUND_SCHAME_KEY_PICTURE_FILENAME "pictureFilename"
#define MATE_BACKGROUND_SCHAME_KEY_SHOW_DESKTOP_ICONS "showDesktopIcons"

AppearanceBackground::AppearanceBackground(QObject *parent) : QObject(parent)
{
    m_delayTimer = new QTimer(this);
    m_mateBackgroundSettings = new QGSettings(MATE_BACKGROUND_SCHEMA_ID, "", this);
    m_appearanceSettings = new QGSettings(APPEARANCE_SCHAME_ID, "", this);
}

void AppearanceBackground::init()
{
    setBackground(m_appearanceSettings->get(APPEARANCE_SCHEMA_KEY_DESKTOP_BG).toString());

    connect(m_delayTimer, &QTimer::timeout, this, &AppearanceBackground::drawBackground);
    connect(m_mateBackgroundSettings, &QGSettings::changed, this, &AppearanceBackground::processMateBackgroundSettingsChanged);
    connect(m_appearanceSettings, &QGSettings::changed, this, &AppearanceBackground::processAppearanceSettingsChanged);

    auto primaryScreen = QApplication::primaryScreen();
    connect(primaryScreen, &QScreen::virtualGeometryChanged, this, &AppearanceBackground::delayDrawBackground);
}

void AppearanceBackground::setBackground(const QString &path)
{
    RETURN_IF_TRUE(m_desktopBackground == path);
    m_desktopBackground = path;

    // 兼容老的mate桌面设置
    m_mateBackgroundSettings->set(MATE_BACKGROUND_SCHAME_KEY_PICTURE_FILENAME, path);

    // TODO: 需要完善画背景逻辑
    // drawBackground();
}

void AppearanceBackground::delayDrawBackground()
{
    if (!m_delayTimer->isActive())
    {
        m_delayTimer->start(10);
    }
}

void AppearanceBackground::drawBackground()
{
    RETURN_IF_FALSE(canDrawBackground());

    m_delayTimer->stop();

    KLOG_INFO(appearance) << "Draw" << m_desktopBackground << "to desktop background.";

    auto xcbConnection = XcbConnection::getDefault();
    auto xcbDefaultScreen = xcbConnection->getDefaultScreen();

    QImage desktopImage(xcbDefaultScreen->width_in_pixels, xcbDefaultScreen->height_in_pixels, QImage::Format_RGB32);
    QImage fileImage(m_desktopBackground);
    QPainter painter(&desktopImage);

    auto screens = qGuiApp->screens();
    for (const auto &screen : screens)
    {
        auto geometry = screen->geometry();
        painter.drawImage(geometry, fileImage);
    }

    auto xpixmap = image2XPixmap(desktopImage);

    // xcb_pixmap_t xpixmap = xcb_generate_id(xcbConnection->getConnection());
    // xcb_create_pixmap(xcbConnection->getConnection(),
    //                   xcbDefaultScreen->root_depth,
    //                   xpixmap,
    //                   xcbDefaultScreen->root,
    //                   xcbDefaultScreen->width_in_pixels,
    //                   xcbDefaultScreen->height_in_pixels);

    // xcb_gcontext_t gc = xcb_generate_id(xcbConnection->getConnection());
    // uint32_t valueList[] = {xcbDefaultScreen->white_pixel};
    // uint32_t redColor[] = {0xFF0000};
    // xcb_change_gc(xcbConnection->getConnection(),
    //               gc,
    //               XCB_GC_FOREGROUND,
    //               redColor);
    // xcb_rectangle_t rect = {0, 0, xcbDefaultScreen->width_in_pixels, xcbDefaultScreen->height_in_pixels};
    // xcb_poly_fill_rectangle(xcbConnection->getConnection(),
    //                         xpixmap,
    //                         gc,
    //                         1,
    //                         &rect);

    xcb_grab_server(xcbConnection->getConnection());
    setXPixmapToRoot(xpixmap);

    xcb_change_window_attributes(xcbConnection->getConnection(),
                                 xcbDefaultScreen->root,
                                 XCB_CW_BACK_PIXMAP, &xpixmap);

    xcb_clear_area(xcbConnection->getConnection(),
                   0,
                   xcbDefaultScreen->root,
                   0,
                   0,
                   xcbDefaultScreen->width_in_pixels,
                   xcbDefaultScreen->height_in_pixels);

    xcb_ungrab_server(xcbConnection->getConnection());

    xcb_flush(xcbConnection->getConnection());
}

bool AppearanceBackground::canDrawBackground()
{
    /* caja也会画背景，这里需要判断一下caja是否会去画桌面背景。
    如果org.mate.background中的show-desktop-icons字段为false，则caja不会去画桌面背景，此时插件需要负责去画背景。*/
    if (!m_mateBackgroundSettings->get(MATE_BACKGROUND_SCHAME_KEY_SHOW_DESKTOP_ICONS).toBool())
    {
        return true;
    }

    // 暂时不支持wayland绘制背景
    if (qGuiApp->platformName() != "xcb")
    {
        return false;
    }

    auto xcbConnection = XcbConnection::getDefault();
    auto xcbDefaultScreen = xcbConnection->getDefaultScreen();
    auto rootWindow = xcbDefaultScreen->root;

    auto cajaXidAtomReply = XCB_REPLY(xcb_intern_atom,
                                      xcbConnection->getConnection(),
                                      true,
                                      strlen("CAJA_DESKTOP_WINDOW_ID"),
                                      "CAJA_DESKTOP_WINDOW_ID");

    RETURN_VAL_IF_TRUE(!cajaXidAtomReply, true);

    auto cajaXidReply = XCB_REPLY(xcb_get_property,
                                  xcbConnection->getConnection(),
                                  false,
                                  rootWindow,
                                  cajaXidAtomReply->atom,
                                  XCB_ATOM_WINDOW,
                                  0,
                                  1);
    RETURN_VAL_IF_TRUE(!cajaXidReply || cajaXidReply->type != XCB_ATOM_WINDOW || cajaXidReply->format != 32, true);
    auto cajaXid = *((xcb_window_t *)xcb_get_property_value(cajaXidReply.get()));

    auto cajaWmClassAtomReply = XCB_REPLY(xcb_intern_atom,
                                          xcbConnection->getConnection(),
                                          true,
                                          strlen("WM_CLASS"),
                                          "WM_CLASS");
    RETURN_VAL_IF_TRUE(!cajaWmClassAtomReply, true);

    auto cajaNameReply = XCB_REPLY(xcb_get_property,
                                   xcbConnection->getConnection(),
                                   false,
                                   cajaXid,
                                   cajaWmClassAtomReply->atom,
                                   XCB_ATOM_STRING,
                                   0,
                                   20);
    // caja代码位置：src/caja-desktop-window.c:caja_desktop_window_new()
    if (cajaNameReply &&
        cajaNameReply->value_len == 20 &&
        cajaNameReply->bytes_after == 0 &&
        cajaNameReply->format == 8 &&
        cajaNameReply->type == XCB_ATOM_STRING)
    {
        auto data = xcb_get_property_value(cajaNameReply.get());
        if (!strcmp(reinterpret_cast<char *>(data), "desktop_window") &&
            !strcmp(reinterpret_cast<char *>(data) + strlen(reinterpret_cast<char *>(data)) + 1, "Caja"))
        {
            return false;
        }
    }

    return true;
}

xcb_pixmap_t AppearanceBackground::image2XPixmap(const QImage &image)
{
    auto xcbConnection = XcbConnection::getDefault();
    auto xcbDefaultScreen = xcbConnection->getDefaultScreen();
    auto rootWindow = xcbDefaultScreen->root;

    QImage bitmap = image.convertToFormat(QImage::Format_MonoLSB);
    const QRgb c0 = QColor(Qt::black).rgb();
    const QRgb c1 = QColor(Qt::white).rgb();
    if (bitmap.color(0) == c0 && bitmap.color(1) == c1)
    {
        bitmap.invertPixels();
        bitmap.setColor(0, c1);
        bitmap.setColor(1, c0);
    }
    const int width = bitmap.width();
    const int height = bitmap.height();
    const int bytesPerLine = bitmap.bytesPerLine();
    // KLOG_INFO(appearance) << bitmap.width() << bitmap.height() << bitmap.bytesPerLine();
    int destLineSize = width / 8;
    if (width % 8)
        ++destLineSize;
    const uchar *map = bitmap.bits();
    uint8_t *buf = new uint8_t[height * destLineSize];
    for (int i = 0; i < height; i++)
        memcpy(buf + (destLineSize * i), map + (bytesPerLine * i), destLineSize);
    xcb_pixmap_t pm = xcb_create_pixmap_from_bitmap_data(xcbConnection->getConnection(),
                                                         rootWindow,
                                                         buf,
                                                         width,
                                                         height,
                                                         1,
                                                         0,
                                                         0,
                                                         nullptr);
    delete[] buf;
    return pm;
}

// Cairo::RefPtr<Cairo::XlibSurface> AppearanceBackground::create_surface(Glib::RefPtr<Gdk::Screen> screen)
// {
//     RETURN_VAL_IF_FALSE(screen, Cairo::RefPtr<Cairo::XlibSurface>());

//     auto window = screen->get_root_window();
//     auto scale = window->get_scale_factor();
//     auto xscreen = gdk_x11_screen_get_xscreen(screen->gobj());
//     auto width = WidthOfScreen(xscreen) / scale;
//     auto height = HeightOfScreen(xscreen) / scale;
//     auto surface = create_surface_by_size(window, width, height);
//     auto cairo = Cairo::Context::create(surface);

//     cairo->scale(scale, scale);

//     auto pixbuf = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, FALSE, 8, width, height);
//     draw_to_pixbuf(pixbuf, screen);
//     Gdk::Cairo::set_source_pixbuf(cairo, pixbuf, 0, 0);
//     cairo->paint();
//     return surface;
// }

// Cairo::RefPtr<Cairo::XlibSurface> AppearanceBackground::create_surface_by_size(Glib::RefPtr<Gdk::Window> window,
//                                                                                int32_t width,
//                                                                                int32_t height)
// {
//     KLOG_DEBUG_APPEARANCE("Create surface which size is %dx%d", width, height);

//     /* 这里会通过display来创建pixmap对象，pixmap对象会存放到根窗口的"_XROOTPMAP_ID"和"ESETROOT_PMAP_ID"属性当中。
//     其他应用程序也可能会设置根窗口的这个属性，在设置之前会通过XKillClient(display,source)函数释放调之前设置的
//     pixmap资源，同时这个函数也会将pixmap资源对应的客户端与display断开连接， 因此我们没有使用默认的display，而是创
//     建一个新的display连接，避免影响到默认的display连接，然后将断开模式设置为RetainPermanent，
//     保证在与新的display断开连接时pixmap资源不被销毁。、    */

//     auto screen = window->get_screen();
//     auto display_name = window->get_display()->get_name();

//     auto dummy_display = XOpenDisplay(display_name.c_str());

//     if (dummy_display == NULL)
//     {
//         KLOG_WARNING_APPEARANCE("Failed to open display '%s'", display_name.c_str());
//         return Cairo::RefPtr<Cairo::XlibSurface>();
//     }

//     auto depth = DefaultDepth(dummy_display, gdk_x11_screen_get_screen_number(screen->gobj()));
//     auto pixmap = XCreatePixmap(dummy_display, GDK_WINDOW_XID(window->gobj()), width, height, depth);

//     XFlush(dummy_display);
//     XSetCloseDownMode(dummy_display, RetainPermanent);
//     XCloseDisplay(dummy_display);

//     auto xlib_surface = Cairo::XlibSurface::create(GDK_SCREEN_XDISPLAY(screen->gobj()),
//                                                    pixmap,
//                                                    GDK_VISUAL_XVISUAL(screen->get_system_visual()->gobj()),
//                                                    width,
//                                                    height);

//     return xlib_surface;
// }

// void AppearanceBackground::draw_to_pixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, Glib::RefPtr<Gdk::Screen> screen)
// {
//     auto display = screen->get_display();

//     for (int32_t i = 0; i < display->get_n_monitors(); ++i)
//     {
//         auto monitor = display->get_monitor(i);
//         Gdk::Rectangle geometry;

//         monitor->get_geometry(geometry);

//         auto monitor_pixbuf = get_pixbuf_by_file(m_desktopBackground, geometry.get_width(), geometry.get_height());
//         if (monitor_pixbuf)
//         {
//             auto dest_x = geometry.get_x() + (geometry.get_width() - monitor_pixbuf->get_width()) / 2;
//             auto dest_y = geometry.get_y() + (geometry.get_height() - monitor_pixbuf->get_height()) / 2;
//             blend_pixbuf(monitor_pixbuf, pixbuf, dest_x, dest_y, 1.0);
//         }
//     }
// }

// Glib::RefPtr<Gdk::Pixbuf> AppearanceBackground::get_pixbuf_by_file(const QString &file_name, int32_t width, int32_t height)
// {
//     // 先从缓存中读取
//     auto cache_pixbuf = m_backgroundCache.get_pixbuf(file_name, width, height);
//     RETURN_VAL_IF_TRUE(cache_pixbuf, cache_pixbuf);

//     auto pixbuf_format = Glib::wrap(gdk_pixbuf_get_file_info(file_name.c_str(), NULL, NULL), true);
//     QString format_name;
//     Glib::RefPtr<Gdk::Pixbuf> pixbuf;

//     if (pixbuf_format.gobj() != NULL)
//     {
//         format_name = pixbuf_format.get_name();
//     }

//     try
//     {
//         if (format_name == "svg")
//         {
//             pixbuf = Gdk::Pixbuf::create_from_file(file_name, width, height);
//         }
//         else
//         {
//             pixbuf = Gdk::Pixbuf::create_from_file(file_name);
//         }

//         if (pixbuf)
//         {
//             pixbuf = pixbuf->apply_embedded_orientation();
//         }
//     }
//     catch (const Glib::Error &e)
//     {
//         KLOG_WARNING_APPEARANCE("%s", e.what().c_str());
//         return Glib::RefPtr<Gdk::Pixbuf>();
//     }

//     auto new_pixbuf = scale_pixbuf_to_size(pixbuf, width, height);
//     m_backgroundCache.set_pixbuf(file_name, width, height, new_pixbuf);
//     return new_pixbuf;
// }

// Glib::RefPtr<Gdk::Pixbuf> AppearanceBackground::scale_pixbuf_to_size(Glib::RefPtr<Gdk::Pixbuf> pixbuf, int32_t min_width, int32_t min_height)
// {
//     RETURN_VAL_IF_FALSE(pixbuf, Glib::RefPtr<Gdk::Pixbuf>());

//     auto src_width = gdk_pixbuf_get_width(pixbuf->gobj());
//     auto src_height = gdk_pixbuf_get_height(pixbuf->gobj());
//     auto factor = MAX(min_width / (double)src_width, min_height / (double)src_height);
//     auto new_width = floor(src_width * factor + 0.5);
//     auto new_height = floor(src_height * factor + 0.5);

//     auto new_pixbuf = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB,
//                                           pixbuf->get_has_alpha(),
//                                           8,
//                                           min_width,
//                                           min_height);

//     RETURN_VAL_IF_FALSE(new_pixbuf, new_pixbuf);

//     pixbuf->scale(new_pixbuf,
//                   0,
//                   0,
//                   min_width,
//                   min_height,
//                   (new_width - min_width) / -2,
//                   (new_height - min_height) / -2,
//                   factor,
//                   factor,
//                   Gdk::INTERP_BILINEAR);

//     return new_pixbuf;
// }

// void AppearanceBackground::blend_pixbuf(Glib::RefPtr<Gdk::Pixbuf> src,
//                                         Glib::RefPtr<Gdk::Pixbuf> dest,
//                                         int dest_x,
//                                         int dest_y,
//                                         double alpha)
// {
//     auto dest_width = dest->get_width();
//     auto dest_height = dest->get_height();
//     auto src_width = src->get_width();
//     auto src_height = src->get_height();

//     if (dest_x + src_width > dest_width)
//     {
//         src_width = dest_width - dest_x;
//     }

//     if (dest_y + src_height > dest_height)
//     {
//         src_height = dest_height - dest_y;
//     }

//     src->composite(dest,
//                    dest_x,
//                    dest_y,
//                    src_width,
//                    src_height,
//                    dest_x,
//                    dest_y,
//                    1,
//                    1,
//                    Gdk::INTERP_NEAREST,
//                    alpha * 0xFF + 0.5);
// }

// bool AppearanceBackground::set_surface_as_root(Glib::RefPtr<Gdk::Screen> screen, Cairo::RefPtr<Cairo::XlibSurface> surface)
// {
//     RETURN_VAL_IF_FALSE(screen, false);
//     RETURN_VAL_IF_FALSE(surface, false);
//     RETURN_VAL_IF_FALSE(surface->get_type() == Cairo::SURFACE_TYPE_XLIB, false);

//     auto display = screen->get_display();
//     Display *xdisplay = GDK_DISPLAY_XDISPLAY(display->gobj());
//     auto pixmap = surface->get_drawable();
//     Window xroot = RootWindow(xdisplay, gdk_x11_screen_get_screen_number(screen->gobj()));

//     XGrabServer(xdisplay);
//     setXPixmapToRoot(screen, pixmap);

//     XSetWindowBackgroundPixmap(xdisplay, xroot, pixmap);
//     XClearWindow(xdisplay, xroot);

//     XFlush(xdisplay);

//     XUngrabServer(xdisplay);
//     // 立即同步UnGrab请求支XServer
//     XFlush(xdisplay);

//     return true;
// }

void AppearanceBackground::setXPixmapToRoot(xcb_pixmap_t xpixmap)
{
    auto xcbConnection = XcbConnection::getDefault();
    auto xcbDefaultScreen = xcbConnection->getDefaultScreen();
    auto rootWindow = xcbDefaultScreen->root;

    auto xRootPixmapIDReply = XCB_REPLY(xcb_intern_atom,
                                        xcbConnection->getConnection(),
                                        true,
                                        strlen("_XROOTPMAP_ID"),
                                        "_XROOTPMAP_ID");

    auto esetRootPixmapIDReply = XCB_REPLY(xcb_intern_atom,
                                           xcbConnection->getConnection(),
                                           true,
                                           strlen("ESETROOT_PMAP_ID"),
                                           "ESETROOT_PMAP_ID");

    // 删除旧的背景图片
    if (xRootPixmapIDReply && xRootPixmapIDReply->atom != XCB_ATOM_NONE)
    {
        auto xRootPixmapPropertyReply = XCB_REPLY(xcb_get_property,
                                                  xcbConnection->getConnection(),
                                                  false,
                                                  rootWindow,
                                                  xRootPixmapIDReply->atom,
                                                  XCB_ATOM_ANY,
                                                  0,
                                                  1);

        if (xRootPixmapPropertyReply &&
            xRootPixmapPropertyReply->type == XCB_ATOM_PIXMAP &&
            xRootPixmapPropertyReply->format == 32 &&
            esetRootPixmapIDReply &&
            esetRootPixmapIDReply->atom != XCB_ATOM_NONE)
        {
            auto esetRootPixmapPropertyReply = XCB_REPLY(xcb_get_property,
                                                         xcbConnection->getConnection(),
                                                         false,
                                                         rootWindow,
                                                         esetRootPixmapIDReply->atom,
                                                         XCB_ATOM_ANY,
                                                         0,
                                                         1);

            if (esetRootPixmapPropertyReply &&
                esetRootPixmapPropertyReply->type == XCB_ATOM_PIXMAP &&
                esetRootPixmapPropertyReply->format == 32)
            {
                auto xRootPixmap = *(xcb_pixmap_t *)xcb_get_property_value(xRootPixmapPropertyReply.get());
                auto esetRootPixmap = *(xcb_pixmap_t *)xcb_get_property_value(esetRootPixmapPropertyReply.get());
                if (xRootPixmap && xRootPixmap == esetRootPixmap)
                {
                    xcb_kill_client(xcbConnection->getConnection(), xRootPixmap);
                }
                if (esetRootPixmap && xRootPixmap != esetRootPixmap)
                {
                    xcb_kill_client(xcbConnection->getConnection(), esetRootPixmap);
                }
            }
        }
    }

    xRootPixmapIDReply = XCB_REPLY(xcb_intern_atom,
                                   xcbConnection->getConnection(),
                                   false,
                                   strlen("_XROOTPMAP_ID"),
                                   "_XROOTPMAP_ID");

    esetRootPixmapIDReply = XCB_REPLY(xcb_intern_atom,
                                      xcbConnection->getConnection(),
                                      false,
                                      strlen("ESETROOT_PMAP_ID"),
                                      "ESETROOT_PMAP_ID");

    // 设置新的背景图片
    if (!xRootPixmapIDReply ||
        xRootPixmapIDReply->atom == XCB_ATOM_NONE ||
        !esetRootPixmapIDReply ||
        esetRootPixmapIDReply->atom == XCB_ATOM_NONE)
    {
        KLOG_WARNING(appearance) << "Could not create atoms needed to set root pixmap id/properties.";
        return;
    }

    xcb_change_property(xcbConnection->getConnection(),
                        XCB_PROP_MODE_REPLACE,
                        rootWindow,
                        xRootPixmapIDReply->atom,
                        XCB_ATOM_PIXMAP,
                        32,
                        1,
                        &xpixmap);

    xcb_change_property(xcbConnection->getConnection(),
                        XCB_PROP_MODE_REPLACE,
                        rootWindow,
                        esetRootPixmapIDReply->atom,
                        XCB_ATOM_PIXMAP,
                        32,
                        1,
                        &xpixmap);
}

void AppearanceBackground::processMateBackgroundSettingsChanged(const QString &key)
{
    // 这里不感知picture-filename的变化，也不使用这个字段，因为这个字段的值可能是一个xml文件路径，新的背景设置插件无法兼容
    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(MATE_BACKGROUND_SCHAME_KEY_SHOW_DESKTOP_ICONS, _hash):
        delayDrawBackground();
        break;
    default:
        break;
    }
}

void AppearanceBackground::processAppearanceSettingsChanged(const QString &key)
{
    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(APPEARANCE_SCHEMA_KEY_DESKTOP_BG, _hash):
        setBackground(m_appearanceSettings->get(APPEARANCE_SCHEMA_KEY_DESKTOP_BG).toString());
        break;
    default:
        break;
    }
}

}  // namespace Kiran
