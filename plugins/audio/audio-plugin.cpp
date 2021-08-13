/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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
    KLOG_PROFILE("active audio plugin.");

    PulseBackend::global_init();
    AudioManager::global_init(PulseBackend::get_instance());
}

void AudioPlugin::deactivate()
{
    KLOG_PROFILE("deactive audio plugin.");

    AudioManager::global_deinit();
    PulseBackend::global_deinit();
}

}  // namespace Kiran