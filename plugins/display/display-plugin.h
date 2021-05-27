/**
 * @FilePath      /kiran-cc-daemon/plugins/display/display-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "plugin-i.h"
namespace Kiran
{
class DisplayPlugin : public Plugin
{
public:
    DisplayPlugin();
    virtual ~DisplayPlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
