/**
 * Copyright (c) 2025 ~ 2026 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */

#pragma once

#include <QGuiApplication>
#include <QPainter>
#include <QScreen>
#include <QVector>
#include <QWidget>

namespace Kiran
{
class DesktopBackground : public QWidget
{
    Q_OBJECT
public:
    DesktopBackground(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        // NOTE: 这个接口可能只有X11 支持
        setAttribute(Qt::WA_X11NetWmWindowTypeDesktop);
        showFullScreen();
    }

    void drawBackground(const QString &imagePath)
    {
        m_backgroundImage = imagePath;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);
        QImage image(m_backgroundImage);
        painter.drawImage(rect(), image);
    }

private:
    QString m_backgroundImage;
};

class DesktopView : public QObject
{
    Q_OBJECT
public:
    DesktopView(QObject *parent = nullptr)
    {
        // 监控 QGuiApplication::screens() 变化
        connect(qApp, &QGuiApplication::primaryScreenChanged, this, &DesktopView::update);
        connect(qApp, &QGuiApplication::screenAdded, this, &DesktopView::update);
        connect(qApp, &QGuiApplication::screenRemoved, this, &DesktopView::update);

        update();
    }
    ~DesktopView()
    {
        while (!m_backgrounds.isEmpty())
        {
            auto background = m_backgrounds.takeLast();
            delete background;
        }
    }

    void update()
    {
        int screenCount = QGuiApplication::screens().size();
        for (int i = 0; i < screenCount; ++i)
        {
            DesktopBackground *background;
            if (m_backgrounds.size() < i + 1)
            {
                background = new DesktopBackground;
                m_backgrounds.push_back(background);
            }
            else
            {
                background = m_backgrounds[i];
            }

            QScreen *screen = QGuiApplication::screens().at(i);
            background->move(screen->geometry().topLeft());
            background->update();
        }

        while (m_backgrounds.size() > screenCount)
        {
            auto background = m_backgrounds.takeLast();
            delete background;
        }
    }

    void setBackground(const QString &imagePath)
    {
        for (auto &background : m_backgrounds)
        {
            background->drawBackground(imagePath);
        }
    }

private:
    QVector<DesktopBackground *> m_backgrounds;
};
}  // namespace Kiran