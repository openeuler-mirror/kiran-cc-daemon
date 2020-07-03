/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 18:16:08
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-03 10:47:38
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/lib/common.h
 */

namespace Kiran
{
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

#define GETTEXT_PACKAGE "kiran-system-daemon"

#define SESSION_DAEMON_SCHEMA "com.unikylin.kiran.system-daemon"

#define SCHEMA_LOG_LEVEL "log-level"

}  // namespace Kiran
