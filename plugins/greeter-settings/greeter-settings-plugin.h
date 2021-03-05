/**
 * @file greeter-settings-plugin.h
 * @brief description
 * @author yangxiaoqing <yangxiaoqing@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
*/
#ifndef GREETERSETTINGSPLUGIN_H
#define GREETERSETTINGSPLUGIN_H

#include "plugin_i.h"
namespace Kiran
{
class GreeterSettingsPlugin : public Plugin
{
public:
    GreeterSettingsPlugin();
    virtual ~GreeterSettingsPlugin();

    virtual void activate();

    virtual void deactivate();
};
}


#endif // GREETERSETTINGSPLUGIN_H
