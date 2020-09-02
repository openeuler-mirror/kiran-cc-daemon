/*
 * @Author       : tangjie02
 * @Date         : 2020-08-19 17:20:11
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 14:43:57
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/lib/dbus/cc-dbus-error.h
 */
#pragma once

#include <giomm.h>

#include "lib/base/error.h"

namespace Kiran
{
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