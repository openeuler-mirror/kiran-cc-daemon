/*
 * @Author       : tangjie02
 * @Date         : 2020-09-02 14:12:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 14:13:55
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/lib/common/error.h
 */
#pragma once

#include <cstdint>

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
    ERROR_GRAB_KEYCOMB,
    ERROR_KEYCOMB_CONFLICT,

    NUM_ERRORS
};

}