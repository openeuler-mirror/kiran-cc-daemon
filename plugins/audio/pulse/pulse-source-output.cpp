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

#include "pulse-source-output.h"
#include "pulse-context.h"

namespace Kiran
{
PulseSourceOutput::PulseSourceOutput(QSharedPointer<PulseContext> context,
                                     const pa_source_output_info *sourceOutputInfo) : PulseStream(PulseStreamInfo(sourceOutputInfo)),
                                                                                      m_context(context)
{
}

void PulseSourceOutput::update(const pa_source_output_info *sourceOutputInfo)
{
    RETURN_IF_FALSE(sourceOutputInfo != NULL);
    PulseStream::update(PulseStreamInfo(sourceOutputInfo));
}

bool PulseSourceOutput::setMute(int32_t mute)
{
    return m_context->setSourceOutputMute(getIndex(), mute);
}

bool PulseSourceOutput::setCvolume(const pa_cvolume &cvolume)
{
    return m_context->setSourceOutputVolume(getIndex(), &cvolume);
}

}  // namespace Kiran