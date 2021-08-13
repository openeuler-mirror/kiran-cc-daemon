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

#include "lib/base/base.h"

namespace Kiran
{
struct FileCacheInfo
{
    using FileSizeType = std::pair<int32_t, int32_t>;

    Glib::RefPtr<Gio::FileMonitor> monitor;
    // 上一次访问时间，长时间未访问进行清理，避免占用过多内存
    time_t visit_time;
    // 不同大小的pixmap
    std::map<FileSizeType, Glib::RefPtr<Gdk::Pixbuf>> pixbufs;
};

class BackgroundCache
{
public:
    BackgroundCache();
    virtual ~BackgroundCache();

    void init();

    Glib::RefPtr<Gdk::Pixbuf> get_pixbuf(const std::string &file_path, int32_t width, int32_t height);
    void set_pixbuf(const std::string &file_path, int32_t width, int32_t height, Glib::RefPtr<Gdk::Pixbuf> pixbuf);

private:
    // 获取文件缓存信息，不存在则创建
    std::shared_ptr<FileCacheInfo> get_file_cache_info(const std::string &file_path);

    // 查找文件缓存信息，不存在不会创建
    std::shared_ptr<FileCacheInfo> lookup_file_cache_info(const std::string &file_path);

    // 定时清理缓存文件
    bool on_cache_clear_timeout();

    void on_background_file_changed(const Glib::RefPtr<Gio::File> &file,
                                    const Glib::RefPtr<Gio::File> &other_file,
                                    Gio::FileMonitorEvent event_type);

private:
    std::map<std::string, std::shared_ptr<FileCacheInfo>> files_cache_;

    sigc::connection timeout_handler_;
};
}  // namespace Kiran