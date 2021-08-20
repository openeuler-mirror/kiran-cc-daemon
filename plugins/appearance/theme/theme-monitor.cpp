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

#include "plugins/appearance/theme/theme-monitor.h"

namespace Kiran
{
ThemeMonitorInfo::ThemeMonitorInfo(Glib::RefPtr<Gio::FileMonitor> monitor,
                                   ThemeMonitorType type,
                                   int32_t priority,
                                   const std::string &path) : monitor_(monitor),
                                                              type_(type),
                                                              priority_(priority),
                                                              path_(path)
{
}

ThemeMonitor::ThemeMonitor()
{
}

void ThemeMonitor::init()
{
    std::string themes_dir;

    // meta主题目录
    themes_dir = Glib::build_path(G_DIR_SEPARATOR_S, std::vector<std::string>{Glib::get_user_data_dir(), "themes"});
    this->add_meta_theme_parent_monitor(themes_dir, 0);

    themes_dir = Glib::build_path(G_DIR_SEPARATOR_S, std::vector<std::string>{Glib::get_home_dir(), ".themes"});
    this->add_meta_theme_parent_monitor(themes_dir, 1);

    int32_t i = 2;
    for (const auto &iter : Glib::get_system_data_dirs())
    {
        themes_dir = Glib::build_path(G_DIR_SEPARATOR_S, std::vector<std::string>{iter, "themes"});
        this->add_meta_theme_parent_monitor(themes_dir, i++);
    }

    // 图标主题目录
    auto icon_theme = Gtk::IconTheme::get_default();
    i = 0;
    for (auto &icon_path : icon_theme->get_search_path())
    {
        this->add_icon_theme_parent_monitor(icon_path, i++);
    }
}

ThemeMonitorInfoVec ThemeMonitor::get_monitor_infos()
{
    ThemeMonitorInfoVec result;
    for (auto &itr : this->monitors_)
    {
        result.push_back(itr.second);
    }
    return result;
}

std::shared_ptr<ThemeMonitorInfo> ThemeMonitor::get_monitor(const std::string &path)
{
    auto iter = this->monitors_.find(path);
    if (iter != this->monitors_.end())
    {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<ThemeMonitorInfo> ThemeMonitor::get_and_check_parent_monitor(const Glib::RefPtr<Gio::File> &file)
{
    RETURN_VAL_IF_FALSE(file, nullptr);

    auto parent_file = file->get_parent();
    RETURN_VAL_IF_FALSE(parent_file, nullptr);

    auto monitor = this->get_monitor(parent_file->get_path());
    if (!monitor)
    {
        KLOG_WARNING("Not found monitor info for: %s.", parent_file->get_path().c_str());
        return nullptr;
    }
    return monitor;
}

bool ThemeMonitor::add_monitor(const std::string &path, std::shared_ptr<ThemeMonitorInfo> monitor)
{
    auto iter = this->monitors_.emplace(path, monitor);
    if (!iter.second)
    {
        KLOG_DEBUG("Path already exists: %s.", path.c_str());
        return false;
    }
    return true;
}

std::shared_ptr<ThemeMonitorInfo> ThemeMonitor::create_and_add_monitor(const std::string &path,
                                                                       int32_t priority,
                                                                       ThemeMonitorType type,
                                                                       const FileMonitorCallBack &callback)
{
    RETURN_VAL_IF_FALSE(path.length() > 0, nullptr);
    RETURN_VAL_IF_FALSE(Glib::file_test(path, Glib::FILE_TEST_IS_DIR), nullptr);

    auto file_monitor = FileUtils::make_monitor_directory(path, callback);
    auto theme_monitor = std::make_shared<ThemeMonitorInfo>(file_monitor, type, priority, path);
    RETURN_VAL_IF_FALSE(this->add_monitor(path, theme_monitor), nullptr);
    return theme_monitor;
}

void ThemeMonitor::add_meta_theme_parent_monitor(const std::string &path, int32_t priority)
{
    auto monitor = this->create_and_add_monitor(path,
                                                priority,
                                                ThemeMonitorType::THEME_MONITOR_TYPE_META_PARENT,
                                                sigc::mem_fun(this, &ThemeMonitor::on_meta_theme_parent_changed));
    RETURN_IF_FALSE(monitor);

    auto file_dir = Gio::File::create_for_path(path);
    try
    {
        auto file_iter = file_dir->enumerate_children(G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_STANDARD_NAME);
        for (auto file_info = file_iter->next_file(); file_info; file_info = file_iter->next_file())
        {
            if (file_info->get_file_type() == Gio::FILE_TYPE_DIRECTORY ||
                file_info->get_file_type() == Gio::FILE_TYPE_SYMBOLIC_LINK)
            {
                auto name = file_info->get_name();
                auto child_path = Glib::build_path(G_DIR_SEPARATOR_S, std::vector<std::string>{path, name});
                this->add_meta_theme_monitor(child_path, priority);
            }
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return;
    }
}

void ThemeMonitor::add_meta_theme_monitor(const std::string &path, int32_t priority)
{
    auto monitor = this->create_and_add_monitor(path,
                                                priority,
                                                ThemeMonitorType::THEME_MONITOR_TYPE_META,
                                                sigc::mem_fun(this, &ThemeMonitor::on_meta_theme_changed));
    RETURN_IF_FALSE(monitor);
    this->events_.emit(monitor, ThemeMonitorEventType::TMET_META_ADD);

    // gtk
    auto gtk_path = Glib::build_path(G_DIR_SEPARATOR_S, std::vector<std::string>{path, this->get_gtk_dirname()});
    this->add_gtk_theme_monitor(gtk_path, priority);

    // metacity-1
    auto metacity_path = Glib::build_path(G_DIR_SEPARATOR_S, std::vector<std::string>{path, "metacity-1"});
    this->add_metacity_theme_monitor(metacity_path, priority);
}

void ThemeMonitor::add_gtk_theme_monitor(const std::string &path, int32_t priority)
{
    auto monitor = this->create_and_add_monitor(path,
                                                priority,
                                                ThemeMonitorType::THEME_MONITOR_TYPE_GTK,
                                                sigc::mem_fun(this, &ThemeMonitor::on_gtk_theme_changed));
    RETURN_IF_FALSE(monitor);
    this->events_.emit(monitor, ThemeMonitorEventType::TMET_GTK_ADD);
}

void ThemeMonitor::add_metacity_theme_monitor(const std::string &path, int32_t priority)
{
    auto monitor = this->create_and_add_monitor(path,
                                                priority,
                                                ThemeMonitorType::THEME_MONITOR_TYPE_METACITY,
                                                sigc::mem_fun(this, &ThemeMonitor::on_metacity_theme_changed));
    RETURN_IF_FALSE(monitor);
    this->events_.emit(monitor, ThemeMonitorEventType::TMET_METACITY_ADD);
}

void ThemeMonitor::del_theme_and_notify(const std::string &path, ThemeMonitorEventType type)
{
    RETURN_IF_FALSE(path.length() > 0);
    RETURN_IF_FALSE(Glib::file_test(path, Glib::FILE_TEST_IS_DIR));
    auto monitor = this->get_monitor(path);
    if (!monitor)
    {
        KLOG_WARNING("Not found monitor info for %s.", path.c_str());
        return;
    }
    this->events_.emit(monitor, type);
    this->monitors_.erase(monitor->get_path());
}

void ThemeMonitor::on_meta_theme_parent_changed(const Glib::RefPtr<Gio::File> &file,
                                                const Glib::RefPtr<Gio::File> &other_file,
                                                Gio::FileMonitorEvent event_type)
{
    auto monitor = this->get_and_check_parent_monitor(file);
    RETURN_IF_FALSE(monitor);

    switch (event_type)
    {
    case Gio::FILE_MONITOR_EVENT_CREATED:
        this->add_meta_theme_monitor(file->get_path(), monitor->get_priority());
        break;
    case Gio::FILE_MONITOR_EVENT_DELETED:
        this->del_theme_and_notify(file->get_path(), ThemeMonitorEventType::TMET_META_DEL);
        break;
    default:
        break;
    }
}

void ThemeMonitor::on_meta_theme_changed(const Glib::RefPtr<Gio::File> &file,
                                         const Glib::RefPtr<Gio::File> &other_file,
                                         Gio::FileMonitorEvent event_type)
{
    auto monitor = this->get_and_check_parent_monitor(file);
    auto basename = file->get_basename();

    RETURN_IF_FALSE(monitor);

    // meta theme
    if (basename == "index.theme")
    {
        this->events_.emit(monitor, ThemeMonitorEventType::TMET_META_CHG);
        return;
    }

    // gtk theme
    if (basename == this->get_gtk_dirname())
    {
        switch (event_type)
        {
        case Gio::FILE_MONITOR_EVENT_CREATED:
            this->add_gtk_theme_monitor(file->get_path(), monitor->get_priority());
            break;
        case Gio::FILE_MONITOR_EVENT_DELETED:
            this->del_theme_and_notify(file->get_path(), ThemeMonitorEventType::TMET_GTK_DEL);
            break;
        default:
            return;
        }
        return;
    }

    // metacity theme
    if (basename == "metacity-1")
    {
        switch (event_type)
        {
        case Gio::FILE_MONITOR_EVENT_CREATED:
            this->add_metacity_theme_monitor(file->get_path(), monitor->get_priority());
            break;
        case Gio::FILE_MONITOR_EVENT_DELETED:
            this->del_theme_and_notify(file->get_path(), ThemeMonitorEventType::TMET_METACITY_DEL);
            break;
        default:
            return;
        }
        return;
    }
}

void ThemeMonitor::on_gtk_theme_changed(const Glib::RefPtr<Gio::File> &file,
                                        const Glib::RefPtr<Gio::File> &other_file,
                                        Gio::FileMonitorEvent event_type)
{
    auto monitor = this->get_and_check_parent_monitor(file);
    auto basename = file->get_basename();
    auto gtk_major = gtk_get_major_version();
    auto regex = Glib::Regex::create("gtk-.*\\.css");

    RETURN_IF_FALSE(monitor);

    if ((basename == "gtkrc" && gtk_major == GTK2_MAJOR) ||
        (regex->match(basename) && gtk_major > GTK2_MAJOR))
    {
        this->events_.emit(monitor, ThemeMonitorEventType::TMET_GTK_CHG);
    }
}

void ThemeMonitor::on_metacity_theme_changed(const Glib::RefPtr<Gio::File> &file,
                                             const Glib::RefPtr<Gio::File> &other_file,
                                             Gio::FileMonitorEvent event_type)
{
    auto monitor = this->get_and_check_parent_monitor(file);
    auto basename = file->get_basename();
    auto regex = Glib::Regex::create("metacity-theme.*\\.xml");

    RETURN_IF_FALSE(monitor);

    if (regex->match(basename))
    {
        this->events_.emit(monitor, ThemeMonitorEventType::TMET_METACITY_CHG);
    }
}

void ThemeMonitor::add_icon_theme_parent_monitor(const std::string &path, int32_t priority)
{
    auto monitor = this->create_and_add_monitor(path,
                                                priority,
                                                ThemeMonitorType::THEME_MONITOR_TYPE_ICON_PARENT,
                                                sigc::mem_fun(this, &ThemeMonitor::on_icon_theme_parent_changed));
    RETURN_IF_FALSE(monitor);

    auto file_dir = Gio::File::create_for_path(path);
    try
    {
        auto file_iter = file_dir->enumerate_children(G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_STANDARD_NAME);
        for (auto file_info = file_iter->next_file(); file_info; file_info = file_iter->next_file())
        {
            if (file_info->get_file_type() == Gio::FILE_TYPE_DIRECTORY ||
                file_info->get_file_type() == Gio::FILE_TYPE_SYMBOLIC_LINK)
            {
                auto name = file_info->get_name();
                auto child_path = Glib::build_path(G_DIR_SEPARATOR_S, std::vector<std::string>{path, name});
                this->add_icon_theme_monitor(child_path, priority);
            }
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return;
    }
}

void ThemeMonitor::add_icon_theme_monitor(const std::string &path, int32_t priority)
{
    auto monitor = this->create_and_add_monitor(path,
                                                priority,
                                                ThemeMonitorType::THEME_MONITOR_TYPE_ICON,
                                                sigc::mem_fun(this, &ThemeMonitor::on_icon_theme_changed));
    RETURN_IF_FALSE(monitor);
    this->events_.emit(monitor, ThemeMonitorEventType::TMET_ICON_ADD);

    // cursor
    auto cursor_path = Glib::build_path(G_DIR_SEPARATOR_S, std::vector<std::string>{path, "cursors"});
    this->add_cursor_theme_monitor(cursor_path, priority);
}

void ThemeMonitor::add_cursor_theme_monitor(const std::string &path, int32_t priority)
{
    auto monitor = this->create_and_add_monitor(path,
                                                priority,
                                                ThemeMonitorType::THEME_MONITOR_TYPE_CURSOR,
                                                sigc::mem_fun(this, &ThemeMonitor::on_cursor_theme_changed));

    RETURN_IF_FALSE(monitor);
    this->events_.emit(monitor, ThemeMonitorEventType::TMET_CURSOR_ADD);
}

void ThemeMonitor::on_icon_theme_parent_changed(const Glib::RefPtr<Gio::File> &file,
                                                const Glib::RefPtr<Gio::File> &other_file,
                                                Gio::FileMonitorEvent event_type)
{
    auto monitor = this->get_and_check_parent_monitor(file);
    RETURN_IF_FALSE(monitor);

    switch (event_type)
    {
    case Gio::FILE_MONITOR_EVENT_CREATED:
        this->add_icon_theme_monitor(file->get_path(), monitor->get_priority());
        break;
    case Gio::FILE_MONITOR_EVENT_DELETED:
        this->del_theme_and_notify(file->get_path(), ThemeMonitorEventType::TMET_ICON_DEL);
        break;
    default:
        break;
    }
}

void ThemeMonitor::on_icon_theme_changed(const Glib::RefPtr<Gio::File> &file,
                                         const Glib::RefPtr<Gio::File> &other_file,
                                         Gio::FileMonitorEvent event_type)
{
    auto monitor = this->get_and_check_parent_monitor(file);
    auto basename = file->get_basename();

    RETURN_IF_FALSE(monitor);

    // icon theme
    if (basename == "index.theme")
    {
        this->events_.emit(monitor, ThemeMonitorEventType::TMET_ICON_CHG);
        return;
    }

    // cursor theme
    if (basename == "cursors")
    {
        switch (event_type)
        {
        case Gio::FILE_MONITOR_EVENT_CREATED:
            this->add_icon_theme_monitor(file->get_path(), monitor->get_priority());
            break;
        case Gio::FILE_MONITOR_EVENT_DELETED:
            this->del_theme_and_notify(file->get_path(), ThemeMonitorEventType::TMET_CURSOR_DEL);
            break;
        default:
            return;
        }
        return;
    }
}

void ThemeMonitor::on_cursor_theme_changed(const Glib::RefPtr<Gio::File> &file,
                                           const Glib::RefPtr<Gio::File> &other_file,
                                           Gio::FileMonitorEvent event_type)
{
    auto monitor = this->get_and_check_parent_monitor(file);
    RETURN_IF_FALSE(monitor);

    if (event_type == Gio::FILE_MONITOR_EVENT_CREATED ||
        event_type == Gio::FILE_MONITOR_EVENT_DELETED ||
        event_type == Gio::FILE_MONITOR_EVENT_CHANGED)
    {
        this->events_.emit(monitor, ThemeMonitorEventType::TMET_CURSOR_CHG);
    }
}
}  // namespace Kiran