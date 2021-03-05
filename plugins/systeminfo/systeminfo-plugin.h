/**
 * @file          /kiran-cc-daemon/plugins/systeminfo/systeminfo-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "plugin_i.h"
namespace Kiran
{
class SystemInfoPlugin : public Plugin
{
public:
    SystemInfoPlugin();
    virtual ~SystemInfoPlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
