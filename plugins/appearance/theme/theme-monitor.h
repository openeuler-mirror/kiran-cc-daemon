/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#pragma once

#include <gtkmm.h>

#include "appearance-i.h"
#include "lib/base/base.h"

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

namespace Kiran
{
#define GTK2_MAJOR 2

enum class ThemeMonitorEventType
{
    // META主题目录添加/删除/变化
    TMET_META_ADD,
    TMET_META_DEL,
    TMET_META_CHG,
    // GTK主题目录添加/删除/变化
    TMET_GTK_ADD,
    TMET_GTK_DEL,
    TMET_GTK_CHG,
    // METACITY主题目录添加/删除/变化
    TMET_METACITY_ADD,
    TMET_METACITY_DEL,
    TMET_METACITY_CHG,
    // 图标主题目录添加/删除/变化
    TMET_ICON_ADD,
    TMET_ICON_DEL,
    TMET_ICON_CHG,
    // 光标主题目录添加/删除/变化
    TMET_CURSOR_ADD,
    TMET_CURSOR_DEL,
    TMET_CURSOR_CHG,
};

enum class ThemeMonitorType
{
    // META主题父目录监控
    THEME_MONITOR_TYPE_META_PARENT,
    // META主题目录监控
    THEME_MONITOR_TYPE_META,
    // GTK主题目录监控
    THEME_MONITOR_TYPE_GTK,
    // 窗口主题目录监控
    THEME_MONITOR_TYPE_METACITY,
    // 图标主题父目录监控
    THEME_MONITOR_TYPE_ICON_PARENT,
    // 图标主题目录监控
    THEME_MONITOR_TYPE_ICON,
    // 光标主题目录监控
    THEME_MONITOR_TYPE_CURSOR,
};

class ThemeMonitorInfo
{
public:
    ThemeMonitorInfo(Glib::RefPtr<Gio::FileMonitor> monitor,
                     ThemeMonitorType type,
                     int32_t priority,
                     const std::string &path);
    virtual ~ThemeMonitorInfo(){};

    ThemeMonitorType get_type() { return this->type_; }
    int32_t get_priority() { return this->priority_; }
    const std::string &get_path() { return this->path_; }

private:
    Glib::RefPtr<Gio::FileMonitor> monitor_;
    ThemeMonitorType type_;
    // 如果存在相同的主题名，则优先级值小的会覆盖优先级值大的主题
    int32_t priority_;
    // 监控路径
    std::string path_;
};

using ThemeMonitorInfoVec = std::vector<std::shared_ptr<ThemeMonitorInfo>>;

class ThemeMonitor
{
public:
    ThemeMonitor();
    virtual ~ThemeMonitor(){};

    void init();

    ThemeMonitorInfoVec get_monitor_infos();

    sigc::signal<void, std::shared_ptr<ThemeMonitorInfo>, ThemeMonitorEventType> signal_monitor_event() { return this->events_; }

private:
    std::shared_ptr<ThemeMonitorInfo> get_monitor(const std::string &path);

    std::shared_ptr<ThemeMonitorInfo> get_and_check_parent_monitor(const Glib::RefPtr<Gio::File> &file);

    bool add_monitor(const std::string &path, std::shared_ptr<ThemeMonitorInfo> monitor);

    std::shared_ptr<ThemeMonitorInfo> create_and_add_monitor(const std::string &path,
                                                             int32_t priority,
                                                             ThemeMonitorType type,
                                                             const FileMonitorCallBack &callback);

    void add_meta_theme_parent_monitor(const std::string &path, int32_t priority);
    void add_meta_theme_monitor(const std::string &path, int32_t priority);
    void add_gtk_theme_monitor(const std::string &path, int32_t priority);
    void add_metacity_theme_monitor(const std::string &path, int32_t priority);
    void del_theme_and_notify(const std::string &path, ThemeMonitorEventType type);

    void on_meta_theme_parent_changed(const Glib::RefPtr<Gio::File> &file,
                                      const Glib::RefPtr<Gio::File> &other_file,
                                      Gio::FileMonitorEvent event_type);
    void on_meta_theme_changed(const Glib::RefPtr<Gio::File> &file,
                               const Glib::RefPtr<Gio::File> &other_file,
                               Gio::FileMonitorEvent event_type);
    void on_gtk_theme_changed(const Glib::RefPtr<Gio::File> &file,
                              const Glib::RefPtr<Gio::File> &other_file,
                              Gio::FileMonitorEvent event_type);
    void on_metacity_theme_changed(const Glib::RefPtr<Gio::File> &file,
                                   const Glib::RefPtr<Gio::File> &other_file,
                                   Gio::FileMonitorEvent event_type);

    void add_icon_theme_parent_monitor(const std::string &path, int32_t priority);
    void add_icon_theme_monitor(const std::string &path, int32_t priority);
    void add_cursor_theme_monitor(const std::string &path, int32_t priority);

    void on_icon_theme_parent_changed(const Glib::RefPtr<Gio::File> &file,
                                      const Glib::RefPtr<Gio::File> &other_file,
                                      Gio::FileMonitorEvent event_type);
    void on_icon_theme_changed(const Glib::RefPtr<Gio::File> &file,
                               const Glib::RefPtr<Gio::File> &other_file,
                               Gio::FileMonitorEvent event_type);
    void on_cursor_theme_changed(const Glib::RefPtr<Gio::File> &file,
                                 const Glib::RefPtr<Gio::File> &other_file,
                                 Gio::FileMonitorEvent event_type);

    std::string get_gtk_dirname() { return fmt::format("gtk-{0}.0", gtk_get_major_version()); }

private:
    std::map<std::string, std::shared_ptr<ThemeMonitorInfo>> monitors_;

    sigc::signal<void, std::shared_ptr<ThemeMonitorInfo>, ThemeMonitorEventType> events_;
};  // namespace Kiran
}  // namespace Kiran