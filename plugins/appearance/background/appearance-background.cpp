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


#include "plugins/appearance/background/appearance-background.h"

#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <gdkmm/pixbuf.h>
//
#include <X11/Xlib.h>

namespace Kiran
{
#define MATE_BACKGROUND_SCHEMA_ID "org.mate.background"
#define MATE_BACKGROUND_SCHAME_KEY_PICTURE_FILENAME "picture-filename"
#define MATE_BACKGROUND_SCHAME_KEY_SHOW_DESKTOP_ICONS "show-desktop-icons"

AppearanceBackground::AppearanceBackground()
{
    this->mate_background_settings_ = Gio::Settings::create(MATE_BACKGROUND_SCHEMA_ID);
}

void AppearanceBackground::init()
{
    this->mate_background_settings_->signal_changed().connect(sigc::mem_fun(this, &AppearanceBackground::on_mate_background_settings_changed));

    auto screen = Gdk::Screen::get_default();
    /* FIXME：size-changed和monitors-changed两个信号不能同时监控，如果同时监控会导致第二次设置背景时调用XOpenDisplay函数连接阻塞，具体原因未知
       触发BUG的时机是在刚进入会话时(可以通过killall lightdm复现)，进入会话后再执行插件不确定是否会触发BUG
       mate是通过判断前后两次屏幕大小变化来决定是否重新绘制背景，没有说明这样做的原因，但这样做的话当显示器位置变化时显然不会对背景进行重绘，
       这和只监控size-changed信号区别不大。*/
    screen->signal_size_changed().connect(sigc::mem_fun(this, &AppearanceBackground::on_screen_size_changed));
    // screen->signal_monitors_changed().connect(sigc::mem_fun(this, &AppearanceBackground::on_screen_size_changed));
}

void AppearanceBackground::set_background(const std::string &path)
{
    KLOG_PROFILE("path: %s", path.c_str());

    RETURN_IF_TRUE(this->desktop_background_ == path);
    this->desktop_background_ = path;

    // 兼容老的mate桌面设置
    this->mate_background_settings_->set_string(MATE_BACKGROUND_SCHAME_KEY_PICTURE_FILENAME, path);

    this->draw_background();
}

void AppearanceBackground::draw_background()
{
    KLOG_PROFILE("")

    RETURN_IF_FALSE(this->can_draw_background());

    auto screen = Gdk::Screen::get_default();
    auto surface = this->create_surface(screen);
    this->set_surface_as_root(screen, surface);
}

bool AppearanceBackground::can_draw_background()
{
    KLOG_PROFILE("");

    /* caja也会画背景，这里需要判断一下caja是否会去画桌面背景。
    如果org.mate.background中的show-desktop-icons字段为false，则caja不会去画桌面背景，此时插件需要负责去画背景。*/
    if (!this->mate_background_settings_->get_boolean(MATE_BACKGROUND_SCHAME_KEY_SHOW_DESKTOP_ICONS))
    {
        return true;
    }

    /* 如果show-desktop-icons字段为true，且caja进程在运行时，会将caja的窗口id写入到根窗口属性CAJA_DESKTOP_WINDOW_ID中
    下面的逻辑用来判断caja是否时真实在运行 */
    auto display = Gdk::Display::get_default();
    auto xdisplay = GDK_DISPLAY_XDISPLAY(display->gobj());
    auto xroot_window = gdk_x11_get_default_root_xwindow();
    bool result = true;

    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char *data;

    auto caja_xid_prop = XInternAtom(xdisplay, "CAJA_DESKTOP_WINDOW_ID", True);
    RETURN_VAL_IF_TRUE(caja_xid_prop == None, true);

    XGetWindowProperty(xdisplay,
                       xroot_window,
                       caja_xid_prop,
                       0,
                       1,
                       False,
                       XA_WINDOW,
                       &type,
                       &format,
                       &nitems,
                       &after,
                       &data);

    RETURN_VAL_IF_TRUE(data == NULL, true);

    auto caja_window = *(Window *)data;
    XFree(data);

    RETURN_VAL_IF_TRUE(type != XA_WINDOW || format != 32, true);

    auto wmclass_prop = XInternAtom(xdisplay, "WM_CLASS", True);
    RETURN_VAL_IF_TRUE(wmclass_prop == None, true);

    gdk_x11_display_error_trap_push(display->gobj());

    XGetWindowProperty(xdisplay,
                       caja_window,
                       wmclass_prop,
                       0,
                       20,
                       False,
                       XA_STRING,
                       &type,
                       &format,
                       &nitems,
                       &after,
                       &data);

    XSync(xdisplay, False);

    RETURN_VAL_IF_TRUE(gdk_x11_display_error_trap_pop(display->gobj()) == BadWindow || data == NULL, true);

    // caja代码位置：src/caja-desktop-window.c:caja_desktop_window_new()
    if (nitems == 20 &&
        after == 0 &&
        format == 8 &&
        !strcmp((char *)data, "desktop_window") &&
        !strcmp((char *)data + strlen((char *)data) + 1, "Caja"))
    {
        result = false;
    }
    XFree(data);

    return result;
}

Cairo::RefPtr<Cairo::XlibSurface> AppearanceBackground::create_surface(Glib::RefPtr<Gdk::Screen> screen)
{
    KLOG_PROFILE("");

    RETURN_VAL_IF_FALSE(screen, Cairo::RefPtr<Cairo::XlibSurface>());

    auto window = screen->get_root_window();
    auto scale = window->get_scale_factor();
    auto xscreen = gdk_x11_screen_get_xscreen(screen->gobj());
    auto width = WidthOfScreen(xscreen) / scale;
    auto height = HeightOfScreen(xscreen) / scale;
    auto surface = this->create_surface_by_size(window, width, height);
    auto cairo = Cairo::Context::create(surface);

    cairo->scale(scale, scale);

    auto pixbuf = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, FALSE, 8, width, height);
    this->draw_to_pixbuf(pixbuf, screen);
    Gdk::Cairo::set_source_pixbuf(cairo, pixbuf, 0, 0);
    cairo->paint();
    return surface;
}

Cairo::RefPtr<Cairo::XlibSurface> AppearanceBackground::create_surface_by_size(Glib::RefPtr<Gdk::Window> window,
                                                                               int32_t width,
                                                                               int32_t height)
{
    KLOG_PROFILE("width: %d, height: %d", width, height);

    /* 这里会通过display来创建pixmap对象，pixmap对象会存放到根窗口的"_XROOTPMAP_ID"和"ESETROOT_PMAP_ID"属性当中。
    其他应用程序也可能会设置根窗口的这个属性，在设置之前会通过XKillClient(display,source)函数释放调之前设置的
    pixmap资源，同时这个函数也会将pixmap资源对应的客户端与display断开连接， 因此我们没有使用默认的display，而是创
    建一个新的display连接，避免影响到默认的display连接，然后将断开模式设置为RetainPermanent，
    保证在与新的display断开连接时pixmap资源不被销毁。、    */

    auto screen = window->get_screen();
    auto display_name = window->get_display()->get_name();

    auto dummy_display = XOpenDisplay(display_name.c_str());

    if (dummy_display == NULL)
    {
        KLOG_WARNING("Failed to open display '%s'", display_name.c_str());
        return Cairo::RefPtr<Cairo::XlibSurface>();
    }

    auto depth = DefaultDepth(dummy_display, gdk_x11_screen_get_screen_number(screen->gobj()));
    auto pixmap = XCreatePixmap(dummy_display, GDK_WINDOW_XID(window->gobj()), width, height, depth);

    XFlush(dummy_display);
    XSetCloseDownMode(dummy_display, RetainPermanent);
    XCloseDisplay(dummy_display);

    auto xlib_surface = Cairo::XlibSurface::create(GDK_SCREEN_XDISPLAY(screen->gobj()),
                                                   pixmap,
                                                   GDK_VISUAL_XVISUAL(screen->get_system_visual()->gobj()),
                                                   width,
                                                   height);

    return xlib_surface;
}

void AppearanceBackground::draw_to_pixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, Glib::RefPtr<Gdk::Screen> screen)
{
    auto display = screen->get_display();

    for (int32_t i = 0; i < display->get_n_monitors(); ++i)
    {
        auto monitor = display->get_monitor(i);
        Gdk::Rectangle geometry;

        monitor->get_geometry(geometry);

        auto monitor_pixbuf = this->get_pixbuf_by_file(this->desktop_background_, geometry.get_width(), geometry.get_height());
        if (monitor_pixbuf)
        {
            auto dest_x = geometry.get_x() + (geometry.get_width() - monitor_pixbuf->get_width()) / 2;
            auto dest_y = geometry.get_y() + (geometry.get_height() - monitor_pixbuf->get_height()) / 2;
            this->blend_pixbuf(monitor_pixbuf, pixbuf, dest_x, dest_y, 1.0);
        }
    }
}

Glib::RefPtr<Gdk::Pixbuf> AppearanceBackground::get_pixbuf_by_file(const std::string &file_name, int32_t width, int32_t height)
{
    // 先从缓存中读取
    auto cache_pixbuf = this->background_cache_.get_pixbuf(file_name, width, height);
    RETURN_VAL_IF_TRUE(cache_pixbuf, cache_pixbuf);

    auto pixbuf_format = Glib::wrap(gdk_pixbuf_get_file_info(file_name.c_str(), NULL, NULL));
    std::string format_name;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;

    if (pixbuf_format.gobj() != NULL)
    {
        format_name = pixbuf_format.get_name();
    }

    try
    {
        if (format_name == "svg")
        {
            pixbuf = Gdk::Pixbuf::create_from_file(file_name, width, height);
        }
        else
        {
            pixbuf = Gdk::Pixbuf::create_from_file(file_name);
        }

        if (pixbuf)
        {
            pixbuf = pixbuf->apply_embedded_orientation();
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return Glib::RefPtr<Gdk::Pixbuf>();
    }

    auto new_pixbuf = this->scale_pixbuf_to_size(pixbuf, width, height);
    this->background_cache_.set_pixbuf(file_name, width, height, new_pixbuf);
    return new_pixbuf;
}

Glib::RefPtr<Gdk::Pixbuf> AppearanceBackground::scale_pixbuf_to_size(Glib::RefPtr<Gdk::Pixbuf> pixbuf, int32_t min_width, int32_t min_height)
{
    RETURN_VAL_IF_FALSE(pixbuf, Glib::RefPtr<Gdk::Pixbuf>());

    auto src_width = gdk_pixbuf_get_width(pixbuf->gobj());
    auto src_height = gdk_pixbuf_get_height(pixbuf->gobj());
    auto factor = MAX(min_width / (double)src_width, min_height / (double)src_height);
    auto new_width = floor(src_width * factor + 0.5);
    auto new_height = floor(src_height * factor + 0.5);

    auto new_pixbuf = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB,
                                          pixbuf->get_has_alpha(),
                                          8,
                                          min_width,
                                          min_height);

    RETURN_VAL_IF_FALSE(new_pixbuf, new_pixbuf);

    pixbuf->scale(new_pixbuf,
                  0,
                  0,
                  min_width,
                  min_height,
                  (new_width - min_width) / -2,
                  (new_height - min_height) / -2,
                  factor,
                  factor,
                  Gdk::INTERP_BILINEAR);

    return new_pixbuf;
}

void AppearanceBackground::blend_pixbuf(Glib::RefPtr<Gdk::Pixbuf> src,
                                        Glib::RefPtr<Gdk::Pixbuf> dest,
                                        int dest_x,
                                        int dest_y,
                                        double alpha)
{
    auto dest_width = dest->get_width();
    auto dest_height = dest->get_height();
    auto src_width = src->get_width();
    auto src_height = src->get_height();

    if (dest_x + src_width > dest_width)
    {
        src_width = dest_width - dest_x;
    }

    if (dest_y + src_height > dest_height)
    {
        src_height = dest_height - dest_y;
    }

    src->composite(dest,
                   dest_x,
                   dest_y,
                   src_width,
                   src_height,
                   dest_x,
                   dest_y,
                   1,
                   1,
                   Gdk::INTERP_NEAREST,
                   alpha * 0xFF + 0.5);
}

bool AppearanceBackground::set_surface_as_root(Glib::RefPtr<Gdk::Screen> screen, Cairo::RefPtr<Cairo::XlibSurface> surface)
{
    RETURN_VAL_IF_FALSE(screen, false);
    RETURN_VAL_IF_FALSE(surface, false);
    RETURN_VAL_IF_FALSE(surface->get_type() == Cairo::SURFACE_TYPE_XLIB, false);

    auto display = screen->get_display();
    Display *xdisplay = GDK_DISPLAY_XDISPLAY(display->gobj());
    auto pixmap = surface->get_drawable();
    Window xroot = RootWindow(xdisplay, gdk_x11_screen_get_screen_number(screen->gobj()));

    XGrabServer(xdisplay);
    this->set_root_pixmap_id(screen, pixmap);

    XSetWindowBackgroundPixmap(xdisplay, xroot, pixmap);
    XClearWindow(xdisplay, xroot);

    XFlush(xdisplay);
    XUngrabServer(xdisplay);

    return true;
}

void AppearanceBackground::set_root_pixmap_id(Glib::RefPtr<Gdk::Screen> screen, Pixmap xpixmap)
{
    auto display = screen->get_display();
    Display *xdisplay = GDK_DISPLAY_XDISPLAY(display->gobj());

    Window xroot = RootWindow(xdisplay, gdk_x11_screen_get_screen_number(screen->gobj()));
    const char *atom_names[] = {"_XROOTPMAP_ID", "ESETROOT_PMAP_ID"};
    Atom atoms[G_N_ELEMENTS(atom_names)] = {0};

    Atom type;
    int format;
    int result;
    unsigned long nitems, after;
    unsigned char *data_root, *data_esetroot;

    // 删除旧的背景图片
    if (XInternAtoms(xdisplay, (char **)atom_names, G_N_ELEMENTS(atom_names), True, atoms) &&
        atoms[0] != None &&
        atoms[1] != None)
    {
        result = XGetWindowProperty(xdisplay,
                                    xroot, atoms[0],
                                    0L,
                                    1L,
                                    False,
                                    AnyPropertyType,
                                    &type,
                                    &format,
                                    &nitems,
                                    &after,
                                    &data_root);

        if (data_root != NULL && result == Success &&
            type == XA_PIXMAP && format == 32 && nitems == 1)
        {
            result = XGetWindowProperty(xdisplay,
                                        xroot,
                                        atoms[1],
                                        0L,
                                        1L,
                                        False,
                                        AnyPropertyType,
                                        &type,
                                        &format,
                                        &nitems,
                                        &after,
                                        &data_esetroot);

            if (data_esetroot != NULL && result == Success &&
                type == XA_PIXMAP && format == 32 && nitems == 1)
            {
                Pixmap xrootpmap = *((Pixmap *)data_root);
                Pixmap esetrootpmap = *((Pixmap *)data_esetroot);

                gdk_x11_display_error_trap_push(display->gobj());
                if (xrootpmap && xrootpmap == esetrootpmap)
                {
                    XKillClient(xdisplay, xrootpmap);
                }
                if (esetrootpmap && esetrootpmap != xrootpmap)
                {
                    XKillClient(xdisplay, esetrootpmap);
                }
                gdk_x11_display_error_trap_pop_ignored(display->gobj());
            }
            if (data_esetroot != NULL)
            {
                XFree(data_esetroot);
            }
        }
        if (data_root != NULL)
        {
            XFree(data_root);
        }
    }

    // 设置新的背景图片
    if (!XInternAtoms(xdisplay, (char **)atom_names, G_N_ELEMENTS(atom_names), False, atoms) ||
        atoms[0] == None || atoms[1] == None)
    {
        KLOG_WARNING("Could not create atoms needed to set root pixmap id/properties.\n");
        return;
    }

    XChangeProperty(xdisplay,
                    xroot,
                    atoms[0],
                    XA_PIXMAP,
                    32,
                    PropModeReplace,
                    (unsigned char *)&xpixmap, 1);

    XChangeProperty(xdisplay,
                    xroot,
                    atoms[1],
                    XA_PIXMAP,
                    32,
                    PropModeReplace,
                    (unsigned char *)&xpixmap, 1);
}

void AppearanceBackground::on_mate_background_settings_changed(const Glib::ustring &key)
{
    KLOG_PROFILE("");

    // 这里不感知picture-filename的变化，也不使用这个字段，因为这个字段的值可能是一个xml文件路径，新的背景设置插件无法兼容
    switch (shash(key.c_str()))
    {
    case CONNECT(MATE_BACKGROUND_SCHAME_KEY_SHOW_DESKTOP_ICONS, _hash):
        this->draw_background();
        break;
    default:
        break;
    }
}

void AppearanceBackground::on_screen_size_changed()
{
    KLOG_PROFILE("");
    this->draw_background();
}
}  // namespace Kiran