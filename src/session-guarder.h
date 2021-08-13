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

#pragma once

#include "lib/base/base.h"

namespace Kiran
{
class SessionGuarder
{
public:
    SessionGuarder();
    virtual ~SessionGuarder(){};

    static SessionGuarder* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    sigc::signal<void> signal_session_end() { return this->session_end_; };

private:
    void init();

    void on_sm_signal(const Glib::ustring& sender_name, const Glib::ustring& signal_name, const Glib::VariantContainerBase& parameters);
    void on_session_query_end();
    void on_session_end();

private:
    static SessionGuarder* instance_;

    // sm: session manager
    Glib::RefPtr<Gio::DBus::Proxy> sm_proxy_;
    Glib::RefPtr<Gio::DBus::Proxy> sm_private_proxy_;

    sigc::signal<void> session_end_;
};
}  // namespace Kiran
