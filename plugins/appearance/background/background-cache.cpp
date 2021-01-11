/**
 * @FilePath      /kiran-cc-daemon/plugins/appearance/background/background-cache.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/appearance/background/background-cache.h"

namespace Kiran
{
// 5分钟未使用进行清理
#define BACKGROUND_CACHE_CLEAR_TIMEOUT_SECONDS 300

BackgroundCache::BackgroundCache()
{
}

BackgroundCache::~BackgroundCache()
{
    if (this->timeout_handler_)
    {
        this->timeout_handler_.disconnect();
    }
}

void BackgroundCache::init()
{
    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    this->timeout_handler_ = timeout.connect_seconds(sigc::mem_fun(this, &BackgroundCache::on_cache_clear_timeout), 120);
}

Glib::RefPtr<Gdk::Pixbuf> BackgroundCache::get_pixbuf(const std::string &file_path,
                                                      int32_t width,
                                                      int32_t height)
{
    auto file_cache_info = this->get_file_cache_info(file_path);
    RETURN_VAL_IF_FALSE(file_cache_info, Glib::RefPtr<Gdk::Pixbuf>());

    auto file_size = std::make_pair(width, height);
    auto iter = file_cache_info->pixbufs.find(file_size);
    RETURN_VAL_IF_TRUE(iter == file_cache_info->pixbufs.end(), Glib::RefPtr<Gdk::Pixbuf>());
    return iter->second;
}

void BackgroundCache::set_pixbuf(const std::string &file_path,
                                 int32_t width,
                                 int32_t height,
                                 Glib::RefPtr<Gdk::Pixbuf> pixbuf)
{
    auto file_cache_info = this->get_file_cache_info(file_path);
    RETURN_IF_FALSE(file_cache_info);

    auto file_size = std::make_pair(width, height);
    file_cache_info->pixbufs.emplace(file_size, pixbuf);
}

std::shared_ptr<FileCacheInfo> BackgroundCache::get_file_cache_info(const std::string &file_path)
{
    // 这里需要转化为绝对路径
    auto file = Gio::File::create_for_path(file_path);
    auto file_absolute_path = file->get_path();

    if (file_absolute_path.length() == 0)
    {
        LOG_WARNING("The file path '%s' is invalid.", file_path.c_str());
        return nullptr;
    }

    auto file_cache_info = this->lookup_file_cache_info(file_absolute_path);

    if (!file_cache_info)
    {
        file_cache_info = std::make_shared<FileCacheInfo>();
        file_cache_info->monitor = FileUtils::make_monitor(file_absolute_path, sigc::mem_fun(this, &BackgroundCache::on_background_file_changed));
        this->files_cache_.emplace(file_absolute_path, file_cache_info);
    }
    file_cache_info->visit_time = time(NULL);

    return file_cache_info;
}

std::shared_ptr<FileCacheInfo> BackgroundCache::lookup_file_cache_info(const std::string &file_path)
{
    auto iter = this->files_cache_.find(file_path);
    RETURN_VAL_IF_TRUE(iter == this->files_cache_.end(), nullptr);

    iter->second->visit_time = time(NULL);

    return iter->second;
}

bool BackgroundCache::on_cache_clear_timeout()
{
    SETTINGS_PROFILE("");

    auto now = time(NULL);
    for (auto iter = this->files_cache_.begin(); iter != this->files_cache_.end();)
    {
        if (iter->second->visit_time + BACKGROUND_CACHE_CLEAR_TIMEOUT_SECONDS < now)
        {
            this->files_cache_.erase(iter++);
        }
        else
        {
            ++iter;
        }
    }
    return true;
}

void BackgroundCache::on_background_file_changed(const Glib::RefPtr<Gio::File> &file,
                                                 const Glib::RefPtr<Gio::File> &other_file,
                                                 Gio::FileMonitorEvent event_type)
{
    auto file_path = file->get_path();
    auto file_cache_info = this->lookup_file_cache_info(file_path);

    // 这里不应该出现查找不到的情况
    if (!file_cache_info)
    {
        LOG_WARNING("Not found file cache info for %s", file_path.c_str());
        return;
    }

    // 文件发生变化则清除缓存
    switch (event_type)
    {
    case Gio::FILE_MONITOR_EVENT_CREATED:
    case Gio::FILE_MONITOR_EVENT_DELETED:
    case Gio::FILE_MONITOR_EVENT_CHANGED:
        this->files_cache_.erase(file_path);
        break;
    default:
        break;
    }
}
}  // namespace Kiran