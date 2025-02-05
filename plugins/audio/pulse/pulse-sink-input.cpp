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

#include "pulse-sink-input.h"
#include "pulse-context.h"

namespace Kiran
{
PulseSinkInput::PulseSinkInput(QSharedPointer<PulseContext> context,
                               const pa_sink_input_info *sinkInputInfo) : PulseStream(PulseStreamInfo(sinkInputInfo)),
                                                                          m_context(context)
{
}

void PulseSinkInput::update(const pa_sink_input_info *sinkInputInfo)
{
    RETURN_IF_FALSE(sinkInputInfo != NULL);
    PulseStream::update(PulseStreamInfo(sinkInputInfo));
}

bool PulseSinkInput::setMute(int32_t mute)
{
    return m_context->setSinkInputMute(getIndex(), mute);
}

bool PulseSinkInput::setCvolume(const pa_cvolume &cvolume)
{
    return m_context->setSinkInputVolume(getIndex(), &cvolume);
}

}  // namespace Kiran