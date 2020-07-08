/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 18:16:08
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-06 15:15:21
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/lib/common.h
 */

namespace Kiran
{
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

#define GETTEXT_PACKAGE "kiran-system-daemon"

#define SESSION_DAEMON_SCHEMA "com.unikylin.kiran.system-daemon"

#define SCHEMA_LOG_LEVEL "log-level"

#define SYSTEMD_NAME "org.freedesktop.systemd1"
#define SYSTEMD_PATH "/org/freedesktop/systemd1"
#define SYSTEMD_MANAGER_INTERFACE "org.freedesktop.systemd1.Manager"
#define SYSTEMD_UNIT_INTERFACE "org.freedesktop.systemd1.Unit"

#define POLKIT_NAME "org.freedesktop.PolicyKit1"
#define POLKIT_PATH "/org/freedesktop/PolicyKit1/Authority"
#define POLKIT_INTERFACE "org.freedesktop.PolicyKit1.Authority"

}  // namespace Kiran
