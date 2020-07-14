/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:33:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-14 15:01:11
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

    virtual void deactivate();
};
}  // namespace Kiran
