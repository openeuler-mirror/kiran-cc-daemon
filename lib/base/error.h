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
#include <giomm.h>

#include <cstdint>

#include "error-i.h"

namespace Kiran
{
#define CC_ERROR2STR(error_code) CCError::get_error_desc(error_code)

#define DBUS_ERROR_REPLY(error_code, ...)                                                    \
    {                                                                                        \
        auto err_message = fmt::format(CC_ERROR2STR(error_code), ##__VA_ARGS__);             \
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, err_message.c_str())); \
    }

#define DBUS_ERROR_REPLY_AND_RET(error_code, ...) \
    DBUS_ERROR_REPLY(error_code, ##__VA_ARGS__);  \
    return;

class CCError
{
public:
    CCError();
    virtual ~CCError(){};

    static std::string get_error_desc(CCErrorCode error_code);
};

}  // namespace Kiran