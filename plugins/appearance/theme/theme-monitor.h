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

#include "appearance-i.h"
#include "lib/base/base.h"
#include "theme-data.h"

/*
GTK3读取css的顺序如下：
    $XDG_CONFIG_HOME/gtk-3.0/gtk.css
    $XDG_DATA_HOME/themes/THEME/gtk-VERSION/gtk.css
    $HOME/.themes/THEME/gtk-VERSION/gtk.css
    $XDG_DATA_DIRS/themes/THEME/gtk-VERSION/gtk.css
    DATADIR/share/themes/THEME/gtk-VERSION/gtk.css
DATADIR是在GTK3配置文件中设置或者来自环境变量GTK_DATA_PREFIX，目前GTK3没有提供API获取这个值，因此这里不对DATADIR/share/themes进行监控

VERSION的的格式为major.minor，假设当前使用的GTK版本为GTK3-3.22，则搜索顺序为：
gtk-3.22 -> gtk-3.20 -> gtk-3.18 -> gtk-3.16 -> gtk-3.14 -> gtk-3.0, 0到14中间不再搜索
只要对应的目录存在则停止搜索，然后使用这个目录作为THEME主题目录
*/

class QFileSystemWatcher;

namespace Kiran
{
class ThemeMonitorInfo
{
public:
    ThemeMonitorInfo(ThemeMonitorType type, int32_t priority, const QString &path);
    virtual ~ThemeMonitorInfo(){};

    ThemeMonitorType getType() { return this->m_type; }
    int32_t getPriority() { return this->m_priority; }
    const QString &getPath() { return this->m_path; }

private:
    ThemeMonitorType m_type;
    // 如果存在相同的主题名，则优先级值小的会覆盖优先级值大的主题
    int32_t m_priority;
    // 监控路径
    QString m_path;
};

using ThemeMonitorInfoVec = std::vector<QSharedPointer<ThemeMonitorInfo>>;

class ThemeMonitor : public QObject
{
    Q_OBJECT

public:
    ThemeMonitor(QObject *parent = nullptr);
    virtual ~ThemeMonitor(){};

    void init();

    ThemeMonitorInfoVec getMonitorInfos();

Q_SIGNALS:
    void themeChanged(QSharedPointer<ThemeMonitorInfo>, ThemeMonitorEventType);

private:
    QSharedPointer<ThemeMonitorInfo> getMonitor(const QString &path);

    bool addMonitor(const QString &path, QSharedPointer<ThemeMonitorInfo> monitor);

    QSharedPointer<ThemeMonitorInfo> createAndAddMonitor(const QString &path, int32_t priority, ThemeMonitorType type);

    void addMetaThemeParent(const QString &path, int32_t priority);
    void addMetaThemeParentChildren(QSharedPointer<ThemeMonitorInfo> monitorInfo);
    void addMetaTheme(const QString &path, int32_t priority);
    void addMetaThemeChildren(QSharedPointer<ThemeMonitorInfo> monitorInfo);
    void addGtkTheme(const QString &path, int32_t priority);
    void addMetacityTheme(const QString &path, int32_t priority);
    void delThemeAndNotify(const QString &path, ThemeMonitorEventType type);

    void addIconThemeParent(const QString &path, int32_t priority);
    void addIconThemeParentChildren(QSharedPointer<ThemeMonitorInfo> monitorInfo);
    void addIconTheme(const QString &path, int32_t priority);
    void addIconThemeChildren(QSharedPointer<ThemeMonitorInfo> monitorInfo);
    void addCursorTheme(const QString &path, int32_t priority);

    void updateThemeWhenDirChange(const QString &dirPath);
    void updateThemeWhenFileChange(const QString &filePath);
    void updateMetaTheme(QSharedPointer<ThemeMonitorInfo> monitorInfo);
    void updateIconTheme(QSharedPointer<ThemeMonitorInfo> monitorInfo);

private:
    QMap<QString, QSharedPointer<ThemeMonitorInfo>> m_monitors;
    QFileSystemWatcher *m_filesWatcher;

};  // namespace Kiran
}  // namespace Kiran