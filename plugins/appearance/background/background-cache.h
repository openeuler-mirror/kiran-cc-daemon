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