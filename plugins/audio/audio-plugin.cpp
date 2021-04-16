/**
 * @file          /kiran-cc-daemon/plugins/audio/audio-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#include "plugins/audio/audio-plugin.h"

#include <cstdio>

#include "plugins/audio/audio-manager.h"
#include "plugins/audio/pulse/pulse-backend.h"

PLUGIN_EXPORT_FUNC_DEF(AudioPlugin);

namespace Kiran
{
AudioPlugin::AudioPlugin()
{
}

AudioPlugin::~AudioPlugin()
{
}

void AudioPlugin::activate()
{
    SETTINGS_PROFILE("active audio plugin.");

    PulseBackend::global_init();
    AudioManager::global_init(PulseBackend::get_instance());
}

void AudioPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive audio plugin.");

    AudioManager::global_deinit();
    PulseBackend::global_deinit();
}

}  // namespace Kiran