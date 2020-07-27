/*
 * @Author       : tangjie02
 * @Date         : 2020-07-24 14:43:31
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-27 13:47:26
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/lib/auth-manager.h
 */

#pragma once

#include <giomm.h>

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

    static void global_deinit() { delete instance_; };

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