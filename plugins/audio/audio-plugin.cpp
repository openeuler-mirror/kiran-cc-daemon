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

#include "audio-plugin.h"
#include "audio-manager.h"
#include "pulse/pulse-backend.h"

namespace Kiran
{
void AudioPlugin::activate()
{
    PulseBackend::globalInit();
    AudioManager::globalInit(PulseBackend::getInstance());
}

void AudioPlugin::deactivate()
{
    AudioManager::globalDeinit();
    PulseBackend::globalDeinit();
}

}  // namespace Kiran