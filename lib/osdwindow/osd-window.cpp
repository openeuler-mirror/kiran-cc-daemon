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
 * Author:     meizhigang <meizhigang@kylinsec.com.cn>
 */
#include "lib/osdwindow/osd-window.h"
#include <gdk/gdk.h>
#include "lib/base/base.h"

namespace Kiran
{
// dialog show timeout seconds
#define DIALOG_TIMEOUT 3

OSDWindow::OSDWindow(Gtk::WindowType window_type)
{
}

OSDWindow::~OSDWindow()
{
}

OSDWindow *OSDWindow::instance_ = nullptr;
void OSDWindow::global_init()
{
    RETURN_IF_TRUE(instance_);
    instance_ = new OSDWindow();
    instance_->init();
}

void OSDWindow::global_deinit()
{
    delete instance_;
    instance_ = nullptr;
}

void OSDWindow::init()
{
    this->set_type_hint(Gdk::WINDOW_TYPE_HINT_DOCK);
    this->set_skip_taskbar_hint(true);
    this->set_decorated(false);
    this->set_position(Gtk::WIN_POS_CENTER);
    this->set_default_size(120, 120);
    this->set_keep_above(true);
    this->set_app_paintable(true);
    this->set_modal(false);

    Glib::RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();
    Glib::RefPtr<Gdk::Visual> visual = screen->get_rgba_visual();
    if (!visual)
    {
        visual = screen->get_system_visual();
    }

    gtk_widget_set_visual(GTK_WIDGET(this->gobj()), visual->gobj());

    this->signal_show().connect(sigc::mem_fun(this, &OSDWindow::on_real_show));
    this->signal_hide().connect(sigc::mem_fun(this, &OSDWindow::on_real_hide));
    this->signal_draw().connect(sigc::mem_fun(this, &OSDWindow::on_real_draw));
}

void OSDWindow::on_real_show()
{
    this->remove_hide_timeout();
    this->add_hide_timeout();
}

void OSDWindow::on_real_hide()
{
    this->remove_hide_timeout();
}

bool OSDWindow::on_real_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    try
    {
        auto allocation = this->get_allocation();
        Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gtk::IconTheme::get_default()->load_icon(this->image_file_,
                                                                                    allocation.get_width(),
                                                                                    Gtk::ICON_LOOKUP_FORCE_SIZE);

        Gdk::Cairo::set_source_pixbuf(cr, pixbuf);

        cr->paint();
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s", e.what().c_str());
    }

    return false;
}

void OSDWindow::dialog_show(std::string icon)
{
    this->hide();

    this->image_file_ = icon;

    this->show();
}

bool OSDWindow::on_hide_timeout()
{
    this->hide();
    return true;
}

void OSDWindow::add_hide_timeout()
{
    if (!this->hide_timeout_id_)
    {
        this->hide_timeout_id_ = Glib::signal_timeout().connect_seconds(
            sigc::mem_fun(this, &OSDWindow::on_hide_timeout),
            DIALOG_TIMEOUT);
    }
}

void OSDWindow::remove_hide_timeout()
{
    if (this->hide_timeout_id_)
    {
        this->hide_timeout_id_.disconnect();
    }
}
}  // namespace  Kiran
