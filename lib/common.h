/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 18:16:08
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-26 13:57:13
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/lib/common.h
 */

namespace Kiran
{
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

#define GETTEXT_PACKAGE "kiran-cc-daemon"

#define CC_DAEMON_SCHEMA "com.unikylin.kiran.cc-daemon"

#define SCHEMA_LOG_LEVEL "log-level"

#define SYSTEMD_NAME "org.freedesktop.systemd1"
#define SYSTEMD_PATH "/org/freedesktop/systemd1"
#define SYSTEMD_MANAGER_INTERFACE "org.freedesktop.systemd1.Manager"
#define SYSTEMD_UNIT_INTERFACE "org.freedesktop.systemd1.Unit"

#define POLKIT_NAME "org.freedesktop.PolicyKit1"
#define POLKIT_PATH "/org/freedesktop/PolicyKit1/Authority"
#define POLKIT_INTERFACE "org.freedesktop.PolicyKit1.Authority"

#define CHECK_XMLPP_ELEMENT(node, err)                                                                       \
    {                                                                                                        \
        const auto element = dynamic_cast<const xmlpp::Element *>(node);                                     \
                                                                                                             \
        if (!element)                                                                                        \
        {                                                                                                    \
            err = fmt::format("the type of the node '{0}' isn't xmlpp::Element.", node->get_name().c_str()); \
            return false;                                                                                    \
        }                                                                                                    \
    }

}  // namespace Kiran
