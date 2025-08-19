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

#include "theme-monitor.h"
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QIcon>
#include <QSignalBlocker>
#include <QStandardPaths>
#include "lib/base/base.h"

namespace Kiran
{
ThemeMonitorInfo::ThemeMonitorInfo(ThemeMonitorType type,
                                   int32_t priority,
                                   const QString &path) : m_type(type),
                                                          m_priority(priority),
                                                          m_path(path)
{
}

ThemeMonitor::ThemeMonitor(QObject *parent) : QObject(parent)
{
    m_filesWatcher = new QFileSystemWatcher(this);
}

void ThemeMonitor::init()
{
    auto dataDirs = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::GenericDataLocation);

    // 等初始化后再发射信号
    QSignalBlocker blocker(this);

    // 第一个为${HOME}/.local/share
    if (dataDirs.size() > 0)
    {
        auto themeDir = QString("%1/%2").arg(dataDirs[0], "themes");
        addMetaThemeParent(themeDir, 0);
    }

    auto homeThemeDir = QString("%1/%2").arg(QDir::homePath()).arg(".themes");
    addMetaThemeParent(homeThemeDir, 1);

    for (int i = 1; i < dataDirs.size(); ++i)
    {
        auto themeDir = QString("%1/%2").arg(dataDirs[i], "themes");
        addMetaThemeParent(themeDir, i + 1);
    }

    // 测试结果：(".local/share/icons", "/usr/share/icons", "/usr/local/share/icons", ":/icons")
    auto iconThemePaths = QIcon::themeSearchPaths();
    for (int i = 0; i < iconThemePaths.size(); ++i)
    {
        auto iconThemePath = iconThemePaths[i];

        CONTINUE_IF_TRUE(iconThemePath.startsWith(":/"));

        if (iconThemePath.startsWith(".local"))
        {
            iconThemePath = QString("%1/%2").arg(QDir::homePath()).arg(iconThemePath);
        }

        addIconThemeParent(iconThemePaths[i], i);
    }

    connect(m_filesWatcher, &QFileSystemWatcher::directoryChanged, this, &ThemeMonitor::updateThemeWhenDirChange);
    connect(m_filesWatcher, &QFileSystemWatcher::fileChanged, this, &ThemeMonitor::updateThemeWhenFileChange);
}

ThemeMonitorInfoVec ThemeMonitor::getMonitorInfos()
{
    ThemeMonitorInfoVec result;
    for (auto &monitor : m_monitors)
    {
        result.push_back(monitor);
    }
    return result;
}

QSharedPointer<ThemeMonitorInfo> ThemeMonitor::getMonitor(const QString &path)
{
    return m_monitors.value(path);
}

bool ThemeMonitor::hasValidMonitor(const QString &path)
{
    bool hasMonitor = m_monitors.contains(path);
    bool hasFile = m_filesWatcher->files().contains(path);
    bool hasDir = m_filesWatcher->directories().contains(path);

    return hasMonitor && (hasFile || hasDir);
}

bool ThemeMonitor::addMonitor(const QString &path, QSharedPointer<ThemeMonitorInfo> monitor)
{
    if (m_monitors.contains(path))
    {
        KLOG_INFO(appearance) << "Path" << path << "already exists";
        return false;
    }
    else
    {
        m_monitors.insert(path, monitor);
    }
    return true;
}

QSharedPointer<ThemeMonitorInfo> ThemeMonitor::createAndAddMonitor(const QString &path,
                                                                   int32_t priority,
                                                                   ThemeMonitorType type)
{
    QFileInfo fileInfo(path);

    m_monitors.remove(path);
    m_filesWatcher->addPath(path);
    auto themeMonitor = QSharedPointer<ThemeMonitorInfo>::create(type, priority, path);
    addMonitor(path, themeMonitor);

    return themeMonitor;
}

void ThemeMonitor::addMetaThemeParent(const QString &path, int32_t priority)
{
    QDir dir(path);
    RETURN_IF_FALSE(dir.exists());

    auto monitor = createAndAddMonitor(path, priority, ThemeMonitorType::THEME_MONITOR_TYPE_META_PARENT);
    RETURN_IF_FALSE(monitor);

    addMetaThemeParentChildren(monitor);
}

void ThemeMonitor::addMetaThemeParentChildren(QSharedPointer<ThemeMonitorInfo> monitorInfo)
{
    QDir dir(monitorInfo->getPath());

    for (const auto &fileInfo : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        auto filePath = fileInfo.absoluteFilePath();
        if (hasValidMonitor(filePath))
        {
            continue;
        }
        else
        {
            addMetaTheme(fileInfo.absoluteFilePath(), monitorInfo->getPriority());
        }
    }
}

void ThemeMonitor::addMetaTheme(const QString &path, int32_t priority)
{
    auto monitor = createAndAddMonitor(path, priority, ThemeMonitorType::THEME_MONITOR_TYPE_META);
    RETURN_IF_FALSE(monitor);
    Q_EMIT themeChanged(monitor, ThemeMonitorEventType::TMET_META_ADD);

    m_filesWatcher->addPath(QString("%1/index.theme").arg(path));
    addMetaThemeChildren(monitor);
}

void ThemeMonitor::addMetaThemeChildren(QSharedPointer<ThemeMonitorInfo> monitorInfo)
{
    auto path = monitorInfo->getPath();
    // gtk
    auto gtkPath = QString("%1/gtk-3.0").arg(path);
    if (!hasValidMonitor(gtkPath))
    {
        addGtkTheme(gtkPath, monitorInfo->getPriority());
    }

    // metacity-1
    auto metacityPath = QString("%1/metacity-1").arg(path);
    if (!hasValidMonitor(metacityPath))
    {
        addMetacityTheme(metacityPath, monitorInfo->getPriority());
    }
}

void ThemeMonitor::addGtkTheme(const QString &path, int32_t priority)
{
    auto monitor = createAndAddMonitor(path, priority, ThemeMonitorType::THEME_MONITOR_TYPE_GTK);
    RETURN_IF_FALSE(monitor);
    Q_EMIT themeChanged(monitor, ThemeMonitorEventType::TMET_GTK_ADD);

    m_filesWatcher->addPath(QString("%1/gtk.css").arg(path));
}

void ThemeMonitor::addMetacityTheme(const QString &path, int32_t priority)
{
    auto monitor = createAndAddMonitor(path, priority, ThemeMonitorType::THEME_MONITOR_TYPE_METACITY);
    RETURN_IF_FALSE(monitor);
    Q_EMIT themeChanged(monitor, ThemeMonitorEventType::TMET_METACITY_ADD);

    m_filesWatcher->addPath(QString("%1/metacity-theme-1.xml").arg(path));
    m_filesWatcher->addPath(QString("%1/metacity-theme-2.xml").arg(path));
    m_filesWatcher->addPath(QString("%1/metacity-theme-3.xml").arg(path));
}

void ThemeMonitor::delThemeAndNotify(const QString &path, ThemeMonitorEventType type)
{
    RETURN_IF_FALSE(path.length() > 0);
    RETURN_IF_FALSE(QFileInfo(path).isDir());
    auto monitor = getMonitor(path);
    if (!monitor)
    {
        KLOG_WARNING(appearance) << "Not found monitor info for" << path;
        return;
    }
    Q_EMIT themeChanged(monitor, type);
    m_monitors.remove(path);
    m_filesWatcher->removePath(path);
}

void ThemeMonitor::addIconThemeParent(const QString &path, int32_t priority)
{
    QDir dir(path);
    RETURN_IF_FALSE(dir.exists());

    auto monitorInfo = createAndAddMonitor(path, priority, ThemeMonitorType::THEME_MONITOR_TYPE_ICON_PARENT);
    RETURN_IF_FALSE(monitorInfo);

    addIconThemeParentChildren(monitorInfo);
}

void ThemeMonitor::addIconThemeParentChildren(QSharedPointer<ThemeMonitorInfo> monitorInfo)
{
    QDir dir(monitorInfo->getPath());

    for (const auto &fileInfo : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        auto filePath = fileInfo.absoluteFilePath();
        if (getMonitor(filePath))
        {
            continue;
        }
        else
        {
            addIconTheme(fileInfo.absoluteFilePath(), monitorInfo->getPriority());
        }
    }
}

void ThemeMonitor::addIconTheme(const QString &path, int32_t priority)
{
    auto monitor = createAndAddMonitor(path, priority, ThemeMonitorType::THEME_MONITOR_TYPE_ICON);
    RETURN_IF_FALSE(monitor);

    Q_EMIT themeChanged(monitor, ThemeMonitorEventType::TMET_ICON_ADD);

    m_filesWatcher->addPath(QString("%1/index.theme").arg(path));
    addIconThemeChildren(monitor);
}

void ThemeMonitor::addIconThemeChildren(QSharedPointer<ThemeMonitorInfo> monitorInfo)
{
    auto path = monitorInfo->getPath();

    // cursor
    auto cursorPath = QString("%1/cursors").arg(path);
    if (!getMonitor(cursorPath))
    {
        addCursorTheme(cursorPath, monitorInfo->getPriority());
    }
}

void ThemeMonitor::addCursorTheme(const QString &path, int32_t priority)
{
    auto monitor = createAndAddMonitor(path, priority, ThemeMonitorType::THEME_MONITOR_TYPE_CURSOR);
    RETURN_IF_FALSE(monitor);
    Q_EMIT themeChanged(monitor, ThemeMonitorEventType::TMET_CURSOR_ADD);
}

void ThemeMonitor::updateThemeWhenDirChange(const QString &dirPath)
{
    QDir dir(dirPath);
    auto monitorInfo = getMonitor(dirPath);

    if (!monitorInfo)
    {
        KLOG_WARNING(appearance) << "Not found monitor info for path" << dirPath;
        return;
    }

    switch (monitorInfo->getType())
    {
    case THEME_MONITOR_TYPE_META_PARENT:
        // 根目录只关注添加，不关注删除，因为删除后就不再监听范围了，没法再恢复监控
        addMetaThemeParentChildren(monitorInfo);
        break;
    case THEME_MONITOR_TYPE_META:
        updateMetaTheme(monitorInfo);
        break;
    case THEME_MONITOR_TYPE_GTK:
    {
        if (!dir.exists())
        {
            delThemeAndNotify(monitorInfo->getPath(), ThemeMonitorEventType::TMET_GTK_DEL);
            m_filesWatcher->removePath(QString("%1/gtk.css").arg(monitorInfo->getPath()));
        }
        else
        {
            // 这里需要重新添加，因为可能文件之前被删除过然后又重新新增了。如果之前被删除过会自动从QFileSystemWatcher中移除
            m_filesWatcher->addPath(QString("%1/gtk.css").arg(monitorInfo->getPath()));
        }
        break;
    }
    case THEME_MONITOR_TYPE_METACITY:
    {
        if (!dir.exists())
        {
            delThemeAndNotify(monitorInfo->getPath(), ThemeMonitorEventType::TMET_METACITY_DEL);
            m_filesWatcher->removePath(QString("%1/metacity-theme-1.xml").arg(monitorInfo->getPath()));
            m_filesWatcher->removePath(QString("%1/metacity-theme-2.xml").arg(monitorInfo->getPath()));
            m_filesWatcher->removePath(QString("%1/metacity-theme-3.xml").arg(monitorInfo->getPath()));
        }
        else
        {
            m_filesWatcher->addPath(QString("%1/metacity-theme-1.xml").arg(monitorInfo->getPath()));
            m_filesWatcher->addPath(QString("%1/metacity-theme-2.xml").arg(monitorInfo->getPath()));
            m_filesWatcher->addPath(QString("%1/metacity-theme-3.xml").arg(monitorInfo->getPath()));
        }
        break;
    }
    case THEME_MONITOR_TYPE_ICON_PARENT:
        addIconThemeParentChildren(monitorInfo);
        break;
    case THEME_MONITOR_TYPE_ICON:
        updateIconTheme(monitorInfo);
        break;
    case THEME_MONITOR_TYPE_CURSOR:
    {
        if (!dir.exists())
        {
            delThemeAndNotify(monitorInfo->getPath(), ThemeMonitorEventType::TMET_CURSOR_DEL);
        }
        else
        {
            Q_EMIT themeChanged(monitorInfo, ThemeMonitorEventType::TMET_CURSOR_CHG);
        }
        break;
    }
    default:
        break;
    }
}

void ThemeMonitor::updateThemeWhenFileChange(const QString &filePath)
{
    auto dirPath = QFileInfo(filePath).absolutePath();
    auto monitorInfo = getMonitor(dirPath);

    if (!monitorInfo)
    {
        KLOG_WARNING(appearance) << "Not found monitor info for path" << dirPath;
        return;
    }

    switch (monitorInfo->getType())
    {
    case THEME_MONITOR_TYPE_META:
    {
        if (filePath.endsWith("index.theme"))
        {
            Q_EMIT themeChanged(monitorInfo, ThemeMonitorEventType::TMET_META_CHG);
        }
        break;
    }
    case THEME_MONITOR_TYPE_GTK:
    {
        if (filePath.endsWith("gtk.css"))
        {
            Q_EMIT themeChanged(monitorInfo, ThemeMonitorEventType::TMET_GTK_CHG);
        }
        break;
    }
    case THEME_MONITOR_TYPE_METACITY:
    {
        if (filePath.endsWith("metacity-theme-1.xml") ||
            filePath.endsWith("metacity-theme-2.xml") ||
            filePath.endsWith("metacity-theme-3.xml"))
        {
            Q_EMIT themeChanged(monitorInfo, ThemeMonitorEventType::TMET_METACITY_CHG);
        }
        break;
    }
    case THEME_MONITOR_TYPE_ICON:
    {
        if (filePath.endsWith("index.theme"))
        {
            Q_EMIT themeChanged(monitorInfo, ThemeMonitorEventType::TMET_ICON_CHG);
        }
        break;
    }
    default:
        break;
    }
}

void ThemeMonitor::updateMetaTheme(QSharedPointer<ThemeMonitorInfo> monitorInfo)
{
    QDir dir(monitorInfo->getPath());

    // 如果目录还存在，说明是目录内容发生了变化，这里只考虑子目录增加的情况，删除的情况由监听子目录变化信号处理
    if (dir.exists())
    {
        addMetaThemeChildren(monitorInfo);
    }
    else
    {
        delThemeAndNotify(monitorInfo->getPath(), ThemeMonitorEventType::TMET_META_DEL);
        m_filesWatcher->removePath(QString("%1/index.theme").arg(monitorInfo->getPath()));
    }
}

void ThemeMonitor::updateIconTheme(QSharedPointer<ThemeMonitorInfo> monitorInfo)
{
    QDir dir(monitorInfo->getPath());

    // 如果目录还存在，说明是目录内容发生了变化，这里只考虑子目录增加的情况，删除的情况由监听子目录变化信号处理
    if (dir.exists())
    {
        addIconThemeChildren(monitorInfo);
    }
    else
    {
        delThemeAndNotify(monitorInfo->getPath(), ThemeMonitorEventType::TMET_ICON_DEL);
        m_filesWatcher->removePath(QString("%1/index.theme").arg(monitorInfo->getPath()));
    }
}

}  // namespace Kiran