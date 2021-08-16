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