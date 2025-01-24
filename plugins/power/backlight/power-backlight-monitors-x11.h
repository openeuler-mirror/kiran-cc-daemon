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

#include <QSocketNotifier>
#include "power-backlight-interface.h"

typedef uint32_t xcb_atom_t;

namespace Kiran
{
class XcbConnection;
class PowerBacklightMonitorsX11 : public PowerBacklightMonitors
{
    Q_OBJECT

public:
    PowerBacklightMonitorsX11();
    virtual ~PowerBacklightMonitorsX11();

    virtual void init();
    // 获取所有显示器亮度设置对象
    virtual PowerBacklightAbsoluteList getMonitors() { return m_backlightMonitors; }

private:
    bool initXrandr();
    xcb_atom_t getBacklightAtom();

    // 是否支持设置亮度
    bool supportBacklightExtension() { return m_extensionSupported; };

    void loadResource();

    void handleXcbEvent();

private Q_SLOTS:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    void
    processXcbSocketActivated(QSocketDescriptor socket, QSocketNotifier::Type activationEvent);
#else
    void
    processXcbSocketActivated(int socket);
#endif

private:
    XcbConnection *m_xcbConnection;
    QSocketNotifier *m_xcbSocketNotifier;
    int32_t m_eventBase;
    int32_t m_errorBase;
    bool m_extensionSupported;
    xcb_atom_t m_backlightAtom;
    PowerBacklightAbsoluteList m_backlightMonitors;
};
}  // namespace Kiran