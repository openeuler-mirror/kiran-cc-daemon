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
 * Author:     meizhigang <meizhigang@kylinsec.com.cn>
 */

#include "osd-window.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QIcon>
#include <QTimer>

namespace Kiran
{
// dialog show timeout seconds
#define ICON_SHOW_TIMOUT_MSEC 3000
#define ICON_SIZE 120

OSDWindow::OSDWindow()
{
    m_timer = new QTimer(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}

OSDWindow *OSDWindow::m_instance = nullptr;
OSDWindow *OSDWindow::getDefault()
{
    if (!m_instance)
    {
        m_instance = new OSDWindow();
        m_instance->init();
    }
    return m_instance;
}

void OSDWindow::init()
{
    setFixedSize(ICON_SIZE, ICON_SIZE);

    connect(m_timer, &QTimer::timeout, this, &OSDWindow::hideIcon);
}

void OSDWindow::showIcon(const QString &iconName)
{
    hide();

    m_iconName = iconName;
    auto icon = QIcon::fromTheme(iconName);
    auto pixmap = icon.pixmap(QSize(ICON_SIZE, ICON_SIZE));
    setPixmap(pixmap);

    auto desktop = QApplication::desktop();
    auto screenWidth = desktop->width();
    auto screenHeight = desktop->height();
    int iconWidth = pixmap.width();
    int iconHeight = pixmap.height();
    move((screenWidth - iconWidth) / 2, (screenHeight - iconHeight) / 2);

    show();
    m_timer->start(ICON_SHOW_TIMOUT_MSEC);
}

void OSDWindow::hideIcon()
{
    m_timer->stop();
    hide();
}

}  // namespace  Kiran
