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

#include "pulse-device.h"

namespace Kiran
{
class PulseContext;

class PulseSource : public PulseDevice
{
    Q_OBJECT

public:
    PulseSource(QSharedPointer<PulseContext> context, const pa_source_info *source_info);
    virtual ~PulseSource() {};

    void update(const pa_source_info *source_info);

    // 设置活动的端口
    virtual bool setActivePort(const QString &port_name) override;
    virtual bool setMute(int32_t mute) override;
    virtual bool setCvolume(const pa_cvolume &cvolume) override;

private:
    QSharedPointer<PulseContext> m_context;
};

}  // namespace Kiran