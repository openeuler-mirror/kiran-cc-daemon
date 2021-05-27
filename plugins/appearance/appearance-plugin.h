/**
 * @file          /kiran-cc-daemon/plugins/appearance/appearance-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "plugin-i.h"

namespace Kiran
{
class AppearancePlugin : public Plugin
{
public:
    AppearancePlugin();
    virtual ~AppearancePlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
