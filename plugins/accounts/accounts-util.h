/*
 * @Author       : tangjie02
 * @Date         : 2020-07-24 13:42:10
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-29 09:47:19
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/accounts/accounts-util.h
 */

#pragma once

#include "lib/base/base.h"

namespace Kiran
{
#define ADMIN_GROUP "wheel"
#define AUTH_USER_ADMIN "com.unikylin.kiran.system-daemon.accounts.user-administration"
#define AUTH_CHANGE_OWN_USER_DATA "com.unikylin.kiran.system-daemon.accounts.change-own-user-data"
#define AUTH_CHANGE_OWN_PASSWORD "com.unikylin.kiran.system-daemon.accounts.change-own-password"

#define SPAWN_WITH_LOGIN_UID(invocation, ...)                                                   \
    {                                                                                           \
        std::vector<std::string> argv = {__VA_ARGS__};                                          \
        std::string err;                                                                        \
        if (!AccountsUtil::spawn_with_login_uid(invocation.getMessage(), argv, err))            \
        {                                                                                       \
            err = fmt::format("running '{0}' failed: {1}", argv[0], err);                       \
            invocation.ret(Glib::Error(CC_ERROR, int32_t(CCError::ERROR_FAILED), err.c_str())); \
            return;                                                                             \
        }                                                                                       \
    }

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