/*
 * @Author       : tangjie02
 * @Date         : 2020-08-12 16:26:07
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-12 16:26:51
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/inputdevices/keyboard/keyboard-plugin.h
 */
#pragma once

#include "plugin-i.h"
namespace Kiran
{
class KeyboardPlugin : public Plugin
{
public:
    KeyboardPlugin();
    virtual ~KeyboardPlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
