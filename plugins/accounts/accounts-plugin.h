/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:33:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-30 14:53:16
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-plugin.h
 */

#include "plugin_i.h"
namespace Kiran
{
class AccountsPlugin : public Plugin
{
public:
    AccountsPlugin();
    virtual ~AccountsPlugin();

    virtual void activate();

    virtual void register_object(const Glib::RefPtr<Gio::DBus::Connection> &connection);
};
}  // namespace Kiran
