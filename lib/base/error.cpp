/*
 * @Author       : tangjie02
 * @Date         : 2020-09-03 09:27:56
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-10 20:31:03
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/lib/base/error.cpp
 */

#include "lib/base/error.h"

namespace Kiran
{
static const GDBusErrorEntry cc_error_entries[] = {
    {int32_t(CCError::ERROR_FAILED), "com.kylinsec.Kiran.CC.Error.Failed"},
    {int32_t(CCError::ERROR_NOT_SUPPORTED), "com.kylinsec.Kiran.CC.Error.NotSupported"},
    {int32_t(CCError::ERROR_PERMISSION_DENIED), "com.kylinsec.Kiran.CC.Error.PermissionDenied"},
    {int32_t(CCError::ERROR_EXCEED_LIMIT), "com.kylinsec.Kiran.CC.Error.ExceedLimit"},
    {int32_t(CCError::ERROR_INVALID_PARAMETER), "com.kylinsec.Kiran.CC.Error.InvalidParameter"},
    {int32_t(CCError::ERROR_UNKNOWN), "com.kylinsec.Kiran.CC.Error.Unknown"},
    {int32_t(CCError::ERROR_GEN_SHORTCUT_ID), "com.kylinsec.Kiran.CC.Error.GenShortCutID"},
    {int32_t(CCError::ERROR_GRAB_KEYCOMB), "com.kylinsec.Kiran.CC.Error.GrabKeyComb"},
    {int32_t(CCError::ERROR_KEYCOMB_CONFLICT), "com.kylinsec.Kiran.CC.Error.KeyCombConflict"},
    {int32_t(CCError::ERROR_BLUEZ_REJECTED), "org.bluez.Error.Rejected"},
    {int32_t(CCError::ERROR_BLUEZ_CANCELED), "org.bluez.Error.Canceled"},
};

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
