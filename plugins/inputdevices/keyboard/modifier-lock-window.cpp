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

#include "plugins/inputdevices/keyboard/modifier-lock-window.h"
#include "lib/base/base.h"

namespace Kiran
{
#define SESSION_DAEMON_GRESOURCE_PATH "/com/kylinsec/Kiran/SessionDaemon/"

#define IMAGE_CAPSLOCK_ON "image/capslock-on"
#define IMAGE_CAPSLOCK_OFF "image/capslock-off"
#define IMAGE_NUMLOCK_ON "image/numlock-on"
#define IMAGE_NUMLOCK_OFF "image/numlock-off"

#define DIALOG_TIMEOUT 3

ModifierLockWindow::ModifierLockWindow()
{
    this->init();
}

ModifierLockWindow::~ModifierLockWindow()
{
}

void ModifierLockWindow::show_capslock_on()
{
    this->hide();
    this->image_file_ = std::string(SESSION_DAEMON_GRESOURCE_PATH IMAGE_CAPSLOCK_ON);
    this->show();
}

void ModifierLockWindow::show_capslock_off()
{
    this->hide();
    this->image_file_ = std::string(SESSION_DAEMON_GRESOURCE_PATH IMAGE_CAPSLOCK_OFF);
    this->show();
}

void ModifierLockWindow::show_numlock_on()
{
    this->hide();
    this->image_file_ = std::string(SESSION_DAEMON_GRESOURCE_PATH IMAGE_NUMLOCK_ON);
    this->show();
}

void ModifierLockWindow::show_numlock_off()
{
    this->hide();
    this->image_file_ = std::string(SESSION_DAEMON_GRESOURCE_PATH IMAGE_NUMLOCK_OFF);
    this->show();
}

void ModifierLockWindow::init()
{
    this->set_type_hint(Gdk::WINDOW_TYPE_HINT_NORMAL);
    this->set_skip_taskbar_hint(true);
    this->set_decorated(false);
    this->set_position(Gtk::WIN_POS_CENTER);
    this->set_default_size(120, 120);
    this->set_keep_above(true);
    this->set_app_paintable(true);

    Glib::RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();
    Glib::RefPtr<Gdk::Visual> visual = screen->get_rgba_visual();
    if (!visual)
    {
        visual = screen->get_system_visual();
    }
    gtk_widget_set_visual(GTK_WIDGET(this->gobj()), visual->gobj());

    this->signal_show().connect(sigc::mem_fun(this, &ModifierLockWindow::on_real_show));
    this->signal_hide().connect(sigc::mem_fun(this, &ModifierLockWindow::on_real_hide));
    this->signal_draw().connect(sigc::mem_fun(this, &ModifierLockWindow::on_real_draw));
}

void ModifierLockWindow::on_real_show()
{
    this->remove_hide_timeout();
    this->add_hide_timeout();
}

void ModifierLockWindow::on_real_hide()
{
    this->remove_hide_timeout();
}

bool ModifierLockWindow::on_real_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;

    KLOG_DEBUG("Do show real image:%s.", this->image_file_.c_str());

    try
    {
        pixbuf = Gdk::Pixbuf::create_from_resource(this->image_file_);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return true;
    }

    Gdk::Cairo::set_source_pixbuf(cr,
                                  pixbuf,
                                  (width - pixbuf->get_width()) / 2,
                                  (height - pixbuf->get_height()) / 2);
    cr->paint();

    return true;
}

bool ModifierLockWindow::on_hide_timeout()
{
    this->hide();
    return true;
}

void ModifierLockWindow::add_hide_timeout()
{
    this->hide_timeout_id_ = Glib::signal_timeout().connect_seconds(
        sigc::mem_fun(this, &ModifierLockWindow::on_hide_timeout),
        DIALOG_TIMEOUT);
}

void ModifierLockWindow::remove_hide_timeout()
{
    if (this->hide_timeout_id_)
    {
        this->hide_timeout_id_.disconnect();
    }
}

}  // namespace Kiran