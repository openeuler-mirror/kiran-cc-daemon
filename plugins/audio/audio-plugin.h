/**
 * @file          /kiran-cc-daemon/plugins/audio/audio-plugin.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "plugin_i.h"
namespace Kiran
{
class AudioPlugin : public Plugin
{
public:
    AudioPlugin();
    virtual ~AudioPlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
