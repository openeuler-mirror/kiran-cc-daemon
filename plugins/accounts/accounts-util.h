/*
 * @Author       : tangjie02
 * @Date         : 2020-07-24 13:42:10
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-27 16:35:46
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-util.h
 */

#pragma once

#include <giomm.h>

namespace Kiran
{
class AccountsUtil
{
public:
    AccountsUtil(){};
    virtual ~AccountsUtil(){};

    static bool get_caller_pid(Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, GPid &pid);
    static bool get_caller_uid(Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, int32_t &uid);

    static void get_caller_loginuid(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, std::string &loginuid);

    static void setup_loginuid(const std::string &id);
    static bool spawn_with_login_uid(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation,
                                     const Glib::ArrayHandle<std::string> argv,
                                     std::string &err);
};
}  // namespace Kiran