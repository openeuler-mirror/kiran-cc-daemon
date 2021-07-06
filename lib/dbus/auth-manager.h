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
#define INVOCATION_RETURN(invocation)                                                    \
    {                                                                                    \
        std::vector<Glib::VariantBase> vlist;                                            \
        invocation->return_value(Glib::Variant<Glib::VariantBase>::create_tuple(vlist)); \
    }

class AuthManager
{
public:
    AuthManager();
    virtual ~AuthManager(){};

    static AuthManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit();

    using AuthCheckHandler = std::function<void(const Glib::RefPtr<Gio::DBus::MethodInvocation>)>;

    struct AuthCheck
    {
        AuthCheck(const Glib::RefPtr<Gio::DBus::MethodInvocation> inv) : invocation(inv){};
        Glib::RefPtr<Gio::Cancellable> cancellable;
        sigc::connection cancel_connection;
        std::string cancel_string;
        const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation;
        AuthCheckHandler handler;
    };

    void start_auth_check(const std::string &action, bool user_interaction, const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, AuthCheckHandler handler);
    bool cancel_auth_check(std::shared_ptr<AuthCheck> auth_check);
    void finish_auth_check(Glib::RefPtr<Gio::AsyncResult> &res, std::shared_ptr<AuthCheck> auth_check);

private:
    void init();

private:
    static AuthManager *instance_;
    int32_t running_auth_checks_;
    Glib::RefPtr<Gio::DBus::Proxy> polkit_proxy_;
};
}  // namespace Kiran