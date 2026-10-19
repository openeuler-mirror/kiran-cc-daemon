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

#include "background-view.h"

#include <QGuiApplication>
#include <QPainter>
#include <QScreen>
#include <QSet>
#include <QTimer>
#include <QWindow>

#include "lib/base/base.h"

namespace Kiran
{
namespace
{
// 屏幕变化事件去抖时间。热插拔过程中屏幕列表可能连续变化多次，合并处理避免反复删除/创建窗口
const int SYNC_DEBOUNCE_MS = 200;
// 热插拔过程中屏幕列表可能瞬时为空，最多重试次数
const int MAX_EMPTY_SCREEN_RETRIES = 5;
}  // namespace

DesktopBackground::DesktopBackground(QWidget *parent)
    : QWidget(parent),
      m_pixmapWarned(false)
{
    // NOTE: 该窗口类型只在 X11(xcb) 下有效
    setAttribute(Qt::WA_X11NetWmWindowTypeDesktop);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::NoFocus);
}

void DesktopBackground::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    m_pixmapWarned = false;
    update();
}

void DesktopBackground::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    if (m_pixmap.isNull())
    {
        // 没有背景图时填充窗口底色，避免窗口内容未定义导致整块屏幕显示异常
        if (!m_pixmapWarned)
        {
            m_pixmapWarned = true;
            KLOG_WARNING(appearance) << "Desktop background pixmap is null.";
        }
        painter.fillRect(rect(), palette().window());
        return;
    }

    painter.drawPixmap(rect(), m_pixmap);
}

DesktopView::DesktopView(QObject *parent)
    : QObject(parent),
      m_syncTimer(nullptr),
      m_emptyRetries(0)
{
    connect(qApp, &QGuiApplication::screenAdded, this, &DesktopView::onScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &DesktopView::onScreenRemoved);
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, &DesktopView::scheduleSync);

    // 首次同步直接执行，避免启动时背景延迟出现
    syncScreens();
}

DesktopView::~DesktopView()
{
    for (auto *background : m_backgrounds)
    {
        delete background;
    }
    m_backgrounds.clear();
}

void DesktopView::setBackground(const QString &imagePath)
{
    m_backgroundImage = imagePath;

    QPixmap pixmap(imagePath);
    if (pixmap.isNull())
    {
        KLOG_WARNING(appearance) << "Load desktop background image failed:" << imagePath;
        m_pixmap = QPixmap();
    }
    else
    {
        m_pixmap = pixmap;
    }

    // 已有窗口立即更新，之后新建的窗口在 ensureBackground() 中继承该图
    for (auto *background : m_backgrounds)
    {
        background->setPixmap(m_pixmap);
    }

    KLOG_INFO(appearance) << "Set desktop background" << imagePath
                          << "windows:" << m_backgrounds.size();
}

void DesktopView::onScreenAdded(QScreen *screen)
{
    KLOG_INFO(appearance) << "Screen added:" << screen->name() << screen->geometry();
    scheduleSync();
}

void DesktopView::onScreenRemoved(QScreen *screen)
{
    KLOG_INFO(appearance) << "Screen removed:" << screen->name();

    // QScreen 在槽函数返回后可能被销毁，必须同步清理，避免 QHash 中残留失效指针
    auto *background = m_backgrounds.take(screen);
    if (background)
    {
        background->hide();
        background->deleteLater();
    }

    scheduleSync();
}

void DesktopView::scheduleSync()
{
    if (!m_syncTimer)
    {
        m_syncTimer = new QTimer(this);
        m_syncTimer->setSingleShot(true);
        connect(m_syncTimer, &QTimer::timeout, this, &DesktopView::syncScreens);
    }

    m_syncTimer->start(SYNC_DEBOUNCE_MS);
}

void DesktopView::syncScreens()
{
    if (m_syncTimer)
    {
        m_syncTimer->stop();
    }

    const auto screens = QGuiApplication::screens();

    // 热插拔时屏幕列表可能瞬时为空，此时不能销毁窗口，否则恢复后新窗口拿不到背景
    if (screens.isEmpty() && !m_backgrounds.isEmpty())
    {
        if (++m_emptyRetries <= MAX_EMPTY_SCREEN_RETRIES)
        {
            KLOG_INFO(appearance) << "Screen list is empty temporarily, retry sync.";
            scheduleSync();
            return;
        }
        KLOG_WARNING(appearance) << "Screen list stays empty, keep desktop background windows.";
    }
    m_emptyRetries = 0;

    if (!screens.isEmpty())
    {
        // 清理已不存在屏幕对应的窗口，只做指针比较，不解引用可能已失效的 key
        const QSet<QScreen *> aliveScreens(screens.begin(), screens.end());
        QList<QScreen *> staleScreens;
        for (auto iter = m_backgrounds.constBegin(); iter != m_backgrounds.constEnd(); ++iter)
        {
            if (!aliveScreens.contains(iter.key()))
            {
                staleScreens.push_back(iter.key());
            }
        }

        for (auto *staleScreen : staleScreens)
        {
            auto *background = m_backgrounds.take(staleScreen);
            if (!background)
            {
                continue;
            }

            background->hide();
            background->deleteLater();
            KLOG_INFO(appearance) << "Destroy stale desktop background window.";
        }
    }

    // 为每块屏幕确保有窗口、有背景图，并与屏幕几何保持一致
    for (auto *screen : screens)
    {
        auto *background = ensureBackground(screen);
        background->setGeometry(screen->geometry());
        background->show();

        connect(screen, &QScreen::geometryChanged, this, &DesktopView::relayout, Qt::UniqueConnection);
    }

    if (!screens.isEmpty())
    {
        KLOG_INFO(appearance) << "Desktop background synced, screens:" << screens.size()
                              << "windows:" << m_backgrounds.size()
                              << "image:" << m_backgroundImage;
    }
}

void DesktopView::relayout()
{
    const auto screens = QGuiApplication::screens();
    for (auto *screen : screens)
    {
        auto *background = m_backgrounds.value(screen, nullptr);
        if (!background)
        {
            continue;
        }

        const QRect geometry = screen->geometry();
        if (background->geometry() != geometry)
        {
            background->setGeometry(geometry);
        }
        background->update();
    }
}

DesktopBackground *DesktopView::ensureBackground(QScreen *screen)
{
    auto *background = m_backgrounds.value(screen, nullptr);
    if (background)
    {
        return background;
    }

    background = new DesktopBackground;

    // 新建窗口立即继承当前背景图，这是热插拔后背景不丢失的关键
    background->setPixmap(m_pixmap);

    // 先创建 native window，才能把窗口绑定到目标屏幕（多 X screen 场景必需）
    background->winId();
    if (background->windowHandle())
    {
        background->windowHandle()->setScreen(screen);
    }
    background->setGeometry(screen->geometry());

    m_backgrounds.insert(screen, background);

    KLOG_INFO(appearance) << "Create desktop background window for screen" << screen->name()
                          << screen->geometry();
    return background;
}

}  // namespace Kiran
