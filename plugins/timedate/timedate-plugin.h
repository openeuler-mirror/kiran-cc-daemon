/*
 * @Author       : tangjie02
 * @Date         : 2020-07-06 09:59:45
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-14 14:59:37
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/timedate/timedate-plugin.h
 */

#include "plugin-i.h"
namespace Kiran
{
class TimedatePlugin : public Plugin
{
public:
    TimedatePlugin();
    virtual ~TimedatePlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
