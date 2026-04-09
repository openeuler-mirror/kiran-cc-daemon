/**
 * Copyright (c) 2020 ~ 2026 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:    gaobo <gaobo@kylinsec.com.cn>
 */

#pragma once

#include "plugin-i.h"

class QTranslator;

namespace Kiran
{
class DiskSpaceMonitorPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPLUGIN_IID FILE "disk-space.json")
    Q_INTERFACES(Kiran::IPlugin)

public:
    void activate() override;
    void deactivate() override;

private:
    QTranslator* m_translator = nullptr;
};
}  // namespace Kiran
