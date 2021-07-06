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


#include <cairomm/cairomm.h>

#include "plugins/appearance/background/background-cache.h"

//
#include <cairomm/xlib_surface.h>

namespace Kiran
{
class AppearanceBackground
{
public:
    AppearanceBackground();
    virtual ~AppearanceBackground(){};

    void init();

    // 更新桌面背景图片路径并画桌面背景
    void set_background(const std::string &path);

private:
    // 画桌面背景
    void draw_background();

    // 是否可以画背景图片
    bool can_draw_background();

    Cairo::RefPtr<Cairo::XlibSurface> create_surface(Glib::RefPtr<Gdk::Screen> screen);

    Cairo::RefPtr<Cairo::XlibSurface> create_surface_by_size(Glib::RefPtr<Gdk::Window> window,
                                                             int32_t width,
                                                             int32_t height);

    void draw_to_pixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, Glib::RefPtr<Gdk::Screen> screen);

    Glib::RefPtr<Gdk::Pixbuf> get_pixbuf_by_file(const std::string &file_name, int32_t width, int32_t height);

    // 将pixbuf缩放到指定的大小
    Glib::RefPtr<Gdk::Pixbuf> scale_pixbuf_to_size(Glib::RefPtr<Gdk::Pixbuf> pixbuf, int32_t min_width, int32_t min_height);

    // 将源图像复制到目标图像中
    void blend_pixbuf(Glib::RefPtr<Gdk::Pixbuf> src,
                      Glib::RefPtr<Gdk::Pixbuf> dest,
                      int dest_x,
                      int dest_y,
                      double alpha);

    bool set_surface_as_root(Glib::RefPtr<Gdk::Screen> screen, Cairo::RefPtr<Cairo::XlibSurface> surface);
    void set_root_pixmap_id(Glib::RefPtr<Gdk::Screen> screen, Pixmap xpixmap);

    void on_mate_background_settings_changed(const Glib::ustring &key);
    void on_screen_size_changed();

private:
    std::string desktop_background_;

    Glib::RefPtr<Gio::Settings> mate_background_settings_;

    BackgroundCache background_cache_;
};

}  // namespace Kiran