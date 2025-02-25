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

#pragma once

#include <pulse/introspect.h>
#include <QSharedPointer>
#include "pulse-stream.h"

namespace Kiran
{
class PulseContext;

class PulseSourceOutput : public PulseStream
{
    Q_OBJECT

public:
    PulseSourceOutput(QSharedPointer<PulseContext> context,
                      const pa_source_output_info *sourceOutputInfo);
    virtual ~PulseSourceOutput(){};

    void update(const pa_source_output_info *sourceOutputInfo);

    virtual bool setMute(int32_t mute) override;
    virtual bool setCvolume(const pa_cvolume &cvolume) override;

private:
    QSharedPointer<PulseContext> m_context;
};

using PulseSourceOutputVec = QList<QSharedPointer<PulseSourceOutput>>;
}  // namespace Kiran