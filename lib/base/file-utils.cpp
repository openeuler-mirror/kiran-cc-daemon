/*
 * @Author       : tangjie02
 * @Date         : 2020-09-29 09:42:50
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-26 14:32:18
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/lib/base/file-utils.cpp
 */
#include "lib/base/file-utils.h"

namespace Kiran
{
Glib::RefPtr<Gio::FileMonitor> FileUtils::make_monitor(const std::string &path,
                                                       const FileMonitorCallBack &callback,
                                                       Gio::FileMonitorFlags flags)
{
    auto file = Gio::File::create_for_path(path);
    try
    {
        auto monitor = file->monitor(flags);
        monitor->signal_changed().connect(callback);
        return monitor;
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("Unable to monitor %s: %s", path.c_str(), e.what().c_str());
    }

    return Glib::RefPtr<Gio::FileMonitor>();
}

Glib::RefPtr<Gio::FileMonitor> FileUtils::make_monitor_file(const std::string &path,
                                                            const FileMonitorCallBack &callback,
                                                            Gio::FileMonitorFlags flags)
{
    auto file = Gio::File::create_for_path(path);
    try
    {
        auto monitor = file->monitor_file(flags);
        monitor->signal_changed().connect(callback);
        return monitor;
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("Unable to file monitor %s: %s", path.c_str(), e.what().c_str());
    }

    return Glib::RefPtr<Gio::FileMonitor>();
}

Glib::RefPtr<Gio::FileMonitor> FileUtils::make_monitor_directory(const std::string &path,
                                                                 const FileMonitorCallBack &callback,
                                                                 Gio::FileMonitorFlags flags)
{
    auto file = Gio::File::create_for_path(path);
    try
    {
        auto monitor = file->monitor_directory(flags);
        monitor->signal_changed().connect(callback);
        return monitor;
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("Unable to directory monitor %s: %s", path.c_str(), e.what().c_str());
    }

    return Glib::RefPtr<Gio::FileMonitor>();
}
}  // namespace Kiran
