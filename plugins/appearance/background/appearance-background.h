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

#include <QObject>

class QGSettings;
class QTimer;

typedef uint32_t xcb_pixmap_t;

namespace Kiran
{
class AppearanceBackground : public QObject
{
    Q_OBJECT

public:
    AppearanceBackground(QObject *parent = nullptr);
    virtual ~AppearanceBackground(){};

    void init();

    // 更新桌面背景图片路径并画桌面背景
    void setBackground(const QString &path);

private:
    // 延时绘制背景
    void delayDrawBackground();
    // 画桌面背景
    void drawBackground();
    // 是否可以画背景图片
    bool canDrawBackground();
    // 将QImage转换为Xorg的Pixmap
    xcb_pixmap_t image2XPixmap(const QImage &image);

    // Cairo::RefPtr<Cairo::XlibSurface> create_surface(Glib::RefPtr<Gdk::Screen> screen);

    // Cairo::RefPtr<Cairo::XlibSurface> create_surface_by_size(Glib::RefPtr<Gdk::Window> window,
    //                                                          int32_t width,
    //                                                          int32_t height);

    // void draw_to_pixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, Glib::RefPtr<Gdk::Screen> screen);

    // Glib::RefPtr<Gdk::Pixbuf> get_pixbuf_by_file(const QString &file_name, int32_t width, int32_t height);

    // // 将pixbuf缩放到指定的大小
    // Glib::RefPtr<Gdk::Pixbuf> scale_pixbuf_to_size(Glib::RefPtr<Gdk::Pixbuf> pixbuf, int32_t min_width, int32_t min_height);

    // // 将源图像复制到目标图像中
    // void blend_pixbuf(Glib::RefPtr<Gdk::Pixbuf> src,
    //                   Glib::RefPtr<Gdk::Pixbuf> dest,
    //                   int dest_x,
    //                   int dest_y,
    //                   double alpha);

    // bool set_surface_as_root(Glib::RefPtr<Gdk::Screen> screen, Cairo::RefPtr<Cairo::XlibSurface> surface);
    void setXPixmapToRoot(xcb_pixmap_t xpixmap);

    void processMateBackgroundSettingsChanged(const QString &key);
    void processAppearanceSettingsChanged(const QString &key);

private:
    // 延时绘制桌面背景的定时器
    QTimer *m_delayTimer;
    QString m_desktopBackground;

    QGSettings *m_mateBackgroundSettings;
    QGSettings *m_appearanceSettings;

    // BackgroundCache m_backgroundCache;
};

}  // namespace Kiran