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
