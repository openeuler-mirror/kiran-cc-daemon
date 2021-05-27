/*
 * @Author       : tangjie02
 * @Date         : 2020-11-24 10:22:11
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-12-01 15:05:03
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/xsettings/xsettings-common.h
 */
#pragma once

#include "xsettings-i.h"

/* X servers sometimes lie about the screen's physical dimensions, so we cannot
 * compute an accurate DPI value.  When this happens, the user gets fonts that
 * are too huge or too tiny.  So, we see what the server returns:  if it reports
 * something outside of the range [DPI_LOW_REASONABLE_VALUE,
 * DPI_HIGH_REASONABLE_VALUE], then we assume that it is lying and we use
 * DPI_FALLBACK instead.
 *
 * See get_dpi_from_gsettings_or_server() below, and also
 * https://bugzilla.novell.com/show_bug.cgi?id=217790
 */
#define DPI_FALLBACK 96
#define DPI_LOW_REASONABLE_VALUE 50
#define DPI_HIGH_REASONABLE_VALUE 500

/* The minimum resolution at which we turn on a window-scale of 2 */
#define HIDPI_LIMIT (DPI_FALLBACK * 2)

/* The minimum screen height at which we turn on a window-scale of 2;
 * below this there just isn't enough vertical real estate for GNOME
 * apps to work, and it's better to just be tiny */
#define HIDPI_MIN_HEIGHT 1500

#define WM_COMMON_MARCO "Metacity (Marco)"
#define WM_COMMON_SAWFISH "Sawfish"
#define WM_COMMON_METACITY "Metacity"
#define WM_COMMON_COMPIZ "Compiz"
#define WM_COMMON_COMPIZ_OLD "compiz"
#define WM_COMMON_UNKNOWN "Unknown"