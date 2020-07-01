/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 18:16:08
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-01 13:59:45
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/lib/common.h
 */

namespace Kiran
{
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

#define SESSION_DAEMON_SCHEMA "com.unikylin.kiran.session-daemon"

#define SCHEMA_LOG_LEVEL "log-level"

}  // namespace Kiran
