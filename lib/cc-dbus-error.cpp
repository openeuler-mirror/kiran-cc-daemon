/*
 * @Author       : tangjie02
 * @Date         : 2020-08-19 17:20:34
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 10:22:17
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/lib/cc-dbus-error.cpp
 */

#include "lib/cc-dbus-error.h"

namespace Kiran
{
static const GDBusErrorEntry cc_error_entries[] =
    {{int32_t(CCError::ERROR_FAILED), "com.unikylin.Kiran.CC.Error.Failed"},
     {int32_t(CCError::ERROR_NOT_SUPPORTED), "com.unikylin.Kiran.CC.Error.NotSupported"},
     {int32_t(CCError::ERROR_PERMISSION_DENIED), "com.unikylin.Kiran.CC.Error.PermissionDenied"},
     {int32_t(CCError::ERROR_EXCEED_LIMIT), "com.unikylin.Kiran.CC.Error.ExceedLimit"},
     {int32_t(CCError::ERROR_INVALID_PARAMETER), "com.unikylin.Kiran.CC.Error.InvalidParameter"},
     {int32_t(CCError::ERROR_UNKNOWN), "com.unikylin.Kiran.CC.Error.Unknown"},
     {int32_t(CCError::ERROR_USER_EXISTS), "com.unikylin.Kiran.CC.Error.UserExists"},
     {int32_t(CCError::ERROR_USER_DOES_NOT_EXIST), "com.unikylin.Kiran.CC.Error.UserDoesNotExist"},
     {int32_t(CCError::ERROR_GEN_SHORTCUT_ID), "com.unikylin.Kiran.CC.Error.GenShortCutID"},
     {int32_t(CCError::ERROR_GRAB_KEYCOMB), "com.unikylin.Kiran.CC.Error.GrabKeyComb"},
     {int32_t(CCError::ERROR_KEYCOMB_CONFLICT), "com.unikylin.Kiran.CC.Error.KeyCombConflict"}};

GQuark cc_error_quark(void)
{
    static volatile gsize quark_volatile = 0;

    if (!quark_volatile)
    {
        g_dbus_error_register_error_domain("cc_error",
                                           &quark_volatile,
                                           cc_error_entries,
                                           G_N_ELEMENTS(cc_error_entries));
    }

    return (GQuark)quark_volatile;
}

}  // namespace Kiran
