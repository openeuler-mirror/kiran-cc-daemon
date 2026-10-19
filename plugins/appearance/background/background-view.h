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

#include <QHash>
#include <QPixmap>
#include <QWidget>

class QScreen;
class QTimer;

namespace Kiran
{
// 单块屏幕上的桌面背景窗口
class DesktopBackground : public QWidget
{
    Q_OBJECT
public:
    explicit DesktopBackground(QWidget *parent = nullptr);
    ~DesktopBackground() override = default;

    // 设置背景图，由 DesktopView 统一持有并下发给各屏幕窗口
    void setPixmap(const QPixmap &pixmap);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap m_pixmap;
    bool m_pixmapWarned;
};

// 桌面背景窗口管理
class DesktopView : public QObject
{
    Q_OBJECT
public:
    explicit DesktopView(QObject *parent = nullptr);
    ~DesktopView() override;

    void setBackground(const QString &imagePath);

private:
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);
    void scheduleSync();
    void syncScreens();
    void relayout();
    DesktopBackground *ensureBackground(QScreen *screen);

private:
    QHash<QScreen *, DesktopBackground *> m_backgrounds;
    QTimer *m_syncTimer;
    QString m_backgroundImage;
    QPixmap m_pixmap;
    int m_emptyRetries;
};
}  // namespace Kiran
