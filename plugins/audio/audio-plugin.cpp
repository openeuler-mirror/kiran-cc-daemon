/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
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
    KLOG_DEBUG_AUDIO("Active audio plugin.");

    PulseBackend::global_init();
    AudioManager::global_init(PulseBackend::get_instance());
}

void AudioPlugin::deactivate()
{
    KLOG_DEBUG_AUDIO("Deactive audio plugin.");

    AudioManager::global_deinit();
    PulseBackend::global_deinit();
}

}  // namespace Kiran