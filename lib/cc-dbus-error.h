/*
 * @Author       : tangjie02
 * @Date         : 2020-08-19 17:20:11
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-27 15:06:16
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/lib/cc-dbus-error.h
 */
#pragma once

#include <gio/gio.h>

namespace Kiran
{
enum class CCError : int32_t
{
    // Common
    SUCCESS,
    ERROR_FAILED,
    ERROR_NOT_SUPPORTED,
    ERROR_PERMISSION_DENIED,
    ERROR_EXCEED_LIMIT,
    ERROR_INVALID_PARAMETER,
    ERROR_UNKNOWN,

    // Accounts
    ERROR_USER_EXISTS,
    ERROR_USER_DOES_NOT_EXIST,

    // Keybinding
    ERROR_GEN_SHORTCUT_ID,

    NUM_ERRORS
};

#define CC_ERROR cc_error_quark()
GQuark cc_error_quark(void);

#define DBUS_ERROR_REPLY(error_code, fmt2, ...)                                                       \
    {                                                                                                 \
        auto err_message = fmt::format(fmt2, ##__VA_ARGS__);                                          \
        invocation.ret(Glib::Error(CC_ERROR, static_cast<int32_t>(error_code), err_message.c_str())); \
    }

#define DBUS_ERROR_REPLY_AND_RET(error_code, fmt2, ...) \
    DBUS_ERROR_REPLY(error_code, fmt2, ##__VA_ARGS__);  \
    return;

}  // namespace Kiran