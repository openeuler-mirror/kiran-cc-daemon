/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:33:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-24 17:10:16
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/keybinding-plugin.h
 */
#pragma once

#include "plugin-i.h"
namespace Kiran
{
class KeybindingPlugin : public Plugin
{
public:
    KeybindingPlugin();
    virtual ~KeybindingPlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
